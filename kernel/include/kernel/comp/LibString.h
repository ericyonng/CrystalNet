/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
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
 *  
 * 
 * Date: 2020-10-18 14:26:22
 * Author: Eric Yonng
 * Description: 默认使用Ascii码,打印日志时候应使用Utf8编码,提供ascii码转utf8接口
 *              有别于标准库,内存分配由用户指定分配器,带码点，数据包括一颗树和一个数组，支持utf8，支持append_format接口等，编码不同直接报错处理
 *              默认使用tls memmory pool
 *              std::string::data只有在std::string内容大小使得capacity发生变化的时候才会失效
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTRING_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIBSTRING_H__

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <cstring>

#include <kernel/kernel_export.h>
#include <kernel/common/macro.h>
#include <kernel/common/func.h>
#include <kernel/common/Buffer.h>

#include <kernel/comp/LibStringOut.h>
#include <stdarg.h>     // c风格格式化vs_arg等接口

KERNEL_BEGIN
class LibString;
template<typename T>
class LibStream;

KERNEL_END

// template<typename ObjType>
// extern KERNEL_EXPORT KERNEL_NS::LibString &operator <<(KERNEL_NS::LibString &dest, const ObjType &obj);

KERNEL_BEGIN

#undef CRYSTAL_FMTFLAGS
#define CRYSTAL_FMTFLAGS	"-+ #0"

// stringfmt params
#undef CRYSTAL_INTEGER_FRMLEN
#define CRYSTAL_INTEGER_FRMLEN	"ll"

static ALWAYS_INLINE Byte8 *InitHexToDecimalValues()
{
    auto arr = new Byte8[128];
    ::memset(arr, -1, 128);

    // 16进制转码
    arr[Int32('0')] = 0;
    arr[Int32('1')] = 1;
    arr[Int32('2')] = 2;
    arr[Int32('3')] = 3;
    arr[Int32('4')] = 4;
    arr[Int32('5')] = 5;
    arr[Int32('6')] = 6;
    arr[Int32('7')] = 7;
    arr[Int32('8')] = 8;
    arr[Int32('9')] = 9;
    arr[Int32('A')] = 10;
    arr[Int32('B')] = 11;
    arr[Int32('C')] = 12;
    arr[Int32('D')] = 13;
    arr[Int32('E')] = 14;
    arr[Int32('F')] = 15;
    arr[Int32('a')] = 10;
    arr[Int32('b')] = 11;
    arr[Int32('c')] = 12;
    arr[Int32('d')] = 13;
    arr[Int32('e')] = 14;
    arr[Int32('f')] = 15;
    return arr;
}

class KERNEL_EXPORT LibString
{
public:
    typedef std::vector<LibString> _These, LibStrings;
    typedef std::string::size_type size_type;
    typedef LibString _ThisType;
    typedef Byte8 _Elem;

    static const Byte8 *endl;
    static const std::string _defStripChars;

public:
    LibString()
    {

    }

    // 销毁自己
    void Release() const
    {
        delete this;
    }

    LibString(LibString &&other)
    {
        _raw.swap(other._raw);
    }
    
    LibString(const LibString &other)
    {
        _raw = other._raw;
    }

    LibString(const std::string &other)
    {
        if(LIKELY(!other.empty()))
            _raw.append(other.data(), other.size());
    }

    LibString(const Byte8 *other)
    {
        if(LIKELY(other))
            _raw.assign(other, strlen(other));
    }

    LibString(const Byte8 *other, UInt64 cacheSize)
    {
        if(LIKELY(other))
            _raw.append(other, cacheSize);
    }

    LibString(Byte8 other)
    {
        _raw = other;
    }

    // 运算符
    LibString &operator = (const Byte8 *other)
    {
        if(LIKELY(other))
            _raw.assign(other, strlen(other));

        return *this;
    }

    LibString &operator = (const std::string &other)
    {
        _raw = other;
        return *this;
    }
    LibString &operator = (const LibString &other)
    {
        _raw = other._raw;
        return *this;
    }
    LibString &operator = (LibString &&other)
    {
        _raw.swap(other._raw);
        return *this;
    }

