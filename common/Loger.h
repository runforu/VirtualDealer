#ifndef _LOGER_H_
#define _LOGER_H_

#include <windows.h>

class Loger {
public:
    static void out(const int code, LPCSTR ip, LPCSTR msg, ...);
};

#endif // !_LOGER_H_

