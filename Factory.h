#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <windows.h>
#include "Synchronizer.h"
#include "config/FileConfig.h"

class Factory {
private:
    FileConfig m_file_config;
    Config m_config;
    static Factory m_instance;

public:
    static FileConfig* GetFileConfig() { return &m_instance.m_file_config; }
    static FileConfig* GetConfig() { return &m_instance.m_config; }

private:
    Factory() {}
    Factory(Factory const&) {}
    void operator=(Factory const&) {}
    ~Factory() {}
};

#endif  //--- _FACTORY_H_