    LibString operator + (const LibString &other) const
    {
        return std::string(_raw).append(other._raw);
    }

    LibString operator + (const Byte8 *other) const
    {
        if(UNLIKELY(!other))
            return _raw;

        return std::string(_raw).append(std::string(other));
    }

    LibString operator + (const std::string &other) const
    {
        return _raw + other;
    }

    LibString &operator += (const LibString &other)
    {
        _raw.append(other._raw);
        return *this;
    }

    LibString &operator += (const Byte8 *other)
    {
        if(LIKELY(other))
            _raw.append(other, ::strlen(other));
        
        return *this;
    }

    LibString &operator += (const std::string &other)
    {
        _raw += other;
        return *this;
    }

    LibString &operator *= (size_t right);

    LibString operator - (const LibString &other) const
    {
        auto pos = _raw.find(other._raw);
        if(pos == std::string::npos)
            return _raw;

        auto cache = _raw;
        cache.erase(pos, other._raw.size());
        return cache;
    }

    LibString operator - (const Byte8 *other) const
    {
        if(UNLIKELY(!other))
            return _raw;

        return LibString::operator -(LibString(other));
    }

    LibString operator - (const std::string &other) const
    {
        return LibString::operator -(LibString(other));
    }

    LibString &operator -= (const LibString &other)
    {
        auto pos = _raw.find(other._raw);
        if(pos == std::string::npos)
            return *this;

        _raw.erase(pos, other._raw.size());
        return *this;
    }

    LibString &operator -= (const Byte8 *other)
    {
        if(UNLIKELY(!other))
            return *this;

        return LibString::operator -=(LibString(other));
    }

    LibString &operator -= (const std::string &other)
    {
        return LibString::operator -=(LibString(other));
    }

    LibString &operator << (const LibString &str)
    {
        _raw += str._raw;
        return *this;
    }

    LibString &operator << (LibString &&str)
    {
        _raw.append(str._raw);
        if (UNLIKELY(this != &str))
            str.clear();
        
        return *this;
    }

    LibString &operator << (LibString &str)
    {
        _raw += str._raw;
        return *this;
    }

    LibString &operator << (const std::string &str)
    {
        _raw += str;
        return *this;
    }

    LibString &operator << (std::string &&str)
    {
        _raw += str;
        if(UNLIKELY(&_raw != &str))
            str.clear();    
            
        return *this;
    }
    LibString &operator << (const bool &val)
    {
        BUFFER4 cache;
        auto len = sprintf(cache, "%d", val);
        cache[len] = 0;
        _raw += cache;
        return *this;
    }

    LibString &operator << (const Byte8 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%d", val);
        cache[len] = 0;
        _raw += cache;
        return *this;
    }

    LibString &operator << (const U8 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%u", val);
        cache[len] = 0;
        _raw += cache;
        return *this;
    }

    LibString &operator << (const Int16 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%hd", val);
        cache[len] = 0;
        _raw += cache;
        return *this;
    }

    LibString &operator << (const UInt16 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%hu", val);
        cache[len] = 0;
        _raw += cache;
        return *this;
    }

    LibString &operator << (const Int32 &val)
    {
        return AppendFormat("%d", val);
    }

    LibString &operator << (const UInt32 &val)
    {
        return AppendFormat("%u", val);
    }

    LibString &operator << (const Long &val)
    {
        return AppendFormat("%ld", val);
    }

    LibString &operator << (const ULong &val)
    {
        return AppendFormat("%lu", val);
    }

    LibString &operator << (const Int64 &val)
    {
        return AppendFormat("%lld", val);
    }

    LibString &operator << (const UInt64 &val)
    {
        return AppendFormat("%llu", val);
    }

    LibString &operator << (const Float &val)
    {
        return AppendFormat("%f", val);
    }

    LibString &operator << (const Double &val)
    {
        return AppendFormat("%lf", val);
    }

