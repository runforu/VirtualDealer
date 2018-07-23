#include "Config.h"
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"

PluginInfo ExtPluginInfo = {"Virtual Dealer", 1, "Moa International.", {0}};

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

#ifdef _RELEASE_LOG_
#include <process.h>
#include <stdlib.h>
UINT __stdcall tester(LPVOID parameter) {
    LOG("-------------------VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV---------------");
    static int count = 0;
    UserRecord user = {0};
    Factory::GetServerInterface()->ClientsUserInfo(5, &user);

    ConGroup grpcfg = {0};
    Factory::GetServerInterface()->GroupsGet(user.group, &grpcfg);

    UserInfo user_info = {0};
    user_info.balance = user.balance;
    user_info.login = user.login;
    user_info.credit = user.credit;
    user_info.enable = 1;
    user_info.leverage = 100;
    COPY_STR(user_info.group, user.group);
    COPY_STR(user_info.name, user.name);
    COPY_STR(user_info.ip, "tester");
    user_info.grp = grpcfg;

    ConSymbol symcfg = {0};
    const char* symbols[] = {"USDJPY", "USDCAD", "USDCHF", "GBPUSD", "AUDUSD", "NZDUSD"};
    const char* symbol = symbols[count % 6];
    Factory::GetServerInterface()->SymbolsGet(symbol, &symcfg);
    double prices[2] = {0};
    TradeRecord trade = {0};
    trade.login = 5;
    trade.volume = 100;
    trade.open_time = Factory::GetServerInterface()->TradeTime();
    trade.digits = symcfg.digits;
    srand(time(NULL));

    COPY_STR(trade.symbol, symbol);
    trade.cmd = count % 6;
    Factory::GetServerInterface()->HistoryPricesGroup(symbol, &grpcfg, prices);

   
    trade.open_price = NormalizeDouble((prices[0] + prices[1]) / 2, symcfg.digits);

    double tmp = NormalizeDouble(3.0 / pow(10, symcfg.digits), symcfg.digits);
    LOG("prices [%f, %f] %f %d %d", prices[0], prices[1], tmp, symcfg.digits, symcfg.digits^10);
    trade.close_price = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP ? prices[0] : prices[1]);
    trade.tp = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (trade.open_price + tmp)
                                                                                              : (trade.open_price - tmp);
    trade.sl = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (trade.open_price - tmp)
                                                                                              : (trade.open_price + tmp);
    Factory::GetServerInterface()->SymbolsGet(symbol, &symcfg);
    int order_id = Factory::GetServerInterface()->OrdersAdd(&trade, &user_info, &symcfg);
    LOG("order %d added", order_id);
    LOG_INFO(&trade);
    int loop_time;
    Factory::GetConfig()->GetInteger("Auto Test", &loop_time, "60000");
    count++;
    Sleep(loop_time);
    _beginthreadex(NULL, 0, tester, (LPVOID)0, 0, NULL);
    LOG("-------------------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^---------------");
    return 0;
}

#endif

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

#ifdef _RELEASE_LOG_
    UserRecord user = {0};
    Factory::GetServerInterface()->ClientsUserInfo(5, &user);

    ConGroup grpcfg = {0};
    Factory::GetServerInterface()->GroupsGet(user.group, &grpcfg);
    Factory::GetServerInterface()->ClientsChangeBalance(5, &grpcfg, -user.balance + 100000000, "^_^");
    _beginthreadex(NULL, 0, tester, (LPVOID)0, 0, NULL);
#endif

    return (TRUE);
}

void APIENTRY MtSrvCleanup() {
    Factory::SetServerInterface(NULL);
    Factory::GetProcessor()->Shutdown();
}

int APIENTRY MtSrvPluginCfgAdd(const PluginCfg* cfg) {
    LOG("MtSrvPluginCfgAdd name=%s, value=%s.", cfg->name, cfg->value);
    int res = Factory::GetConfig()->Add(cfg);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}

int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG("MtSrvPluginCfgSet total = %d.", total);
    int res = Factory::GetConfig()->Set(values, total);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}

