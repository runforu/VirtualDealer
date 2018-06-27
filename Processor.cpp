#include <stdio.h>
#include "Factory.h"
#include "Processor.h"
#include "common/Loger.h"
#include "config/FileConfig.h"

struct RequestHelper {
    RequestInfo* m_request_info;
    PriceOption m_price_option;
    int m_delay_milisecond;
};

//+------------------------------------------------------------------+
//| Constructor                                                      |
//+------------------------------------------------------------------+
Processor::Processor()
    : m_reinitialize_flag(0),
      m_delay_milisecond(1000),
      m_virtual_dealer_login(31415),
      m_disable_virtual_dealer(0),
      m_enable_comment(0),
      m_update_config(0),
      m_enable_tp_slippage(0),
      m_requests_total(0),
      m_requests_processed(0) {
    ZeroMemory(&m_manager, sizeof(m_manager));
    m_manager.login = 31415;
    COPY_STR(m_manager.name, "Delayed Dealer");
    COPY_STR(m_manager.ip, "DelayedDealer");
    COPY_STR(m_symbols, "*");
}
//+------------------------------------------------------------------+
//| Destructor                                                       |
//+------------------------------------------------------------------+
Processor::~Processor() {}
//+------------------------------------------------------------------+
//| Reading of config file                                           |
//+------------------------------------------------------------------+
void Processor::Initialize() {
    Factory::GetConfig()->GetInteger("Delayed Miliseconds", &m_delay_milisecond, "1000");
    Factory::GetConfig()->GetInteger("Virtual Dealer", &m_virtual_dealer_login, "31415");
    m_manager.login = m_virtual_dealer_login;
    Factory::GetConfig()->GetInteger("Disable Plugin", &m_disable_virtual_dealer, "0");
    Factory::GetConfig()->GetInteger("Enable Comment", &m_enable_comment, "0");
    Factory::GetConfig()->GetInteger("Update Config", &m_update_config, "0");
    Factory::GetConfig()->GetInteger("Enable TP Slippage", &m_enable_tp_slippage, "0");

    Factory::GetConfig()->GetString("Price Option", m_price_option, sizeof(m_price_option) - 1, "wp");
    if (m_price_option[0] == 0) {
        COPY_STR(m_price_option, "wp");
    }
    Factory::GetConfig()->GetString("Group", m_group, sizeof(m_group) - 1, "*");
    if (m_group[0] == 0) {
        COPY_STR(m_group, "*");
    }
    Factory::GetConfig()->GetString("Symbols", m_symbols, sizeof(m_symbols) - 1, "*");
    if (m_symbols[0] == 0) {
        COPY_STR(m_symbols, "*");
    }
}
//+------------------------------------------------------------------+
//| Show statistics                                                  |
//+------------------------------------------------------------------+
void Processor::ShowStatus() {
    if (Factory::GetServerInterface() != NULL && m_requests_total > 0) {
        //--- this line is used by the Log Analyser in order to calculate the requests
        //--- processed by the Helper, this is why it is not recommended to modify it
        LOG(31415, "DelayedDealer", "'%d': %d of %d requests processed (%.2lf%%)", m_manager.login, m_requests_processed,
            m_requests_total, m_requests_processed * 100.0 / m_requests_total);
    }
}
//+------------------------------------------------------------------+
//| Delay Request                                                    |
//+------------------------------------------------------------------+
DWORD WINAPI Processor::Delay(LPVOID parameter) {
    if (Factory::GetServerInterface() == NULL) {
        return 0;
    }

    RequestHelper* requestHelper = (RequestHelper*)parameter;
    LOG(31415, "DelayedDealer", "In delayed thread, request id = %d; thread id = %d.", requestHelper->m_request_info->id,
        GetCurrentThreadId());

    Sleep(requestHelper->m_delay_milisecond);

    Factory::GetServerInterface()->RequestsFree(requestHelper->m_request_info->id, Factory::GetProcessor()->m_manager.login);
    double prices[2] = {0};
    Factory::GetServerInterface()->HistoryPricesGroup(requestHelper->m_request_info, prices);
    Factory::GetServerInterface()->RequestsConfirm(requestHelper->m_request_info->id, &Factory::GetProcessor()->m_manager,
                                                   prices);

    delete requestHelper;

    LOG(31415, "DelayedDealer", "In delayed thread, Request freed thread.");
    return 0;
}
//+------------------------------------------------------------------+
//| Request processing                                               |
//+------------------------------------------------------------------+
void Processor::ProcessRequest(RequestInfo* request) {
    if (Factory::GetServerInterface() == NULL) {
        return;
    }

    LOG_INFO(31415, "DelayedDealer", request);
    LOG_INFO(31415, "DelayedDealer", &request->trade);

    //--- reinitialize if configuration changed
    if (InterlockedExchange(&m_reinitialize_flag, 0) != 0) {
        Initialize();
        if (m_update_config == 1) {
            InterlockedExchange(&m_reinitialize_flag, 0);
            UpdateConfig();
        }
    }

    //--- plugin disabled
    if (m_disable_virtual_dealer == 1) {
        LOG(31415, "DelayedDealer", "m_disable_virtual_dealer == 1.");
        return;
    }

    //--- global group check
    if (strcmp(m_group, "*") != 0 && strstr(m_group, request->group) == NULL) {
        LOG(31415, "DelayedDealer", "group check");
        return;
    }

    //--- global symbol check
    TradeTransInfo* trans = &request->trade;
    if (strcmp(m_symbols, "*") != 0 && strstr(m_symbols, trans->symbol) == NULL) {
        LOG(31415, "DelayedDealer", "symbol check");
        return;
    }

    RequestHelper* requestHelper = new RequestHelper;
    requestHelper->m_request_info = request;
    //--- global price option
    requestHelper->m_price_option = GetPriceOption(m_price_option);
    requestHelper->m_delay_milisecond = m_delay_milisecond;

    //--- apply cfg file config

    Factory::GetServerInterface()->RequestsLock(request->id, m_manager.login);
    LOG(31415, "DelayedDealer", "In main thread, Request locked.");

    HANDLE hThread = CreateThread(NULL, 0, Processor::Delay, (LPVOID)requestHelper, 0, NULL);
    m_requests_total++;
}
//+------------------------------------------------------------------+
//| Update config                                               |
//+------------------------------------------------------------------+
void Processor::UpdateConfig() { Factory::GetFileConfig(); }

//+------------------------------------------------------------------+
//| get price option                                               |
//+------------------------------------------------------------------+
PriceOption Processor::GetPriceOption(char* price_option) {
    if (price_option == NULL) {
        return PO_WORST_PRICE;
    }
    if (strcmp("bp", price_option) == 0) {
        return PO_BEST_PRICE;
    }
    if (strcmp("fp", price_option) == 0) {
        return PO_FIRST_PRICE;
    }
    if (strcmp("np", price_option) == 0) {
        return PO_NEXT_PRICE;
    }
    if (strcmp("op", price_option) == 0) {
        return PO_ORDER_PRICE;
    }
    return PO_WORST_PRICE;
}