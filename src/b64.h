#pragma once

#include <Arduino.h>

/**
 * @brief encode a string to base64
 * 
 * @param s The String to encode
 * @return The encoded String
 */
String b64Encode(String src);
/**
 * @brief decode a string previously encoded in base64
 * 
 * @param data The string to decode
 * @return The decoded String
 */
String b64Decode(const char* data);