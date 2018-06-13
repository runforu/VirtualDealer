//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#ifndef _COMMON_H_
#define _COMMON_H_
#define TERMINATE_STR(str) str[sizeof(str)-1]=0;
#define COPY_STR(dst,src) { strncpy(dst,src,sizeof(dst)-1); dst[sizeof(dst)-1]=0; }
//+------------------------------------------------------------------+
//|                                                                  |
//+------------------------------------------------------------------+
double __fastcall NormalizeDouble(const double val, int digits);
int               CheckGroup(char* grouplist, const char *group);
double            DecPow(const int digits);
//+------------------------------------------------------------------+
#endif // !_COMMON_H_