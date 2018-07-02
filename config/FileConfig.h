#ifndef _FILECONFIG_H_
#define _FILECONFIG_H_

#include <time.h>
#include <windows.h>

#include "../Synchronizer.h"
#include "../common/common.h"

#define MAX_CONFIG 32

enum PriceOption { PO_WORST_PRICE = 0, PO_BEST_PRICE, PO_FIRST_PRICE, PO_NEXT_PRICE, PO_ORDER_PRICE };
enum ExecutionType { ET_NONE = 0x0, ET_MARKET = 0x01, ET_ASKING = 0x02, ET_ALL = 0x03 };
enum OrderType {
    OT_NONE = 0,
    OT_OPEN = 0x01,
    OT_CLOSE = 0x02,
    OT_TP = 0x04,
    OT_SL = 0x08,
    OT_ALL = 0x0F,
};

struct ExternalConfig {
    // char m_execution[32];
    // "market" or "asking" or "*"
    ExecutionType m_execution_type;
    char m_symbol[32];
    char m_group[32];
    char m_login[32];
    // char m_min_volume[32];
    int m_min_volume;
    // char m_max_volume[32];
    int m_max_volume;
    // char m_order_type[32];
    // combination of "open", "close", "tp", "sl"; otherwise, "*"
    int m_order_type;
    // char m_delay_milisecond[24];
    int m_delay_milisecond;
    // char m_price_option[8];
    // one of "wp", "bp", "np", "fp", "op"
    PriceOption m_price_option;
};

class FileConfig {
    friend class Factory;

public:
    void SetCfgFile(LPCSTR filename);
    void Load();
    bool Search(ExecutionType execution_type, const char* symbol, const char* group, int client_login, int volume,
                int order_type, ExternalConfig* external_config);
    static PriceOption ToPriceOption(char* price_option);
    static bool ToExecutionType(char* type, ExecutionType* execution_type, ExecutionType default_value);
    static int ToOrderType(char* type, int default_value);
    static int CStrToInt(char* string);

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