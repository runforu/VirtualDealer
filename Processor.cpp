#include <process.h>
#include <stdio.h>
#include "Factory.h"
#include "Loger.h"
#include "Processor.h"

void Processor::GetPrice(RequestHelper* helper, double* prices) {
    RequestInfo* request_info = helper->m_request_info;

    // LOG("request_info->prices[0] = %f, request_info->prices[1] = %f, request_info->trade.price = %f.",
    // request_info->prices[0], request_info->prices[1], request_info->trade.price);
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
    switch (helper->m_price_option) {
        case PO_WORST_PRICE:
            if (trade.type == TT_ORDER_MK_OPEN) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.ask > prices[0]) {
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.bid < prices[1]) {
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                }
            } else if (trade.type == TT_ORDER_MK_CLOSE) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.bid < prices[1]) {
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.ask > prices[0]) {
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
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.ask < prices[0]) {
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.bid > prices[1]) {
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                }
            } else if (trade.type == TT_ORDER_MK_CLOSE) {
                if (trade.cmd == OP_BUY) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.bid > prices[1]) {
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                } else if (trade.cmd == OP_SELL) {
                    find_tick = Factory::GetProcessor()->m_tick_history.FindMinAsk(trade.symbol, helper->m_start_time, tick);
                    if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff) &&
                        tick.ask < prices[0]) {
                        prices[0] = tick.bid;
                        prices[1] = tick.ask;
                    }
                }
            }
            break;
        case PO_FIRST_PRICE:
            find_tick = Factory::GetProcessor()->m_tick_history.GetFirstPrice(trade.symbol, helper->m_start_time, tick);
            if (find_tick && SpreadDiff(request_info->group, request_info->trade.symbol, &tick, helper->m_diff)) {
                prices[0] = tick.bid;
                prices[1] = tick.ask;
            }
            break;
    }
}

