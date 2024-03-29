#include <Arduino.h>
#include <mbedtls/aes.h>

#include "crypt.h"

const String pencrypt(const char *plainText, const char *key)
{
    unsigned char buffer[16];

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char *)key, strlen(key) * 8);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char *)plainText, buffer);
    mbedtls_aes_free(&aes);
    String s;

    for (int i = 0; i < 16; i++)
    {
        s += (char)buffer[i];
    }
    return s;
}

const String pdecrypt(const unsigned char *cipherText, const char *key)
{
    mbedtls_aes_context aes;
    unsigned char buffer[16];

    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, (const unsigned char *)key, strlen(key) * 8);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char *)cipherText, buffer);
    mbedtls_aes_free(&aes);
    String s;
    for (int i = 0; i < 16; i++)
    {
        s += (char)buffer[i];
    }
    return s;
}

const char *privateKey()
{
    static char k[20];
    unsigned long long mac = ESP.getEfuseMac();

    sprintf(k, "%016llx", mac);
    return k;
}



/* Base 64 characters */
static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Magic */
static const int B64index[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
                                  56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6,
                                  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,
                                  0, 0, 0, 63, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                                  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};


String b64Encode(const String s)
{
    char *pos;
    const unsigned char *end, *in;
    const unsigned char *src = (unsigned char *)s.c_str();
    int len = s.length();

    size_t olen;

    olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

    if (olen < len)
        return String(); /* integer overflow */

    char out[olen + 1];

    end = src + len;
    in = src;
    pos = out;
    while (end - in >= 3)
    {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];

        in += 3;
    }

    if (end - in)
    {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                                  (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    *pos = '\00';
    return String(out);
}


String b64Decode(const char* data)
{
    size_t len = strlen(data);
    int pad = len > 0 && (len % 4 || data[len - 1] == '=');
    const size_t L = ((len + 3) / 4 - pad) * 4;
    int blen = L / 4 * 3 + pad;
    char str[blen];

    for (size_t i = 0, j = 0; i < L; i += 4)
    {
        int n = B64index[data[i]] << 18 | B64index[data[i + 1]] << 12 | B64index[data[i + 2]] << 6 | B64index[data[i + 3]];
        str[j++] = (char)(n >> 16);
        str[j++] = (char)(n >> 8 & 0xFF);
        str[j++] = (char)(n & 0xFF);
    }
    String s;
    for (int i = 0; i < blen; i++)
    {
            s += (char)str[i];
    }
    if (pad)
    {
        int n = B64index[data[L]] << 18 | B64index[data[L + 1]] << 12;
        s[s.length() - 1] = n >> 16;

        if (len > L + 2 && data[L + 2] != '=')
        {
            n |= B64index[data[L + 2]] << 6;
            s += (char)(n >> 8 & 0xFF);
        }
    }
    return s;
}

const String pencrypt64(const char *text, const char *key)
{
    const char* k = (key == NULL ? privateKey() : key);
    String encrypted = pencrypt(text, k);
    String r = b64Encode(encrypted);
    return r;
}

const String pdecrypt64(const char *text, const char *key)
{
    const char* k = (key == NULL ? privateKey() : key);
    String encrypted = b64Decode(text);
    return pdecrypt((unsigned char *)encrypted.c_str(), k);
}