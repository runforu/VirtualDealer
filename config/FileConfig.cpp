#include <stdio.h>
#include "FileConfig.h"
#include "FileUtil.h"

FileConfig::FileConfig() : m_config_total(0) {
    m_filename[0] = 0;
    ZeroMemory(m_config, sizeof(m_config));
}

void FileConfig::SetCfgFile(LPCSTR filename) {
    if (filename == NULL) {
        return;
    }

    FileUtil in;
    m_sync.Lock();
    if (!in.Open(filename, GENERIC_READ, OPEN_EXISTING)) {
        m_sync.Unlock();
        return;
    }

    COPY_STR(m_filename, filename);
}

void FileConfig::Load() {
    char tmp[256], *cp, *start;

    if (m_filename[0] == 0) {
        return;
    }

    ZeroMemory(m_config, sizeof(m_config));

    FileUtil in;
    m_sync.Lock();
    if (!in.Open(m_filename, GENERIC_READ, OPEN_EXISTING)) {
        m_sync.Unlock();
        return;
    }

    while (in.GetNextLine(tmp, sizeof(tmp) - 1) > 0) {
        _strlwr_s(tmp);
        start = tmp;

        //--- omit white space
        while (*start == ' ') {
            start++;
        }

        if (*start == '#') {
            continue;
        }

        if ((cp = strchr(start, ';')) == NULL) {
            continue;
        }
        *cp = 0;

        //--- execution type
        // COPY_STR(m_config[m_config_total].m_execution, start);
        ExecutionType et;
        if (ToExecutionType(start, &et, ET_ALL)) {
            m_config[m_config_total].m_execution_type = et;
        } else {
            continue;
        }

        //--- copy symbol
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        if (strlen(start) != 0) {
            COPY_STR(m_config[m_config_total].m_symbol, start);
        } else {
            continue;
        }

        //--- copy group
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        if (strlen(start) != 0) {
            COPY_STR(m_config[m_config_total].m_group, start);
        } else {
            continue;
        }

        //--- copy login
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        if (strlen(start) != 0) {
            COPY_STR(m_config[m_config_total].m_login, start);
        } else {
            continue;
        }

        //--- copy min_volume
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        if (strlen(start) != 0) {
            m_config[m_config_total].m_min_volume = CStrToInt(start);
        } else {
            m_config[m_config_total].m_min_volume = -1;
        }

        //--- copy max_volume
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        if (strlen(start) != 0) {
            m_config[m_config_total].m_max_volume = CStrToInt(start);
        } else {
            m_config[m_config_total].m_max_volume = -1;
        }

        //--- copy order_type
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        m_config[m_config_total].m_order_type = ToOrderType(start, OT_ALL);

        //--- copy delay_milisecond
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        m_config[m_config_total].m_delay_milisecond = CStrToInt(start);
        if (m_config[m_config_total].m_delay_milisecond == 0) {
            m_config[m_config_total].m_delay_milisecond = -1;
        }

        //--- copy price_option
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        m_config[m_config_total].m_price_option = ToPriceOption(start);

        m_config_total++;
        if (m_config_total >= MAX_CONFIG) {
            break;
        }
    }
    in.Close();
    m_sync.Unlock();
}

PriceOption FileConfig::ToPriceOption(char* price_option) {
    if (price_option == NULL || strlen(price_option) == 0) {
        return PO_WORST_PRICE;
    }
    if (strcmp("bp", price_option) == 0 && strlen(price_option) == 2) {
        return PO_BEST_PRICE;
    }
    if (strcmp("fp", price_option) == 0 && strlen(price_option) == 2) {
        return PO_FIRST_PRICE;
    }
    if (strcmp("np", price_option) == 0 && strlen(price_option) == 2) {
        return PO_NEXT_PRICE;
    }
    if (strcmp("op", price_option) == 0 && strlen(price_option) == 2) {
        return PO_ORDER_PRICE;
    }
    return PO_WORST_PRICE;
}

bool FileConfig::ToExecutionType(char* type, ExecutionType* execution_type, ExecutionType default_value) {
    if (type == NULL || strlen(type) == 0) {
        *execution_type = default_value;
        return true;
    }
    if (strcmp(type, "*") == 0) {
        *execution_type = ET_ALL;
        return true;
    }
    if (strcmp(type, "market") == 0) {
        *execution_type = ET_MARKET;
        return true;
    }
    if (strcmp(type, "asking") == 0) {
        *execution_type = ET_ASKING;
        return true;
    }
    return false;
}

int FileConfig::ToOrderType(char* type, int default_value) {
    if (type == NULL || strlen(type) == 0) {
        return default_value;
    }

    if (strcmp(type, "*") == 0) {
        return OT_ALL;
    }

    int mask = 0;
    char tmp[32], *start, *cp;
    strncpy_s(tmp, type, sizeof(tmp));
    start = tmp;

    do {
        cp = strstr(start, "|");
        if (cp != NULL) {
            *cp = 0;
            cp++;
        }
        if (strcmp(start, "open") == 0) {
            mask |= OT_OPEN;
        } else if (strcmp(start, "close") == 0) {
            mask |= OT_CLOSE;
        } else if (strcmp(start, "tp") == 0) {
            mask |= OT_TP;
        } else if (strcmp(start, "sl") == 0) {
            mask |= OT_SL;
        } else {
            return default_value;
        }
        start = cp;
    } while (start != NULL && strlen(start) != 0);

    return mask;
}

int FileConfig::CStrToInt(char* string) {
    if (string != NULL) {
        return atoi(string);
    }
    return 0;
}

bool FileConfig::Search(ExecutionType execution_type, const char* symbol, const char* group, int client_login, int volume,
                        int order_type, ExternalConfig* external_config) {
    for (int i = 0; i < m_config_total; i++) {
        if (strcmp(symbol, this->m_config[i].m_symbol) != 0) {
            continue;
        }
        if (strcmp(group, this->m_config[i].m_group) != 0) {
            continue;
        }
        if (strcmp(this->m_config[i].m_login, "*") != 0) {
            int client = atoi(this->m_config[i].m_login);
            if (client != client_login) {
                continue;
            }
        }
        if (this->m_config[i].m_min_volume != -1) {
            if (volume < this->m_config[i].m_min_volume) {
                continue;
            }
        }
        if (this->m_config[i].m_max_volume != -1) {
            if (volume > this->m_config[i].m_max_volume) {
                continue;
            }
        }
        if (order_type != OT_NONE && (order_type & this->m_config[i].m_order_type) == 0) {
            continue;
        }
        if (execution_type != ET_NONE && this->m_config[i].m_execution_type != ET_ALL) {
            if (this->m_config[i].m_execution_type != execution_type) {
                continue;
            }
        }
        // find the fist file config
        *external_config = this->m_config[i];
        return true;
    }
    return false;
}