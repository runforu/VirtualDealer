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
    COPY_STR(m_manager.name, "Virtual Dealer");
    COPY_STR(m_manager.ip, "VirtualDealer");
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
    Factory::GetConfig()->GetLong("Update Config", &m_update_config, "0");
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
        LOG("'%d': %d of %d requests processed (%.2lf%%)", m_manager.login, m_requests_processed,
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
    LOG("In delayed thread, request id = %d; thread id = %d.", requestHelper->m_request_info->id,
        GetCurrentThreadId());

    Sleep(requestHelper->m_delay_milisecond);

    Factory::GetServerInterface()->RequestsFree(requestHelper->m_request_info->id, Factory::GetProcessor()->m_manager.login);
    double prices[2] = {0};  // Factory::GetServerInterface()->HistoryTicksGet()
    Factory::GetServerInterface()->HistoryPricesGroup(requestHelper->m_request_info, prices);
    Factory::GetServerInterface()->RequestsConfirm(requestHelper->m_request_info->id, &Factory::GetProcessor()->m_manager,
                                                   prices);

    delete requestHelper;

    LOG("In delayed thread, Request freed thread.");
    return 0;
}
//+------------------------------------------------------------------+
//| Request processing                                               |
//+------------------------------------------------------------------+
void Processor::ProcessRequest(RequestInfo* request) {
    if (Factory::GetServerInterface() == NULL) {
        return;
    }

    LOG_INFO(request);
    LOG_INFO(&request->trade);
    double prices[2] = {1, 1};
    Factory::GetServerInterface()->RequestsConfirm(request->id, &Factory::GetProcessor()->m_manager, prices);
    return;

    //--- reinitialize if configuration changed
    if (InterlockedExchange(&m_reinitialize_flag, 0) != 0) {
        Initialize();
        if (m_update_config == 1) {
            InterlockedExchange(&m_update_config, 0);
            UpdateFileConfig();
        }
    }

    //--- plugin disabled
    if (m_disable_virtual_dealer == 1) {
        LOG("m_disable_virtual_dealer == 1.");
        return;
    }

    //--- global group check
    if (strcmp(m_group, "*") != 0 && strstr(m_group, request->group) == NULL) {
        LOG("group check");
        return;
    }

    //--- global symbol check
    TradeTransInfo* trans = &request->trade;
    if (strcmp(m_symbols, "*") != 0 && strstr(m_symbols, trans->symbol) == NULL) {
        LOG("symbol check");
        return;
    }

    RequestHelper* request_helper = new RequestHelper;
    request_helper->m_request_info = request;
    //--- global price option
    request_helper->m_price_option = Factory::GetFileConfig()->ToPriceOption(m_price_option);
    request_helper->m_delay_milisecond = m_delay_milisecond;

    //--- apply cfg file config
    int order_type = OT_NONE;
    if (trans->cmd == TT_ORDER_REQ_OPEN || trans->cmd == TT_ORDER_MK_OPEN || trans->cmd == TT_ORDER_REQ_OPEN) {
        order_type |= OT_OPEN;
    }
    if (trans->cmd == TT_ORDER_MK_CLOSE || trans->cmd == TT_ORDER_IE_CLOSE || trans->cmd == TT_ORDER_REQ_CLOSE) {
        order_type |= OT_CLOSE;
    }
    if (trans->tp > 0.0) {
        order_type |= OT_TP;
    }
    if (trans->sl > 0.0) {
        order_type |= OT_SL;
    }

    ExternalConfig ec;
    if (Factory::GetFileConfig()->Search(trans->symbol, request->group, request->login, trans->volume, order_type, &ec)) {
        request_helper->m_price_option = ec.m_price_option;
        request_helper->m_delay_milisecond = ec.m_delay_milisecond;
    }

    Factory::GetServerInterface()->RequestsLock(request->id, m_manager.login);
    LOG("In main thread, Request locked.");

    HANDLE hThread = CreateThread(NULL, 0, Processor::Delay, (LPVOID)request_helper, 0, NULL);
    m_requests_total++;
}

bool Processor::ActivatePendingOrder(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    // modify the price of trade
    return true;
}

bool Processor::AllowSLTP(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                          const int isTP) {
    // modify the price of trade
    return true;
}

void Processor::UpdateFileConfig() { Factory::GetFileConfig()->Load(); }
