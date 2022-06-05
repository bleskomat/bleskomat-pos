/**
 * MIT License
 *
 * Copyright (c) 2019 Stepan Snigirev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef UBITCOIN_HASH_H
#define UBITCOIN_HASH_H

#include "BaseClasses.h"
#include <stdint.h>
#include <string>
#include "utility/trezor/sha2.h"
#include "utility/trezor/hmac.h"

/** \brief Abstract hashing class */
class HashAlgorithm : public SerializeStream {
public:
    size_t available(){ return 100; };
	void begin(){};
    virtual size_t write(const uint8_t * data, size_t len) = 0;
    virtual size_t write(uint8_t b) = 0;
    virtual size_t end(uint8_t * hash) = 0;
};


/************************** SHA-256 **************************/

/** \brief sha256 one-line hashing function → 32 bytes output */
int sha256(const uint8_t * data, size_t len, uint8_t hash[32]);
int sha256(const char * data, size_t len, uint8_t hash[32]);
int sha256(const std::string data, uint8_t hash[32]);
int sha256Hmac(const uint8_t * key, size_t keyLen, const uint8_t * data, size_t dataLen, uint8_t hash[32]);

class SHA256 : public HashAlgorithm{
public:
    SHA256(){ begin(); };
    void begin();
    void beginHMAC(const uint8_t * key, size_t keySize);
    size_t write(const uint8_t * data, size_t len);
    size_t write(uint8_t b);
    size_t end(uint8_t hash[32]);
    size_t endHMAC(uint8_t hmac[32]);
protected:
    HMAC_SHA256_CTX ctx;
};

#endif // __HASH_H__18NLNNCSJ2
