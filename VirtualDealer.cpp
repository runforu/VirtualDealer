#include "Config/Config.h"
#include "Factory.h"
#include "Processor.h"
#include "common/Loger.h"

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

            //--- load file configuration
            Factory::GetConfig()->Load(tmp);
            if ((cp = strrchr(tmp, '.')) != NULL) {
                *cp = 0;
                strcat(tmp, ".cfg");
            }
            Factory::GetFileConfig()->SetCfgFile(tmp);
            Factory::GetFileConfig()->Load();
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
    LOG(31415, "VirtualDealer", "VirtualDealer initialized by server.");
    return (TRUE);
}
//+------------------------------------------------------------------+
//| Cleanup                                |
//+------------------------------------------------------------------+
void APIENTRY MtSrvCleanup() {
    // noop
}
//+------------------------------------------------------------------+
//| Standard configuration functions                                 |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgAdd(const PluginCfg* cfg) {
    LOG(31415, "VirtualDealer", "MtSrvPluginCfgAdd name=%s, value=%s.", cfg->name, cfg->value);
    int res = Factory::GetConfig()->Add(cfg);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG(31415, "VirtualDealer", "MtSrvPluginCfgSet total = %d.", total);
    int res = Factory::GetConfig()->Set(values, total);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgDelete(LPCSTR name) {
    LOG(31415, "VirtualDealer", "MtSrvPluginCfgDelete %s.", name);
    int res = Factory::GetConfig()->Delete(name);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg* cfg) {
    LOG(31415, "VirtualDealer", "MtSrvPluginCfgGet name=%s.", name);
    return Factory::GetConfig()->Get(name, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg* cfg) {
    LOG(31415, "VirtualDealer", "MtSrvPluginCfgNext index=%d, name=%s, value=%s.", index, cfg->name, cfg->value);
    return Factory::GetConfig()->Next(index, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgTotal() {
    LOG(31415, "VirtualDealer", "MtSrvPluginCfgTotal.");
    return Factory::GetConfig()->Total();
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeRequestFilter(RequestInfo* request, const int isdemo) {
    LOG(31415, "VirtualDealer", "MtSrvTradeRequestFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id) {
    LOG(31415, "VirtualDealer", "MtSrvTradeTransaction.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void APIENTRY MtSrvTradeRequestApply(RequestInfo* request, const int isdemo) {
    LOG(31415, "VirtualDealer", "MtSrvTradeRequestApply.");
    if (request != NULL && isdemo == FALSE) {
        Factory::GetProcessor()->ProcessRequest(request);
    }
    LOG(31415, "VirtualDealer", "MtSrvTradeRequestApply end.");
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    LOG(31415, "VirtualDealer", "MtSrvTradeStopsFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                                  const int isTP) {
    LOG(31415, "VirtualDealer", "MtSrvTradeStopsApply.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradePendingsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    LOG(31415, "VirtualDealer", "MtSrvTradePendingsFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradePendingsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    LOG(31415, "VirtualDealer", "MtSrvTradePendingsApply.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopoutsFilter(const ConGroup* group, const ConSymbol* symbol, const int login, const double equity,
                                      const double margin) {
    LOG(31415, "VirtualDealer", "MtSrvTradeStopoutsFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopoutsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     TradeRecord* stopout) {
    LOG(31415, "VirtualDealer", "MtSrvTradeStopoutsApply.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void APIENTRY MtSrvTradesAddExt(TradeRecord* trade, const UserInfo* user, const ConSymbol* symb, const int mode) {
    LOG(31415, "VirtualDealer", "MtSrvTradesAddExt.");
}
//+------------------------------------------------------------------+
//|                                                                  |
/**/
void APIENTRY MtSrvTradesUpdate(TradeRecord* trade, UserInfo* user, const int mode) {
    LOG(31415, "VirtualDealer", "MtSrvTradesUpdate.");
}
