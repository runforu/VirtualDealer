//+------------------------------------------------------------------+
//|                                            MetaTrader Server API |
//|                   Copyright 2001-2014, MetaQuotes Software Corp. |
//|                                        http://www.metaquotes.net |
//+------------------------------------------------------------------+
#ifndef _STRINGFILE_H_
#define _STRINGFILE_H_
#include <windows.h>
//+------------------------------------------------------------------+
//|  String reading class                                            |
//+------------------------------------------------------------------+
class CStringFile {
private:
    HANDLE            m_file;                 // file handle
    DWORD             m_file_size;            // file size
    BYTE             *m_buffer;               // the buffer for reading
    int               m_buffer_size;          // its size
    int               m_buffer_index;         // current parsing position
    int               m_buffer_readed;        // the size of the buffer read in memory
    int               m_buffer_line;          // the counter of lines in a file

public:
    CStringFile(const int nBufSize = 65536);
    ~CStringFile();
    //---
    bool              Open(LPCTSTR lpFileName, const DWORD dwAccess, const DWORD dwCreationFlags);
    inline void       Close() {
        if (m_file != INVALID_HANDLE_VALUE) {
            CloseHandle(m_file); m_file = INVALID_HANDLE_VALUE;
        }
        m_file_size = 0;
    }
    inline DWORD      Size() const {
        return(m_file_size);
    }
    int               Read(void  *buffer, const DWORD length);
    int               Write(const void *buffer, const DWORD length);
    void              Reset();
    int               GetNextLine(char *line, const int maxsize);
};
//+------------------------------------------------------------------+
#endif // !_STRINGFILE_H_