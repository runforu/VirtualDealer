#include <stdio.h>
#include <time.h>
#include "Factory.h"
#include "Loger.h"

#ifdef _RELEASE_LOG_
static char* trans_type[] = {
    "TT_ORDER_IE_OPEN",      "TT_ORDER_REQ_OPEN",  "TT_ORDER_MK_OPEN",     "TT_ORDER_PENDING_OPEN", "TT_ORDER_IE_CLOSE",
    "TT_ORDER_REQ_CLOSE",    "TT_ORDER_MK_CLOSE",  "TT_ORDER_MODIFY",      "TT_ORDER_DELETE",       "TT_ORDER_CLOSE_BY",
    "TT_ORDER_CLOSE_ALL",    "TT_BR_ORDER_OPEN",   "TT_BR_ORDER_CLOSE",    "TT_BR_ORDER_DELETE",    "TT_BR_ORDER_CLOSE_BY",
    "TT_BR_ORDER_CLOSE_ALL", "TT_BR_ORDER_MODIFY", "TT_BR_ORDER_ACTIVATE", "TT_BR_ORDER_COMMENT",   "TT_BR_BALANCE",
};

static char* trans_cmd[] = {"OP_BUY",      "OP_SELL",      "OP_BUY_LIMIT", "OP_SELL_LIMIT",
                            "OP_BUY_STOP", "OP_SELL_STOP", "OP_BALANCE",   "OP_CREDIT"};

void Loger::out(const int code, LPCSTR ip, LPCSTR msg, ...) {
    if (Factory::GetServerInterface() == NULL || msg == NULL) {
        return;
    }

    char buffer[1024];
    va_list arg_ptr;
    va_start(arg_ptr, msg);
    _vsnprintf(buffer, sizeof(buffer) - 1, msg, arg_ptr);
    va_end(arg_ptr);

    Factory::GetServerInterface()->LogsOut(code, ip, buffer);
}

void Loger::out(const int code, LPCSTR ip, const RequestInfo* request) {
    Loger::out(code, ip,
               "                RequestInfo: [\n"
               "                        id =               %d\n"
               "                        status =           %d\n"
               "                        time =             %d\n"
               "                        manager =          %d\n"
               "                        login =            %d\n"
               "                        group=             %s\n"
               "                        balance =          %f\n"
               "                        credit =           %f\n"
               "                        pricees =          [%f, %f]\n"
               "                        gw_volume =        %d\n"
               "                        gw order ticket =  %d\n"
               "                        gw_price =         %f ].",
               request->id, request->status, request->time, request->manager, request->login, request->group, request->balance,
               request->credit, request->prices[0], request->prices[1], request->gw_volume, request->gw_order,
               request->gw_price);
}

void Loger::out(const int code, LPCSTR ip, const TradeTransInfo* transaction) {
    Loger::out(code, ip,
               "                TradeTransInfo: [\n"
               "                        type =          %s\n"
               "                        flags =         %d\n"
               "                        cmd =           %s\n"
               "                        order =         %d\n"
               "                        order by =      %d\n"
               "                        symbols =       %s\n"
               "                        volume =        %d\n"
               "                        price =         %f\n"
               "                        sl =            %f\n"
               "                        tp =            %f\n"
               "                        ie_deviation =  %d\n"
               "                        comment =       %s\n"
               "                        expiration =    %d ].",
               transaction->type >= 64 ? trans_type[transaction->type - 64] : "Other Type", transaction->flags,
               trans_cmd[transaction->cmd], transaction->order, transaction->orderby, transaction->symbol, transaction->volume,
               transaction->price, transaction->sl, transaction->tp, transaction->ie_deviation, transaction->comment,
               transaction->expiration);
}
void Loger::out(const int code, LPCSTR ip, const UserInfo* user_info) {
    Loger::out(code, ip,
               "                UserInfo: [\n"
               "                        login =                  %d\n"
               "                        group =                  %s\n"
               "                        password =               %s\n"
               "                        name =                   %s\n"
               "                        ip =                     %s\n"
               "                        enable =                 %d\n"
               "                        enable_change_password = %d\n"
               "                        enable_read_only =       %d\n"
               "                        flags =                  %d\n"
               "                        leverage =               %d\n"
               "                        agent_account =          %d\n"
               "                        balance =                %d\n"
               "                        credit =                 %d\n"
               "                        prevbalance =            %d ].",
               user_info->login, user_info->group, user_info->password, user_info->name, user_info->ip, user_info->enable,
               user_info->enable_change_password, user_info->enable_read_only, user_info->flags, user_info->leverage,
               user_info->agent_account, user_info->balance, user_info->credit, user_info->prevbalance);
}
void Loger::out(const int code, LPCSTR ip, const ConSymbol* con_symbol) {
    Loger::out(code, ip, "ConSymbol: [ symbol = %s ].", con_symbol->symbol);
}
void Loger::out(const int code, LPCSTR ip, const ConGroup* con_group) {
    Loger::out(code, ip, "ConGroup: [ group = %s ].", con_group->group);
}

void Loger::out(const int code, LPCSTR ip, const TickAPI* tick) {
    Loger::out(code, ip, "TickAPI: [ time = %d, bid = %f, ask = %f ].", tick->ctm, tick->bid, tick->ask);
}

void Loger::out(const int code, LPCSTR ip, const TradeRecord* trade_record) {
    Loger::out(code, ip,
               "                TradeRecord: [\n"
               "                        order =            %d\n"
               "                        login =            %d\n"
               "                        symbol =           %s\n"
               "                        digits =           %d\n"
               "                        cmd =              %s\n"
               "                        volume =           %d\n"
               "                        open_time =        %d\n"
               "                        state =            %d\n"
               "                        open_price =       %f\n"
               "                        sl =               %f\n"
               "                        tp =               %f\n"
               "                        close_time =       %d\n"
               "                        gw_volume =        %d\n"
               "                        expiration =       %d\n"
               "                        reason =           %s\n"
               "                        commission =       %f\n"
               "                        commission_agent = %f\n"
               "                        storage =          %f\n"
               "                        close_price =      %f\n"
               "                        profit =           %f\n"
               "                        taxes =            %f\n"
               "                        magic =            %d\n"
               "                        comment =          %s\n"
               "                        gw_order =         %d\n"
               "                        activation =       %d\n"
               "                        gw_open_price =    %d\n"
               "                        gw_close_price =   %d\n"
               "                        margin_rate =      %f\n"
               "                        timestamp =        %d ].",
               trade_record->order, trade_record->login, trade_record->symbol, trade_record->digits,
               trans_cmd[trade_record->cmd], trade_record->volume, trade_record->open_time, trade_record->state,
               trade_record->open_price, trade_record->sl, trade_record->tp, trade_record->close_time, trade_record->gw_volume,
               trade_record->expiration, trade_record->reason, trade_record->commission, trade_record->commission_agent,
               trade_record->storage, trade_record->close_price, trade_record->profit, trade_record->taxes, trade_record->magic,
               trade_record->comment, trade_record->gw_order, trade_record->activation, trade_record->gw_open_price,
               trade_record->gw_close_price, trade_record->margin_rate, trade_record->timestamp);
}

#endif
