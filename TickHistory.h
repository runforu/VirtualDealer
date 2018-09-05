#ifndef _TICKHISTORY_H_
#define _TICKHISTORY_H_
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "../include/MT4ServerAPI.h"
#include "Loger.h"

#define MAX_TICK_SIZE 64
#define MAX_POOL_SIZE 128
struct TickPool {
    char m_symbol[12];
    TickAPI m_buffer[MAX_TICK_SIZE];
    int m_tail;
    TickPool() : m_tail(0) {}
};

class TickHistory {
public:
    void AddTick(const ConSymbol* symbol, FeedTick* tick);
    bool FindMaxBid(const char* symbol, time_t from, TickAPI& tick);
    bool FindMinBid(const char* symbol, time_t from, TickAPI& tick);
    bool FindMaxAsk(const char* symbol, time_t from, TickAPI& tick);
    bool FindMinAsk(const char* symbol, time_t from, TickAPI& tick);
    bool GetFirstPrice(const char* symbol, time_t from, TickAPI& tick);
    void DumpTickPool(const char* symbol, time_t from = 0);

    TickHistory() : m_symbol_count(0) { ZeroMemory(m_tick_pool, sizeof(m_tick_pool)); }

private:
    inline void Lock() { m_synchronizer.Lock(); }
    inline void Unlock() { m_synchronizer.Unlock(); }

    // Not locked, be careful
    int FindTickPool(const char* symbol);

    Synchronizer m_synchronizer;
    bool FindTick(const char* symbol, time_t from, bool use_bid, bool max, TickAPI& tick);
    TickPool m_tick_pool[MAX_POOL_SIZE];
    int m_symbol_count;
};

#endif  // !_TICKHISTORY_H_
