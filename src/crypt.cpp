#include <Arduino.h>
#include <mbedtls/aes.h>

#include "crypt.h"
#include "b64.h"

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

/*
void pdecrypt(unsigned char * chipherText, char * key, unsigned char * outputBuffer){

  mbedtls_aes_context aes;

  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)chipherText, outputBuffer);
  mbedtls_aes_free( &aes );
}
*/
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