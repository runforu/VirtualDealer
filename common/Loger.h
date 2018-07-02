#ifndef _LOGER_H_
#define _LOGER_H_

#include <windows.h>
#include "../../include/MT4ServerAPI.h"
#ifdef _RELEASE_LOG_   

class Loger {
public:
    static void out(const int code, LPCSTR ip, LPCSTR msg, ...);
    static void out(const int code, LPCSTR ip, RequestInfo* request);
    static void out(const int code, LPCSTR ip, TradeTransInfo* transaction);
    static void out(const int code, LPCSTR ip, UserInfo* user_info);
    static void out(const int code, LPCSTR ip, ConGroup* con_group);
    static void out(const int code, LPCSTR ip, ConSymbol* con_symbol);
    static void out(const int code, LPCSTR ip, TradeRecord* trade_record);
};

#define LOG(code, ip, format, ...) Loger::out(code, ip, format, ##__VA_ARGS__);
#define LOG_INFO(code, ip, info) Loger::out(code, ip, info);

#else

#define LOG(code, ip, format, ...) 
#define LOG_INFO(code, ip, inf)

#endif

#endif // !_LOGER_H_

