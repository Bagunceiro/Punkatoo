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
/**
 * @param fileName name of the JSON file
 */
    ConfBlk(const char* fileName);


/**
 * @brief Write the contents to a Stream for debug purposes
 * @param s The receiving stream
 */
    void dump(Stream& s) const;
    /**
     * @brief Write the configuration to a JSON file
     */
    bool writeFile() const;
        /**
     * @brief Read the configuration from a JSON file
     */
    bool readFile();
    // void setFileName(const char* n) { _fileName = n; }
    // const char* const getFileName() const { return _fileName.c_str(); }
    private:
    /** @brief The name of the JSON file */
    String _fileName;
};