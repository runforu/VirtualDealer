#ifndef _FILECONFIG_H_
#define _FILECONFIG_H_

#include <time.h>
#include <windows.h>

#include "Synchronizer.h"
#include "common.h"

#define MAX_CONFIG 32

enum PriceOption { PO_WORST_PRICE = 0, PO_BEST_PRICE, PO_FIRST_PRICE, PO_NEXT_PRICE, PO_ORDER_PRICE };

enum OrderType {
    OT_NONE = 0,
    OT_OPEN = 0x01,
    OT_CLOSE = 0x02,
    OT_TP = 0x04,
    OT_SL = 0x08,
    OT_ALL = 0x0F,
};

struct ExternalConfig {
    char m_symbol[12];
    char m_group[16];
    char m_login[16];
    int m_min_volume;
    int m_max_volume;
    // combination of "open", "close", "tp", "sl", "pending"; otherwise, "*"
    int m_order_type;
    int m_delay_milisecond;
    // one of "wp", "bp", "np", "fp", "op"
    PriceOption m_price_option;

public:
   static bool ParseConfig(char* line, ExternalConfig * external_config);
   static PriceOption ToPriceOption(char* price_option);
   static int ToOrderType(char* type, int default_value);
};

class FileConfig {
    friend class Factory;

public:
    void SetCfgFile(LPCSTR filename);
    void Load();
    //bool ParseConfig(char* line, ExternalConfig * external_config);
    bool Search(const char* symbol, const char* group, int client_login, int volume, int order_type,
                ExternalConfig* external_config);


//private:
    Synchronizer m_sync;                  // synchronizer
    char m_filename[MAX_PATH];            // name of the configuration file
    ExternalConfig m_config[MAX_CONFIG];  // configs
    int m_config_total;                   // total number of records

//private:
    FileConfig();
    FileConfig(FileConfig const&) {}
    void operator=(FileConfig const&) {}
    ~FileConfig() {}

private:
};

#endif  //--- _FILECONFIG_H_