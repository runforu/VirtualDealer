#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include <time.h>
#include <windows.h>
#include "../../include/MT4ServerAPI.h"
#include "../common/common.h"

//+------------------------------------------------------------------+
//| Simple synchronizer                                              |
//+------------------------------------------------------------------+
class CSync {
private:
    CRITICAL_SECTION m_cs;

public:
    CSync() {
        ZeroMemory(&m_cs, sizeof(m_cs));
        InitializeCriticalSection(&m_cs);
    }
    ~CSync() {
        DeleteCriticalSection(&m_cs);
        ZeroMemory(&m_cs, sizeof(m_cs));
    }
    inline void Lock() { EnterCriticalSection(&m_cs); }
    inline void Unlock() { LeaveCriticalSection(&m_cs); }
};
//+------------------------------------------------------------------+
//| Simple configuration                                             |
//+------------------------------------------------------------------+
class CConfiguration {
private:
    CSync m_sync; // synchronizer
    char m_filename[MAX_PATH]; // name of the configuration file
    PluginCfg* m_cfg; // configs
    int m_cfg_total; // total number of records
    int m_cfg_max; // max number of records

public:
    CConfiguration();
    ~CConfiguration();
    //--- Initializing the database (reading the config file)
    void Load(LPCSTR filename);
    //--- access
    int Add(const PluginCfg* cfg);
    int Set(const PluginCfg* values, const int total);
    int Get(LPCSTR name, PluginCfg* cfg);
    int Next(const int index, PluginCfg* cfg);
    int Delete(LPCSTR name);
    inline int Total(void) {
        m_sync.Lock();
        int total = m_cfg_total;
        m_sync.Unlock();
        return (total);
    }

    int GetInteger(LPCSTR name, int* value, LPCSTR defvalue = NULL);
    int GetString(LPCSTR name, LPTSTR value, const int maxlen, LPCSTR defvalue = NULL);

private:
    void Save(void);
    PluginCfg* Search(LPCSTR name);
    static int SortByName(const void* left, const void* right);
    static int SearchByName(const void* left, const void* right);
};

extern CConfiguration ExtConfig;
//+------------------------------------------------------------------+
#endif // !_CONFIGURATION_H_