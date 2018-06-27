#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include "Config/Config.h"

enum PriceOption { PO_WORST_PRICE, PO_BEST_PRICE, PO_FIRST_PRICE, PO_NEXT_PRICE, PO_ORDER_PRICE };

//+------------------------------------------------------------------+
//| Processor                                                        |
//+------------------------------------------------------------------+
class Processor {
    friend class Factory;

public:
    //--- dealer user info
    UserInfo m_manager;

    //--- configurations
    int m_delay_milisecond;
    int m_virtual_dealer_login;
    int m_disable_virtual_dealer;
    int m_enable_comment;
    int m_update_config;
    int m_enable_tp_slippage;

    //--- wp: worst price; bp: best price; fp: first price; np: next price; op: price of order opening
    char m_price_option[4];
    char m_group[256];
    char m_symbols[256];

    //--- statistics
    LONG m_reinitialize_flag;
    int m_requests_total;
    int m_requests_processed;

    Synchronizer m_sync;

public:
    void Initialize();
    void UpdateConfig();
    inline void Reinitialize() { InterlockedExchange(&m_reinitialize_flag, 1); }
    void ShowStatus();
    void ProcessRequest(RequestInfo* request);
    PriceOption GetPriceOption(char* price_option);

private:
    Processor();
    ~Processor();
    static DWORD WINAPI Delay(LPVOID parameter);
};

//+------------------------------------------------------------------+
#endif  // !_PROCESSOR_H_
