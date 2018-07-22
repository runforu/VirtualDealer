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
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::RemoveOrder(int order_id) {
    int index = 0;
    for (index = 0; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == 0) {
            m_processing_order[index].m_order_id = 0;
            m_processing_order[index].m_handler = 0;
            break;
        }
    }
    return index < MAX_PROCESSING_ORDER;
}

bool ProcessingOrder::IsOrderProcessing(int order_id) {
    int index = 0;
    for (; index < MAX_PROCESSING_ORDER; index++) {
        if (m_processing_order[index].m_order_id == 0) {
            break;
        }
    }
    return index < MAX_PROCESSING_ORDER;
}
