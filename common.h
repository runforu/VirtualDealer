#ifndef _COMMON_H_
#define _COMMON_H_

#include <string.h>

#define TERMINATE_STR(str) str[sizeof(str) - 1] = 0;
#define COPY_STR(dst, src)                      \
    {                                           \
        if (dst != NULL && src != NULL) {       \
            strncpy(dst, src, sizeof(dst) - 1); \
            dst[sizeof(dst) - 1] = 0;           \
        }                                       \
    }

enum PriceOption { PO_WORST_PRICE = 0, PO_BEST_PRICE, PO_FIRST_PRICE, PO_NEXT_PRICE, PO_ORDER_PRICE };

enum OrderType {
    OT_NONE = 0,
    OT_OPEN = 0x01,
    OT_CLOSE = 0x02,
    OT_TP = 0x04,
    OT_SL = 0x08,
    OT_PENDING = 0x10,
    OT_ALL = 0x1F,
};

double __fastcall NormalizeDouble(const double val, int digits);

int CheckGroup(char* grouplist, const char* group);

double DecPow(const int digits);

char* RemoveWhiteChar(char* str);

char* StrRange(char* str, const char begin, const char end, char** buf);

int CStrToInt(char* string);

PriceOption ToPriceOption(const char* price_option);

int ToOrderType(const char* type, int default_value);

bool IsDigitalStr(char* string);

int FindToken(const char* source, const char* delimiter, const char* symbol);


const char* TradeTypeStr(int trade_type);

const char* TradeCmdStr(int trade_cmd);

const char* PriceOptionStr(PriceOption price_option);

const char* OrderTypeStr(int order_type);

//--- "OP_BUY", "OP_SELL", "OP_BUY_LIMIT", "OP_SELL_LIMIT", "OP_BUY_STOP", "OP_SELL_STOP"
int ToCmd(const char* cmd_str, int default_value = -1);

//--- TS_OPEN_NORMAL, TS_OPEN_REMAND, TS_OPEN_RESTORED, TS_CLOSED_NORMAL, TS_CLOSED_PART, TS_CLOSED_BY, TS_DELETED
const char* ToTradeRecordStateStr(int state);

#endif  // !_COMMON_H_