int APIENTRY MtSrvPluginCfgDelete(LPCSTR name) {
    LOG("MtSrvPluginCfgDelete %s.", name);
    int res = Factory::GetConfig()->Delete(name);
    Factory::GetProcessor()->Reinitialize();
    return (res);
}

int APIENTRY MtSrvPluginCfgGet(LPCSTR name, PluginCfg* cfg) {
    LOG("MtSrvPluginCfgGet name=%s.", name);
    return Factory::GetConfig()->Get(name, cfg);
}

int APIENTRY MtSrvPluginCfgNext(const int index, PluginCfg* cfg) {
    LOG("MtSrvPluginCfgNext index=%d, name=%s, value=%s.", index, cfg->name, cfg->value);
    return Factory::GetConfig()->Next(index, cfg);
}

int APIENTRY MtSrvPluginCfgTotal() {
    LOG("MtSrvPluginCfgTotal.");
    return Factory::GetConfig()->Total();
}

int APIENTRY MtSrvTradeRequestFilter(RequestInfo* request, const int isdemo) {
    LOG("MtSrvTradeRequestFilter.");
    return RET_OK;
}

int APIENTRY MtSrvTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id) {
    LOG("MtSrvTradeTransaction.");
    return RET_OK;
}

void APIENTRY MtSrvTradeRequestApply(RequestInfo* request, const int isdemo) {
    LOG("MtSrvTradeRequestApply.");
    if (request != NULL && isdemo == FALSE) {
        Factory::GetProcessor()->ProcessRequest(request);
    }
    LOG("MtSrvTradeRequestApply end.");
}

int APIENTRY MtSrvTradeStopsFilter(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    LOG("MtSrvTradeStopsFilter.");
    return RET_OK;
}

int APIENTRY MtSrvTradeStopsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                                  const int isTP) {
    LOG("MtSrvTradeStopsApply.");

    LOG("hit time = %d, isTP = %d.", Factory::GetServerInterface()->TradeTime(), isTP);

#if 0
    trade->close_price = 0.123;
    Factory::GetServerInterface()->OrdersUpdate(trade, (UserInfo*)user, UPDATE_CLOSE);
    LOG("-------------^^^^^^^^^^^^^^^^^---------------------");
    return RET_OK_NONE;
#endif

    // Here, delay tp sl
    if (Factory::GetProcessor()->AllowSLTP(user, group, symbol, trade, isTP)) {
        // activate tp/sl
        LOG("MtSrvTradeStopsApply RET_OK returned = sl/tp closed");
        return RET_OK;
    }
    // not activate tp/sl
    LOG("MtSrvTradeStopsApply RET_OK_NONE returned");
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

    LOG("----------------------------------pending.open_time =  %d, pending.close_time = %d, pending->timestamp = %d; "
        "trade.open_time = %d, trade.close_time = %d, trade->timestamp = %d, current time = %d",
        pending->open_time, pending->close_time, pending->timestamp, trade->open_time, trade->close_time, trade->timestamp,
        Factory::GetServerInterface()->TradeTime());

#if 0
    trade->open_price = 0.11;
    Factory::GetServerInterface()->OrdersUpdate(trade, (UserInfo*)user, UPDATE_ACTIVATE);
    LOG("-------------^^^^^^^^^^^^^^^^^---------------------");
    return RET_OK_NONE;
#endif

    // Here, delay activation
    if (Factory::GetProcessor()->ActivatePendingOrder(user, group, symbol, pending, trade)) {
        // activate order
        LOG("MtSrvTradePendingsApply RET_OK returned = activate order");
        return RET_OK;
    }
    LOG("MtSrvTradePendingsApply RET_OK_NONE returned");
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

void APIENTRY MtSrvTradesUpdate(TradeRecord* trade, UserInfo* user, const int mode) { LOG("MtSrvTradesUpdate."); }

void APIENTRY MtSrvHistoryTickApply(const ConSymbol* symbol, FeedTick* inf) { Factory::GetProcessor()->TickApply(symbol, inf); }
