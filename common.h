#ifndef _COMMON_H_
#define _COMMON_H_


#define TERMINATE_STR(str) str[sizeof(str) - 1] = 0;
#define COPY_STR(dst, src)                    \
    {                                         \
        strncpy_s(dst, src, sizeof(dst) - 1); \
        dst[sizeof(dst) - 1] = 0;             \
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

#endif  // !_COMMON_H_