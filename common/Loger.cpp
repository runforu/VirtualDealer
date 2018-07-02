#include <stdio.h>
#include <time.h>
#include "../Factory.h"
#include "Loger.h"

#ifdef _RELEASE_LOG_

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

void Loger::out(const int code, LPCSTR ip, RequestInfo* request) {
    Loger::out(code, ip,
               "RequestInfo: [ id = %d; status = %d; time = %d; manager = %d; login = %d; group= %s; balance = %f; "
               "credit = %f; pricees = [%f, %f]; gw_volume = %d; gw order ticket = %d, gw_price = %f ].",
               request->id, request->status, request->time, request->manager, request->login, request->group, request->balance,
               request->credit, request->prices[0], request->prices[1], request->gw_volume, request->gw_order,
               request->gw_price);
}

void Loger::out(const int code, LPCSTR ip, TradeTransInfo* transaction) {
    char* trans_type[] = {
        "TT_ORDER_IE_OPEN",      "TT_ORDER_REQ_OPEN",  "TT_ORDER_MK_OPEN",     "TT_ORDER_PENDING_OPEN", "TT_ORDER_IE_CLOSE",
        "TT_ORDER_REQ_CLOSE",    "TT_ORDER_MK_CLOSE",  "TT_ORDER_MODIFY",      "TT_ORDER_DELETE",       "TT_ORDER_CLOSE_BY",
        "TT_ORDER_CLOSE_ALL",    "TT_BR_ORDER_OPEN",   "TT_BR_ORDER_CLOSE",    "TT_BR_ORDER_DELETE",    "TT_BR_ORDER_CLOSE_BY",
        "TT_BR_ORDER_CLOSE_ALL", "TT_BR_ORDER_MODIFY", "TT_BR_ORDER_ACTIVATE", "TT_BR_ORDER_COMMENT",   "TT_BR_BALANCE",
    };
    char* trans_cmd[] = {"OP_BUY",      "OP_SELL",      "OP_BUY_LIMIT", "OP_SELL_LIMIT",
                         "OP_BUY_STOP", "OP_SELL_STOP", "OP_BALANCE",   "OP_CREDIT"};
    Loger::out(code, ip,
               "TradeTransInfo: [type = %s; flags = %d; cmd = %s; order = %d; order by = %d; symbols = %s; volume = %d; "
               "price = %f; sl = %f; tp = %f; ie_deviation = %d; comment = %s; expiration = %d].",
               transaction->type >= 64 ? trans_type[transaction->type - 64] : "Other Type", transaction->flags,
               trans_cmd[transaction->cmd], transaction->order, transaction->orderby, transaction->symbol, transaction->volume,
               transaction->price, transaction->sl, transaction->tp, transaction->ie_deviation, transaction->comment,
               transaction->expiration);
}
void Loger::out(const int code, LPCSTR ip, UserInfo* user_info) {
    Loger::out(
        code, ip,
        "UserInfo: ["
        "login =                  %d"
        "group =                  %s"
        "password =               %s"
        "name =                   %s"
        "ip =                     %s"
        "enable =                 %d"
        "enable_change_password = %d"
        "enable_read_only =       %d"
        "flags =                  %d"
        "leverage =               %d"
        "agent_account =          %d"
        "balance =                %d"
        "credit =                 %d"
        "prevbalance =            %d ].",
        user_info->login, user_info->group, user_info->password, user_info->name, user_info->ip, user_info->enable,
        user_info->enable_change_password, user_info->enable_read_only, user_info->flags, user_info->leverage,
        user_info->agent_account, user_info->balance, user_info->credit, user_info->prevbalance);
}
void Loger::out(const int code, LPCSTR ip, ConSymbol* con_symbol) {
    Loger::out(code, ip, "ConSymbol: [ symbol = %s ].", con_symbol->symbol);
}
void Loger::out(const int code, LPCSTR ip, ConGroup* con_group) {
    Loger::out(code, ip, "ConGroup: [ group = %s ].", con_group->group);
}
void Loger::out(const int code, LPCSTR ip, TradeRecord* trade_record) {
    Loger::out(code, ip,
               "TradeRecord: ["
        "order =            %d"
        "login =            %d"
        "symbol =           %s"
        "digits =           %d"
        "cmd =              %d"
        "volume =           %d"
        "open_time =        %d"
        "state =            %d"
        "open_price =       %f"
        "sl =               %f"
        "tp =               %f"
        "close_time =       %d"
        "gw_volume =        %d"
        "expiration =       %d"
        "reason =           %s"
        "commission =       %f"
        "commission_agent = %f"
        "storage =          %f"
        "close_price =      %f"
        "profit =           %f"
        "taxes =            %f"
        "magic =            %d"
        "comment =          %s"
        "gw_order =         %d"
        "activation =       %d"
        "gw_open_price =    %d"
        "gw_close_price =   %d"
        "margin_rate =      %f"
        "timestamp =        %d ].",
        trade_record->order,
        trade_record->login,
        trade_record->symbol,
        trade_record->digits,
        trade_record->cmd,
        trade_record->volume,
        trade_record->open_time,
        trade_record->state,
        trade_record->open_price,
        trade_record->sl,
        trade_record->tp,
        trade_record->close_time,
        trade_record->gw_volume,
        trade_record->expiration,
        trade_record->reason,
        trade_record->commission,
        trade_record->commission_agent,
        trade_record->storage,
        trade_record->close_price,
        trade_record->profit,
        trade_record->taxes,
        trade_record->magic,
        trade_record->comment,
        trade_record->gw_order,
        trade_record->activation,
        trade_record->gw_open_price,
        trade_record->gw_close_price,
        trade_record->margin_rate,
        trade_record->timestamp);
}

#endif
