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

#include "Hash.h"

// generic funtcions for single line hash
static size_t hashData(HashAlgorithm * algo, const uint8_t * data, size_t len, uint8_t * hash){
    algo->begin();
    algo->write(data, len);
    return algo->end(hash);
}

static size_t hashString(HashAlgorithm * algo, const std::string s, uint8_t * hash){
    return hashData(algo, (const uint8_t *)s.c_str(), s.length(), hash);
}

/************************** SHA-256 **************************/

int sha256(const uint8_t * data, size_t len, uint8_t hash[32]){
    SHA256 sha;
    return hashData(&sha, data, len, hash);
}
int sha256(const char * data, size_t len, uint8_t hash[32]){
    return sha256((uint8_t*)data, len, hash);
}
int sha256(const std::string data, uint8_t hash[32]){
    SHA256 sha;
    return hashString(&sha, data, hash);
}

int sha256Hmac(const uint8_t * key, size_t keyLen, const uint8_t * data, size_t dataLen, uint8_t hash[32]){
    ubtc_hmac_sha256(key, keyLen, data, dataLen, hash);
    return 32;
}

void SHA256::begin(){
    sha256_Init(&ctx.ctx);
};
void SHA256::beginHMAC(const uint8_t * key, size_t keySize){
    ubtc_hmac_sha256_Init(&ctx, key, keySize);
}
size_t SHA256::write(const uint8_t * data, size_t len){
    sha256_Update(&ctx.ctx, data, len);
    return len;
}
size_t SHA256::write(uint8_t b){
    uint8_t arr[1] = { b };
    sha256_Update(&ctx.ctx, arr, 1);
    return 1;
}
size_t SHA256::end(uint8_t hash[32]){
    sha256_Final(&ctx.ctx, hash);
    return 32;
}
size_t SHA256::endHMAC(uint8_t hmac[32]){
    ubtc_hmac_sha256_Final(&ctx, hmac);
    return 32;
}
