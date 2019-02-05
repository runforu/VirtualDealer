#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include "Config.h"
#include "ProcessingHandle.h"
#include "ProcessingOrder.h"
#include "RuleContainer.h"
#include "TickHistory.h"

struct RequestHelper {
    RequestInfo* m_request_info;
    PriceOption m_price_option;
    HANDLE m_handle;
    time_t m_start_time;
    int m_delay_milisecond;
    int m_diff;
};

struct PendingDelayHelper {
    UserInfo* m_user_info;
    // TradeRecord* m_trade_record;
    const TradeRecord* m_pending_trade_record;
    const ConGroup* m_group;
    const ConSymbol* m_symbol;
    double open_price;
    PriceOption m_price_option;
    time_t m_start_time;
    int m_delay_milisecond;
    int m_diff;
};

struct SlTpDelayHelper {
    UserInfo* m_user_info;
    int m_order_id;
    int m_is_tp;

    PriceOption m_price_option;
    time_t m_start_time;
    int m_delay_milisecond;
    int m_diff;
};

class Processor {
public:
    static Processor& Instance();

    inline void Reinitialize() {
        InterlockedExchange(&m_reinitialize_flag, 1);
    }

    void ShowStatus(void);

    void ProcessRequest(RequestInfo* request);

    bool ActivatePendingOrder(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, const TradeRecord* pending,
                              TradeRecord* trade);

    bool AllowSLTP(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade, const int isTP);

    void TickApply(const ConSymbol* symbol, FeedTick* tick);

    // for test purpose
    void OnTradeTransaction(TradeTransInfo* trans, const UserInfo* user);

    bool IsPendingProcessing(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade);

    void Initialize(void);

    void Shutdown(void);

private:
    Processor();
    ~Processor();
    Processor(Processor const&) {}
    void operator=(Processor const&) {}

    //--- Hanlde triger order price like pending, sl and tp, trade cmd in
    //{OP_BUY,OP_SELL,OP_BUY_LIMIT,OP_SELL_LIMIT,OP_BUY_STOP,OP_SELL_STOP} price is the trigered price.
    double GetPrice(const char* symbol, const UserInfo* user_info, int cmd, PriceOption price_option, time_t from, int diff,
                    double trigered_price);

    void GetPrice(RequestHelper* helper, double* prices);

    static int GetSpreadDiff(RequestInfo* request);

    static int GetSpreadDiff(const char* group);

    static bool SpreadDiff(const char* group, const char* symbol, TickAPI* tick, int diff);

    UINT Delay(LPVOID parameter);
    static void __cdecl DelayWrapper(LPVOID parameter);

    UINT DelaySlTpTriger(LPVOID parameter);
    static void __cdecl DelaySlTpTrigerWrapper(LPVOID parameter);

    UINT DelayPendingTriger(LPVOID parameter);
    static void __cdecl DelayPendingTrigerWrapper(LPVOID parameter);

    bool GetDelayOption(const char* symbol, const char* group, int client_login, int volume, int order_type,
                        PriceOption& price_option, int& delay_milisecond);

private:
    //--- dealer user info
    UserInfo m_manager;

    //--- configurations
    int m_virtual_dealer_login;
    int m_disable_virtual_dealer;

    //--- default rule to filter transanction
    char m_global_rule_symbol[256];
    char m_global_rule_group[256];
    int m_global_rule_login;
    int m_global_rule_min_volume;
    int m_global_rule_max_volume;
    int m_global_rule_order_type;
    int m_global_rule_delay_milisecond;
    PriceOption m_global_rule_price_option;

    //--- specific rules to filter transanction
    //--- no need to be locked because all the access in the hook thread.
    RuleContainer m_rule_container;

    //--- record the tick; any access to m_tick_history should be locked
    TickHistory m_tick_history;

    //--- statistics
    LONG m_reinitialize_flag;
    int m_requests_total;
    unsigned int m_requests_processed;

    //Synchronizer m_synchronizer;

    ProcessingOrder m_processing_pending_order;
    ProcessingOrder m_processing_sltp_order;
    ProcessingHandle m_processing_handle;

    LONG m_is_shuting_down;
};

//+------------------------------------------------------------------+
#endif  // !_PROCESSOR_H_
