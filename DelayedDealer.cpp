#include "Config/Config.h"
#include "Processor.h"
#include "common/Loger.h"
#include "Factory.h"

PluginInfo ExtPluginInfo = { "Delayed Dealer", 1, "Moa International.", { 0 } };
CServerInterface* ExtServer = NULL;

//+------------------------------------------------------------------+
//| DLL entry point                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/) {
    char tmp[256], *cp;

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
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
        Factory::GetFileConfig()->Load(tmp);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        ExtProcessor.ShowStatus();
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
    ExtServer = server;
    //--- initialize dealer helper
    ExtProcessor.Initialize();
    LOG(31415, "DelayedDealer", "DelayedDealer initialized by server.");
    return (TRUE);
}
//+------------------------------------------------------------------+
//| Cleanup                                |
//+------------------------------------------------------------------+
void APIENTRY MtSrvCleanup() {
    //noop
}
//+------------------------------------------------------------------+
//| Standard configuration functions                                 |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgAdd(const PluginCfg* cfg) {
    LOG(31415, "DelayedDealer", "MtSrvPluginCfgAdd name=%s, value=%s.", cfg->name, cfg->value);
    int res = ExtConfig.Add(cfg);
    ExtProcessor.Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG(31415, "DelayedDealer", "MtSrvPluginCfgSet total = %d.", total);
    int res = ExtConfig.Set(values, total);
    ExtProcessor.Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgDelete(LPCSTR name) {
    LOG(31415, "DelayedDealer", "MtSrvPluginCfgDelete %s.", name);
    int res = ExtConfig.Delete(name);
    ExtProcessor.Reinitialize();
    return (res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg* cfg) {
    LOG(31415, "DelayedDealer", "MtSrvPluginCfgGet name=%s.", name);
    return ExtConfig.Get(name, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg* cfg) {
    LOG(31415, "DelayedDealer", "MtSrvPluginCfgNext index=%d, name=%s, value=%s.", index, cfg->name, cfg->value);
    return ExtConfig.Next(index, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgTotal() {
    LOG(31415, "DelayedDealer", "MtSrvPluginCfgTotal.");
    return ExtConfig.Total();
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeRequestFilter(RequestInfo* request, const int isdemo) {
    LOG(31415, "DelayedDealer", "MtSrvTradeRequestFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id) {
    LOG(31415, "DelayedDealer", "MtSrvTradeTransaction.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void APIENTRY MtSrvTradeRequestApply(RequestInfo* request, const int isdemo) {
    LOG(31415, "DelayedDealer", "MtSrvTradeRequestApply.");
    if (request != NULL && isdemo == FALSE) {
        ExtProcessor.ProcessRequest(request);
    }
    LOG(31415, "DelayedDealer", "MtSrvTradeRequestApply end.");
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    LOG(31415, "DelayedDealer", "MtSrvTradeStopsFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade, const int isTP) {
    LOG(31415, "DelayedDealer", "MtSrvTradeStopsApply.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradePendingsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    LOG(31415, "DelayedDealer", "MtSrvTradePendingsFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradePendingsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, const TradeRecord* pending, TradeRecord* trade) {
    LOG(31415, "DelayedDealer", "MtSrvTradePendingsApply.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopoutsFilter(const ConGroup* group, const ConSymbol* symbol, const int login, const double equity, const double margin) {
    LOG(31415, "DelayedDealer", "MtSrvTradeStopoutsFilter.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvTradeStopoutsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* stopout) {
    LOG(31415, "DelayedDealer", "MtSrvTradeStopoutsApply.");
    return RET_OK;
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void APIENTRY MtSrvTradesAddExt(TradeRecord* trade, const UserInfo* user, const ConSymbol* symb, const int mode) {
    LOG(31415, "DelayedDealer", "MtSrvTradesAddExt.");
}
//+------------------------------------------------------------------+
//|                                                                  |
/**/
void APIENTRY MtSrvTradesUpdate(TradeRecord* trade, UserInfo* user, const int mode) {
    LOG(31415, "DelayedDealer", "MtSrvTradesUpdate.");
}
