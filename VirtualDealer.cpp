#include "Config.h"
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"

PluginInfo ExtPluginInfo = {"Virtual Dealer", 1, "Moa International.", {0}};

//+------------------------------------------------------------------+
//| DLL entry point                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            char tmp[256], *cp;
            //--- create configuration filename
            GetModuleFileName((HMODULE)hModule, tmp, sizeof(tmp) - 5);
            if ((cp = strrchr(tmp, '.')) != NULL) {
                *cp = 0;
                strcat(tmp, ".ini");
            }
            Factory::GetConfig()->Load(tmp);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            Factory::GetProcessor()->ShowStatus();
            break;
    }
    return (TRUE);
}
//+------------------------------------------------------------------+
//| About, must be present always!                                   |
//+------------------------------------------------------------------+
void APIENTRY MtSrvAbout(PluginInfo* info) {
    if (info != NULL) {
        memcpy(info, &ExtPluginInfo, sizeof(PluginInfo));
    }
}
//+------------------------------------------------------------------+
//| Set server interface point                                       |
//+------------------------------------------------------------------+
int APIENTRY MtSrvStartup(CServerInterface* server) {
    if (server == NULL) {
        return (FALSE);
    }
    //--- check version
    if (server->Version() != ServerApiVersion) {
        return (FALSE);
    }
    //--- save server interface link
    Factory::SetServerInterface(server);

    //--- initialize dealer helper
    Factory::GetProcessor()->Initialize();

    return (TRUE);
}
//+------------------------------------------------------------------+
//| Cleanup                                |
//+------------------------------------------------------------------+
void APIENTRY MtSrvCleanup() {
    Factory::SetServerInterface(NULL);
    Factory::GetProcessor()->Shutdown();
}
//+------------------------------------------------------------------+
//| Standard configuration functions                                 |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgAdd(const PluginCfg* cfg) {
    LOG("MtSrvPluginCfgAdd name=%s, value=%s.", cfg->name, cfg->value);
    int res = Factory::GetConfig()->Add(cfg);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG("MtSrvPluginCfgSet total = %d.", total);
    int res = Factory::GetConfig()->Set(values, total);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgDelete(LPCSTR name) {
    LOG("MtSrvPluginCfgDelete %s.", name);
    int res = Factory::GetConfig()->Delete(name);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg* cfg) {
    LOG("MtSrvPluginCfgGet name=%s.", name);
    return Factory::GetConfig()->Get(name, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg* cfg) {
    LOG("MtSrvPluginCfgNext index=%d, name=%s, value=%s.", index, cfg->name, cfg->value);
    return Factory::GetConfig()->Next(index, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgTotal() {
    LOG("MtSrvPluginCfgTotal.");
    return Factory::GetConfig()->Total();
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeRequestFilter(RequestInfo* request, const int isdemo) {
    LOG("MtSrvTradeRequestFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id) {
    LOG("MtSrvTradeTransaction.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void APIENTRY MtSrvTradeRequestApply(RequestInfo* request, const int isdemo) {
    LOG("MtSrvTradeRequestApply.");
    if (request != NULL && isdemo == FALSE) {
        Factory::GetProcessor()->ProcessRequest(request);
    }
    LOG("MtSrvTradeRequestApply end.");
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    LOG("MtSrvTradeStopsFilter.");
    LOG_INFO(trade);
    return RET_OK;
}

int APIENTRY MtSrvTradeStopsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                                  const int isTP) {
    LOG("MtSrvTradeStopsApply.");
    LOG_INFO(user);
    LOG_INFO(group);
    LOG_INFO(symbol);
    LOG_INFO(trade);
    LOG("hit time = %d, isTP = %d.", time(NULL), isTP);

    trade->close_price = 0.123;
    Factory::GetServerInterface()->OrdersUpdate(trade, (UserInfo*)user, UPDATE_CLOSE);
    LOG("-------------^^^^^^^^^^^^^^^^^---------------------");
    return RET_OK_NONE;

    // Here, delay tp sl
    if (Factory::GetProcessor()->AllowSLTP(user, group, symbol, trade, isTP)) {
        // activate tp/sl
        return RET_OK;
    }
    // not activate tp/sl
    return RET_OK_NONE;
}

int APIENTRY MtSrvTradePendingsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    // LOG("MtSrvTradePendingsFilter. order = %d", trade->order);
    // LOG("----------------------------------symbol->spread =  %d", symbol->spread);
    return RET_OK;
}

int APIENTRY MtSrvTradePendingsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    LOG("MtSrvTradePendingsApply.");

    LOG_INFO(pending);
    LOG_INFO(trade);

    LOG("----------------------------------pending.open_time =  %d, pending.close_time = %d, pending->timestamp = %d; "
        "trade.open_time = %d, trade.close_time = %d, trade->timestamp = %d, current time = %d",
        pending->open_time, pending->close_time, pending->timestamp, trade->open_time, trade->close_time, trade->timestamp,
        time(NULL));
    trade->open_price = 0.11;
    Factory::GetServerInterface()->OrdersUpdate(trade, (UserInfo*)user, UPDATE_ACTIVATE);
    LOG("-------------^^^^^^^^^^^^^^^^^---------------------");
    return RET_OK_NONE;

    // Here, delay activation
    if (Factory::GetProcessor()->ActivatePendingOrder(user, group, symbol, pending, trade)) {
        // activate order
        return RET_OK;
    }
    // not activate order
    return RET_OK_NONE;
}

int APIENTRY MtSrvTradeStopoutsFilter(const ConGroup* group, const ConSymbol* symbol, const int login, const double equity,
                                      const double margin) {
    LOG("MtSrvTradeStopoutsFilter.");
    return RET_OK;
}

int APIENTRY MtSrvTradeStopoutsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     TradeRecord* stopout) {
    LOG("MtSrvTradeStopoutsApply. order = %d", stopout->order);
    LOG_INFO(user);
    LOG_INFO(group);
    LOG_INFO(symbol);
    LOG_INFO(stopout);
    // Here, set user's balance to zero
    return RET_OK;
}

void APIENTRY MtSrvTradesAddExt(TradeRecord* trade, const UserInfo* user, const ConSymbol* symb, const int mode) {
    LOG("MtSrvTradesAddExt.");
}

void APIENTRY MtSrvTradesUpdate(TradeRecord* trade, UserInfo* user, const int mode) {
    LOG("MtSrvTradesUpdate.");
    LOG_INFO(trade);
}

void APIENTRY MtSrvHistoryTickApply(const ConSymbol* symbol, FeedTick* inf) { Factory::GetProcessor()->TickApply(symbol, inf); }
