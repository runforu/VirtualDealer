#include "Config.h"
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"

PluginInfo ExtPluginInfo = {"Virtual Dealer", 1, "DH Copyright.", {0}};

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

void APIENTRY MtSrvAbout(PluginInfo* info) {
    if (info != NULL) {
        memcpy(info, &ExtPluginInfo, sizeof(PluginInfo));
    }
}

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

void APIENTRY MtSrvCleanup() {
    LOG("MtSrvCleanup");
    Factory::GetProcessor()->Shutdown();
    Factory::SetServerInterface(NULL);
}

int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG("MtSrvPluginCfgSet total = %d.", total);
    int res = Factory::GetConfig()->Set(values, total);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}

int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg* cfg) {
    LOG("MtSrvPluginCfgNext index=%d, name=%s, value=%s.", index, cfg->name, cfg->value);
    return Factory::GetConfig()->Next(index, cfg);
}

int APIENTRY MtSrvPluginCfgTotal() {
    LOG("MtSrvPluginCfgTotal.");
    return Factory::GetConfig()->Total();
}

int APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id) {
    LOG("MtSrvTradeTransaction.");
    Factory::GetProcessor()->OnTradeTransaction(trans, user);
    return RET_OK;
}

int APIENTRY MtSrvTradeRequestFilter(RequestInfo* request, const int isdemo) {
    LOG("MtSrvTradeRequestFilter.");
    return RET_OK;
}

void APIENTRY MtSrvTradeRequestApply(RequestInfo* request, const int isdemo) {
    LOG("MtSrvTradeRequestApply.");
    if (request != NULL && isdemo == FALSE) {
        Factory::GetProcessor()->ProcessRequest(request);
    }
}

int APIENTRY MtSrvTradeStopsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) { return RET_OK; }

int APIENTRY MtSrvTradeStopsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                                  const int isTP) {
    // Here, delay tp sl
    if (Factory::GetProcessor()->AllowSLTP(user, group, symbol, trade, isTP)) {
        // activate tp/sl
        LOG("MtSrvTradeStopsApply RET_OK returned: sl/tp closed");
        return RET_OK;
    }
    // not activate tp/sl
    LOG("MtSrvTradeStopsApply RET_OK_NONE returned: delay sl/tp closing");
    return RET_OK_NONE;
}

int APIENTRY MtSrvTradePendingsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    //if (Factory::GetProcessor()->IsPendingProcessing(group, symbol, trade)) {
    //    // order is in processing
    //    Factory::GetServerInterface()->LogsOut(trade->order, "Virtual Dealer",
    //        "MtSrvTradePendingsFilter: Order is already pending.");
    //    return RET_OK_NONE;
    //}
    return RET_OK;
}

int APIENTRY MtSrvTradePendingsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    // Here, delay activation
    if (Factory::GetProcessor()->ActivatePendingOrder(user, group, symbol, pending, trade)) {
        // activate order
        LOG("MtSrvTradePendingsApply RET_OK returned: activate order immediately");
        return RET_OK;
    }
    LOG("MtSrvTradePendingsApply RET_OK_NONE returned: delay activating order");
    // not activate order
    return RET_OK_NONE;
}

void APIENTRY MtSrvHistoryTickApply(const ConSymbol* symbol, FeedTick* inf) { Factory::GetProcessor()->TickApply(symbol, inf); }
