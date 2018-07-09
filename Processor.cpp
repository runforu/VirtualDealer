#include <stdio.h>
#include "Factory.h"
#include "Processor.h"
#include "Loger.h"

void Processor::GetPrice(RequestHelper* helper, double* prices) {
    RequestInfo* request_info = helper->m_request_info;
    if (helper->m_price_option == PO_ORDER_PRICE) {
        prices[0] = request_info->prices[0];
        prices[1] = request_info->prices[1];
        return;
    }

    if (helper->m_price_option == PO_NEXT_PRICE) {
        if (Factory::GetServerInterface()->HistoryPricesGroup(request_info, prices) != RET_OK) {
            prices[0] = request_info->prices[0];
            prices[1] = request_info->prices[1];
        }
        return;
    }

    TradeTransInfo trade = request_info->trade;
    TickAPI tick;
    bool find_tick = false;
    Lock();
    switch (helper->m_price_option) {
        case PO_WORST_PRICE:
            if (trade.type == TT_ORDER_MK_OPEN) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick);
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick);
                }
            } else if (trade.type == TT_ORDER_MK_CLOSE) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick);
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick);
                }
            }
            break;
        case PO_BEST_PRICE:
            if (trade.type == TT_ORDER_MK_OPEN) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinAsk(trade.symbol, helper->m_start_time, tick);
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick);
                }
            } else if (trade.type == TT_ORDER_MK_CLOSE) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick);
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinAsk(trade.symbol, helper->m_start_time, tick);
                }
            }
            break;
        case PO_FIRST_PRICE:
            find_tick = Factory::GetProcessor()->m_tick_history.GetFirstPrice(trade.symbol, helper->m_start_time, tick);
            break;
    }

#if 1
    LOG("---------------------------");
    Factory::GetProcessor()->m_tick_history.DumpTickPool(trade.symbol);
    Factory::GetProcessor()->m_tick_history.FindMinAsk(trade.symbol, helper->m_start_time, tick);
    LOG_INFO(&tick);
    Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick);
    LOG_INFO(&tick);
    Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick);
    LOG_INFO(&tick);
    Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick);
    LOG_INFO(&tick);
    LOG("---------------------------        from = %d", helper->m_start_time);
#endif

    Unlock();

    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick)) {
        prices[0] = tick.bid;
        prices[1] = tick.ask;
    } else {
        prices[0] = request_info->prices[0];
        prices[1] = request_info->prices[1];
    }
}

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

Processor::~Processor() {}

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
        LOG("'%d': %d of %d requests processed (%.2lf%%)", m_manager.login, m_requests_processed, m_requests_total,
            m_requests_processed * 100.0 / m_requests_total);
    }
}

DWORD WINAPI Processor::Delay(LPVOID parameter) {
    RequestHelper* request_helper = (RequestHelper*)parameter;
    if (Factory::GetServerInterface() == NULL || Factory::GetProcessor() == NULL) {
        delete request_helper;
        return 0;
    }

    LOG("In delayed thread, request id = %d; thread id = %d.", request_helper->m_request_info->id, GetCurrentThreadId());

    Sleep(request_helper->m_delay_milisecond);

    //--- Modify the price
    RequestInfo* request_info = request_helper->m_request_info;

    double prices[2];
    Factory::GetProcessor()->GetPrice(request_helper, prices);

    Factory::GetServerInterface()->RequestsFree(request_helper->m_request_info->id, Factory::GetProcessor()->m_manager.login);
    Factory::GetServerInterface()->RequestsConfirm(request_helper->m_request_info->id, &Factory::GetProcessor()->m_manager,
                                                   prices);

    delete request_helper;

    LOG("In delayed thread, Request freed thread.");
    return 0;
}

