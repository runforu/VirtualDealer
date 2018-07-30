#include <time.h>
#include "Loger.h"
#include "ProcessingHandle.h"

bool ProcessingHandle::AddHandle(HANDLE handle) {
    int index = 0;
    for (; index < MAX_PROCESSING_HANADLE; index++) {
        if (m_processing_handle[index] == 0) {
            m_processing_handle[index] = handle;
            break;
        }
    }
    LOG("|--> Adding an handle to process %d %s ", handle, index < MAX_PROCESSING_HANADLE ? "successful" : "failed");
    return index < MAX_PROCESSING_HANADLE;
}

bool ProcessingHandle::RemoveHandle(HANDLE handle) {
    int index = 0;
    for (index = 0; index < MAX_PROCESSING_HANADLE; index++) {
        if (m_processing_handle[index] == handle) {
            m_processing_handle[index] = 0;
            break;
        }
    }
    LOG("|--> Remove an handle in processing %d %s ", handle, index < MAX_PROCESSING_HANADLE ? "successful" : "failed");
    return index < MAX_PROCESSING_HANADLE;
}

bool ProcessingHandle::IsOrderProcessing(HANDLE handle) {
    int index = 0;
    for (; index < MAX_PROCESSING_HANADLE; index++) {
        if (m_processing_handle[index] == handle) {
            break;
        }
    }
    LOG("|--> Handle %d is in the queue: %s ", handle, index < MAX_PROCESSING_HANADLE ? "yes" : "no");
    return index < MAX_PROCESSING_HANADLE;
}

bool ProcessingHandle::IsEmpty() {
    for (int i = 0; i < MAX_PROCESSING_HANADLE; i++) {
        if (m_processing_handle[i] != 0) {
            return false;
        }
    }
    return true;
}

void ProcessingHandle::CloseAll() {
    for (int i = 0; i < MAX_PROCESSING_HANADLE; i++) {
        if (m_processing_handle[i] != 0) {
            TerminateThread(m_processing_handle[i], 0);
            CloseHandle(m_processing_handle[i]);
            m_processing_handle[i] = 0;
        }
    }
}
