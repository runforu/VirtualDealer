#include <process.h>
#include <stdio.h>
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"

void Processor::GetPrice(RequestHelper* helper, double* prices) {
    RequestInfo* request_info = helper->m_request_info;

    LOG("request_info->prices[0] = %f, request_info->prices[1] = %f, request_info->trade.price = %f.", request_info->prices[0],
        request_info->prices[1], request_info->trade.price);
    prices[0] = request_info->trade.price;
    prices[1] = request_info->trade.price;

    if (helper->m_price_option == PO_ORDER_PRICE) {
        return;
    }

    if (helper->m_price_option == PO_NEXT_PRICE) {
        Factory::GetServerInterface()->HistoryPricesGroup(request_info, prices);
        return;
    }

    TradeTransInfo trade = request_info->trade;
    TickAPI tick = {0};
    bool find_tick = false;
    Lock();
    switch (helper->m_price_option) {
        case PO_WORST_PRICE:
            if (trade.type == TT_ORDER_MK_OPEN) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.ask > prices[0]) {
                        LOG("PO_WORST_PRICE TT_ORDER_MK_OPEN & OP_BUY & FindMaxAsk");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.bid < prices[1]) {
                        LOG("PO_WORST_PRICE TT_ORDER_MK_OPEN & OP_SELL & FindMinBid");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                }
            } else if (trade.type == TT_ORDER_MK_CLOSE) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.bid < prices[1]) {
                        LOG("PO_WORST_PRICE TT_ORDER_MK_CLOSE & OP_BUY & FindMinBid");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.ask > prices[0]) {
                        LOG("PO_WORST_PRICE TT_ORDER_MK_CLOSE & OP_SELL & FindMaxAsk");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                }
            }
            break;
        case PO_BEST_PRICE:
            if (trade.type == TT_ORDER_MK_OPEN) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinAsk(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.ask < prices[0]) {
                        LOG("PO_BEST_PRICE TT_ORDER_MK_OPEN & OP_BUY & FindMinAsk");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.bid > prices[1]) {
                        LOG("PO_BEST_PRICE TT_ORDER_MK_OPEN & OP_SELL & FindMaxBid");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                }
            } else if (trade.type == TT_ORDER_MK_CLOSE) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.bid > prices[1]) {
                        LOG("PO_BEST_PRICE TT_ORDER_MK_CLOSE & OP_BUY & FindMaxBid");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinAsk(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick) &&
                        tick.ask < prices[0]) {
                        LOG("PO_BEST_PRICE TT_ORDER_MK_CLOSE & OP_SELL & FindMinAsk");
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                }
            }
            break;
        case PO_FIRST_PRICE:
            find_tick = Factory::GetProcessor()->m_tick_history.GetFirstPrice(trade.symbol, helper->m_start_time, tick);
            if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick)) {
                LOG("PO_FIRST_PRICE");
                prices[0] = tick.bid;
                prices[1] = tick.ask;
            }
            break;
    }

#if 1
    TickAPI tick1;
    Factory::GetProcessor()->m_tick_history.DumpTickPool(trade.symbol, helper->m_start_time);
    Factory::GetProcessor()->m_tick_history.FindMinAsk(trade.symbol, helper->m_start_time, tick1);
    LOG("FindMinAsk");
    LOG_INFO(&tick1);
    Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick1);
    LOG("FindMaxAsk");
    LOG_INFO(&tick1);
    Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick1);
    LOG("FindMaxBid");
    LOG_INFO(&tick1);
    Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick1);
    LOG("FindMinBid");
    LOG_INFO(&tick1);
    LOG("---------------------------        from = %d", helper->m_start_time);
#endif

    Unlock();
}

void Processor::OrderProcessed(int order_id) {
    Lock();
    m_processing_order.RemoveOrder(order_id);
    Unlock();
}

