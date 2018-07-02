#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include "Config/Config.h"

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
    LONG m_update_config;
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
    void UpdateFileConfig();
    inline void Reinitialize() { InterlockedExchange(&m_reinitialize_flag, 1); }
    void ShowStatus();
    void ProcessRequest(RequestInfo* request);

private:
    Processor();
    ~Processor();
    static DWORD WINAPI Delay(LPVOID parameter);
};

//+------------------------------------------------------------------+
#endif  // !_PROCESSOR_H_