    LibString &operator << (const Byte8 *val)
    {
        if(UNLIKELY(!val))
            return *this;

        _raw += val;
        return *this;
    }

    LibString &operator << (const void *addr)
    {
        return AppendFormat("%p", addr);
    }

    LibString &operator << (void *&&addr)
    {
        return AppendFormat("%p", addr);
    }

    template<typename T>
    LibString &operator <<(const T &value)
    {
        return StringOutAdapter<T>::output(*this, value);
    }

    Byte8 &operator [] (UInt64 index)
    {
        return _raw[index];
    }

    const Byte8 &operator [] (UInt64 index) const
    {
        return _raw[index];
    }

    Byte8 &at (UInt64 index)
    {
        return _raw.at(index);
    }

    const Byte8 &at (UInt64 index) const
    {
        return _raw.at(index);
    }

    bool operator == (const LibString &other) const
    {
        return _raw == other._raw;
    }

    bool operator == (const std::string &other) const
    {
        return _raw == other;
    }

    bool operator == (const Byte8 *other) const
    {
        if(UNLIKELY(!other))
            return false;

        return _raw == other;
    }

    bool operator != (const Byte8 *other) const
    {
        if(UNLIKELY(!other))
            return true;

        return _raw != other;
    }

    bool operator != (const LibString &other) const
    {
        return _raw != other._raw;
    }

    bool operator < (const LibString &right) const
    {
        return _raw < right._raw;
    }

    friend std::basic_ostream<_Elem> &operator <<(std::basic_ostream<_Elem> &o, const _ThisType &str)
    {
        o.write(str.data(), str.size());
        return o;
    }

    size_t CopyTo(char *destData, UInt64 destSize, UInt64 cntToCopy, UInt64 srcOffset = 0) const
    {
        if(UNLIKELY(!destData))
            return 0;

        const UInt64 maxLen = std::min<UInt64>(destSize, cntToCopy);
        ::memcpy(destData, _raw.data() + srcOffset, maxLen);

        return maxLen;
    }
    
    // '\0'结尾
    const Byte8 *c_str() const;
    // 没有'\0'结尾
    const Byte8 *data() const;
    Byte8 *data();
    bool empty() const;
    size_t size() const;
    size_t length() const;
    void clear();
    void resize(UInt64 bytes);
    std::string &GetRaw();
    const std::string &GetRaw() const;
    bool Contain(const LibString &piece) const;

    LibString ToHexString() const;    
    LibString ToString() const;    

    void ToHexString(LibString &target) const;

    void ToHexView(LibString &target) const;

    bool FromHexString(const LibString &hexString);

    void Swap(std::string &str)
    {
        if(UNLIKELY(&_raw == &str))
        {
            return;
        }

        _raw.swap(str);
    }

    void Swap(LibString &&str)
    {
        if (UNLIKELY(this == &str))
            return;
            
        _raw.swap(str._raw);
    }

    void Swap(LibString &str)
    {
        if (UNLIKELY(this == &str))
            return;
        _raw.swap(str._raw);
    }

    // 去除字符串末尾的\0压缩空间只对0结尾的字符串有效
    void CompressString();

     // 移除容器中尾部的0
    const LibString &RemoveZeroTail();  

    // 移除容器中首部的0
    const LibString &RemoveHeadZero();

    // 如果是字符串的话会带很多的\0, 如果bitData有的话，这会造成再次Append的时候之后追加的字符串打印不出来
    LibString &AppendData(const Byte8 *bitData, Int64 dataSize);

    LibString &AppendData(const Byte8 *str)
    {
        return AppendData(str, static_cast<Int64>(strlen(str)));
    }
    
    LibString &AppendData(const LibString &data);
    LibString &AppendFormat(const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3);    

    static UInt64 CheckFormatSize(const Byte8 *fmt, va_list va)
    {
        if (UNLIKELY(!fmt))
            return 0;

        // try detach detach format require buffers and resize it.
        Int32 len = ::vsnprintf(nullptr, 0, fmt, va);
        if (len <= 0)
            return 0;

        return static_cast<UInt64>(len);
    }

