#pragma once

#include <map>
#include <Stream.h>

/**
 * @class ConfBlk
 * @brief Map of configuration keys to values and its representation as a JSON file
 */
class ConfBlk: public std::map<String, String>
{
public:
    ConfBlk(const char* fileName);

    void dump(Stream& s) const;
    bool writeFile() const;
    bool readFile();
    void setFileName(const char* n) { _fileName = n; }
    const char* const getFileName() const { return _fileName.c_str(); }
    private:
    String _fileName;
};