//--- only handle trigered order
double Processor::GetPrice(TrigerDelayHelper* helper, int cmd, double trigered_price) {
    TradeRecord* trade_record = helper->m_trade_record;
    const UserInfo* user_info = helper->m_user_info;

    if (helper->m_price_option == PO_ORDER_PRICE || helper->m_price_option == PO_FIRST_PRICE) {
        return trigered_price;
    }

    if (helper->m_price_option == PO_NEXT_PRICE) {
        double prices[] = {0.0, 0.0};
        if (Factory::GetServerInterface()->HistoryPricesGroup(trade_record->symbol, &user_info->grp, prices) == RET_OK) {
            //--- OP_BUY of tp/sl, OP_SELL_LIMIT, OP_SELL_STOP
            if (cmd == OP_BUY || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP) {
                return prices[0];
            } else {  //--- OP_SELL of tp/sl, OP_BUY_LIMIT, OP_BUY_STOP
                return prices[1];
            }
        }
        return trigered_price;
    }

    TickAPI tick = {0};
    bool find_tick = false;
    Lock();

    LOG(" cmd = %d, price option = %s", cmd, PRICEOPTION(helper->m_price_option));

    //--- OP_BUY of tp/sl, OP_SELL_LIMIT, OP_SELL_STOP
    if (cmd == OP_BUY || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP) {
        if (helper->m_price_option == PO_WORST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(trade_record->symbol, helper->m_start_time, tick);
            LOG("PO_WORST_PRICE & FindMinBid  trigered_price = %f", trigered_price);
            if (find_tick && SpreadDiff(user_info->grp.group, trade_record->symbol, &tick) && tick.bid < trigered_price) {
                LOG("PO_WORST_PRICE & FindMinBid");
                trigered_price = tick.bid;
            }
        } else if (helper->m_price_option == PO_BEST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(trade_record->symbol, helper->m_start_time, tick);
            LOG("PO_BEST_PRICE & FindMaxBid  trigered_price = %f", trigered_price);
            if (find_tick && SpreadDiff(user_info->grp.group, trade_record->symbol, &tick) && tick.bid > trigered_price) {
                LOG("PO_BEST_PRICE & FindMaxBid");
                trigered_price = tick.bid;
            }
        }
    } else if (cmd == OP_SELL || cmd == OP_BUY_LIMIT || cmd == OP_BUY_STOP) {  //--- OP_SELL of tp/sl, OP_BUY_LIMIT, OP_BUY_STOP
        if (helper->m_price_option == PO_WORST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade_record->symbol, helper->m_start_time, tick);
            LOG("PO_WORST_PRICE & FindMaxAsk  trigered_price = %f", trigered_price);
            if (find_tick && SpreadDiff(user_info->grp.group, trade_record->symbol, &tick) && tick.ask > trigered_price) {
                LOG("PO_WORST_PRICE & FindMaxAsk");
                trigered_price = tick.ask;
            }
        } else if (helper->m_price_option == PO_BEST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMinAsk(trade_record->symbol, helper->m_start_time, tick);
            LOG("PO_BEST_PRICE & FindMinAsk  trigered_price = %f", trigered_price);
            if (find_tick && SpreadDiff(user_info->grp.group, trade_record->symbol, &tick) && tick.ask < trigered_price) {
                LOG("PO_BEST_PRICE & FindMinAsk");
                trigered_price = tick.ask;
            }
        }
    }

#if 1
    TickAPI tick1;
    Factory::GetProcessor()->m_tick_history.DumpTickPool(trade_record->symbol, helper->m_start_time);
    Factory::GetProcessor()->m_tick_history.FindMinAsk(trade_record->symbol, helper->m_start_time, tick1);
    LOG("FindMinAsk");
    LOG_INFO(&tick1);
    Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade_record->symbol, helper->m_start_time, tick1);
    LOG("FindMaxAsk");
    LOG_INFO(&tick1);
    Factory::GetProcessor()->m_tick_history.FindMaxBid(trade_record->symbol, helper->m_start_time, tick1);
    LOG("FindMaxBid");
    LOG_INFO(&tick1);
    Factory::GetProcessor()->m_tick_history.FindMinBid(trade_record->symbol, helper->m_start_time, tick1);
    LOG("FindMinBid");
    LOG_INFO(&tick1);
    LOG("---------------------------        from = %d", helper->m_start_time);
#endif

    Unlock();

    return trigered_price;
}

void Processor::Shutdown(void) {
    // release threads
}

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

Processor::~Processor() {}

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
        goto exit;
    }

    LOG("In delayed thread, request id = %d; delay = %d.", request_helper->m_request_info->id,
        request_helper->m_delay_milisecond);

    Sleep(request_helper->m_delay_milisecond);

    if (Factory::GetServerInterface() == NULL) {
        goto exit;
    }

    //--- Modify the price
    RequestInfo* request_info = request_helper->m_request_info;

    double prices[2];
    Factory::GetProcessor()->GetPrice(request_helper, prices);
    LOG("Delay--> order id = %d with request price = [%f, %f], confirmed at prices = [ %f %f ].", request_info->trade.order,
        request_info->prices[0], request_info->prices[1], prices[0], prices[1]);
    Factory::GetServerInterface()->RequestsFree(request_helper->m_request_info->id, Factory::GetProcessor()->m_manager.login);
    Factory::GetServerInterface()->RequestsConfirm(request_helper->m_request_info->id, &Factory::GetProcessor()->m_manager,
                                                   prices);

