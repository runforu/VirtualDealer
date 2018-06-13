//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include "Processor.h"
#include "Config/Configuration.h"
//---
PluginInfo        ExtPluginInfo = { "Delayed Dealer", 1, "Moa International.", {0} };
CServerInterface *ExtServer = NULL;
//+------------------------------------------------------------------+
//| DLL entry point                                                  |
//+------------------------------------------------------------------+
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/) {
    char tmp[256], *cp;
    //---
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        //--- create configuration filename
        GetModuleFileName((HMODULE)hModule, tmp, sizeof(tmp) - 5);
        if ((cp = strrchr(tmp, '.')) != NULL) {
            *cp = 0; strcat(tmp, ".ini");
        }
        //--- load configuration
        ExtConfig.Load(tmp);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        ExtProcessor.ShowStatus();
        break;
    }
    //---
    return(TRUE);
}
//+------------------------------------------------------------------+
//| About, must be present always!                                   |
//+------------------------------------------------------------------+
void APIENTRY MtSrvAbout(PluginInfo *info) {
    if (info != NULL) {
        memcpy(info, &ExtPluginInfo, sizeof(PluginInfo));
    }
}
//+------------------------------------------------------------------+
//| Set server interface point                                       |
//+------------------------------------------------------------------+
int APIENTRY MtSrvStartup(CServerInterface *server) {
    //--- check version
    if (server == NULL) {
        return(FALSE);
    }
    server->LogsOut(31415, "DelayedDealer", "DelayedDealer invoked by server.");
    if (server->Version() != ServerApiVersion) {
        return(FALSE);
    }
    //--- save server interface link
    ExtServer = server;
    //--- initialize dealer helper
    ExtProcessor.Initialize();
    server->LogsOut(31415, "DelayedDealer", "DelayedDealer initialized by server.");
    return(TRUE);
}
//+------------------------------------------------------------------+
//| Standard configuration functions                                 |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgAdd(const PluginCfg *cfg) {
    int res = ExtConfig.Add(cfg);
    ExtProcessor.Reinitialize();
    return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgSet(const PluginCfg *values, const int total) {
    int res = ExtConfig.Set(values, total);
    ExtProcessor.Reinitialize();
    return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgDelete(LPCSTR name) {
    int res = ExtConfig.Delete(name);
    ExtProcessor.Reinitialize();
    return(res);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg *cfg) {
    return ExtConfig.Get(name, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg *cfg) {
    return ExtConfig.Next(index, cfg);
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
int APIENTRY MtSrvPluginCfgTotal() { return ExtConfig.Total(); }
//+------------------------------------------------------------------+
//| Incoming requests                                                |
//+------------------------------------------------------------------+ 
void APIENTRY MtSrvTradeRequestApply(RequestInfo *request, const int isdemo) {
    if (request != NULL && isdemo == FALSE) {
        ExtProcessor.ProcessRequest(request);
    }
}
//+------------------------------------------------------------------+