    LibString &AppendFormatWithVaList(UInt64 finalFormatStrLen, const Byte8 *fmt, va_list va)
    {
        // if fmt args is null, return.
        if (UNLIKELY(!fmt || (finalFormatStrLen == 0)))
            return *this;

        // try detach detach format require buffers and resize it.
        const Int64 oldSize = static_cast<Int64>(_raw.size());

        // exec format.
        _raw.resize(static_cast<UInt64>(oldSize) + finalFormatStrLen);
        Int32 len = static_cast<Int32>(finalFormatStrLen);
        len = ::vsnprintf(const_cast<Byte8 *>(_raw.data() + oldSize),
                            len + 1,
                            fmt,
                            va);

        // len < 0 then _raw.size() - len > oldSize back to old string
        if (UNLIKELY(oldSize != static_cast<Int64>((_raw.size() - len))))
        {
            CRYSTAL_TRACE("wrong apend format and back to old string, oldSize:%llu, len:%d, new size:%llu", static_cast<UInt64>(oldSize), len, static_cast<UInt64>(_raw.size()));
            throw std::logic_error("rong apend format and back to old string");
            _raw.resize(oldSize);
        }

        return *this;
    }

    LibString &AppendEnd();

    // 支持任意类型的追加只要对象重载<<即可
    template< typename... Args>
    LibString &Append(Args&&... rest);
    // 当参数为0时会匹配非泛型版本
    LibString &Append();

    _ThisType &findreplace(const _Elem &dest, const _Elem &with, Int32 count = -1);

    _ThisType &findreplace(const _ThisType &dest, const _ThisType &with, Int32 count = -1);

    _ThisType &findFirstAppendFormat(const _ThisType &dest, const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(3, 4);

    _ThisType &EraseAnyOf(const _ThisType &dest);

    // @param(sep):切割关键字符
    // @param(max_split):切割成多少份
    // @param(enableEmptyPart):允许空字符串
    _These Split(Byte8 sep, size_type max_split = -1, bool enableEmptyPart = true) const;    // only ascii max_split分割的次数，一次分割两块数据一共分割max_split+1块数据
    // @param(sep):切割关键字符串
    // @param(max_split):切割成多少份
    // @param(onlyLikely):true:模糊匹配（匹配字符串中的某个字符即可）, false:必须完全匹配字符串
    // @param(enableEmptyPart):允许空字符串
    _These Split(const Byte8 *sep, size_type max_split = -1, bool onlyLikely = false, bool enableEmptyPart = true) const;    // only ascii
    // @param(sep):切割关键字符串
    // @param(max_split):切割成多少份
    // @param(onlyLikely):true:模糊匹配（匹配字符串中的某个字符即可）, false:必须完全匹配字符串
    // @param(enableEmptyPart):允许空字符串
    _These Split(const LibString &sep, std::string::size_type max_split = -1, bool onlyLikely = false, bool enableEmptyPart = true) const;
   
    // @param(seps):切割关键字符串， 完全匹配这组字符串中的一个即可
    // @param(max_split):切割成多少份
    // @param(enableEmptyPart):允许空字符串
    _These Split(const _These &seps, size_type max_split = -1, bool enableEmptyPart = true) const;

    // strip operation: strip left. 去首部连续字符
    LibString &lstrip(const LibString &chars = LibString());
   
    LibString lstrip(const LibString &chars = LibString()) const;
    LibString &lstripString(const LibString &str);
   
    LibString lstripString(const LibString &str) const;

    // strip operation: strip right. 去尾部连续字符
    LibString &rstrip(const LibString &chars = LibString());
    LibString rstrip(const LibString &chars = LibString()) const;

    LibString &rstripString(const LibString &str);
    LibString rstripString(const LibString &str) const;

    // strip operation: 去除首尾字符
    LibString &strip(const LibString &chars = LibString());
    LibString strip(const LibString &chars = LibString()) const;
    LibString &stripString(const LibString &str);
    LibString stripString(const LibString &str) const;

    LibString DragAfter(const LibString &start) const;
    LibString DragAfter(const LibString &start, size_t &startPos, size_t &endPos) const;

    LibString DragBefore(const LibString &start) const;
    LibString DragRange(const LibString &startStr, const LibString &endStr) const;

    LibString lsub(const LibString &flagStr) const;
    LibString rsub(const LibString &flagStr) const;

    LibString sub(const LibString &leftStr, const LibString &rightStr) const;

    // isalpha/isupper/islower 是否字母
    static bool isalpha(const char &c);
    static bool isalpha(const LibString &s);
    bool isalpha() const;
    static bool islower(const char &c);
    static bool islower(const LibString &s);

    bool islower() const;
    static bool isupper(const char &c);
    static bool isupper(const LibString &s);

    bool isupper() const;
    // 是否数值
    static bool isdigit(const char &c);
    static bool isdigit(const LibString &s);

    bool isdigit() const;
    // isspace: space[' '], carriage return['\r'], line feed['\n'], form feed['\f'], horizontal tab['\t'], vertical tab['\v']
    static bool isspace(const char &c);
    static bool isspace(const LibString &s);
    bool isspace() const;

    // startswith/endswith
    bool IsStartsWith(const LibString &s) const;

    bool IsEndsWith(const LibString &s) const;

    // start/end cut
    LibString StartCut(const LibString &startStr) const;
    LibString EndCut(const LibString &endStr) const;

    // tolower/toupper operations. 请确保是英文字符串 isalpha
    LibString tolower() const;

    LibString toupper() const;

    LibString FirstCharToUpper() const;

    LibString FirstCharToLower() const;

    // escape support: escape string
    _ThisType &escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar);