bool Processor::SpreadDiff(char* group, char* symbol, TickAPI* tick) {
    ConSymbol con_symbol;
    if (Factory::GetServerInterface()->SymbolsGet(symbol, &con_symbol) != FALSE) {
        LOG("SpreadDiff----------TickAPI [%d %f %f]", tick->ctm, tick->bid, tick->ask);
        int diff = GetSpreadDiff(group);
        double delta = NormalizeDouble(con_symbol.point * diff / 2, con_symbol.digits);
        tick->bid -= delta;
        tick->ask += delta;
        LOG("SpreadDiff----------TickAPI [%d %f %f]", tick->ctm, tick->bid, tick->ask);
        return true;
    }
    return false;
}

int Processor::GetSpreadDiff(RequestInfo* request) { return GetSpreadDiff(request->group); }

int Processor::GetSpreadDiff(char* group) {
    ConGroup con_group;
    Factory::GetServerInterface()->GroupsGet(group, &con_group);
    return con_group.secgroups[0].spread_diff;
}

void Processor::ProcessRequest(RequestInfo* request) {
    if (Factory::GetServerInterface() == NULL) {
        return;
    }

    LOG_INFO(request);
    LOG_INFO(&request->trade);

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
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        LOG("m_disable_virtual_dealer == 1.");
        return;
    }

    //--- close all
    TradeTransInfo* trans = &request->trade;
    if (trans->type < TT_ORDER_IE_OPEN || trans->type > TT_ORDER_MK_CLOSE || trans->type == TT_ORDER_PENDING_OPEN) {
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        return;
    }

    //--- global group check
    if (strcmp(m_group, "*") != 0 && strstr(m_group, request->group) == NULL) {
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        LOG("group check fail");
        return;
    }

    //--- global symbol check
    if (strcmp(m_symbols, "*") != 0 && strstr(m_symbols, trans->symbol) == NULL) {
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        LOG("symbol check fail");
        return;
    }

    RequestHelper* request_helper = new RequestHelper;
    request_helper->m_request_info = request;
    request_helper->m_price_option = Factory::GetFileConfig()->ToPriceOption(m_price_option);
    request_helper->m_delay_milisecond = m_delay_milisecond;
    request_helper->m_start_time = time(NULL) + TIME_ZONE_DIFF;

    //--- apply cfg file config
    int order_type = OT_NONE;
    if (trans->type == TT_ORDER_IE_OPEN || trans->type == TT_ORDER_MK_OPEN || trans->type == TT_ORDER_REQ_OPEN) {
        order_type |= OT_OPEN;
    }
    if (trans->type == TT_ORDER_MK_CLOSE || trans->type == TT_ORDER_IE_CLOSE || trans->type == TT_ORDER_REQ_CLOSE) {
        order_type |= OT_CLOSE;
    }

    ExternalConfig ec;
    if (Factory::GetFileConfig()->Search(trans->symbol, request->group, request->login, trans->volume, order_type, &ec)) {
        request_helper->m_price_option = ec.m_price_option;
        request_helper->m_delay_milisecond = ec.m_delay_milisecond;
    }

    if (request_helper->m_delay_milisecond == 0) {
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        return;
    }

    Factory::GetServerInterface()->RequestsLock(request->id, m_manager.login);

    HANDLE hThread = CreateThread(NULL, 0, Processor::Delay, (LPVOID)request_helper, 0, NULL);
    m_requests_total++;
}

bool Processor::ActivatePendingOrder(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    int order_type = OT_OPEN;

    // modify the price of trade
    return true;
}

bool Processor::AllowSLTP(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                          const int isTP) {
    int order_type = OT_NONE;
    if (trade->profit > 0) {
        order_type |= OT_TP;
    } else {
        order_type |= OT_SL;
    }

    // modify the price of trade
    return true;
}

void Processor::TickApply(const ConSymbol* symbol, FeedTick* tick) {
    Lock();
    m_tick_history.AddTick(symbol, tick);
    Unlock();
}

void Processor::UpdateFileConfig() { Factory::GetFileConfig()->Load(); }
