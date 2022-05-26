#include <Arduino.h>

const String pencrypt(const char *plainText, const char *key);
const String pdecrypt(const unsigned char *cipherText, const char *key);

const String pencrypt64(const char *text, const char *key = NULL);
const String pdecrypt64(const char *text, const char *key = NULL);