#ifndef _PROCESSINGORDER_H_
#define _PROCESSINGORDER_H_
#include <windows.h>
#include "Synchronizer.h"

#define MAX_PROCESSING_ORDER 2048

struct HandledOrder {
    int m_order_id;
    HANDLE m_handler;  // _beginthread HANDLE is not reliable
    HandledOrder() : m_order_id(0), m_handler(0) {}
};

//--- Not thread safety
class ProcessingOrder {
    HandledOrder m_processing_order[MAX_PROCESSING_ORDER];
    Synchronizer m_synchronizer;
public:
    bool AddOrder(int order_id, HANDLE handle = 0);
    bool ModifyOrder(int order_id, HANDLE handle);
    bool RemoveOrder(int order_id);
    bool IsOrderProcessing(int order_id);
    bool IsEmpty();
    void EmptyOrders();

    ProcessingOrder(){};
    ~ProcessingOrder(){};
};

#endif  // !_PROCESSINGORDER_H_