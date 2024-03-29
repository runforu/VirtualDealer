#ifndef _PROCESSINGHANDLE_H_
#define _PROCESSINGHANDLE_H_
#include <windows.h>
#include "Synchronizer.h"

#define MAX_PROCESSING_HANADLE 1024

//--- Not thread safety
class ProcessingHandle {
    HANDLE m_processing_handle[MAX_PROCESSING_HANADLE];
    Synchronizer m_synchronizer;

public:
    bool AddHandle(HANDLE handle );
    bool RemoveHandle(HANDLE handle);
    bool IsOrderProcessing(HANDLE handle);
    bool IsEmpty();
    void CloseAll();

    ProcessingHandle(){};
    ~ProcessingHandle(){};
};

#endif  // !_PROCESSINGHANDLE_H_