#ifndef _LOGER_H_
#define _LOGER_H_

#include <windows.h>
#include "../include/MT4ServerAPI.h"
#include "RuleContainer.h"
#include "Synchronizer.h"

#ifdef _RELEASE_LOG_

#ifdef _FILE_LOG_

class FileLoger {
public:
    static void out(const int code, LPCSTR ip, LPCSTR msg, ...);
    static void out(const int code, LPCSTR ip, const RequestInfo* request);
    static void out(const int code, LPCSTR ip, const TradeTransInfo* transaction);
    static void out(const int code, LPCSTR ip, const UserInfo* user_info);
    static void out(const int code, LPCSTR ip, const ConGroup* con_group);
    static void out(const int code, LPCSTR ip, const ConSymbol* con_symbol);
    static void out(const int code, LPCSTR ip, const TradeRecord* trade_record);
    static void out(const int code, LPCSTR ip, const TickAPI* tick);
    static void out(const int code, LPCSTR ip, const Rule* rule);
    static const char* TradeTypeStr(int trade_type);
    static const char* TradeCmdStr(int trade_cmd);
    static const char* PriceOptionStr(PriceOption price_option);
    static const char* OrderTypeStr(int order_type);
};

#define TRADETYPE(trade_type) Loger::TradeTypeStr(trade_type)
#define TRADECMD(trade_cmd) Loger::TradeCmdStr(trade_cmd)
#define PRICEOPTION(price_option) Loger::PriceOptionStr(price_option)
#define ORDERTYPE(order_type) Loger::OrderTypeStr(order_type)
#define LOG(prefix, id, format, ...) Loger::out(prefix, id, format, ##__VA_ARGS__)
#define LOG_INFO(prefix, id, info) Loger::out(prefix, id, info)

#endif

#define _CODE_ 31415
#define _IP_ "VirtualDealer"

class Loger {
public:
    static Synchronizer s_synchronizer;
    static void out(const int code, LPCSTR ip, LPCSTR msg, ...);
    static void out(const int code, LPCSTR ip, const RequestInfo* request);
    static void out(const int code, LPCSTR ip, const TradeTransInfo* transaction);
    static void out(const int code, LPCSTR ip, const UserInfo* user_info);
    static void out(const int code, LPCSTR ip, const ConGroup* con_group);
    static void out(const int code, LPCSTR ip, const ConSymbol* con_symbol);
    static void out(const int code, LPCSTR ip, const TradeRecord* trade_record);
    static void out(const int code, LPCSTR ip, const TickAPI* tick);
    static void out(const int code, LPCSTR ip, const Rule* rule);
    static const char* TradeTypeStr(int trade_type);
    static const char* TradeCmdStr(int trade_cmd);
    static const char* PriceOptionStr(PriceOption price_option);
    static const char* OrderTypeStr(int order_type);
};

#define TRADETYPE(trade_type) Loger::TradeTypeStr(trade_type)
#define TRADECMD(trade_cmd) Loger::TradeCmdStr(trade_cmd)
#define PRICEOPTION(price_option) Loger::PriceOptionStr(price_option)
#define ORDERTYPE(order_type) Loger::OrderTypeStr(order_type)
#define LOG(format, ...) Loger::out(_CODE_, _IP_, format, ##__VA_ARGS__)
#define LOG_INFO(info) Loger::out(_CODE_, _IP_, info)


#else  //_RELEASE_LOG_

#define TRADETYPE(trade_type)
#define TRADECMD(trade_cmd)
#define PRICEOPTION(price_option)
#define ORDERTYPE(order_type)

#define LOG(format, ...)
#define LOG_INFO(inf)

#endif  //_RELEASE_LOG_

#endif  // !_LOGER_H_
