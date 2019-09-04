#include "Config.h"
#include "Loger.h"
#include "Processor.h"
#include "ServerApi.h"

extern const char *PLUGIN_VERSION_STRING;
#define PLUGIN_NAME "Virtual Dealer"
#define PLUGIN_VERSION 1
#define PLUGIN_COPYRIGHT "DH Copyrigh."

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
            Config::Instance().Load(tmp);
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
    return (TRUE);
}

void APIENTRY MtSrvAbout(PluginInfo* info) {
    if (info != NULL) {
        sprintf(info->name, "%s %s", PLUGIN_NAME, PLUGIN_VERSION_STRING);
        info->version = PLUGIN_VERSION;
        sprintf(info->copyright, "%s", PLUGIN_COPYRIGHT);
        memset(info->reserved, 0, sizeof(info->reserved));
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
    ServerApi::Initialize(server);

    //--- initialize dealer helper
    Processor::Instance().Initialize();

    return (TRUE);
}

void APIENTRY MtSrvCleanup() {
    LOG("MtSrvCleanup");
    Processor::Instance().Shutdown();
    Processor::Instance().ShowStatus();
}

int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG("MtSrvPluginCfgSet total = %d.", total);
    int res = Config::Instance().Set(values, total);
    Processor::Instance().Reinitialize();
    return (res);
}

int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg* cfg) {
    LOG("MtSrvPluginCfgNext index=%d, name=%s, value=%s.", index, cfg->name, cfg->value);
    return Config::Instance().Next(index, cfg);
}

int APIENTRY MtSrvPluginCfgTotal() {
    LOG("MtSrvPluginCfgTotal.");
    return Config::Instance().Total();
}

int APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id) {
    Processor::Instance().OnTradeTransaction(trans, user);
    return RET_OK;
}

int APIENTRY MtSrvTradeRequestFilter(RequestInfo* request, const int isdemo) {
    return RET_OK;
}

void APIENTRY MtSrvTradeRequestApply(RequestInfo* request, const int isdemo) {
    if (request != NULL && isdemo == FALSE) {
        Processor::Instance().ProcessRequest(request);
    }
}

int APIENTRY MtSrvTradeStopoutsFilter(const ConGroup* group, const ConSymbol* symbol, const int login, const double equity,
                                      const double margin) {
    return RET_OK;
}

int APIENTRY MtSrvTradeStopoutsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     TradeRecord* stopout) {
    return RET_OK;
}

int APIENTRY MtSrvTradeStopsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    return RET_OK;
}

int APIENTRY MtSrvTradeStopsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                                  const int isTP) {
    // Here, delay tp sl
    if (Processor::Instance().AllowSLTP(user, group, symbol, trade, isTP)) {
        // activate tp/sl
        LOG("MtSrvTradeStopsApply RET_OK returned: sl/tp closed");
        return RET_OK;
    }
    // not activate tp/sl
    LOG("MtSrvTradeStopsApply RET_OK_NONE returned: delay sl/tp closing");
    return RET_OK_NONE;
}

int APIENTRY MtSrvTradePendingsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    return RET_OK;
}

int APIENTRY MtSrvTradePendingsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    return RET_OK;
    // Here, delay activation
    if (Processor::Instance().ActivatePendingOrder(user, group, symbol, pending, trade)) {
        // activate order
        LOG("MtSrvTradePendingsApply RET_OK returned: activate order immediately");
        return RET_OK;
    }
    LOG("MtSrvTradePendingsApply RET_OK_NONE returned: delay activating order");
    // not activate order
    return RET_OK_NONE;
}

void APIENTRY MtSrvHistoryTickApply(const ConSymbol* symbol, FeedTick* inf) {
    Processor::Instance().TickApply(symbol, inf);
}
