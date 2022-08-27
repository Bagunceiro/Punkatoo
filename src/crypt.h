#include <Arduino.h>

/**
 * @file crypt.h
 * 
 * @todo Return types to const char*
 */

/** @brief encrypt a string and return the result in base64 */
const String pencrypt64(const char *text, const char *key = NULL);
/** @brief decrypt a base 64 encoded string created by pencrypt64() */
const String pdecrypt64(const char *text, const char *key = NULL);
