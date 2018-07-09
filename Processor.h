#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include "Config.h"
#include "TickHistory.h"
#include "FileConfig.h"

#define TIME_ZONE_DIFF 10800

struct DelayedOrder {
    int order;
    time_t m_firt_hit;
    double m_worst_price[2];
};

struct RequestHelper {
    RequestInfo* m_request_info;
    PriceOption m_price_option;
    time_t m_start_time;
    int m_delay_milisecond;
};

class Processor {
    friend class Factory;

private:
    //--- dealer user info
    UserInfo m_manager;

    //--- configurations
    int m_delay_milisecond;
    int m_virtual_dealer_login;
    int m_disable_virtual_dealer;
    int m_enable_comment;
    LONG m_update_config;
    int m_enable_tp_slippage;

    //--- wp: worst price; bp: best price; fp: first price; np: next price; op: price of order opening
    char m_price_option[4];
    char m_group[32];
    char m_symbols[32];

    TickHistory m_tick_history;

    //--- statistics
    LONG m_reinitialize_flag;
    int m_requests_total;
    int m_requests_processed;

    Synchronizer m_sync;

public:
    void Initialize();
    void UpdateFileConfig();
    inline void Reinitialize() { InterlockedExchange(&m_reinitialize_flag, 1); }
    void ShowStatus();
    void ProcessRequest(RequestInfo* request);
    bool ActivatePendingOrder(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, const TradeRecord* pending,
                              TradeRecord* trade);
    bool AllowSLTP(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade, const int isTP);
    void TickApply(const ConSymbol* symbol, FeedTick* tick);

    inline void Lock() { m_sync.Lock(); }
    inline void Unlock() { m_sync.Unlock(); }

    void GetPrice(RequestHelper* helper, double* prices);

private:
    Processor();
    ~Processor();

    static int GetSpreadDiff(RequestInfo* request);
    static int GetSpreadDiff(char* group);
    static DWORD WINAPI Delay(LPVOID parameter);
    static bool  SpreadDiff(char* group, char* symbol, TickAPI * tick);

private:  // TODO
    //--- delay activation of pending order
    DelayedOrder m_pending_order[256];

    //--- delay stop loss
    DelayedOrder m_stop_loss[256];

    //--- delay take profit
    DelayedOrder m_take_profit[256];
};

//+------------------------------------------------------------------+
#endif  // !_PROCESSOR_H_