exit:
    Factory::GetProcessor()->OrderProcessed(request_info->trade.order);
    delete request_helper;
    _endthreadex(0);
    return 0;
}

UINT __stdcall Processor::DelaySlTpTriger(LPVOID parameter) {
    TrigerDelayHelper* helper = (TrigerDelayHelper*)parameter;

    if (Factory::GetServerInterface() == NULL || Factory::GetProcessor() == NULL) {
        goto exit;
    }

    LOG("DelaySlTpTriger order id =%d, sleep = %d, comment = %s", helper->m_trade_record->order, helper->m_delay_milisecond,
        helper->m_trade_record->comment);
    Sleep(helper->m_delay_milisecond);
    LOG("DelaySlTpTriger order id =%d wake up, comment = %s", helper->m_trade_record->order, helper->m_trade_record->comment);

    if (Factory::GetServerInterface() == NULL) {
        goto exit;
    }

    //--- Modify the price
    TradeRecord* trade_record = helper->m_trade_record;
    trade_record->close_price = Factory::GetProcessor()->GetPrice(helper, trade_record->cmd, trade_record->close_price);
    Factory::GetServerInterface()->OrdersUpdate(helper->m_trade_record, helper->m_user_info, UPDATE_CLOSE);
    LOG("DelaySlTpTriger--> order id = %d closed at = %f.", trade_record->order, trade_record->close_price);

exit:
    Factory::GetProcessor()->OrderProcessed(trade_record->order);
    delete helper->m_trade_record;
    delete helper;
    _endthreadex(0);
    return 0;
}

UINT __stdcall Processor::DelayPendingTriger(LPVOID parameter) {
    TrigerDelayHelper* helper = (TrigerDelayHelper*)parameter;

    if (Factory::GetServerInterface() == NULL || Factory::GetProcessor() == NULL) {
        goto exit;
    }

    LOG("DelayPendingTriger %d", helper->m_delay_milisecond);
    Sleep(helper->m_delay_milisecond);
    LOG("DelayPendingTriger wake up");

    if (Factory::GetServerInterface() == NULL) {
        goto exit;
    }

    //--- Modify the price
    TradeRecord* trade_record = helper->m_trade_record;
    LOG("m_trade_record.order = %d", trade_record->order);
    LOG_INFO(helper->m_user_info);
    // LOG("m_pending_trade_record.order = %d", helper->m_pending_trade_record->order);
    LOG_INFO(trade_record);
    trade_record->open_price =
        Factory::GetProcessor()->GetPrice(helper, helper->m_pending_trade_record->cmd, trade_record->open_price);
    Factory::GetServerInterface()->OrdersUpdate(helper->m_trade_record, helper->m_user_info, UPDATE_ACTIVATE);
    LOG("DelayPendingTriger--> order id = %d activated at = %f.", trade_record->order, trade_record->open_price);
exit:
    //--- release order
    Factory::GetProcessor()->OrderProcessed(trade_record->order);
    delete helper->m_trade_record;
    delete helper;
    _endthreadex(0);
    return 0;
}

bool Processor::SpreadDiff(const char* group, char* symbol, TickAPI* tick) {
    ConSymbol con_symbol;
    if (Factory::GetServerInterface()->SymbolsGet(symbol, &con_symbol) != FALSE) {
        LOG("SpreadDiff----------TickAPI [%d %f %f]", tick->ctm, tick->bid, tick->ask);
        int diff = GetSpreadDiff(group);
        LOG("SpreadDiff----------diff = %d", diff);
        if (diff != 0) {
            tick->bid = NormalizeDouble(tick->bid - con_symbol.point * diff / 2, con_symbol.digits);
            tick->ask = NormalizeDouble(tick->ask + con_symbol.point * diff / 2, con_symbol.digits);
            LOG("SpreadDiff----------TickAPI [%d %f %f]", tick->ctm, tick->bid, tick->ask);
        }
        return true;
    }
    return false;
}