//--- only handle trigered order
double Processor::GetPrice(const char* symbol, const UserInfo* user_info, int cmd, PriceOption price_option, time_t from,
                           int diff, double trigered_price) {
    // TradeRecord* trade_record = helper->m_trade_record;
    // const UserInfo* user_info = helper->m_user_info;

    if (price_option == PO_ORDER_PRICE || price_option == PO_FIRST_PRICE) {
        return trigered_price;
    }

    if (price_option == PO_NEXT_PRICE) {
        double prices[] = {0.0, 0.0};
        if (Factory::GetServerInterface()->HistoryPricesGroup(symbol, &user_info->grp, prices) == RET_OK) {
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

    //--- OP_BUY of tp/sl, OP_SELL_LIMIT, OP_SELL_STOP
    if (cmd == OP_BUY || cmd == OP_SELL_LIMIT || cmd == OP_SELL_STOP) {
        if (price_option == PO_WORST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMinBid(symbol, from, tick);
            if (find_tick && SpreadDiff(user_info->grp.group, symbol, &tick, diff) && tick.bid < trigered_price) {
                trigered_price = tick.bid;
            }
        } else if (price_option == PO_BEST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMaxBid(symbol, from, tick);
            if (find_tick && SpreadDiff(user_info->grp.group, symbol, &tick, diff) && tick.bid > trigered_price) {
                trigered_price = tick.bid;
            }
        }
    } else if (cmd == OP_SELL || cmd == OP_BUY_LIMIT || cmd == OP_BUY_STOP) {  //--- OP_SELL of tp/sl, OP_BUY_LIMIT, OP_BUY_STOP
        if (price_option == PO_WORST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMaxAsk(symbol, from, tick);
            if (find_tick && SpreadDiff(user_info->grp.group, symbol, &tick, diff) && tick.ask > trigered_price) {
                trigered_price = tick.ask;
            }
        } else if (price_option == PO_BEST_PRICE) {
            find_tick = Factory::GetProcessor()->m_tick_history.FindMinAsk(symbol, from, tick);
            if (find_tick && SpreadDiff(user_info->grp.group, symbol, &tick, diff) && tick.ask < trigered_price) {
                trigered_price = tick.ask;
            }
        }
    }

    return trigered_price;
}

void Processor::Shutdown(void) {
    // release threads
    InterlockedExchange(&m_is_shuting_down, 1L);
    m_processing_pending_order.EmptyOrders();
    m_processing_sltp_order.EmptyOrders();
    m_processing_handle.CloseAll();
}

Processor::Processor()
    : m_reinitialize_flag(0),
      m_global_rule_delay_milisecond(1000),
      m_virtual_dealer_login(31415),
      m_disable_virtual_dealer(0),
      m_requests_total(0),
      m_requests_processed(0),
      m_is_shuting_down(0) {
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

bool Processor::SpreadDiff(const char* group, const char* symbol, TickAPI* tick, int diff) {
    ConSymbol con_symbol;
    if (Factory::GetServerInterface()->SymbolsGet(symbol, &con_symbol) != FALSE) {
        if (diff != 0) {
            tick->bid = NormalizeDouble(tick->bid - con_symbol.point * diff / 2, con_symbol.digits);
            tick->ask = NormalizeDouble(tick->ask + con_symbol.point * diff / 2, con_symbol.digits);
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
            return false;
        }

        //--- global group check
        if (strcmp(m_global_rule_group, "*") != 0 && strcmp(m_global_rule_group, group) != 0) {
            return false;
        }

        //--- global login check
        if (m_global_rule_login != -1 && m_global_rule_login != login) {
            return false;
        }

        //--- global min volume check
        if (m_global_rule_min_volume != -1) {
            if (volume < m_global_rule_min_volume) {
                return false;
            }
        }
        //--- global max volume check
        if (m_global_rule_max_volume != -1) {
            if (volume > m_global_rule_max_volume) {
                return false;
            }
        }
        //--- global order type check
        if ((order_type & m_global_rule_order_type) == 0) {
            return false;
        }

        price_option = m_global_rule_price_option;
        delay_milisecond = m_global_rule_delay_milisecond;
        LOG("Apply global rule to symbol = %s, group = %s, login = %d, volume = %d, order_type = %s", symbol, group, login,
            volume, ORDERTYPE(order_type));
    }

    return true;
}

int Processor::GetSpreadDiff(RequestInfo* request) {
    return GetSpreadDiff(request->group);
}

int Processor::GetSpreadDiff(const char* group) {
    ConGroup con_group;
    Factory::GetServerInterface()->GroupsGet(group, &con_group);
    return con_group.secgroups[0].spread_diff;
}

void Processor::Initialize() {
    m_synchronizer.Lock();
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
    _snprintf(buffer, sizeof(buffer) - 1, "Rule_%02d", i++);
    while (Factory::GetConfig()->HasKey(buffer)) {
        //--- Add a rule
        Factory::GetConfig()->GetString(buffer, rule, sizeof(rule) - 1, "");
        LOG("Add rule \"%s\": %s", buffer, rule);
        m_rule_container.AddRule(rule, buffer);
        RemoveWhiteChar(rule);
        Factory::GetConfig()->Add(buffer, rule, false);
        _snprintf(buffer, sizeof(buffer) - 1, "Rule_%02d", i++);
    }
    Factory::GetConfig()->Save();
    m_synchronizer.Unlock();
}

UINT Processor::Delay(LPVOID parameter) {
    RequestHelper* request_helper = (RequestHelper*)parameter;
    if (Factory::GetServerInterface() == NULL) {
        goto exit_label;
    }

    // LOG("In delayed thread, request id = %d; delay = %d.", request_helper->m_request_info->id,
    //    request_helper->m_delay_milisecond);

    Sleep(request_helper->m_delay_milisecond);

    if (Factory::GetServerInterface() == NULL || m_is_shuting_down) {
        goto exit_label;
    }

    //--- Modify the price
    RequestInfo* request_info = request_helper->m_request_info;

    double prices[2];
    GetPrice(request_helper, prices);
    Factory::GetServerInterface()->RequestsFree(request_helper->m_request_info->id, m_manager.login);

    Factory::GetServerInterface()->RequestsConfirm(request_helper->m_request_info->id, &m_manager, prices);

    LOG("request [%d] with trade price [%f, %f] confirmed at prices [%f, %f]", request_helper->m_request_info->id,
        request_helper->m_request_info->trade.price, request_helper->m_request_info->trade.price, prices[0], prices[1]);

exit_label:
    InterlockedIncrement(&m_requests_processed);
    m_processing_handle.RemoveHandle(request_helper->m_handle);
    delete request_helper;
    return 0;
}

inline void Processor::DelayWrapper(LPVOID parameter) {
    Factory::GetProcessor()->Delay(parameter);
    _endthread();
}

void Processor::ProcessRequest(RequestInfo* request) {
    if (m_is_shuting_down) {
        return;
    }

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

    request_helper->m_diff = GetSpreadDiff(request);

    Factory::GetServerInterface()->RequestsLock(request->id, m_manager.login);
    HANDLE handle = (HANDLE)_beginthread(DelayWrapper, 0, (LPVOID)request_helper);
    request_helper->m_handle = handle;
    m_processing_handle.AddHandle(handle);
    m_requests_total++;
    return;

without_delay:
    LOG("No rules to apply the order = %d ", trade->order);
    Factory::GetServerInterface()->RequestsConfirm(request->id, &m_manager, request->prices);
    delete request_helper;
}

UINT Processor::DelayPendingTriger(LPVOID parameter) {
    PendingDelayHelper* helper = (PendingDelayHelper*)parameter;

    if (Factory::GetServerInterface() == NULL) {
        goto exit_label;
    }

    Sleep(helper->m_delay_milisecond);

    if (Factory::GetServerInterface() == NULL || m_is_shuting_down) {
        goto exit_label;
    }

    //--- Modify the price
    // TradeRecord* trade_record = helper->m_trade_record;
    TradeRecord trade_record;
    Factory::GetServerInterface()->OrdersGet(helper->m_pending_trade_record->order, &trade_record);
    LOG("DelayPendingTriger-------------------");
    LOG_INFO(&trade_record);

    trade_record.open_price = GetPrice(trade_record.symbol, helper->m_user_info, helper->m_pending_trade_record->cmd,
                                       helper->m_price_option, helper->m_start_time, helper->m_diff, helper->open_price);

    trade_record.cmd = (trade_record.cmd == OP_BUY_LIMIT || trade_record.cmd == OP_BUY_STOP) ? OP_BUY : OP_SELL;
    trade_record.profit = 0;
    trade_record.storage = 0;
    trade_record.expiration = 0;
    trade_record.taxes = 0;

    Factory::GetServerInterface()->TradesCommission(&trade_record, helper->m_group->group, helper->m_symbol);
    Factory::GetServerInterface()->TradesCalcProfit(helper->m_group->group, &trade_record);
    trade_record.conv_rates[0] = Factory::GetServerInterface()->TradesCalcConvertation(
        helper->m_group->group, FALSE, trade_record.open_price, helper->m_symbol);
    trade_record.margin_rate = Factory::GetServerInterface()->TradesCalcConvertation(helper->m_group->group, TRUE,
                                                                                     trade_record.open_price, helper->m_symbol);
    Factory::GetServerInterface()->OrdersUpdate(&trade_record, helper->m_user_info, UPDATE_ACTIVATE);

    LOG("Pending order [%d] activated at price %f [original price %f]", trade_record.order, trade_record.open_price,
        helper->open_price);

exit_label:
    LOG("A pending order complete");
    InterlockedIncrement(&m_requests_processed);
    m_processing_pending_order.RemoveOrder(trade_record.order);
    delete helper;
    return 0;
}

inline void Processor::DelayPendingTrigerWrapper(LPVOID parameter) {
    Factory::GetProcessor()->DelayPendingTriger(parameter);
    _endthread();
}

bool Processor::ActivatePendingOrder(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                     const TradeRecord* pending, TradeRecord* trade) {
    if (m_is_shuting_down) {
        return true;
    }

    if (Factory::GetServerInterface() == NULL) {
        return true;
    }

    if (m_processing_pending_order.IsOrderProcessing(trade->order)) {
        LOG("ActivatePendingOrder: Order is already pending.");
        //--- the order is delayed already
        return false;
    }

    if (!m_processing_pending_order.AddOrder(trade->order)) {
        LOG("ActivatePendingOrder: pending order processing queue overflow");
        return true;
    }

    //--- reinitialize if configuration changed
    if (InterlockedExchange(&m_reinitialize_flag, 0) != 0) {
        Initialize();
    }

    //--- plugin disabled
    if (m_disable_virtual_dealer == 1) {
        LOG("m_disable_virtual_dealer == 1.");
        return true;
    }

    PendingDelayHelper* request_helper = new PendingDelayHelper;
    request_helper->m_user_info = (UserInfo*)user;
    request_helper->open_price = trade->open_price;
    request_helper->m_pending_trade_record = pending;
    request_helper->m_group = group;
    request_helper->m_symbol = symbol;
    request_helper->m_start_time = Factory::GetServerInterface()->TradeTime() - 1;
    if (!GetDelayOption(trade->symbol, group->group, user->login, trade->volume, OT_PENDING, request_helper->m_price_option,
                        request_helper->m_delay_milisecond)) {
        goto without_delay;
    }

    if (request_helper->m_delay_milisecond == 0) {
        LOG("delay 0 milisecond, go out");
        goto without_delay;
    }

    request_helper->m_diff = group->secgroups[0].spread_diff;

    HANDLE handle = (HANDLE)_beginthread(DelayPendingTrigerWrapper, 0, (LPVOID)request_helper);
    m_processing_pending_order.ModifyOrder(trade->order, handle);
    m_requests_total++;

    return false;

without_delay:
    LOG("No rules to apply the order = %d ", trade->order);
    delete request_helper;
    return true;
}

UINT Processor::DelaySlTpTriger(LPVOID parameter) {
    SlTpDelayHelper* helper = (SlTpDelayHelper*)parameter;

    if (Factory::GetServerInterface() == NULL) {
        goto exit_label;
    }

    Sleep(helper->m_delay_milisecond);

    if (Factory::GetServerInterface() == NULL || m_is_shuting_down) {
        goto exit_label;
    }

    //--- Modify the price
    TradeRecord trade_record;
    Factory::GetServerInterface()->OrdersGet(helper->m_order_id, &trade_record);

    if (helper->m_is_tp) {
        COPY_STR(trade_record.comment, "[tp]");
    } else {
        COPY_STR(trade_record.comment, "[sl]");
    }

    trade_record.close_price =
        GetPrice(trade_record.symbol, helper->m_user_info, trade_record.cmd, helper->m_price_option, helper->m_start_time,
                 helper->m_diff, helper->m_is_tp ? trade_record.tp : trade_record.sl);

    trade_record.close_time = helper->m_start_time + 1;
    Factory::GetServerInterface()->OrdersUpdate(&trade_record, helper->m_user_info, UPDATE_CLOSE);

    LOG("trade [%d] with %s price %f closed at price %f as %s", trade_record.order, trade_record.comment,
        helper->m_is_tp ? trade_record.tp : trade_record.sl, trade_record.close_price, trade_record.comment);

exit_label:
    LOG("A sl/tp order complete");
    InterlockedIncrement(&m_requests_processed);
    m_processing_sltp_order.RemoveOrder(trade_record.order);
    delete helper;
    return 0;
}

inline void Processor::DelaySlTpTrigerWrapper(LPVOID parameter) {
    Factory::GetProcessor()->DelaySlTpTriger(parameter);
    _endthread();
}

bool Processor::AllowSLTP(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                          const int isTP) {
    if (m_is_shuting_down) {
        return true;
    }

    if (Factory::GetServerInterface() == NULL) {
        return true;
    }

    if (m_processing_sltp_order.IsOrderProcessing(trade->order)) {
        LOG("AllowSLTP: Order is already in processing queue.");
        return false;
    }

    if (!m_processing_sltp_order.AddOrder(trade->order)) {
        LOG("AllowSLTP: sl/tp order processing queue overflow");
        return true;
    }

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

    SlTpDelayHelper* request_helper = new SlTpDelayHelper;
    request_helper->m_user_info = (UserInfo*)user;
    request_helper->m_order_id = trade->order;
    request_helper->m_start_time = Factory::GetServerInterface()->TradeTime() - 1;
    request_helper->m_is_tp = isTP;

    int order_type = isTP ? OT_TP : OT_SL;
    if (!GetDelayOption(trade->symbol, group->group, user->login, trade->volume, order_type, request_helper->m_price_option,
                        request_helper->m_delay_milisecond)) {
        goto without_delay;
    }

    if (request_helper->m_delay_milisecond == 0) {
        LOG("delay 0 milisecond, go out");
        goto without_delay;
    }

    request_helper->m_diff = group->secgroups[0].spread_diff;

    HANDLE handle = (HANDLE)_beginthread(DelaySlTpTrigerWrapper, 0, (LPVOID)request_helper);
    m_processing_sltp_order.ModifyOrder(trade->order, handle);

    m_requests_total++;
    return false;

without_delay:
    delete request_helper;
    return true;
}

void Processor::TickApply(const ConSymbol* symbol, FeedTick* tick) {
    m_tick_history.AddTick(symbol, tick);
}

void Processor::OnTradeTransaction(TradeTransInfo* trans, const UserInfo* user) {
#if 0
    TickAPI tick;
    time_t from = Factory::GetServerInterface()->TradeTime() - 10;
    Factory::GetProcessor()->m_tick_history.DumpTickPool(trans->symbol, from);
    Factory::GetProcessor()->m_tick_history.FindMinAsk(trans->symbol, from, tick);
    LOG("FindMinAsk");
    LOG_INFO(&tick);
    Factory::GetProcessor()->m_tick_history.FindMaxAsk(trans->symbol, from, tick);
    LOG("FindMaxAsk");
    LOG_INFO(&tick);
    Factory::GetProcessor()->m_tick_history.FindMaxBid(trans->symbol, from, tick);
    LOG("FindMaxBid");
    LOG_INFO(&tick);
    Factory::GetProcessor()->m_tick_history.FindMinBid(trans->symbol, from, tick);
    LOG("FindMinBid");
    LOG_INFO(&tick);
    LOG("---------------------------        from = %d", from);
#endif
}

bool Processor::IsPendingProcessing(const ConGroup* group, const ConSymbol* symbol, const TradeRecord* trade) {
    if (m_processing_pending_order.IsOrderProcessing(trade->order)) {
        LOG("PendingsFilter: Order [%d] is already in processing queue.", trade->order);
        return true;
    }
    return false;
}