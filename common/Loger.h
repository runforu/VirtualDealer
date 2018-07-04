#ifndef _LOGER_H_
#define _LOGER_H_

#include <windows.h>
#include "../../include/MT4ServerAPI.h"
#ifdef _RELEASE_LOG_
#define _CODE_ 31415
#define _IP_ "VirtualDealer"
class Loger {
public:
    static void out(const int code, LPCSTR ip, LPCSTR msg, ...);
    static void out(const int code, LPCSTR ip, const RequestInfo* request);
    static void out(const int code, LPCSTR ip, const TradeTransInfo* transaction);
    static void out(const int code, LPCSTR ip, const UserInfo* user_info);
    static void out(const int code, LPCSTR ip, const ConGroup* con_group);
    static void out(const int code, LPCSTR ip, const ConSymbol* con_symbol);
    static void out(const int code, LPCSTR ip, const TradeRecord* trade_record);
};

#define LOG(format, ...) Loger::out(_CODE_, _IP_, format, ##__VA_ARGS__);
#define LOG_INFO(info) Loger::out(_CODE_, _IP_, info);

#else

#define LOG(format, ...)
#define LOG_INFO(inf)

#endif

#endif  // !_LOGER_H_