bool Processor::GetDelayOption(const char* symbol, const char* group, int login, int volume, int order_type,
                               PriceOption& price_option, int& delay_milisecond) {
    Rule rule = {0};
    if (m_rule_container.Search(symbol, group, login, volume, order_type, &rule)) {
        //--- apply specific rule
        price_option = rule.m_price_option;
        delay_milisecond = rule.m_delay_milisecond;
        LOG("Apply \"%s\" to symbol = %s, group = %s, login = %d, volume = %d, order_type = %s", rule.m_name, symbol, group,
            login, volume, ORDERTYPE(order_type));
        LOG_INFO(&rule);
    } else {
        //--- apply global rule

        //--- global symbol check
        if (strcmp(m_global_rule_symbol, "*") != 0 && strcmp(m_global_rule_symbol, symbol) != 0) {
            LOG("global symbol check");
            return false;
        }

        //--- global group check
        if (strcmp(m_global_rule_group, "*") != 0 && strcmp(m_global_rule_group, group) != 0) {
            LOG("global group check");
            return false;
        }

        //--- global login check
        if (m_global_rule_login != -1 && m_global_rule_login != login) {
            LOG("global group check");
            return false;
        }

        //--- global min volume check
        if (m_global_rule_min_volume != -1) {
            if (volume < m_global_rule_min_volume) {
                LOG("global min volume check");
                return false;
            }
        }
        //--- global max volume check
        if (m_global_rule_max_volume != -1) {
            if (volume > m_global_rule_max_volume) {
                LOG("global max volume check");
                return false;
            }
        }
        //--- global order type check
        if ((order_type & m_global_rule_order_type) == 0) {
            LOG("global order type check");
            return false;
        }

        price_option = m_global_rule_price_option;
        delay_milisecond = m_global_rule_delay_milisecond;
        LOG("Apply global rule to symbol = %s, group = %s, login = %d, volume = %d, order_type = %s", symbol, group, login,
            volume, ORDERTYPE(order_type));
    }

    return true;
}

int Processor::GetSpreadDiff(RequestInfo* request) { return GetSpreadDiff(request->group); }

int Processor::GetSpreadDiff(const char* group) {
    ConGroup con_group;
    Factory::GetServerInterface()->GroupsGet(group, &con_group);
    return con_group.secgroups[0].spread_diff;
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
    sprintf_s(buffer, "Rule_%02d", i++);
    while (Factory::GetConfig()->HasKey(buffer)) {
        //--- Add a rule
        Factory::GetConfig()->GetString(buffer, rule, sizeof(rule) - 1, "");
        LOG("Add rule \"%s\": %s", buffer, rule);
        m_rule_container.AddRule(rule, buffer);
        RemoveWhiteChar(rule);
        Factory::GetConfig()->Add(buffer, rule, false);
        sprintf_s(buffer, "Rule_%02d", i++);
    }
    Factory::GetConfig()->Save();
}

void Processor::ProcessRequest(RequestInfo* request) {
    if (Factory::GetServerInterface() == NULL) {
        return;
    }

    LOG_INFO(request);
    LOG_INFO(&request->trade);

    Lock();
    if (m_processing_order.IsOrderProcessing(request->trade.order)) {
        Unlock();
        LOG("ProcessRequest: Order [%d] is in pending queue.", request->trade.order);
        //--- the order is delayed already
        return ;
    }
    m_processing_order.AddOrder(request->trade.order);
    Unlock();

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

    TradeTransInfo* trade = &request->trade;
    if (trade->type < TT_ORDER_IE_OPEN || trade->type > TT_ORDER_MK_CLOSE || trade->type == TT_ORDER_PENDING_OPEN) {
        Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
        return;
    }

    RequestHelper* request_helper = new RequestHelper;
    request_helper->m_request_info = request;
    request_helper->m_start_time = Factory::GetServerInterface()->TradeTime() - 1;

    //--- apply rules

    int order_type = OT_NONE;
    if (trade->type == TT_ORDER_IE_OPEN || trade->type == TT_ORDER_MK_OPEN || trade->type == TT_ORDER_REQ_OPEN) {
        order_type |= OT_OPEN;
    }
    if (trade->type == TT_ORDER_MK_CLOSE || trade->type == TT_ORDER_IE_CLOSE || trade->type == TT_ORDER_REQ_CLOSE) {
        order_type |= OT_CLOSE;
    }

    if (!GetDelayOption(trade->symbol, request->group, request->login, trade->volume, order_type,
                        request_helper->m_price_option, request_helper->m_delay_milisecond)) {
        goto without_delay;
    }

    if (request_helper->m_delay_milisecond == 0) {
        LOG("delay 0 milisecond, go out");
        goto without_delay;
    }

    Factory::GetServerInterface()->RequestsLock(request->id, m_manager.login);
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Processor::Delay, (LPVOID)request_helper, 0, NULL);
    Lock();
    m_processing_order.ModifyOrder(trade->order, hThread);
    Unlock();
    m_requests_total++;

    return;

without_delay:
    LOG("No rules to apply the order = %d ", trade->order);
    Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
    delete request_helper;
}

