#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <windows.h>
#include "Processor.h"
#include "Synchronizer.h"
#include "config/Config.h"
#include "config/FileConfig.h"

class Factory {
private:
    FileConfig m_file_config;
    Config m_config;
    Processor m_processor;
    CServerInterface* m_server = NULL;

private:
    static Factory m_instance;

public:
    inline static FileConfig* GetFileConfig() { return &m_instance.m_file_config; }
    inline static Config* GetConfig() { return &m_instance.m_config; }
    inline static Processor* GetProcessor() { return &m_instance.m_processor; }
    inline static void SetServerInterface(CServerInterface* server) { m_instance.m_server = server; }
    inline static CServerInterface* GetServerInterface() { return m_instance.m_server; }

private:
    Factory() : m_server(NULL) {}
    Factory(Factory const&) {}
    void operator=(Factory const&) {}
    ~Factory() {}
};

#endif  //--- _FACTORY_H_