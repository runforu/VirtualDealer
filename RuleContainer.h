#ifndef _FILECONFIG_H_
#define _FILECONFIG_H_

#include <time.h>
#include <windows.h>

#include "Synchronizer.h"
#include "common.h"

#define MAX_CONFIG 32

struct Rule {
    char m_symbol[12];
    char m_group[16];
    int m_login;
    int m_min_volume;
    int m_max_volume;
    // combination of "open", "close", "tp", "sl", "pending"; otherwise, "*"
    int m_order_type;
    int m_delay_milisecond;
    // one of "wp", "bp", "np", "fp", "op"
    PriceOption m_price_option;
};

class RuleContainer {
    friend class Processor;

public:
    bool AddRule(const char* rule_string);
    void Clear();
    bool Search(const char* symbol, const char* group, int client_login, int volume, int order_type, Rule* rule);
    static bool ParseRule(const char* line, Rule* rule);

private:
    Synchronizer m_sync;       // synchronizer
    Rule m_rules[MAX_CONFIG];  // configs
    int m_rule_total;          // total number of records

    RuleContainer();
    RuleContainer(RuleContainer const&) {}
    void operator=(RuleContainer const&) {}
    ~RuleContainer() {}
};

#endif  //--- _FILECONFIG_H_