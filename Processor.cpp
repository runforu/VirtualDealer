#include "Processor.h"
#include <stdio.h>
#include "common/Loger.h"

extern CServerInterface* ExtServer;
CProcessor ExtProcessor;

//+------------------------------------------------------------------+
//| Constructor                                                      |
//+------------------------------------------------------------------+
CProcessor::CProcessor() : m_reinitialize_flag(0), m_requests_total(0), m_requests_processed(0) {
    //--- fill user info
    ZeroMemory(&m_manager, sizeof(m_manager));
    m_manager.login = 31415;
    COPY_STR(m_manager.name, "Delayed Dealer");
    COPY_STR(m_manager.ip, "DelayedDealer");
    COPY_STR(m_symbols, "*");
}
//+------------------------------------------------------------------+
//| Destructor                                                       |
//+------------------------------------------------------------------+
CProcessor::~CProcessor() {}
//+------------------------------------------------------------------+
//| Reading of config file                                           |
//+------------------------------------------------------------------+
void CProcessor::Initialize() {
    //--- delayed miliseconds
    ExtConfig.GetInteger("Delayed Miliseconds", &m_delay_miliseconds, "1000");
    //--- virtual dealer login
    ExtConfig.GetInteger("Virtual Dealer", &m_virtual_dealer_login, "31415");
    //--- group
    ExtConfig.GetString("Group", m_group, sizeof(m_group) - 1, "*");
    //--- symbols
    ExtConfig.GetString("Symbols", m_symbols, sizeof(m_symbols) - 1, "*");
    if (m_symbols[0] == 0) {
        COPY_STR(m_symbols, "*");
    }
}
//+------------------------------------------------------------------+
//| Show statistics                                                  |
//+------------------------------------------------------------------+
void CProcessor::ShowStatus() {
    char tmp[256];

    if (ExtServer != NULL && m_requests_total > 0) {
        //--- this line is used by the Log Analyser in order to calculate the requests
        //--- processed by the Helper, this is why it is not recommended to modify it
        _snprintf(tmp, sizeof(tmp) - 1,
                  "'%d': %d of %d requests processed (%.2lf%%)", m_manager.login,
                  m_requests_processed, m_requests_total,
                  m_requests_processed * 100.0 / m_requests_total);
        Loger::out(0, "DelayedDealer", tmp);
    }
}
//+------------------------------------------------------------------+
//| Request processing                                               |
//+------------------------------------------------------------------+
void CProcessor::ProcessTradeAdd(TradeRecord* trade, const UserInfo* user, const ConSymbol* symb, const int mode) {
    Loger::out(31415, "DelayedDealer", "CProcessor::ProcessTradeAdd.");
}
//+------------------------------------------------------------------+
//| Request processing                                               |
//+------------------------------------------------------------------+
int CProcessor::ProcessTradeTransaction(TradeTransInfo* trans, const UserInfo* user, int* request_id) {
    Loger::out(0, "DelayedDealer", "In main thread, trans order = %d; trans type = %d; thread id = %d; ", trans->order, trans->type, GetCurrentThreadId());
    return RET_OK;
}
//+------------------------------------------------------------------+
//| Delay Request                                                    |
//+------------------------------------------------------------------+
DWORD WINAPI CProcessor::Delay(LPVOID lpParameter) {
    if (ExtServer == NULL) {
        return 0;
    }
    RequestInfo* request = (RequestInfo*)lpParameter;
    Loger::out(31415, "DelayedDealer", "In delayed thread, request id = %d; thread id = %d.",
               request->id, GetCurrentThreadId());
    Sleep(ExtProcessor.m_delay_miliseconds);
    ExtServer->RequestsFree(request->id, ExtProcessor.m_manager.login);
    double prices[2] = { 0 };
    ExtServer->HistoryPricesGroup(request, prices);
    ExtServer->RequestsConfirm(request->id, &ExtProcessor.m_manager, prices);
    Loger::out(31415, "DelayedDealer", "In delayed thread, Request freed thread.");
    return 0;
}
//+------------------------------------------------------------------+
//| Request processing                                               |
//+------------------------------------------------------------------+
void CProcessor::ProcessRequest(RequestInfo* request) {
    TradeTransInfo* trans = &request->trade;

    if (ExtServer == NULL) {
        return;
    }
    //--- increase counter
    m_requests_total++;
    //--- reinitialize if configuration changed
    if (InterlockedExchange(&m_reinitialize_flag, 0) != 0)
        Initialize();
    char* trans_type[] = {
        "TT_ORDER_IE_OPEN",
        "TT_ORDER_REQ_OPEN",
        "TT_ORDER_MK_OPEN",
        "TT_ORDER_PENDING_OPEN",
        "TT_ORDER_IE_CLOSE",
        "TT_ORDER_REQ_CLOSE",
        "TT_ORDER_MK_CLOSE",
        "TT_ORDER_MODIFY",
        "TT_ORDER_DELETE",
        "TT_ORDER_CLOSE_BY",
        "TT_ORDER_CLOSE_ALL",
        "TT_BR_ORDER_OPEN",
        "TT_BR_ORDER_CLOSE",
        "TT_BR_ORDER_DELETE",
        "TT_BR_ORDER_CLOSE_BY",
        "TT_BR_ORDER_CLOSE_ALL",
        "TT_BR_ORDER_MODIFY",
        "TT_BR_ORDER_ACTIVATE",
        "TT_BR_ORDER_COMMENT",
        "TT_BR_BALANCE",
    };
    Loger::out(31415, "DelayedDealer", "In main thread, request id = %d; trans type = %s; thread id = %x; ",
               request->id, trans_type[trans->type - 64], GetCurrentThreadId());
    ExtServer->RequestsLock(request->id, m_manager.login);
    Loger::out(31415, "DelayedDealer", "In main thread, Request locked.");
    HANDLE hThread1 = CreateThread(NULL, 0, CProcessor::Delay, (LPVOID)request, 0, NULL);
}
