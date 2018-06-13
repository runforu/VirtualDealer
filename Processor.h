//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_
#include "Config/Configuration.h"
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
#define PRICE_BUFFER_SIZE  7
//+------------------------------------------------------------------+
//| Plugin configuration base                                        |
//+------------------------------------------------------------------+
class CProcessor {
public:
    //--- dealer helper user info
    UserInfo          m_manager;
    //--- set delay time in seconds
    int               m_delay_seconds;                 // seconds delayed
                                                       //--- statistic
    LONG              m_reinitialize_flag;             //
    int               m_requests_total;                // number of all transactions
    int               m_requests_processed;            // number of processed transactions
                                                       //---
    char              m_symbols[256];
    //---
    CSync             m_sync;

public:
    CProcessor();
    ~CProcessor();

    void              Initialize();
    inline void       Reinitialize() { InterlockedExchange(&m_reinitialize_flag, 1); }
    void              ShowStatus();
    void              ProcessRequest(RequestInfo *request);
private:
    static void       Out(const int code, LPCSTR ip, LPCSTR msg, ...);
    static DWORD WINAPI Delay(LPVOID lpParameter);
};

extern CProcessor ExtProcessor;
//+------------------------------------------------------------------+
#endif // !_PROCESSOR_H_

