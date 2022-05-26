#pragma once

#include <map>
#include <Stream.h>

class ConfBlk: public std::map<String, String>
{
public:
    ConfBlk(const char* fileName = "/etc/config.json");

    void dump(Stream& s) const;
    bool writeStream(Stream& s) const;
    bool writeFile() const;
    bool readStream(Stream& s);
    bool readFile();
    void setFileName(const char* n) { _fileName = n; }
    const char* const getFileName() const { return _fileName.c_str(); }
    private:
    String _fileName;
};