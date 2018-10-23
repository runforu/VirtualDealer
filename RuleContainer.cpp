#include <stdio.h>
#include <time.h>
#include <windows.h>
#include "FileUtil.h"
#include "Loger.h"
#include "RuleContainer.h"
#include "common.h"

RuleContainer::RuleContainer() : m_rule_total(0) {
    ZeroMemory(m_rules, sizeof(m_rules));
}

bool RuleContainer::ParseRule(const char* line, Rule* rule) {
    if (line == NULL || strlen(line) == 0 && rule == NULL) {
        return false;
    }

    char buf[256], *cp;

    ZeroMemory(rule, sizeof(Rule));

    COPY_STR(buf, line);

    RemoveWhiteChar(buf);

    //--- copy symbol
    char* pchar = StrRange(buf, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0) {
        COPY_STR(rule->m_group, pchar);
    } else {
        return false;
    }
    LOG("rule ---  group = %s", pchar);

    //--- copy group
    pchar = StrRange(NULL, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0) {
        COPY_STR(rule->m_symbol, pchar);
    } else {
        return false;
    }
    LOG("rule ---  symbol = %s", pchar);

    //--- copy login
    pchar = StrRange(NULL, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0 && (IsDigitalStr(pchar) || *pchar == '*')) {
        if (*pchar == '*') {
            rule->m_login = -1;
        } else {
            rule->m_login = atoi(pchar);
        }
    } else {
        return false;
    }
    LOG("rule ---  login = %s", pchar);

    //--- copy min_volume
    pchar = StrRange(NULL, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0 && IsDigitalStr(pchar)) {
        rule->m_min_volume = atoi(pchar);
    } else {
        rule->m_min_volume = -1;
    }
    LOG("rule ---  min_volume = %s", pchar);

    //--- copy max_volume
    pchar = StrRange(NULL, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0 && IsDigitalStr(pchar)) {
        rule->m_max_volume = atoi(pchar);
    } else {
        rule->m_max_volume = -1;
    }
    LOG("rule ---  max_volume = %s", pchar);

    //--- copy order_type
    pchar = StrRange(NULL, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0) {
        rule->m_order_type = ToOrderType(pchar, OT_ALL);
    } else {
        return false;
    }
    LOG("rule ---  order_type = %s, [%s]", pchar, OrderTypeStr(rule->m_order_type));

    //--- copy delay_milisecond
    pchar = StrRange(NULL, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0) {
        rule->m_delay_milisecond = atoi(pchar);
    } else {
        return false;
    }
    LOG("rule ---  delay_milisecond = %s", pchar);

    //--- copy price_option
    pchar = StrRange(NULL, '<', '>', &cp);
    if (pchar != NULL && strlen(pchar) != 0) {
        rule->m_price_option = ToPriceOption(pchar);
    } else {
        return false;
    }
    LOG("rule ---  price_option = %s [%s]", pchar, PriceOptionStr(rule->m_price_option));

    return true;
}

bool RuleContainer::AddRule(const char* rule_string, const char* name) {
    Rule rule;
    if (ParseRule(rule_string, &rule)) {
        if (name != NULL) {
            COPY_STR(rule.m_name, name);
        }
        LOG_INFO(&rule);
        m_sync.Lock();
        m_rules[m_rule_total++] = rule;
        m_sync.Unlock();
        return true;
    }
    return false;
}

void RuleContainer::Clear() {
    m_sync.Lock();
    m_rule_total = 0;
    // ZeroMemory(m_rules, sizeof(m_rules));
    m_sync.Unlock();
}

bool RuleContainer::Search(const char* symbol, const char* group, int client_login, int volume, int order_type, Rule* rule) {
    m_sync.Lock();
    LOG("Search symbole = %s, group = %s, login = %d, volume = %d, order type = %s", symbol, group, client_login, volume,
        OrderTypeStr(order_type));
    for (int i = 0; i < m_rule_total; i++) {
        // LOG("--> rule name = %s", this->m_rules[i].m_name);
        if (strcmp(this->m_rules[i].m_symbol, "*") != 0 && strcmp(symbol, this->m_rules[i].m_symbol) != 0) {
            continue;
        }
        if (strcmp("*", this->m_rules[i].m_group) != 0 && strcmp(group, this->m_rules[i].m_group) != 0) {
            continue;
        }
        if (this->m_rules[i].m_login != -1 && this->m_rules[i].m_login != client_login) {
            continue;
        }
        if (this->m_rules[i].m_min_volume != -1) {
            if (volume < this->m_rules[i].m_min_volume) {
                continue;
            }
        }
        if (this->m_rules[i].m_max_volume != -1) {
            if (volume > this->m_rules[i].m_max_volume) {
                continue;
            }
        }
        if ((order_type & this->m_rules[i].m_order_type) == 0) {
            continue;
        }

        *rule = this->m_rules[i];
        m_sync.Unlock();
        return true;
    }
    m_sync.Unlock();
    return false;
}