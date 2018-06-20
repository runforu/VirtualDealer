#include <time.h>
#include <stdio.h>
#include "Loger.h"
#include "../../include/MT4ServerAPI.h"

extern CServerInterface* ExtServer;
//char LogBuffer[4096];

void Loger::out(const int code, LPCSTR ip, LPCSTR msg, ...) {
#ifdef _RELEASE_LOG_    
    if (ExtServer == NULL || msg == NULL) {
        return;
    }

    char buffer[256];
    va_list arg_ptr;
    va_start(arg_ptr, msg);
    _vsnprintf(buffer, sizeof(buffer) - 1, msg, arg_ptr);
    va_end(arg_ptr);

    ExtServer->LogsOut(code, ip, buffer);

    //TODO: Add file log
#endif
}