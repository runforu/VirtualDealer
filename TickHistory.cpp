#include <time.h>
#include "Loger.h"
#include "TickHistory.h"

void TickHistory::AddTick(const ConSymbol* symbol, FeedTick* tick) {
    if (tick->ctm == 0) {
        return;
    }

    int index = FindTickPool(symbol->symbol);
    Lock();
    TickPool* tp = NULL;
    if (index != -1) {
        tp = &m_tick_pool[index];
    } else {
        if (m_symbol_count == MAX_POOL_SIZE) {
            Unlock();
            return;
        } else {
            tp = &m_tick_pool[m_symbol_count++];
        }
    }

    strncpy_s(tp->m_symbol, symbol->symbol, sizeof(tp->m_symbol));
    tp->m_buffer[tp->m_tail].ask = tick->ask;
    tp->m_buffer[tp->m_tail].bid = tick->bid;
    tp->m_buffer[tp->m_tail].ctm = tick->ctm;
    tp->m_tail = (tp->m_tail + 1) % MAX_TICK_SIZE;
    Unlock();
    return;
}

bool TickHistory::FindMaxBid(const char* symbol, time_t from, TickAPI& tick) {
    return FindTick(symbol, from, true, true, tick);
}

bool TickHistory::FindMinBid(const char* symbol, time_t from, TickAPI& tick) {
    return FindTick(symbol, from, true, false, tick);
}

bool TickHistory::FindMaxAsk(const char* symbol, time_t from, TickAPI& tick) {
    return FindTick(symbol, from, false, true, tick);
}

bool TickHistory::FindMinAsk(const char* symbol, time_t from, TickAPI& tick) {
    return FindTick(symbol, from, false, false, tick);
}

bool TickHistory::GetFirstPrice(const char* symbol, time_t from, TickAPI& tick) {
    int index = FindTickPool(symbol);

    if (index == -1) {
        return false;
    }
    Lock();
    TickPool* tp = &m_tick_pool[index];

    for (int i = tp->m_tail, j = 0; j < MAX_TICK_SIZE; j++, i = (i + 1) % MAX_TICK_SIZE) {
        TickAPI* tick_api = &tp->m_buffer[i];

        if (tick_api->ctm != 0 && tick_api->ctm >= from) {
            tick = *tick_api;
            Unlock();
            return true;
        }
    }
    Unlock();
    return false;
}

void TickHistory::DumpTickPool(const char* symbol, time_t from) {
    int index = FindTickPool(symbol);
    if (index == -1) {
        return;
    }
    Lock();
    TickPool* tp = &m_tick_pool[index];
    for (int i = tp->m_tail, j = 0; j < MAX_TICK_SIZE; j++, i = (i + 1) % MAX_TICK_SIZE) {
        TickAPI* tick_api = &tp->m_buffer[i];
        if (tick_api->ctm < from) {
            continue;
        }
        LOG("tail = %d, No. = %d: [%d %f %f];", i, j, tick_api->ctm, tick_api->bid, tick_api->ask);
    }
    Unlock();
}

int TickHistory::FindTickPool(const char* symbol) {
    int index = 0;
    Lock();
    for (; index < m_symbol_count; index++) {
        if (strcmp(m_tick_pool[index].m_symbol, symbol) == 0) {
            break;
        }
    }
    Unlock();
    return index == m_symbol_count ? -1 : index;
}

bool TickHistory::FindTick(const char* symbol, time_t from, bool use_bid, bool need_max, TickAPI& tick) {
    int index = FindTickPool(symbol);

    if (index == -1) {
        return false;
    }
    Lock();
    TickPool* tp = &m_tick_pool[index];

    bool initialized = false;
    for (int i = tp->m_tail, j = 0; j < MAX_TICK_SIZE; j++, i = (i + 1) % MAX_TICK_SIZE) {
        TickAPI* tick_api = &tp->m_buffer[i];

        if (tick_api->ctm < from) {
            continue;
        }

        if (!initialized) {
            if (tick_api->ctm != 0 && tick_api->ctm >= from) {
                tick = *tick_api;
                initialized = true;
            }
            continue;
        }

        if (use_bid) {
            if (need_max) {
                if (tick.bid < tick_api->bid) {
                    tick = *tick_api;
                }
            } else {
                if (tick.bid > tick_api->bid) {
                    tick = *tick_api;
                }
            }
        } else {
            if (need_max) {
                if (tick.ask < tick_api->ask) {
                    tick = *tick_api;
                }
            } else {
                if (tick.ask > tick_api->ask) {
                    tick = *tick_api;
                }
            }
        }
    }
    Unlock();
    return initialized;
}
