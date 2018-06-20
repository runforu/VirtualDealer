#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_
#include "Config/Configuration.h"
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
#define PRICE_BUFFER_SIZE 7
//+------------------------------------------------------------------+
//| Processor                                                        |
//+------------------------------------------------------------------+
struct TransactionInfo {
    TradeTransInfo* trans;
    const UserInfo* user;
    int* request_id;
};
//+------------------------------------------------------------------+
//| Processor                                                        |
//+------------------------------------------------------------------+
class CProcessor {
public:
    //--- dealer user info
    UserInfo m_manager;

    int m_delay_miliseconds;
    int m_virtual_dealer_login;
    //--- wp: worst price; bp: best price; fp: first price; np: next price; op: price of order opening 
    char m_price_option[8];
    int m_disable_virtual_dealer;
    char m_group[256];
    char m_symbols[256];

    //--- statistics
    LONG m_reinitialize_flag;
    int m_requests_total;
    int m_requests_processed; 

    CSync m_sync;

public:
    CProcessor();
    ~CProcessor();

    void Initialize();
    inline void Reinitialize() {
        InterlockedExchange(&m_reinitialize_flag, 1);
    }
    void ShowStatus();
    int ProcessTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id);
    void ProcessRequest(RequestInfo* request);
    void ProcessTradeAdd(TradeRecord* trade, const UserInfo* user, const ConSymbol* symb, const int mode);

private:
    static DWORD WINAPI Delay(LPVOID lpParameter);
};

extern CProcessor ExtProcessor;
//+------------------------------------------------------------------+
#endif // !_PROCESSOR_H_
