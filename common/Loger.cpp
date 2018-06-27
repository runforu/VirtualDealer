#include <time.h>
#include <stdio.h>
#include "Loger.h"
#include "../../include/MT4ServerAPI.h"
#include "../Factory.h"

#ifdef _RELEASE_LOG_  


void Loger::out(const int code, LPCSTR ip, LPCSTR msg, ...) {
    if (Factory::GetServerInterface() == NULL || msg == NULL) {
        return;
    }

    char buffer[256];
    va_list arg_ptr;
    va_start(arg_ptr, msg);
    _vsnprintf(buffer, sizeof(buffer) - 1, msg, arg_ptr);
    va_end(arg_ptr);

    Factory::GetServerInterface()->LogsOut(code, ip, buffer);
}

void Loger::out(const int code, LPCSTR ip, RequestInfo* request) {
    Loger::out(31415, "DelayedDealer", "RequestInfo, request id = %d; status = %d; time = %d; manager = %d; login = %d; group= %s; balance = %f; credit = %f; pricees = [%f, %f]; gw_volume = %d; gw order ticket = %d, gw_price = %f.",
               request->id, request->status, request->time, request->manager, request->login, request->group, request->balance, request->credit, request->prices[0], request->prices[1], request->gw_volume, request->gw_order, request->gw_price);
}


void Loger::out(const int code, LPCSTR ip, TradeTransInfo* transaction) {
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
    char* trans_cmd[] = {
        "OP_BUY",
        "OP_SELL",
        "OP_BUY_LIMIT",
        "OP_SELL_LIMIT",
        "OP_BUY_STOP",
        "OP_SELL_STOP",
        "OP_BALANCE",
        "OP_CREDIT"
    };
    Loger::out(31415, "DelayedDealer", "TradeTransInfo, trans type = %s; flags = %d; cmd = %s; order = %d; order by = %d; symbols = %s; volume = %d; price = %f; sl = %f; tp = %f; ie_deviation = %d; comment = %s; expiration = %d.",
               trans_type[transaction->type - 64], transaction->flags, trans_cmd[transaction->cmd], transaction->order, transaction->orderby, transaction->symbol, transaction->volume, transaction->price, transaction->sl, transaction->tp, transaction->ie_deviation, transaction->comment, transaction->expiration);
}


#endif