    _ThisType escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar) const
    {
        if (_raw.empty())
            return *this;

        return _ThisType(*this).escape(willbeEscapeChars, escapeChar);
    }

    // escape support: unescape string
    _ThisType &unescape(const _Elem &escapeChar);

    _ThisType unescape(const _Elem &escapeChar) const;

    // UTF8 Surport
    // 是否utf8判断(只对不带bom utf8判断, 如果带bom请移除后判断)
    bool IsUtf8() const;
    // 添加bomb
    void add_utf8_bomb();
    // utf8字符个数
    std::string::size_type length_with_utf8() const;
    // 截取utf8字符串pos是按照utf8字符算的（不是按照字节）
    _ThisType substr_with_utf8(std::string::size_type pos = 0, std::string::size_type n = std::string::npos) const;

    // 从第charIndex个字符拆分成两个utf8字符串（前charIndex个utf8字符在strs的第一个）
    void split_utf8_string(size_type charIndex, _These &strs) const;   

    // 打散ut8f字符串 scatterCount:按顺序打散n次，生产scatterCount个utf8字符串
    void scatter_utf8_string(_These &chars, std::string::size_type scatterCount = 0) const;

    // utf8有没带bomb（windows下utf8识别带bomb（字符串前三个字节：\xef\xbb\xbf）一般在处理文件时用到）
    bool has_utf8_bomb() const;
    // 移除bomb
    void remove_utf8_bomb();

private:
    // 下一个utf8字符索引pos
    std::string::size_type _next_utf8_char_pos(std::string::size_type &beginBytePos) const;
    
    // hex=>decimal
    U8 _TurnDecimal(const Byte8 hexChar);

private:
    std::string _raw;
};

ALWAYS_INLINE const Byte8 *LibString::c_str() const
{
    return _raw.c_str();
}

ALWAYS_INLINE const Byte8 *LibString::data() const
{
    return _raw.data();
}

ALWAYS_INLINE Byte8 *LibString::data()
{
    return const_cast<Byte8 *>(_raw.data());
}

ALWAYS_INLINE bool LibString::empty() const
{
    return _raw.empty();
}

ALWAYS_INLINE size_t LibString::size() const
{
    return _raw.size();
}

ALWAYS_INLINE size_t LibString::length() const
{
    return _raw.length();
}

