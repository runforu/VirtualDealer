//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#include <stdio.h>
#include "Processor.h"

//--- server interface
extern CServerInterface *ExtServer;
//--- processor
CProcessor ExtProcessor;
//+------------------------------------------------------------------+
//| Constructor                                                      |
//+------------------------------------------------------------------+
CProcessor::CProcessor()
    : m_reinitialize_flag(0), m_requests_total(0), m_requests_processed(0) {
    //--- fill user info
    ZeroMemory(&m_manager, sizeof(m_manager));
    m_manager.login = 31415;
    COPY_STR(m_manager.name, "Delayed Dealer");
    COPY_STR(m_manager.ip, "DelayedDealer");
    COPY_STR(m_symbols, "*");
    //---
}
//+------------------------------------------------------------------+
//| Destructor                                                       |
//+------------------------------------------------------------------+
CProcessor::~CProcessor() {}
//+------------------------------------------------------------------+
//| Reading of config file                                           |
//+------------------------------------------------------------------+
void CProcessor::Initialize() {
    //---
    ExtConfig.GetInteger("Delayed Seconds", &m_delay_seconds, "10");
    //--- receive symbols
    ExtConfig.GetString("Symbols", m_symbols, sizeof(m_symbols) - 1, "*");
    if (m_symbols[0] == 0) {
        COPY_STR(m_symbols, "*");
    }
    //--- notify
    Out(CmdOK, "DelayedDealer", "'%d': initialized as virtual dealer",
        m_manager.login);
}
//+------------------------------------------------------------------+
//| Show statistics                                                  |
//+------------------------------------------------------------------+
void CProcessor::ShowStatus() {
    char tmp[256];
    //---
    if (ExtServer != NULL && m_requests_total > 0) {
        //--- this line is used by the Log Analyser in order to calculate the requests
        //--- processed by the Helper, this is why it is not recommended to modify it
        _snprintf(tmp, sizeof(tmp) - 1,
                  "'%d': %d of %d requests processed (%.2lf%%)", m_manager.login,
                  m_requests_processed, m_requests_total,
                  m_requests_processed * 100.0 / m_requests_total);
        Out(CmdOK, "DelayedDealer", tmp);
    }
    //---
}
//+------------------------------------------------------------------+
//| Delay Request                                                    |
//+------------------------------------------------------------------+
DWORD WINAPI CProcessor::Delay(LPVOID lpParameter) {
    int id = (int)lpParameter;
    Out(31415, "DelayedDealer", "In delayed thread, request id = %d; thread id = %x.", id, GetCurrentThreadId());
    Sleep(ExtProcessor.m_delay_seconds);
    ExtServer->RequestsFree(id, ExtProcessor.m_manager.login);
    Out(31415, "DelayedDealer", "In delayed thread, Request freed.");
    return 0;
}
//+------------------------------------------------------------------+
//| Request processing                                               |
//+------------------------------------------------------------------+
void CProcessor::ProcessRequest(RequestInfo *request) {
    TradeTransInfo *trans = &request->trade;
    //--- check
    if (ExtServer == NULL) {
        return;
    }
    //--- increase counter
    m_requests_total++;
    //--- reinitialize if configuration changed
    if (InterlockedExchange(&m_reinitialize_flag, 0) != 0) Initialize();
    Out(31415, "DelayedDealer", "In main thread, request id = %d; trans type = %d; thread id = %x; ", request->id, trans->type, GetCurrentThreadId());
    switch (trans->type) {
    case TT_ORDER_IE_OPEN:
    case TT_ORDER_REQ_OPEN:
    case TT_ORDER_MK_OPEN:
    case TT_ORDER_PENDING_OPEN:
        ExtServer->RequestsLock(request->id, m_manager.login);
        Out(31415, "DelayedDealer", "In delayed thread, Request locked.");
        HANDLE hThread1 = CreateThread(NULL, 0, CProcessor::Delay, (LPVOID)request->id, 0, NULL);
        break;
    }
    //---
}
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
void CProcessor::Out(const int code, LPCSTR ip, LPCSTR msg, ...) {
    char buffer[1024];
    //--- check
    if (ExtServer == NULL || msg == NULL) {
        return;
    }
    //--- format string
    va_list arg_ptr;
    va_start(arg_ptr, msg);
    _vsnprintf(buffer, sizeof(buffer) - 1, msg, arg_ptr);
    va_end(arg_ptr);
    //--- output
    ExtServer->LogsOut(code, ip, buffer);
}