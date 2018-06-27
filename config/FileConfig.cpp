#include <stdio.h>
#include "FileConfig.h"
#include "FileUtil.h"

FileConfig::FileConfig() : m_config_total(0) {
    m_filename[0] = 0;
    ZeroMemory(m_config, sizeof(m_config));
}

void FileConfig::Load(LPCSTR filename) {
    char tmp[256], *cp, *start;
    FileUtil in;

    if (filename == NULL) {
        return;
    }

    ZeroMemory(m_config, sizeof(m_config));

    //--- copy file name
    m_sync.Lock();
    COPY_STR(m_filename, filename);
    //--- open file
    if (!in.Open(m_filename, GENERIC_READ, OPEN_EXISTING)) {
        m_sync.Unlock();
        return;
    }

    while (in.GetNextLine(tmp, sizeof(tmp) - 1) > 0) {
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

        //--- copy execution
        COPY_STR(m_config[m_config_total].m_execution, start);

        //--- copy symbol
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_symbol, start);

        //--- copy group
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_group, start);

        //--- copy login
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_login, start);

        //--- copy min_volume
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_min_volume, start);

        //--- copy max_volume
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_max_volume, start);

        //--- copy order_type
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_order_type, start);

        //--- copy delay_milisecond
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_delay_milisecond, start);

        //--- copy price_option
        cp++;
        start = cp;
        if ((cp = strchr(cp, ';')) == NULL) {
            continue;
        }
        *cp = 0;
        COPY_STR(m_config[m_config_total].m_price_option, start);

        m_config_total++;
        if (m_config_total >= MAX_CONFIG) {
            break;
        }
    }
    in.Close();
    m_sync.Unlock();
}
