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

#include "BaseClasses.h"

size_t SerializeStream::serialize(const Streamable * s, size_t offset){
    return s->to_stream(this, offset);
}

size_t ParseStream::parse(Streamable * s){
    return s->from_stream(this);
}
/************ Parse Byte Stream Class ************/

ParseByteStream::ParseByteStream(const uint8_t * arr, size_t length, encoding_format f){
    last = -1;
    format = f;
    cursor = 0;
    len = (arr == NULL) ? 0 : length;
    buf = arr;
}
// TODO: call prev constructor
ParseByteStream::ParseByteStream(const char * arr, encoding_format f){
    last = -1;
    format = f;
    cursor = 0;
    len = (arr == NULL) ? 0 : strlen(arr);
    buf = (const uint8_t *) arr;
}
ParseByteStream::~ParseByteStream(){
    buf = NULL;
}
size_t ParseByteStream::available(){
    if(format == HEX_ENCODING){
        return (len - cursor)/2;
    }else{
        return len-cursor;
    }
};
int ParseByteStream::read(){
    if(format == HEX_ENCODING){
        if(cursor < len-1){
            uint8_t c1 = hexToVal(buf[cursor]);
            uint8_t c2 = hexToVal(buf[cursor+1]);
            if(c1 < 0x10 && c2 < 0x10){
                cursor +=2;
                last = (c1<<4) + c2;
                return last;
            }
        }
    }else{
        if(cursor < len){
            uint8_t c = buf[cursor];
            cursor ++;
            last = c;
            return c;
        }
    }
    return -1;
}
int ParseByteStream::getLast(){
    return last;
}
size_t ParseByteStream::read(uint8_t *arr, size_t length){
    size_t cc = 0;
    while(cc<length){
        int b = read();
        if(b<0){
            return cc;
        }
        arr[cc] = (uint8_t)b & 0xFF;
        cc++;
    }
    return cc;
}

/************ Serialize Byte Stream Class ************/

SerializeByteStream::SerializeByteStream(uint8_t * arr, size_t length, encoding_format f){
    format = f; cursor = 0; buf = arr; len = length;
    memset(arr, 0, length);
}
// TODO: should length be here? See above - we used strlen
SerializeByteStream::SerializeByteStream(char * arr, size_t length, encoding_format f){
    format = f; cursor = 0; buf = (uint8_t *)arr; len = length;
    memset(arr, 0, length);
};
size_t SerializeByteStream::available(){
    size_t a = len-cursor;
    if(format == HEX_ENCODING){
        a = a/2;
    }
    return a;
};
size_t SerializeByteStream::write(uint8_t b){
    if(available() > 0){
        if(format == HEX_ENCODING){
            buf[cursor] = ((b >> 4) & 0x0F) + '0';
            if(buf[cursor] > '9'){
                    buf[cursor] += 'a'-'9'-1;
            }
            cursor++;
            buf[cursor] = (b & 0x0F) + '0';
            if(buf[cursor] > '9'){
                    buf[cursor] += 'a'-'9'-1;
            }
            cursor++;
        }else{
            buf[cursor] = b;
            cursor++;
        }
        return 1;
    }
    return 0;
};
size_t SerializeByteStream::write(const uint8_t *arr, size_t length){
    size_t l = 0;
    while(available()>0 && l < length){
        write(arr[l]);
        l++;
    }
    return l;
};

/************ Readable Class ************/

std::string Readable::toString() const{
    size_t len = this->stringLength()+1;
    char * arr = (char *)calloc(len, sizeof(char));
    toString(arr, len);
    std::string s = arr;
    free(arr);
    return s;
};

/************ Streamable Class ************/

std::string Streamable::serialize(size_t offset, size_t len) const{
    if(len == 0){
        len = (length()-offset);
    }
    char * arr = (char *)calloc(2*len+1, sizeof(char));
    serialize(arr, 2*len, offset, HEX_ENCODING);
    std::string s = arr;
    free(arr);
    return s;
};

size_t Streamable::serialize(uint8_t * arr, size_t len, size_t offset, encoding_format format) const{
    SerializeByteStream s(arr, len, format);
    return to_stream(&s, offset);
}
