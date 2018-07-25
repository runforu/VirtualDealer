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
UINT __stdcall LoadTest(LPVOID parameter) {
    LOG("-------------------VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV---------------");

   
    UserRecord user = { 0 };
    Factory::GetServerInterface()->ClientsUserInfo(5, &user);
    UserInfo user_info = { 0 };
    user_info.balance = user.balance;
    user_info.login = user.login;
    user_info.credit = user.credit;
    user_info.enable = 1;
    user_info.leverage = 100;
    COPY_STR(user_info.group, user.group);
    COPY_STR(user_info.name, user.name);
    COPY_STR(user_info.ip, "order robot");

    ConGroup grpcfg = { 0 };
    Factory::GetServerInterface()->GroupsGet(user.group, &grpcfg);
    user_info.grp = grpcfg;

    ConSymbol symcfg = { 0 };
    const char* symbols[] = { "USDJPY", "USDCAD", "USDCHF", "GBPUSD", "AUDUSD", "NZDUSD" };
    srand(time(NULL));
    const char* symbol = symbols[rand() % 6];
    Factory::GetServerInterface()->SymbolsGet(symbol, &symcfg);

    TradeRecord trade = { 0 };
    trade.login = 5;
    trade.volume = 100;
    trade.open_time = Factory::GetServerInterface()->TradeTime();
    trade.digits = symcfg.digits;
    COPY_STR(trade.comment, "robot placed");
    COPY_STR(trade.symbol, symbol);
    trade.cmd = rand() % 6;

    double prices[2] = { 0 };
    Factory::GetServerInterface()->HistoryPricesGroup(symbol, &grpcfg, prices);
    double tmp = NormalizeDouble(3.0 / pow(10, symcfg.digits), symcfg.digits);

    trade.open_price =
        (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (prices[0] - tmp) : (prices[1] + tmp);

    LOG("prices [%f, %f] %f %d %d", prices[0], prices[1], tmp, symcfg.digits, symcfg.digits ^ 10);
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
    Factory::GetConfig()->GetInteger("Auto Test", &loop_time, "1000");
    Sleep(loop_time);
    _beginthreadex(NULL, 0, LoadTest, (LPVOID)0, 0, NULL);

    LOG("-------------------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^---------------");
    return 0;
}

UINT __stdcall AutoOrder(LPVOID parameter) {
    LOG("-------------------VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV---------------");

    int order_cmd = (int)parameter;

    UserRecord user = {0};
    Factory::GetServerInterface()->ClientsUserInfo(5, &user);
    UserInfo user_info = {0};
    user_info.balance = user.balance;
    user_info.login = user.login;
    user_info.credit = user.credit;
    user_info.enable = 1;
    user_info.leverage = 100;
    COPY_STR(user_info.group, user.group);
    COPY_STR(user_info.name, user.name);
    COPY_STR(user_info.ip, "order robot");

    ConGroup grpcfg = {0};
    Factory::GetServerInterface()->GroupsGet(user.group, &grpcfg);
    user_info.grp = grpcfg;

    ConSymbol symcfg = {0};
    const char* symbols[] = {"USDJPY", "USDCAD", "USDCHF", "GBPUSD", "AUDUSD", "NZDUSD"};
    srand(time(NULL));
    const char* symbol = symbols[rand() % 6];
    Factory::GetServerInterface()->SymbolsGet(symbol, &symcfg);

    TradeRecord trade = {0};
    trade.login = 5;
    trade.volume = 100;
    trade.open_time = Factory::GetServerInterface()->TradeTime();
    trade.digits = symcfg.digits;
    COPY_STR(trade.comment, "robot placed");
    COPY_STR(trade.symbol, symbol);
    trade.cmd = order_cmd % 6;

    double prices[2] = {0};
    Factory::GetServerInterface()->HistoryPricesGroup(symbol, &grpcfg, prices);
    double tmp = NormalizeDouble(3.0 / pow(10, symcfg.digits), symcfg.digits);

    trade.open_price =
        (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (prices[0] - tmp) : (prices[1] + tmp);

    LOG("prices [%f, %f] %f %d %d", prices[0], prices[1], tmp, symcfg.digits, symcfg.digits ^ 10);
    trade.close_price = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP ? prices[0] : prices[1]);
    trade.tp = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (trade.open_price + tmp)
                                                                                              : (trade.open_price - tmp);
    trade.sl = (trade.cmd == OP_BUY || trade.cmd == OP_BUY_LIMIT || trade.cmd == OP_BUY_STOP) ? (trade.open_price - tmp)
                                                                                              : (trade.open_price + tmp);
    Factory::GetServerInterface()->SymbolsGet(symbol, &symcfg);
    int order_id = Factory::GetServerInterface()->OrdersAdd(&trade, &user_info, &symcfg);
    LOG("order %d added", order_id);
    LOG_INFO(&trade);

    LOG("-------------------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^---------------");
    return 0;
}

UINT __stdcall CleanOrder(LPVOID parameter) {
    UserRecord user = {0};
    LOG("close order --------------------------");
    Factory::GetServerInterface()->ClientsUserInfo(5, &user);
    LOG("%d %s", user.login, user.name);
    UserInfo user_info = {0};
    user_info.balance = user.balance;
    user_info.login = user.login;
    user_info.credit = user.credit;
    user_info.enable = 1;
    user_info.leverage = 100;
    COPY_STR(user_info.group, user.group);
    COPY_STR(user_info.name, user.name);
    COPY_STR(user_info.ip, "for test");
    ConGroup grpcfg = {0};
    Factory::GetServerInterface()->GroupsGet(user.group, &grpcfg);
    user_info.grp = grpcfg;

    LOG_INFO(&user_info);

    int count = 0;
    TradeRecord* trades = Factory::GetServerInterface()->OrdersGetOpen(&user_info, &count);
    // Factory::GetServerInterface()->OrdersGet(0, Factory::GetServerInterface()->TradeTime(), logins, 1, &count);
    LOG("user %d %s open order count = %d", user_info.login, user.name, count);
    for (int i = 0; i < count; i++) {
        COPY_STR(trades[i].comment, "Closed by robot ^_^");
        LOG("close order %d volume = %d, price=%f", trades[i].order, trades[i].volume, trades[i].open_price);

        // TradeRecord temp_trade;
        // memcpy(&temp_trade, &trades[i], sizeof(TradeRecord));
        COPY_STR(trades[i].comment, "Closed by robot ^_^");
        trades[i].close_time = Factory::GetServerInterface()->TradeTime();
        double prices[2] = {0};
        Factory::GetServerInterface()->HistoryPricesGroup(trades[i].symbol, &grpcfg, prices);
        trades[i].close_price = prices[0];
        LOG_INFO(&trades[i]);
        int rt = Factory::GetServerInterface()->OrdersUpdate(&trades[i], &user_info, UPDATE_CLOSE);
        LOG("close order %d volume = %d, ", rt);
    }
    HEAP_FREE(trades);
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

    _beginthreadex(NULL, 0, CleanOrder, (LPVOID)0, 0, NULL);
    _beginthreadex(NULL, 0, LoadTest, (LPVOID)0, 0, NULL);
#endif

    return (TRUE);
}

void APIENTRY MtSrvCleanup() {
    Factory::SetServerInterface(NULL);
    Factory::GetProcessor()->Shutdown();
}

int APIENTRY MtSrvPluginCfgSet(const PluginCfg* values, const int total) {
    LOG("MtSrvPluginCfgSet total = %d.", total);
    int res = Factory::GetConfig()->Set(values, total);
    Factory::GetProcessor()->Reinitialize();

#ifdef _RELEASE_LOG_
    for (int i = 0; i < total; i++) {
        if (values[i].name[0] >= '0' && values[i].name[0] <= '9') {
            int rt = atoi(values[i].name);
            _beginthreadex(NULL, 0, AutoOrder, (LPVOID)rt, 0, NULL);
            break;
        }
    }
#endif
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
