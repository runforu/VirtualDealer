#ifndef _FILECONFIG_H_
#define _FILECONFIG_H_

#include <time.h>
#include <windows.h>
#include "../../include/MT4ServerAPI.h"
#include "../Synchronizer.h"
#include "../common/common.h"

#define MAX_CONFIG 32

struct ExternalConfig {
    char m_execution[32];
    char m_symbol[32];
    char m_group[32];
    char m_login[32];
    char m_min_volume[32];
    char m_max_volume[32];
    char m_order_type[32];
    char m_delay_milisecond[24];
    char m_price_option[8];
};

class FileConfig {
    friend class Factory;

public:
    void Load(LPCSTR filename);

private:
    Synchronizer m_sync;                  // synchronizer
    char m_filename[MAX_PATH];            // name of the configuration file
    ExternalConfig m_config[MAX_CONFIG];  // configs
    int m_config_total;                   // total number of records

private:
    FileConfig();
    FileConfig(FileConfig const&) {}
    void operator=(FileConfig const&) {}
    ~FileConfig() {}

private:
};

#endif  //--- _FILECONFIG_H_