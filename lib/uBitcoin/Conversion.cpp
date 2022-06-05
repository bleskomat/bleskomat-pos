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

#include "Conversion.h"

static const char BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/******************* Base 64 conversion *******************/

size_t toBase64Length(const uint8_t * array, size_t arraySize, uint8_t flags){
    if(array == NULL){ return 0; }
    size_t v = (arraySize / 3) * 4;
    if(arraySize % 3 != 0){
        if(flags & BASE64_NOPADDING){
            v += (arraySize % 3) + 1;
        }else{
            v += 4;
        }
    }
    return v;
}
size_t toBase64(const uint8_t * array, size_t arraySize, char * output, size_t outputSize, uint8_t flags){
    if(array == NULL || output == NULL){ return 0; }
    memset(output, 0, outputSize);
    size_t cur = 0;
    if(outputSize < toBase64Length(array, arraySize, flags)){
        return 0;
    }
    while(3 * cur + 3 < arraySize){
        uint32_t val = bigEndianToInt(array+3*cur, 3);
        for(uint8_t i=0; i<4; i++){
            output[4*cur + i] = BASE64_CHARS[((val >> (6*(3-i))) & 0x3F)];
        }
        cur++;
    }
    size_t len = cur * 4;
    if(arraySize % 3 != 0){
        uint8_t rem = arraySize % 3;
        uint32_t val = bigEndianToInt(array+3*cur, rem);
        val = val << ((3-rem) * 8);
        for(uint8_t i=0; i<(rem+1); i++){
            output[4*cur + i] = BASE64_CHARS[((val >> (6*(3-i))) & 0x3F)];
        }
        if(flags & BASE64_NOPADDING){
            len += (rem+1);
        }else{
            memset(output + 4 * cur + 1 + rem, '=', 3-rem);
            len += 4;
        }
    }else{
        uint32_t val = bigEndianToInt(array+3*cur, 3);
        for(uint8_t i=0; i<4; i++){
            output[4*cur + i] = BASE64_CHARS[((val >> (6*(3-i))) & 0x3F)];
        }
        len += 4;
    }
    // replace with urlsafe characters
    if(flags & BASE64_URLSAFE){
        for(size_t i=0; i<len; i++){
            if(output[i] == '+'){
                output[i] = '-';
            }
            if(output[i] == '/'){
                output[i] = '_';
            }
        }
    }
    return len;
}
std::string toBase64(const uint8_t * array, size_t arraySize, uint8_t flags){
    if(array == NULL){ return std::string(); }
    size_t len = toBase64Length(array, arraySize, flags) + 1; // +1 for null terminator
    char * buf = (char *)malloc(len);
    if(buf==NULL){ return std::string(); }
    toBase64(array, arraySize, buf, len, flags);
    std::string result(buf);
    free(buf);
    return result;
}
size_t fromBase64Length(const char * array, size_t arraySize, uint8_t flags){
    if(array == NULL){ return 0; }
    if(arraySize % 4 != 0 && (flags & BASE64_NOPADDING) == 0){ return 0; }
    size_t v = (arraySize / 4) * 3;
    if(flags & BASE64_NOPADDING){
        if(arraySize % 4 != 0){
            v += (arraySize % 4)-1;
        }
    }else{
        if(array[arraySize-1] == '='){
            v--;
        }
        if(array[arraySize-2] == '='){
            v--;
        }
    }
    return v;
}
size_t fromBase64(const char * encoded, size_t encodedSize, uint8_t * output, size_t outputSize, uint8_t flags){
    if(encoded == NULL || output == NULL){ return 0; }
    size_t cur = 0;
    memset(output, 0, outputSize);
    if(outputSize < fromBase64Length(encoded, encodedSize, flags)){
        return 0;
    }
    while(cur*4 < encodedSize){
        if(cur*4+3 >= encodedSize && (flags & BASE64_NOPADDING) == 0){
            memset(output, 0, outputSize);
            return 0;
        }
        uint32_t val = 0;
        for(size_t i=0; i<4; i++){
            char c = encoded[cur*4+i];
            // replace characters for urlsafe version
            if(c=='-' && (flags & BASE64_URLSAFE)){
                c = '+';
            }
            if(c=='_' && (flags & BASE64_URLSAFE)){
                c = '/';
            }
            const char * pch = strchr(BASE64_CHARS, c);
            if(pch==NULL || ((flags & BASE64_NOPADDING) && (c == 0))){
                if((encoded[cur*4+i] == '=') || (c == 0 && (flags & BASE64_NOPADDING))){
                    if(i==3){
                        val = (val >> 2);
                        if(outputSize < 3*cur+2){
                            memset(output, 0, outputSize);
                            return 0;
                        }
                        intToBigEndian(val, output+3*cur, 2);
                        return 3*cur + 2;
                    }
                    if(i==2){
                        val = (val >> 4);
                        if(outputSize < 3*cur+1){
                            memset(output, 0, outputSize);
                            return 0;
                        }
                        output[3*cur] = (val & 0xFF);
                        return 3*cur + 1;
                    }
                }
                memset(output, 0, outputSize);
                return 0;
            }else{
                val = (val << 6) + ((pch - BASE64_CHARS) & 0x3F);
            }
        }
        if(outputSize < 3*(cur+1)){
            memset(output, 0, outputSize);
            return 0;
        }
        intToBigEndian(val, output+3*cur, 3);
        cur++;
    }
    return 3 * cur;
}
size_t fromBase64(std::string encoded, uint8_t * output, size_t outputSize, uint8_t flags){
    return fromBase64(encoded.c_str(), encoded.length(), output, outputSize, flags);
};