bool Processor::ActivatePendingOrder(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    if (Factory::GetServerInterface() == NULL) {
        return true;
    }

    Lock();
    if (m_processing_order.IsOrderProcessing(trade->order)) {
        Unlock();
        LOG("ActivatePendingOrder: Order [%d] is already pending.", trade->order);
        //--- the order is delayed already
        return false;
    }
    m_processing_order.AddOrder(trade->order);
    Unlock();

    LOG_INFO(pending);
    LOG_INFO(trade);

    //--- reinitialize if configuration changed
    if (InterlockedExchange(&m_reinitialize_flag, 0) != 0) {
        Initialize();
    }

    //--- plugin disabled
    if (m_disable_virtual_dealer == 1) {
        LOG("m_disable_virtual_dealer == 1.");
        return true;
    }

    int order_type = OT_PENDING;

    TrigerDelayHelper* request_helper = new TrigerDelayHelper;
    request_helper->m_user_info = (UserInfo*)user;
    TradeRecord* trade_record = new TradeRecord;
    memcpy_s(trade_record, sizeof(TradeRecord), trade, sizeof(TradeRecord));
    request_helper->m_trade_record = trade_record;
    request_helper->m_pending_trade_record = pending;
    request_helper->m_start_time = Factory::GetServerInterface()->TradeTime() - 1;

    if (!GetDelayOption(trade->symbol, group->group, user->login, trade->volume, order_type, request_helper->m_price_option,
                        request_helper->m_delay_milisecond)) {
        goto without_delay;
    }

    if (request_helper->m_delay_milisecond == 0) {
        LOG("delay 0 milisecond, go out");
        goto without_delay;
    }

    LOG("Begin a new thread to delay the pending order.");
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Processor::DelayPendingTriger, (LPVOID)request_helper, 0, NULL);
    Lock();
    m_processing_order.ModifyOrder(trade->order, hThread);
    Unlock();

    m_requests_total++;
    return false;

without_delay:
    LOG("No rules to apply the order = %d ", trade->order);
    delete request_helper->m_trade_record;
    delete request_helper;
    return true;
}

bool Processor::AllowSLTP(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                          const int isTP) {
    if (Factory::GetServerInterface() == NULL) {
        return true;
    }
    
    Lock();
    if (m_processing_order.IsOrderProcessing(trade->order)) {
        Unlock();
        LOG("AllowSLTP: Order [%d] is already in processing queue.", trade->order);
        return false;
    }
    m_processing_order.AddOrder(trade->order);
    Unlock();

    LOG_INFO(trade);

    //--- reinitialize if configuration changed
    if (InterlockedExchange(&m_reinitialize_flag, 0) != 0) {
        Initialize();
    }

    //--- plugin disabled
    if (m_disable_virtual_dealer == 1) {
        LOG("m_disable_virtual_dealer == 1.");
        return true;
    }

    int order_type = OT_NONE;
    if (isTP) {
        order_type = OT_TP;
        COPY_STR(trade->comment, "[tp]");
    } else {
        order_type = OT_SL;
        COPY_STR(trade->comment, "[sl]");
    }

    TrigerDelayHelper* request_helper = new TrigerDelayHelper;
    request_helper->m_user_info = (UserInfo*)user;
    TradeRecord* trade_record = new TradeRecord;
    memcpy_s(trade_record, sizeof(TradeRecord), trade, sizeof(TradeRecord));
    request_helper->m_trade_record = trade_record;
    request_helper->m_start_time = Factory::GetServerInterface()->TradeTime() - 1;

    if (!GetDelayOption(trade->symbol, group->group, user->login, trade->volume, order_type, request_helper->m_price_option,
                        request_helper->m_delay_milisecond)) {
        goto without_delay;
    }

    if (request_helper->m_delay_milisecond == 0) {
        LOG("delay 0 milisecond, go out");
        goto without_delay;
    }

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Processor::DelaySlTpTriger, (LPVOID)request_helper, 0, NULL);
    Lock();
    m_processing_order.ModifyOrder(trade->order, hThread);
    Unlock();

    m_requests_total++;

    return false;

without_delay:
    LOG("No rules to apply the order = %d ", trade->order);
    delete request_helper->m_trade_record;
    delete request_helper;
    return true;
}

void Processor::TickApply(const ConSymbol* symbol, FeedTick* tick) {
    Lock();
#if 0
    time_t t = time(NULL);
    LOG("current time = %d; tick time = %d", t + TIME_ZONE_DIFF, tick->ctm);
#endif
    m_tick_history.AddTick(symbol, tick);
    Unlock();
}
