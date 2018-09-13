#include <windows.h>
#include "common.h"
#include "Loger.h"

static const double ExtDecimalArray[9] = {1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0};

double __fastcall NormalizeDouble(const double val, int digits) {
    if (digits < 0) digits = 0;
    if (digits > 8) digits = 8;

    const double p = ExtDecimalArray[digits];
    return ((val >= 0.0) ? (double(__int64(val * p + 0.5000001)) / p) : (double(__int64(val * p - 0.5000001)) / p));
}

int CheckTemplate(char* expr, char* tok_end, const char* group, char* prev, int* deep) {
    char tmp = 0;
    char *lastwc, *prev_tok;
    const char* cp;
    //---check depth
    if ((*deep)++ >= 10) return (FALSE);
    //--- skip repetition
    while (*expr == '*' && expr != tok_end) expr++;
    if (expr == tok_end) return (TRUE);
    //--- look for next "*"
    lastwc = expr;
    while (*lastwc != '*' && *lastwc != 0) lastwc++;
    //--- temporarily restrict the line
    if ((tmp = *(lastwc)) != 0)  // current not the last line
    {
        tmp = *(lastwc);
        *(lastwc) = 0;
        if ((prev_tok = (char*)strstr(group, expr)) == NULL) {
            if (tmp != 0) *(lastwc) = tmp;
            return (FALSE);
        }
        *(lastwc) = tmp;
    } else {  // last line

        cp = group + strlen(group);
        for (; cp >= group; cp--)
            if (*cp == expr[0] && strcmp(cp, expr) == 0) return (TRUE);
        return (FALSE);
    }
    //--- broken up
    if (prev != NULL && prev_tok <= prev) return (FALSE);
    prev = prev_tok;

    group = prev_tok + (lastwc - expr - 1);
    //--- end
    if (lastwc != tok_end) return CheckTemplate(lastwc, tok_end, group, prev, deep);
    return (TRUE);
}

int CheckGroup(char* grouplist, const char* group) {
    if (grouplist == NULL || group == NULL) return (FALSE);
    //--- go through all the groups
    char *tok_start = grouplist, end;
    int res = TRUE, deep = 0, normal_mode;
    while (*tok_start != 0) {
        //--- skip ','
        while (*tok_start == ',') tok_start++;

        if (*tok_start == '!') {
            tok_start++;
            normal_mode = FALSE;
        } else
            normal_mode = TRUE;
        //--- find the boundaries of the token
        char* tok_end = tok_start;
        while (*tok_end != ',' && *tok_end != 0) tok_end++;
        end = *tok_end;
        *tok_end = NULL;

        char* tp = tok_start;
        const char* gp = group;
        char* prev = NULL;
        //--- go through the token
        res = TRUE;
        while (tp != tok_end && *gp != NULL) {
            //--- find "*"
            if (*tp == '*') {
                deep = 0;
                if ((res = CheckTemplate(tp, tok_end, gp, prev, &deep)) == TRUE) {
                    *tok_end = end;
                    return (normal_mode);
                }
                break;
            }
            //--- just check
            if (*tp != *gp) {
                *tok_end = end;
                res = FALSE;
                break;
            }
            tp++;
            gp++;
        }
        //--- restore
        *tok_end = end;
        //--- we found all tokens
        if (*gp == NULL && (tp == tok_end || *tp == '*') && res == TRUE) return (normal_mode);
        //--- next token
        if (*tok_end == 0) break;
        tok_start = tok_end + 1;
    }

    return (FALSE);
}

double DecPow(const int digits) {
    static double decarray[9] = {1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0};

    if (digits < 0 || digits > 8) return (0);

    return decarray[digits];
}

int CStrToInt(char* string) {
    if (string != NULL) {
        return atoi(string);
    }
    return 0;
}

char* RemoveWhiteChar(char* str) {
    char *pend = str, *pch = str;
    while (*pch != 0) {
        if (isspace(*pch)) {
            pch++;
            continue;
        }
        if (pend != pch) {
            *pend = *pch;
        }
        pend++;
        pch++;
    }
    *pend = 0;
    return str;
}

char* StrRange(char* str, const char begin, const char end, char** buf) {
    char* cp = str == NULL ? *buf : str;
    if (cp == NULL) {
        return NULL;
    }
    if (*cp == 0) {
        return NULL;
    }
    *buf = NULL;

    if ((cp = strchr(cp, begin)) == NULL) {
        return NULL;
    }
    cp++;
    char* result = cp;

    if ((cp = strchr(cp, end)) == NULL) {
        return NULL;
    }
    *cp = 0;
    cp++;
    if (strchr(result, begin) == NULL) {
        *buf = cp;
        return result;
    }
    return NULL;
}

PriceOption ToPriceOption(const char* price_option) {
    if (price_option == NULL || strlen(price_option) == 0) {
        return PO_WORST_PRICE;
    }
    if (strcmp("bp", price_option) == 0 && strlen(price_option) == 2) {
        return PO_BEST_PRICE;
    }
    if (strcmp("fp", price_option) == 0 && strlen(price_option) == 2) {
        return PO_FIRST_PRICE;
    }
    if (strcmp("np", price_option) == 0 && strlen(price_option) == 2) {
        return PO_NEXT_PRICE;
    }
    if (strcmp("op", price_option) == 0 && strlen(price_option) == 2) {
        return PO_ORDER_PRICE;
    }
    return PO_WORST_PRICE;
}

int ToOrderType(const char* type, int default_value) {
    if (type == NULL || strlen(type) == 0) {
        return default_value;
    }
    if (strcmp(type, "*") == 0) {
        return OT_ALL;
    }
    int mask = 0;
    char tmp[32], *start, *cp;
    strncpy_s(tmp, type, sizeof(tmp));
    start = tmp;
    do {
        cp = strstr(start, "|");
        if (cp != NULL) {
            *cp = 0;
            cp++;
        }
        if (strcmp(start, "open") == 0) {
            mask |= OT_OPEN;
        } else if (strcmp(start, "close") == 0) {
            mask |= OT_CLOSE;
        } else if (strcmp(start, "tp") == 0) {
            mask |= OT_TP;
        } else if (strcmp(start, "sl") == 0) {
            mask |= OT_SL;
        } else if (strcmp(start, "pending") == 0) {
            mask |= OT_PENDING;
        } else {
            return default_value;
        }
        start = cp;
    } while (start != NULL && strlen(start) != 0);
    return mask;
}

bool IsDigitalStr(char* string) {
    if (string == NULL) {
        return false;
    }

    while (*string != 0) {
        if (*string < '0' || *string > '9') {
            return false;
        }
        string++;
    }
    return true;
}

int FindToken(const char *source, const char *delimiter, const char *symbol) {
    if (source == NULL || delimiter == NULL || symbol == NULL) {
        return -1;
    }

    char *pstr; 
    char buf[1024];
    strncpy_s(buf, source, sizeof(buf));

    char *token = strtok_s(buf, delimiter, &pstr);
    while (token != NULL) {
        if (strcmp(symbol, token) == 0) {
            return token - buf;
        }
        token = strtok_s(NULL, delimiter, &pstr);
    }
    return -1;
}