uint8_t hexToVal(char c){
    if(c >= '0' && c <= '9'){
        return ((uint8_t)(c - '0')) & 0x0F;
    }
    if(c >= 'A' && c <= 'F'){
        return ((uint8_t)(c-'A'+10)) & 0x0F;
    }
    if(c >= 'a' && c <= 'f'){
        return ((uint8_t)(c-'a'+10)) & 0x0F;
    }
    return 0xFF;
}
size_t fromHex(const char * hex, size_t hexLen, uint8_t * array, size_t arraySize){
    if(array == NULL || hex == NULL){ return 0; }
    memset(array, 0, arraySize);
    // ignoring all non-hex characters in the beginning
    size_t offset = 0;
    while(offset < hexLen){
        uint8_t v = hexToVal(hex[offset]);
        if(v > 0x0F){ // if invalid char
            offset++;
        }else{
            break;
        }
    }
    hexLen -= offset;
    for(size_t i=0; i<hexLen/2; i++){
        uint8_t v1 = hexToVal(hex[offset+2*i]);
        uint8_t v2 = hexToVal(hex[offset+2*i+1]);
        if((v1 > 0x0F) || (v2 > 0x0F)){ // if invalid char stop parsing
            return i;
        }
        array[i] = (v1<<4) | v2;
    }
    return hexLen/2;
}
size_t fromHex(std::string encoded, uint8_t * output, size_t outputSize){
    if(output == NULL){ return 0; }
    return fromHex(encoded.c_str(), encoded.length(), output, outputSize);
};

/* Integer conversion */

uint64_t littleEndianToInt(const uint8_t * array, size_t arraySize){
    uint64_t num = 0;
    for(size_t i = 0; i < arraySize; i++){
        num <<= 8;
        num += (array[arraySize-i-1] & 0xFF);
    }
    return num;
}

void intToLittleEndian(uint64_t num, uint8_t * array, size_t arraySize){
    for(size_t i = 0; i < arraySize; i++){
        array[i] = ((num >> (8*i)) & 0xFF);
    }
}

uint64_t bigEndianToInt(const uint8_t * array, size_t arraySize){
    uint64_t num = 0;
    for(size_t i = 0; i < arraySize; i++){
        num <<= 8;
        num += (array[i] & 0xFF);
    }
    return num;
}

void intToBigEndian(uint64_t num, uint8_t * array, size_t arraySize){
    for(size_t i = 0; i < arraySize; i++){
        array[arraySize-i-1] = ((num >> (8*i)) & 0xFF);
    }
}

/* Varint */

uint8_t lenVarInt(uint64_t num){
    if(num < 0xfd){
        return 1;
    }
    if((num >> 16) == 0){
        return 3;
    }
    if((num >> 32) == 0){
        return 5;
    }
    return 9;
}
uint64_t readVarInt(const uint8_t * array, size_t arraySize){
    if(array[0] < 0xfd){
        return array[0];
    }else{
        size_t len = (1 << (array[0] - 0xfc));
        if(len+1 > arraySize){
            return 0;
        }
        return littleEndianToInt(array + 1, len);
    }
}

// TODO: don't repeat yourself!
size_t writeVarInt(uint64_t num, uint8_t * array, size_t arraySize){
    uint8_t len = lenVarInt(num);
    if(arraySize < len){
        return 0;
    }
    if(len == 1){
        array[0] = (uint8_t)(num & 0xFF);
    }else{
        switch(len){
            case 3: array[0] = 0xfd;
                    break;
            case 5: array[0] = 0xfe;
                    break;
            case 9: array[0] = 0xff;
                    break;
        }
        intToLittleEndian(num, array+1, len-1);
    }
    return len;
}
