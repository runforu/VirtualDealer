#include "TrigerPriceManager.h"
#include "common.h"

bool TrigerOrderManager::AddPrice(const int order_id, double price, time_t first_time) {
    int index = FindTrigerOrderIndex(order_id);

    if (index == -1) {
        index = FindAvailableSlot();
        if (index == -1) {
            return false;
        }
    }

    if (m_triger_order[index].m_price_count < MAX_TRIGER_PRICE) {
        if (m_triger_order[index].m_price_count == 0) {
            m_triger_order[index].m_first_hit = first_time == 0 ? time(NULL) : first_time;
        }
        m_triger_order[index].m_price_history[m_triger_order[index].m_price_count++] = price;
        return true;
    } else {
        return false;
    }
}

bool TrigerOrderManager::FindMaxPrice(const int order_id, double &price) {
    TrigerOrder *triger_order = FindTrigerOrder(order_id);
    if (triger_order == NULL || triger_order->m_price_count < 1) {
        return false;
    }
    price = triger_order->m_price_history[0];
    for (int i = 1; i < triger_order->m_price_count; i++) {
        if (price < triger_order->m_price_history[i]) {
            price = triger_order->m_price_history[i];
        }
    }
    return true;
}

bool TrigerOrderManager::FindMinPrice(const int order_id, double &price) {
    TrigerOrder *triger_order = FindTrigerOrder(order_id);
    if (triger_order == NULL || triger_order->m_price_count < 1) {
        return false;
    }
    price = triger_order->m_price_history[0];
    for (int i = 1; i < triger_order->m_price_count; i++) {
        if (price > triger_order->m_price_history[i]) {
            price = triger_order->m_price_history[i];
        }
    }
    return true;
}

bool TrigerOrderManager::FindFirstPrice(const int order_id, double &price) {
    TrigerOrder *triger_order = FindTrigerOrder(order_id);
    if (triger_order == NULL || triger_order->m_price_count < 1) {
        return false;
    }
    price = triger_order->m_price_history[0];
    return true;
}

bool TrigerOrderManager::FindLastPrice(const int order_id, double &price) {
    TrigerOrder *triger_order = FindTrigerOrder(order_id);
    if (triger_order == NULL || triger_order->m_price_count < 1) {
        return false;
    }

    price = triger_order->m_price_history[triger_order->m_price_count - 1];
    return true;
}

bool TrigerOrderManager::TimeExpired(const int order_id, int miliseconds) {
    TrigerOrder *triger_order = FindTrigerOrder(order_id);
    if (triger_order == NULL || (time(NULL) + TIME_ZONE_DIFF) * 1000 - triger_order->m_first_hit * 1000 > miliseconds) {
        return true;
    }

    return false;
}

TrigerOrder *TrigerOrderManager::FindTrigerOrder(const int order_id) {
    int index = FindTrigerOrderIndex(order_id);
    if (index == -1) {
        return NULL;
    }
    return &m_triger_order[index];
}

int TrigerOrderManager::FindTrigerOrderIndex(const int order_id) {
    for (int i = 0; i < MAX_TRIGER_ORDER; i++) {
        if (m_triger_order[i].m_order == order_id) {
            return i;
        }
    }
    return -1;
}

int TrigerOrderManager::FindAvailableSlot() {
    for (int i = 0; i < MAX_TRIGER_ORDER; i++) {
        if (!m_triger_order[i].m_in_use) {
            return i;
        }
    }
    return -1;
}
