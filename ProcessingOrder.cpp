#include <time.h>
#include "Loger.h"
#include "ProcessingOrder.h"

bool ProcessingOrder::AddOrder(int order_id, HANDLE handle) {
    int index = 0;
    m_synchronizer.Lock();
    for (; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == 0) {
            m_processing_order[index].m_order_id = order_id;
            m_processing_order[index].m_handler = handle;
            break;
        }
    }
    m_synchronizer.Unlock();
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::ModifyOrder(int order_id, HANDLE handle) {
    int index = 0;
    m_synchronizer.Lock();
    for (; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == order_id) {
            m_processing_order[index].m_handler = handle;
            break;
        }
    }
    m_synchronizer.Unlock();
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::RemoveOrder(int order_id) {
    int index = 0;
    m_synchronizer.Lock();
    for (index = 0; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == order_id) {
            m_processing_order[index].m_order_id = 0;
            if (m_processing_order[index].m_handler != 0) {
                CloseHandle(m_processing_order[index].m_handler);
            }
            m_processing_order[index].m_handler = 0;
            break;
        }
    }
    m_synchronizer.Unlock();
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::IsOrderProcessing(int order_id) {
    int index = 0;
    m_synchronizer.Lock();
    for (; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == order_id) {
            break;
        }
    }
    m_synchronizer.Unlock();
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::IsEmpty() {
    m_synchronizer.Lock();
    for (int i = 0; i < MAX_PROCESSING_ORDER; i++) {
        if (m_processing_order[i].m_order_id != 0) {
            return false;
        }
    }
    m_synchronizer.Unlock();
    return true;
}

void ProcessingOrder::EmptyOrders() {
    m_synchronizer.Lock();
    for (int i = 0; i < MAX_PROCESSING_ORDER; i++) {
        if (m_processing_order[i].m_order_id != 0) {
            TerminateThread(m_processing_order[i].m_handler, 0);
            CloseHandle(m_processing_order[i].m_handler);
            m_processing_order[i].m_order_id = 0;
        }
    }
    m_synchronizer.Unlock();
}
