#include <time.h>
#include "Loger.h"
#include "ProcessingOrder.h"

bool ProcessingOrder::AddOrder(int order_id, HANDLE handle) {
    int index = 0;
    for (; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == 0) {
            m_processing_order[index].m_order_id = order_id;
            m_processing_order[index].m_handler = handle;
            break;
        }
    }
    LOG("|--> Adding an order to process %d %s ", order_id, index < MAX_PROCESSING_ORDER ? "successful" : "failed");
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::ModifyOrder(int order_id, HANDLE handle) {
    int index = 0;
    for (; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == order_id) {
            m_processing_order[index].m_handler = handle;
            break;
        }
    }
    LOG("|--> Modify an order in processing %d %s ", order_id, index < MAX_PROCESSING_ORDER ? "successful" : "failed");
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::RemoveOrder(int order_id) {
    int index = 0;
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
    LOG("|--> Remove an order in processing %d %s ", order_id, index < MAX_PROCESSING_ORDER ? "successful" : "failed");
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::IsOrderProcessing(int order_id) {
    int index = 0;
    for (; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == order_id) {
            break;
        }
    }
    LOG("|--> Order %d is in processing: %s ", order_id, index < MAX_PROCESSING_ORDER ? "yes" : "no");
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::IsEmpty() {
    for (int i = 0; i < MAX_PROCESSING_ORDER; i++) {
        if (m_processing_order[i].m_order_id != 0) {
            return false;
        }
    }
    return true;
}

void ProcessingOrder::EmptyOrders() {
    for (int i = 0; i < MAX_PROCESSING_ORDER; i++) {
        if (m_processing_order[i].m_order_id != 0) {
            TerminateThread(m_processing_order[i].m_handler, 0);
            CloseHandle(m_processing_order[i].m_handler);
            m_processing_order[i].m_order_id = 0;
        }
    }
}
