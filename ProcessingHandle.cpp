#include <time.h>
#include "Loger.h"
#include "ProcessingHandle.h"

bool ProcessingHandle::AddHandle(HANDLE handle) {
    int index = 0;
    m_synchronizer.Lock();
    for (; index < MAX_PROCESSING_HANADLE; index++) {
        if (m_processing_handle[index] == 0) {
            m_processing_handle[index] = handle;
            break;
        }
    }
    m_synchronizer.Unlock();
    return index < MAX_PROCESSING_HANADLE;
}

bool ProcessingHandle::RemoveHandle(HANDLE handle) {
    int index = 0;
    m_synchronizer.Lock();
    for (; index < MAX_PROCESSING_HANADLE; index++) {
        if (m_processing_handle[index] == handle) {
            m_processing_handle[index] = 0;
            break;
        }
    }
    m_synchronizer.Unlock();
    return index < MAX_PROCESSING_HANADLE;
}

bool ProcessingHandle::IsOrderProcessing(HANDLE handle) {
    int index = 0;
    m_synchronizer.Lock();
    for (; index < MAX_PROCESSING_HANADLE; index++) {
        if (m_processing_handle[index] == handle) {
            break;
        }
    }
    m_synchronizer.Unlock();
    return index < MAX_PROCESSING_HANADLE;
}

bool ProcessingHandle::IsEmpty() {
    int index = 0;
    m_synchronizer.Lock();
    for (; index < MAX_PROCESSING_HANADLE; index++) {
        if (m_processing_handle[index] != 0) {
            break;
        }
    }
    m_synchronizer.Unlock();
    return index >= MAX_PROCESSING_HANADLE;
}

void ProcessingHandle::CloseAll() {
    m_synchronizer.Lock();
    for (int i = 0; i < MAX_PROCESSING_HANADLE; i++) {
        if (m_processing_handle[i] != 0) {
            TerminateThread(m_processing_handle[i], 0);
            CloseHandle(m_processing_handle[i]);
            m_processing_handle[i] = 0;
        }
    }
    m_synchronizer.Unlock();
}
