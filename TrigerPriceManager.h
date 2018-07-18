#ifndef _TRIGERPRICEMANAGER_H_
#define _TRIGERPRICEMANAGER_H_
#include <time.h>

#define MAX_TRIGER_ORDER 256
#define MAX_TRIGER_PRICE 16

struct TrigerOrder {
    bool m_in_use;
    int m_order;
    time_t m_first_hit;
    double m_price_history[MAX_TRIGER_PRICE];
    int m_price_count;
    TrigerOrder() : m_price_count(0) {}
};

class TrigerOrderManager {
public:
    TrigerOrderManager(){};
    ~TrigerOrderManager(){};

    bool AddPrice(const int order_id, double price, time_t first_time = 0);
    bool FindMaxPrice(const int order_id, double &price);
    bool FindMinPrice(const int order_id, double &price);
    bool FindFirstPrice(const int order_id, double &price);
    bool FindLastPrice(const int order_id, double &price);
    bool TimeExpired(const int order_id, int miliseconds);

private:
    TrigerOrder *FindTrigerOrder(const int order_id);
    int FindTrigerOrderIndex(const int order_id);
    int FindAvailableSlot();
    TrigerOrder m_triger_order[MAX_TRIGER_ORDER];
};

#endif  // !_TRIGERPRICEMANAGER_H_
