#include <stdio.h>
#include <process.h>
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"

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

void Processor::Shutdown(void)
{
    //release threads

}

//+------------------------------------------------------------------+
//| Constructor                                                      |
//+------------------------------------------------------------------+
Processor::Processor()
    : m_reinitialize_flag(0),
      m_global_rule_delay_milisecond(1000),
      m_virtual_dealer_login(31415),
      m_disable_virtual_dealer(0),
      m_requests_total(0),
      m_requests_processed(0) {
    ZeroMemory(&m_manager, sizeof(m_manager));
    m_manager.login = 31415;
    COPY_STR(m_manager.name, "Virtual Dealer");
    COPY_STR(m_manager.ip, "VirtualDealer");
    COPY_STR(m_global_rule_symbol, "*");
}

Processor::~Processor() {
}

void Processor::Initialize() {
    Factory::GetConfig()->GetInteger("Virtual Dealer ID", &m_virtual_dealer_login, "31415");
    m_manager.login = m_virtual_dealer_login;

    Factory::GetConfig()->GetInteger("Disable Plugin", &m_disable_virtual_dealer, "0");

    //--- Default rule
    Factory::GetConfig()->GetString("Symbol", m_global_rule_symbol, sizeof(m_global_rule_symbol) - 1, "*");
    if (m_global_rule_symbol[0] == 0) {
        COPY_STR(m_global_rule_symbol, "*");
    }

    Factory::GetConfig()->GetString("Group", m_global_rule_group, sizeof(m_global_rule_group) - 1, "*");
    if (m_global_rule_group[0] == 0) {
        COPY_STR(m_global_rule_group, "*");
    }

    char buffer[16] = {0};
    Factory::GetConfig()->GetString("Customer ID", buffer, sizeof(buffer) - 1, "*");
    RemoveWhiteChar(buffer);
    if (buffer[0] == 0 || buffer[0] == '*') {
        m_global_rule_login = -1;
    } else {
        m_global_rule_login = CStrToInt(buffer);
    }

    Factory::GetConfig()->GetInteger("Min Volume", &m_global_rule_min_volume, "-1");

    Factory::GetConfig()->GetInteger("Max Volume", &m_global_rule_max_volume, "-1");

    Factory::GetConfig()->GetString("Oder Type", buffer, sizeof(buffer) - 1, "*");
    m_global_rule_order_type = ToOrderType(buffer, OT_ALL);

    Factory::GetConfig()->GetInteger("Delayed Miliseconds", &m_global_rule_delay_milisecond, "1000");

    buffer[0] = 0;
    Factory::GetConfig()->GetString("Price Option", buffer, sizeof(buffer) - 1, "wp");
    m_global_rule_price_option = ToPriceOption(buffer);

    //--- specific rules
    m_rule_container.Clear();
    char rule[128];
    int i = 0;
    sprintf_s(buffer, "Rule_%d", i++);
    while (Factory::GetConfig()->HasKey(buffer)) {
        //--- Add a rule
        Factory::GetConfig()->GetString(buffer, rule, sizeof(rule) - 1, "");
        LOG("Add a rule: %s", rule);
        m_rule_container.AddRule(rule);
        sprintf_s(buffer, "Rule_%d", i++);
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

UINT __stdcall Processor::Delay(LPVOID parameter) {
    RequestHelper* request_helper = (RequestHelper*)parameter;
    if (Factory::GetServerInterface() == NULL || Factory::GetProcessor() == NULL) {
        delete request_helper;
        _endthreadex(0);
        return 0;
    }

    LOG("In delayed thread, request id = %d; thread id = %d.", request_helper->m_request_info->id, GetCurrentThreadId());

    Sleep(request_helper->m_delay_milisecond);

    if (Factory::GetServerInterface() == NULL) {
        delete request_helper;
        _endthreadex(0);
        return 0;
    }

    //--- Modify the price
    RequestInfo* request_info = request_helper->m_request_info;

    double prices[2];
    Factory::GetProcessor()->GetPrice(request_helper, prices);

    Factory::GetServerInterface()->RequestsFree(request_helper->m_request_info->id, Factory::GetProcessor()->m_manager.login);
    Factory::GetServerInterface()->RequestsConfirm(request_helper->m_request_info->id, &Factory::GetProcessor()->m_manager,
                                                   prices);

    delete request_helper;

    LOG("In delayed thread, Request freed thread.");
    _endthreadex(0);
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
    }

    //--- plugin disabled
    if (m_disable_virtual_dealer == 1) {
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        LOG("m_disable_virtual_dealer == 1.");
        return;
    }

    TradeTransInfo* trans = &request->trade;
    if (trans->type < TT_ORDER_IE_OPEN || trans->type > TT_ORDER_MK_CLOSE || trans->type == TT_ORDER_PENDING_OPEN) {
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        return;
    }

    RequestHelper* request_helper = new RequestHelper;
    request_helper->m_request_info = request;
    request_helper->m_start_time = time(NULL) + TIME_ZONE_DIFF;

    //--- apply rules

    int order_type = OT_NONE;
    if (trans->type == TT_ORDER_IE_OPEN || trans->type == TT_ORDER_MK_OPEN || trans->type == TT_ORDER_REQ_OPEN) {
        order_type |= OT_OPEN;
    }
    if (trans->type == TT_ORDER_MK_CLOSE || trans->type == TT_ORDER_IE_CLOSE || trans->type == TT_ORDER_REQ_CLOSE) {
        order_type |= OT_CLOSE;
    }

    Rule rule;
    if (m_rule_container.Search(trans->symbol, request->group, request->login, trans->volume, order_type, &rule)) {
        //--- apply specific rule
        request_helper->m_price_option = rule.m_price_option;
        request_helper->m_delay_milisecond = rule.m_delay_milisecond;
    } else {
        //--- apply global rule

        //--- global symbol check
        if (strcmp(m_global_rule_symbol, "*") != 0 && strcmp(m_global_rule_symbol, trans->symbol) == NULL) {
            LOG("global symbol check");
            goto without_delay;
        }

        //--- global group check
        if (strcmp(m_global_rule_group, "*") != 0 && strcmp(m_global_rule_group, request->group) == NULL) {
            LOG("global group check");
            goto without_delay;
        }

        //--- global login check
        if (m_global_rule_login != -1 && m_global_rule_login == request->login) {
            LOG("global group check");
            goto without_delay;
        }

        //--- global min volume check
        if (m_global_rule_min_volume != -1) {
            if (trans->volume < m_global_rule_min_volume) {
                LOG("global min volume check");
                goto without_delay;
            }
        }
        //--- global max volume check
        if (m_global_rule_max_volume != -1) {
            if (trans->volume > m_global_rule_max_volume) {
                LOG("global max volume check");
                goto without_delay;
            }
        }
        //--- global order type check
        if ((order_type & m_global_rule_order_type) == 0) {
            LOG("globalorder type check");
            goto without_delay;
        }

        request_helper->m_price_option = m_global_rule_price_option;
        request_helper->m_delay_milisecond = m_global_rule_delay_milisecond;
    }

    if (request_helper->m_delay_milisecond == 0) {
        LOG("delay 0 milisecond, go out");
        goto without_delay;
    }

    Factory::GetServerInterface()->RequestsLock(request->id, m_manager.login);
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Processor::Delay, (LPVOID)request_helper, 0, NULL);
    m_requests_total++;
    return;

without_delay:
    Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
    delete request_helper;
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