ALWAYS_INLINE void LibString::clear()
{
    _raw.clear();
}

ALWAYS_INLINE void LibString::resize(UInt64 bytes)
{
    _raw.resize(bytes);
}

ALWAYS_INLINE std::string &LibString::GetRaw()
{
    return _raw;
}

ALWAYS_INLINE const std::string &LibString::GetRaw() const
{
    return _raw;
}

ALWAYS_INLINE  bool LibString::Contain(const LibString &piece) const
{
    return _raw.find(piece._raw) != std::string::npos;
}

ALWAYS_INLINE LibString LibString::ToHexString() const
{
    LibString info;
    ToHexString(info);
    return info;
}

ALWAYS_INLINE LibString LibString::ToString() const
{
    return *this;
}

    
ALWAYS_INLINE LibString &LibString::AppendData(const Byte8 *bitData, Int64 dataSize)
{
    if(UNLIKELY(!bitData))
        return *this;

    _raw.append(bitData, dataSize);
    return *this;
}

ALWAYS_INLINE LibString &LibString::AppendData(const LibString &data)
{
    _raw.append(data._raw);
    return *this;
}

ALWAYS_INLINE LibString &LibString::AppendEnd()
{
    _raw += LibString::endl;
    return *this;
}

template< typename... Args>
ALWAYS_INLINE LibString &LibString::Append(Args&&... rest)
{
    // 使用数组展开参数包，并同时调用<<
    char _[] = { ((*this) << rest, char(0))... };
    UNUSED(_);
    
    return *this;
}

ALWAYS_INLINE LibString &LibString::Append()
{
    return *this;
}

ALWAYS_INLINE LibString::_These LibString::Split(Byte8 sep, size_type max_split, bool enableEmptyPart) const
{
    return this->Split(LibString(sep), max_split, false, enableEmptyPart);
}

ALWAYS_INLINE LibString::_These LibString::Split(const Byte8 *sep, size_type max_split, bool onlyLikely, bool enableEmptyPart) const
{
    return this->Split(LibString(sep), max_split, onlyLikely, enableEmptyPart);
}

ALWAYS_INLINE LibString LibString::lstrip(const LibString &chars) const
{
    LibString copyThis(*this);
    return copyThis.lstrip(chars);
}

ALWAYS_INLINE LibString LibString::lstripString(const LibString &str) const
{
    auto copyThis = *this;
    return copyThis.lstripString(str);
}

ALWAYS_INLINE LibString LibString::rstrip(const LibString &chars) const
{
    _ThisType copyThis(*this);
    return copyThis.rstrip(chars);
}

ALWAYS_INLINE LibString LibString::rstripString(const LibString &str) const
{
    auto copyThis = *this;
    return copyThis.rstripString(str);
}

ALWAYS_INLINE LibString &LibString::strip(const LibString &chars)
{
    return this->lstrip(chars).rstrip(chars);
}

ALWAYS_INLINE LibString LibString::strip(const LibString &chars) const
{
    _ThisType copyThis(*this);
    return copyThis.lstrip(chars).rstrip(chars);
}

ALWAYS_INLINE LibString &LibString::stripString(const LibString &str)
{
    return this->lstripString(str).rstripString(str);
}

ALWAYS_INLINE LibString LibString::stripString(const LibString &str) const
{
    _ThisType copyThis(*this);
    return copyThis.lstripString(str).rstripString(str);
}

ALWAYS_INLINE bool LibString::isalpha(const char &c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}


ALWAYS_INLINE LibString LibString::DragRange(const LibString &startStr, const LibString &endStr) const
{
    const auto &cache = DragAfter(startStr);
    return cache.DragBefore(endStr);
}

ALWAYS_INLINE LibString LibString::sub(const LibString &leftStr, const LibString &rightStr) const
{
    return lsub(leftStr).rsub(rightStr);
}

ALWAYS_INLINE bool LibString::isalpha() const
{
    return isalpha(*this);
}

ALWAYS_INLINE bool LibString::islower(const char &c)
{
    return 'a' <= c && c <= 'z';
}

