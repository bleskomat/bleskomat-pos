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

#ifndef UBITCOIN_CONVERSION_H
#define UBITCOIN_CONVERSION_H

#include <string>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define BASE64_STANDARD  0
#define BASE64_NOPADDING 1
#define BASE64_URLSAFE   2

size_t toBase64Length(const uint8_t * array, size_t arraySize, uint8_t flags = BASE64_STANDARD);
size_t toBase64(const uint8_t * array, size_t arraySize, char * output, size_t outputSize, uint8_t flags = BASE64_STANDARD);
std::string toBase64(const uint8_t * array, size_t arraySize, uint8_t flags = BASE64_STANDARD);
size_t fromBase64Length(const char * array, size_t arraySize, uint8_t flags);
size_t fromBase64(const char * encoded, size_t encodedSize, uint8_t * output, size_t outputSize, uint8_t flags);
size_t fromBase64(std::string encoded, uint8_t * output, size_t outputSize, uint8_t flags);

uint8_t hexToVal(char c);
size_t fromHex(const char * hex, size_t hexLen, uint8_t * array, size_t arraySize);
size_t fromHex(std::string encoded, uint8_t * output, size_t outputSize);

/* int conversion */
uint64_t littleEndianToInt(const uint8_t * array, size_t arraySize);
void intToLittleEndian(uint64_t num, uint8_t * array, size_t arraySize);
uint64_t bigEndianToInt(const uint8_t * array, size_t arraySize);
void intToBigEndian(uint64_t num, uint8_t * array, size_t arraySize);

/* varint */
uint8_t lenVarInt(uint64_t num); // returns length of the array required for varint encoding
uint64_t readVarInt(const uint8_t * array, size_t arraySize);
size_t writeVarInt(uint64_t num, uint8_t * array, size_t arraySize);

#endif
