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

#ifndef UBITCOIN_BASECLASSES_H
#define UBITCOIN_BASECLASSES_H

#include "Conversion.h"

#include <cstring>
#include <stdint.h>
#include <string>

enum encoding_format{
    RAW = 0,
    HEX_ENCODING = 16
    // TODO: would be nice to have base64 encoding here
};

enum parse_status{
    PARSING_DONE = 0,
    PARSING_INCOMPLETE = 1,
    PARSING_FAILED = 2
};
// error codes struct / class?

class Streamable;

class ParseStream{
public:
    virtual size_t available(){ return 0; };
    virtual int read(){ return -1; };
    virtual size_t read(uint8_t *arr, size_t length){ return 0; };
    virtual int getLast(){ return -1; };
    size_t parse(Streamable * s);
};

// TODO: skip leading non-hex if it's hex format
class ParseByteStream: public ParseStream{
private:
    const uint8_t * buf;
    size_t cursor;
    size_t len;
    encoding_format format;
    int last;
public:
    ParseByteStream(const uint8_t * arr, size_t length, encoding_format f=RAW);
    ParseByteStream(const char * arr, encoding_format f=HEX_ENCODING);
    ~ParseByteStream();
    size_t available();
    int read();
    size_t read(uint8_t *arr, size_t length);
    virtual int getLast();
};

class SerializeStream{
public:
    virtual size_t available(){ return 0; };
    virtual size_t write(uint8_t b){ return 0; };
    virtual size_t write(const uint8_t *arr, size_t len){ return 0; };
    size_t serialize(const Streamable * s, size_t offset);
};

class SerializeByteStream: public SerializeStream{
private:
    uint8_t * buf;
    size_t cursor;
    size_t len;
    encoding_format format;
public:
    SerializeByteStream(uint8_t * arr, size_t length, encoding_format f=RAW);
    explicit SerializeByteStream(char * arr, size_t length, encoding_format f=HEX_ENCODING);
    size_t available();
    size_t write(uint8_t b);
    size_t write(const uint8_t *arr, size_t length);
};

/** Abstract Readable class that can be encoded as a string and displayed to the user
 *  Can be converted to and from a string (char *, Arduino String and std::string)
 *  In Arduino it can be directly printed to the serial port, display or other Print device
 */
class Readable{
private:
protected:
    /* override these functions in your class */
    virtual size_t to_str(char * buf, size_t len) const = 0;
    virtual size_t from_str(const char * buf, size_t len) = 0;
public:
    /* override these function in your class */
    virtual size_t stringLength() const = 0;

    size_t toString(char * buf, size_t len) const{ return this->to_str(buf, len); };
    size_t fromString(const char * buf, size_t len){ return this->from_str(buf, len); };
    size_t fromString(const char * buf){ return this->from_str(buf, strlen(buf)); };
    std::string toString() const;
    operator std::string(){ return this->toString(); };
};

/** Abstract Streamable class that can be serialized to/from a sequence of bytes
 *  and sent to Stream (File, Serial, Bluetooth) without allocating extra memory
 *  Class can be parsed and serialized in raw and hex formats
 */
class Streamable: public Readable{
    friend class SerializeStream;
    friend class ParseStream;
private:
protected:
    virtual size_t from_stream(ParseStream *s) = 0;
    virtual size_t to_stream(SerializeStream *s, size_t offset) const = 0;
    virtual size_t to_str(char * buf, size_t len) const{
        return serialize(buf, len);
    }
    virtual size_t from_str(const char * buf, size_t len){
        return parse(buf, len);
    }
    parse_status status;
    size_t bytes_parsed;
public:
    Streamable() { status = PARSING_DONE; bytes_parsed = 0; };
    virtual void reset(){ status = PARSING_DONE; bytes_parsed = 0; }; // used to reset parsing and mb object
    virtual size_t length() const = 0;
    virtual size_t stringLength() const{ return 2*length(); };
    /** \brief Gets parsing status. 
     *         PARSING_DONE - everything is ok, 
     *         PARSING_INCOMPLETE - some data is still missing
     *         PARSING_FAILED - something went wrong, the data is probably incorrect.
     */
    parse_status getStatus(){ return status; };
    /** \brief Sets parsing status. Should be used with care. */
    void setStatus(parse_status s){ status = s; };
    size_t parse(const uint8_t * arr, size_t len, encoding_format format=RAW){
        ParseByteStream s(arr, len, format);
        return from_stream(&s);
    }
    size_t parse(std::string arr, encoding_format format=HEX_ENCODING){
        return parse(arr.c_str(), strlen(arr.c_str()), format);
    }
    size_t parse(const char * arr, encoding_format format=HEX_ENCODING){
        return parse(arr, strlen(arr), format);
    }
    size_t serialize(uint8_t * arr, size_t len, size_t offset = 0, encoding_format format=RAW) const;
    size_t parse(const char * arr, size_t len, encoding_format format=HEX_ENCODING){
        return parse((const uint8_t *) arr, len, format);
    }
    size_t serialize(char * arr, size_t len, size_t offset = 0, encoding_format format=HEX_ENCODING) const{
        return serialize((uint8_t *)arr, len, offset, format);
    }
    std::string serialize(size_t offset=0, size_t len=0) const;
    std::string serialize(int offset, int len) const{
        return serialize((size_t)offset, (size_t)len);
    };
    std::string serialize(size_t offset, int len) const{
        return serialize((size_t)offset, (size_t)len);
    };
};

#endif // __BITCOIN_BASE_CLASSES_H__