ALWAYS_INLINE bool LibString::islower() const
{
    return islower(*this);
}

ALWAYS_INLINE bool LibString::isupper(const char &c)
{
    return 'A' <= c && c <= 'Z';
}

ALWAYS_INLINE bool LibString::isupper() const
{
    return isupper(*this);
}

ALWAYS_INLINE bool LibString::isdigit(const char &c)
{
    return '0' <= c && c <= '9';
}

ALWAYS_INLINE bool LibString::isdigit() const
{
    return isdigit(*this);
}

ALWAYS_INLINE bool LibString::isspace(const char &c)
{
    return  c == ' ' || c == '\t' || c == '\v' || c == '\r' || c == '\n' || c == '\f';
}

ALWAYS_INLINE bool LibString::isspace() const
{
    return isspace(*this);
}

ALWAYS_INLINE LibString::_ThisType LibString::unescape(const _Elem &escapeChar) const
{
    return _ThisType(*this).unescape(escapeChar);   
}

ALWAYS_INLINE void LibString::add_utf8_bomb()
{
    if (!this->has_utf8_bomb())
        _raw.insert(0, reinterpret_cast<const _Elem *>("\xef\xbb\xbf"));
}

ALWAYS_INLINE std::string::size_type LibString::length_with_utf8() const
{
    size_type count = 0;
    size_type bytePos = 0;
    while ((bytePos = _ThisType::_next_utf8_char_pos(bytePos)) != std::string::npos)
        ++count;

    return count;
}

ALWAYS_INLINE bool LibString::has_utf8_bomb() const
{
    if (_raw.size() < 3)
        return false;
    
    return (::memcmp(reinterpret_cast<const Byte8 *>(_raw.data()), 
        reinterpret_cast<const char *>("\xef\xbb\xbf"), 3) == 0) ? true : false;
}

ALWAYS_INLINE void LibString::remove_utf8_bomb()
{
    if (this->has_utf8_bomb())
        _raw.erase(0, 3);
}

KERNEL_EXPORT LibString &KernelAppendFormat(LibString &o, const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3);

KERNEL_END

// 重载运算符实现 std::string += LibString
extern KERNEL_EXPORT ALWAYS_INLINE std::string &operator +=(std::string &o, const KERNEL_NS::LibString &input)
{
    o.append(input.GetRaw());
    return o;
}

// 重载运算符实现 std::string + LibString
extern KERNEL_EXPORT ALWAYS_INLINE std::string operator +(const std::string &o, const KERNEL_NS::LibString &input)
{
    return o + input.GetRaw();
}

// 重载运算符实现 std::string += LibString
extern KERNEL_EXPORT ALWAYS_INLINE std::string &operator +=(std::string &o, KERNEL_NS::LibString &&input)
{
    o.append(input.GetRaw());
    return o;
}

// 重载运算符实现 std::string + LibString
extern KERNEL_EXPORT ALWAYS_INLINE std::string operator +(const std::string &o, KERNEL_NS::LibString &&input)
{
    return o + input.GetRaw();
}

namespace std
{

/**
 * \brief The explicit specialization of std::hash<KERNEL_NS::LibString> impl.
 * 
 */
template <>
struct hash<KERNEL_NS::LibString>
{
    // gcc 或者clang下pure属性可以当作内联
    #if CRYSTAL_CUR_COMP == CRYSTAL_COMP_GCC || CRYSTAL_CUR_COMP == CRYSTAL_COMP_CLANG
    CRYSTAL_ATTRIBUTE_PURE
    #endif // Comp == Gcc or Comp == Clang
    size_t operator()(const KERNEL_NS::LibString &str) const noexcept
    {
            #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            if (!str.empty())
                return ::std::_Hash_array_representation(str.data(), str.size());
            else
                return ::std::_Hash_representation(nullptr);
            #else
            if (!str.empty())
                return ::std::_Hash_impl::hash(str.data(), str.size());
            else
                return ::std::_Hash_impl::hash(nullptr, 0);
            #endif
    }
};

}

#endif
