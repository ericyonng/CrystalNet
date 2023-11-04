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

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibStringOut.h>

KERNEL_BEGIN
class LibString;
template<typename T>
class LibStream;

KERNEL_END

// template<typename ObjType>
// extern KERNEL_EXPORT KERNEL_NS::LibString &operator <<(KERNEL_NS::LibString &dest, const ObjType &obj);

/**
 * \brief Variant stream output function.
 */
template<typename T>
extern KERNEL_EXPORT KERNEL_NS::LibStream<T> &operator <<(KERNEL_NS::LibStream<T> &o, const KERNEL_NS::LibString &str);

extern KERNEL_EXPORT std::string &operator <<(std::string &o, const KERNEL_NS::LibString &str);

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
    arr['0'] = 0;
    arr['1'] = 1;
    arr['2'] = 2;
    arr['3'] = 3;
    arr['4'] = 4;
    arr['5'] = 5;
    arr['6'] = 6;
    arr['7'] = 7;
    arr['8'] = 8;
    arr['9'] = 9;
    arr['A'] = 10;
    arr['B'] = 11;
    arr['C'] = 12;
    arr['D'] = 13;
    arr['E'] = 14;
    arr['F'] = 15;
    arr['a'] = 10;
    arr['b'] = 11;
    arr['c'] = 12;
    arr['d'] = 13;
    arr['e'] = 14;
    arr['f'] = 15;
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
    LibString();
    LibString(LibString &&other);
    LibString(const LibString &other);
    LibString(const std::string &other);
    LibString(const Byte8 *other);
    LibString(const Byte8 *other, UInt64 cacheSize);
    LibString(Byte8 other);

    // 运算符
    LibString &operator = (const Byte8 *other);
    LibString &operator = (const std::string &other);
    LibString &operator = (const LibString &other);

    LibString operator + (const LibString &other) const;
    LibString operator + (const Byte8 *other) const; 
    LibString operator + (const std::string &other) const;
    LibString &operator += (const LibString &other);
    LibString &operator += (const Byte8 *other);
    LibString &operator += (const std::string &other);
    LibString &operator *= (size_t right);

    LibString operator - (const LibString &other) const;
    LibString operator - (const Byte8 *other) const; 
    LibString operator - (const std::string &other) const;
    LibString &operator -= (const LibString &other);
    LibString &operator -= (const Byte8 *other);
    LibString &operator -= (const std::string &other);

    LibString &operator << (const LibString &str);
    LibString &operator << (LibString &&str);
    LibString &operator << (LibString &str);
    LibString &operator << (const std::string &str);
    LibString &operator << (std::string &&str);
    LibString &operator << (const bool &val);
    LibString &operator << (const Byte8 &val);
    LibString &operator << (const U8 &val);
    LibString &operator << (const Int16 &val);
    LibString &operator << (const UInt16 &val);
    LibString &operator << (const Int32 &val);
    LibString &operator << (const UInt32 &val);
    LibString &operator << (const Long &val);
    LibString &operator << (const ULong &val);
    LibString &operator << (const Int64 &val);
    LibString &operator << (const UInt64 &val);
    LibString &operator << (const Float &val);
    LibString &operator << (const Double &val);
    LibString &operator << (const Byte8 *val);
    LibString &operator << (const void *addr); 
    LibString &operator << (void *&&addr);
//     template<typename T>
//     LibString &operator <<(T &&value);
    template<typename T>
    LibString &operator <<(const T &value);

    Byte8 &operator [] (UInt64 index);
    const Byte8 &operator [] (UInt64 index) const;
    Byte8 &at (UInt64 index);
    const Byte8 &at (UInt64 index) const;

    bool operator == (const LibString &other) const;
    bool operator == (const std::string &other) const;
    bool operator == (const Byte8 *other) const;
    bool operator != (const Byte8 *other) const;
    bool operator != (const LibString &other) const;
    bool operator < (const LibString &right) const;

    friend std::basic_ostream<_Elem> &operator <<(std::basic_ostream<_Elem> &o, const _ThisType &str)
    {
        o.write(str.data(), str.size());
        return o;
    }

    // TODO:测试const std::string &a = LibString()是否会走该接口
//     operator std::string &();
//     operator const std::string &() const;
//     operator const Byte8 *() const;
    

    size_t CopyTo(char *destData, UInt64 destSize, UInt64 cntToCopy, UInt64 srcOffset = 0) const;
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
    void ToHexString(LibString &target) const;   
    void ToHexView(LibString &target) const; 
    bool FromHexString(const LibString &hexString);
    void Swap(LibString &&str);
    void Swap(LibString &str);
    void CompressString();                  // 去除字符串末尾的\0压缩空间只对0结尾的字符串有效
    const LibString &RemoveZeroTail();      // 移除容器中尾部的0
    const LibString &RemoveHeadZero();      // 移除容器中首部的0

    // 如果是字符串的话会带很多的\0, 如果bitData有的话，这会造成再次Append的时候之后追加的字符串打印不出来
    LibString &AppendData(const Byte8 *bitData, Int64 dataSize);
    LibString &AppendData(const Byte8 *str)
    {
        return AppendData(str, static_cast<Int64>(strlen(str)));
    }
    
    LibString &AppendData(const LibString &data);
    LibString &AppendFormat(const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3)
    {
        // if fmt args is null, return.
        if (UNLIKELY(!fmt))
            return *this;

        // try detach detach format require buffers and resize it.
        va_list va;
        const size_type oldSize = _raw.size();
        va_start(va, fmt);
        Int32 len =::vsnprintf(nullptr, 0, fmt, va);
        va_end(va);
        if (len <= 0)
            return *this;

        // exec format.
        _raw.resize(oldSize + len);
        va_start(va, fmt);
        len = ::vsnprintf(const_cast<Byte8 *>(_raw.data() + oldSize),
                            len + 1,
                            fmt,
                            va);
        va_end(va);

        // len < 0 then _raw.size() - len > oldSize back to old string TODO:release情况有可能报错
//         if (UNLIKELY(oldSize != (_raw.size() - len)))
//         {
//             CRYSTAL_TRACE("wrong apend format and back to old string, oldSize:%llu, len:%d, new size:%llu", static_cast<UInt64>(oldSize), len, static_cast<UInt64>(_raw.size()));
//             throw std::logic_error("rong apend format and back to old string");
//             _raw.resize(oldSize);
//         }

        return *this;
    }

    static UInt64 CheckFormatSize(const Byte8 *fmt, va_list va);
    LibString &AppendFormatWithVaList(UInt64 finalFormatStrLen, const Byte8 *fmt, va_list va);
    LibString &AppendEnd();

    // 支持任意类型的追加只要对象重载<<即可
    template< typename... Args>
    LibString &Append(Args&&... rest);
    // 当参数为0时会匹配非泛型版本
    LibString &Append();

    _ThisType &findreplace(const _Elem &dest, const _Elem &with, Int32 count = -1);
    _ThisType &findreplace(const _ThisType &dest, const _ThisType &with, Int32 count = -1);
    _ThisType &findFirstAppendFormat(const _ThisType &dest, const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(3, 4)
    {
        auto &destRaw = dest._raw;
        auto pos = _raw.find(destRaw);
        if(pos == std::string::npos)
            return *this;

        // try detach detach format require buffers and resize it.
        va_list va;
        va_start(va, fmt);
        Int32 len =::vsnprintf(nullptr, 0, fmt, va);
        va_end(va);
        if (len <= 0)
            return *this;

        std::string cache;
        cache.resize(len);

        // exec format.
        va_start(va, fmt);
        len = ::vsnprintf(const_cast<Byte8 *>(cache.data()),
                            len + 1,
                            fmt,
                            va);
        va_end(va);

        // len < 0 then _raw.size() - len > oldSize back to old string
        if (UNLIKELY(0 != (cache.size() - len)))
        {
            CRYSTAL_TRACE("wrong apend format len:%d, new size:%llu", len, static_cast<UInt64>(cache.size()));
            return *this;
        }

        if(pos + 1 == _raw.size())
        {
            _raw.append(cache);
        }
        else
        {
            _raw.insert(pos + 1, cache);
        }

        return *this;
    }

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

    // tolower/toupper operations. 请确保是英文字符串 isalpha
    LibString tolower() const;
    LibString toupper() const;
    LibString FirstCharToUpper() const;
    LibString FirstCharToLower() const;

    // escape support: escape string
    _ThisType &escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar);
    _ThisType escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar) const;
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

ALWAYS_INLINE LibString::LibString()
{
    
}

ALWAYS_INLINE LibString::LibString(LibString &&other)
{
    _raw.swap(other._raw);
}

ALWAYS_INLINE LibString::LibString(const Byte8 *other, UInt64 cacheSize)
{
    if(LIKELY(other))
        _raw.append(other, cacheSize);
}

ALWAYS_INLINE LibString::LibString(const LibString &other)
{
    _raw = other._raw;
}

ALWAYS_INLINE LibString::LibString(const std::string &other)
{
    if(LIKELY(!other.empty()))
        _raw.append(other.data(), other.size());
}

ALWAYS_INLINE LibString::LibString(const Byte8 *other)
{
    if(LIKELY(other))
        _raw.assign(other, strlen(other));
}

ALWAYS_INLINE LibString::LibString(Byte8 other)
{
    _raw = other;
}

ALWAYS_INLINE LibString &LibString::operator = (const Byte8 *other)
{
    if(LIKELY(other))
        _raw.assign(other, strlen(other));

    return *this;
}

ALWAYS_INLINE LibString &LibString::operator = (const std::string &other)
{
    _raw = other;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator = (const LibString &other)
{
    _raw = other._raw;
    return *this;
}

ALWAYS_INLINE LibString LibString::operator + (const LibString &other) const
{
    return std::string(_raw).append(other._raw);
}

ALWAYS_INLINE LibString LibString::operator + (const Byte8 *other) const
{
    if(UNLIKELY(!other))
        return _raw;

    return std::string(_raw).append(std::string(other));
}

ALWAYS_INLINE LibString LibString::operator + (const std::string &other) const
{
    return _raw + other;
}

ALWAYS_INLINE LibString &LibString::operator += (const LibString &other)
{
    _raw.append(other._raw);
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator += (const Byte8 *other)
{
    if(LIKELY(other))
        _raw.append(other, ::strlen(other));
    
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator += (const std::string &other)
{
    _raw += other;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator *= (size_t right)
{
    if (_raw.empty() || right == 1)
        return *this;
        
    if (right == 0)
    {
        _raw.clear();
        return *this;
    }

    _ThisType unitStr(*this);
    auto &unitRaw = unitStr._raw;

    const _Elem *unitStrBuf = unitRaw.data();
    typename LibString::size_type unitStrSize = unitRaw.size();

    _raw.resize(unitStrSize * right);
    _Elem *buf = const_cast<_Elem *>(_raw.data());
    for (size_type i = 1; i < right; ++i)
        ::memcpy(buf + i * unitStrSize, unitStrBuf, unitStrSize * sizeof(_Elem));

    return *this;
}

ALWAYS_INLINE LibString LibString::operator - (const LibString &other) const
{
    auto pos = _raw.find(other._raw);
    if(pos == std::string::npos)
        return _raw;

    auto cache = _raw;
    cache.erase(pos, other._raw.size());
    return cache;
}

ALWAYS_INLINE LibString LibString::operator - (const Byte8 *other) const
{
    if(UNLIKELY(!other))
        return _raw;

    return LibString::operator -(LibString(other));
}

ALWAYS_INLINE LibString LibString::operator - (const std::string &other) const
{
    return LibString::operator -(LibString(other));
}


ALWAYS_INLINE LibString &LibString::operator -= (const LibString &other)
{
    auto pos = _raw.find(other._raw);
    if(pos == std::string::npos)
        return *this;

    _raw.erase(pos, other._raw.size());
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator -= (const Byte8 *other)
{
    if(UNLIKELY(!other))
        return *this;

    return LibString::operator -=(LibString(other));
}

ALWAYS_INLINE LibString &LibString::operator -= (const std::string &other)
{
    return LibString::operator -=(LibString(other));
}


ALWAYS_INLINE LibString &LibString::operator << (const LibString &str)
{
    _raw += str._raw;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (LibString &&str)
{
    _raw.append(str._raw);
    if (UNLIKELY(this != &str))
        str.clear();
    
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (LibString &str)
{
    _raw += str._raw;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const std::string &str)
{
    _raw += str;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (std::string &&str)
{
    _raw += str;
    if (UNLIKELY(&_raw != &str))
        str.clear();    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const bool &val)
{
    BUFFER4 cache;
    auto len = sprintf(cache, "%d", val);
    cache[len] = 0;
    _raw += cache;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const Byte8 &val)
{
    BUFFER8 cache;
    auto len = sprintf(cache, "%d", val);
    cache[len] = 0;
    _raw += cache;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const U8 &val)
{
    BUFFER8 cache;
    auto len = sprintf(cache, "%u", val);
    cache[len] = 0;
    _raw += cache;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const Int16 &val)
{
    BUFFER8 cache;
    auto len = sprintf(cache, "%hd", val);
    cache[len] = 0;
    _raw += cache;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const UInt16 &val)
{
    BUFFER8 cache;
    auto len = sprintf(cache, "%hu", val);
    cache[len] = 0;
    _raw += cache;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const Int32 &val)
{
    return AppendFormat("%d", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const UInt32 &val)
{
    return AppendFormat("%u", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const Long &val)
{
    return AppendFormat("%ld", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const ULong &val)
{
    return AppendFormat("%lu", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const Int64 &val)
{
    return AppendFormat("%lld", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const UInt64 &val)
{
    return AppendFormat("%llu", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const Float &val)
{
    return AppendFormat("%f", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const Double &val)
{
    return AppendFormat("%lf", val);
}

ALWAYS_INLINE LibString &LibString::operator << (const Byte8 *val)
{
    if(UNLIKELY(!val))
        return *this;

    _raw += val;
    return *this;
}

ALWAYS_INLINE LibString &LibString::operator << (const void *addr)
{
    return AppendFormat("%p", addr);
}

ALWAYS_INLINE LibString &LibString::operator << (void *&&addr)
{
    return AppendFormat("%p", addr);
}

// template<typename T>
// ALWAYS_INLINE LibString &LibString::operator <<(T &&value)
// {
//     return StringOutAdapter<T>::output(*this, std::forward<T>(value));
// }

template<typename T>
ALWAYS_INLINE LibString &LibString::operator <<(const T &value)
{
    return StringOutAdapter<T>::output(*this, value);
}

ALWAYS_INLINE Byte8 &LibString::operator [] (UInt64 index)
{
    return _raw[index];
}

ALWAYS_INLINE const Byte8 &LibString::operator [] (UInt64 index) const
{
    return _raw[index];
}

ALWAYS_INLINE Byte8 &LibString::at (UInt64 index)
{
    return _raw.at(index);
}

ALWAYS_INLINE const Byte8 &LibString::at (UInt64 index) const
{
    return _raw.at(index);
}

ALWAYS_INLINE bool LibString::operator == (const LibString &other) const
{
    return _raw == other._raw;
}

ALWAYS_INLINE bool LibString::operator == (const std::string &other) const
{
    return _raw == other;
}

ALWAYS_INLINE bool LibString::operator == (const Byte8 *other) const
{
    if(UNLIKELY(!other))
        return false;

    return _raw == other;
}

ALWAYS_INLINE bool LibString::operator != (const Byte8 *other) const
{
    if(UNLIKELY(!other))
        return true;

    return _raw != other;
}

ALWAYS_INLINE bool LibString::operator != (const LibString &other) const
{
    return _raw != other._raw;
}

ALWAYS_INLINE bool LibString::operator < (const LibString &right) const
{
    return _raw < right._raw;
}
// 
// ALWAYS_INLINE LibString::operator std::string &()
// {
//     return _raw;
// }
// 
// ALWAYS_INLINE LibString::operator const std::string &() const
// {
//     return _raw;
// }
// 
// ALWAYS_INLINE LibString::operator const Byte8 *() const
// {
//     return _raw.c_str();
// }

ALWAYS_INLINE size_t LibString::CopyTo(char *destData, UInt64 destSize, UInt64 cntToCopy, UInt64 srcOffset) const
{
    if(UNLIKELY(!destData))
        return 0;

    const UInt64 maxLen = std::min<UInt64>(destSize, cntToCopy);
    ::memcpy(destData, _raw.data() + srcOffset, maxLen);

    return maxLen;
}

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

ALWAYS_INLINE void LibString::ToHexString(LibString &target) const
{
    const Int64 bufferSize = static_cast<Int64>(_raw.size());
    if(bufferSize == 0)
        return;

    static const Byte8 ChToHexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    std::string info;
    char cache[4] = {0};
    info.reserve(bufferSize * 2);
    for(Int64 i = 0; i < bufferSize; ++i)
    {
        auto &ch = _raw[i];
        cache[0] = ChToHexChars[U8(ch) >> 4];
        cache[1] = ChToHexChars[ch & 0X0F];
        cache[2] = 0;

        info.append(cache, 2);
    }

    target += info;
} 

ALWAYS_INLINE void LibString::ToHexView(LibString &target) const
{
    const Int64 bufferSize = static_cast<Int64>(_raw.size());
    if(bufferSize == 0)
        return;

    auto &targetRaw = target.GetRaw();
    targetRaw.reserve(target.size() + (bufferSize * 3));

    // 每16字节一行
    Byte8 cache[8] = { 0 };
    Int32 cacheLen = 0;
    for (Int64 i = 0; i < bufferSize; ++i)
    {
        cacheLen = ::sprintf(cache, "%02x%s"
            , static_cast<U8>(_raw[i]), ((i + 1) % 16 == 0) ? "\n" : " ");

        cache[cacheLen] = 0;
        targetRaw.append(cache, cacheLen);
    }
}

ALWAYS_INLINE U8 LibString::_TurnDecimal(const Byte8 hexChar)
{
    switch (hexChar)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': 
    case 'A': return 10;
    case 'b': 
    case 'B': return 11;
    case 'c': 
    case 'C': return 12;
    case 'd': 
    case 'D': return 13;
    case 'e': 
    case 'E': return 14;
    case 'f': 
    case 'F': return 15;
    default:
        break;
    }

    throw std::logic_error("_TurnDecimal hex char not standard hex number");

    return 0;
}

ALWAYS_INLINE bool LibString::FromHexString(const LibString &hexString)
{
    const UInt64 hexLen = hexString.size();
    if(UNLIKELY(hexLen == 0 || ((hexLen % 2) != 0) ))
        return false;

    static const Byte8 *ChHexToDecimalValues = InitHexToDecimalValues();

    _raw.reserve(_raw.size() + hexLen / 2);
    for(UInt64 idx = 0; idx < hexLen; idx += 2)
    {
        auto &hi = ChHexToDecimalValues[hexString[idx]];
        auto &lo = ChHexToDecimalValues[hexString[idx + 1]];
        U8 decimalNumber = (hi << 4) | lo;
        _raw.append(reinterpret_cast<Byte8 *>(&decimalNumber), 1);
    }

    return true;
}

ALWAYS_INLINE void LibString::Swap(LibString &&str)
{
    if (UNLIKELY(this == &str))
        return;
    _raw.swap(str._raw);
}

ALWAYS_INLINE void LibString::Swap(LibString &str)
{
    if (UNLIKELY(this == &str))
        return;
    _raw.swap(str._raw);
}

ALWAYS_INLINE void LibString::CompressString()  
{
    const auto strSize = _raw.size();
    if(strSize == 0)
        return;

    auto len = strlen(_raw.c_str());
    if(strSize > len)
    {
        _raw.erase(len + 1, strSize - len - 1);
    }
}                
    
ALWAYS_INLINE const LibString &LibString::RemoveZeroTail()
{
    const Int64 bufferSize = static_cast<Int64>(_raw.size());
    if(bufferSize == 0)
        return *this;

    auto pos = _raw.find_first_of((const char)(0), 0);
    if(pos == std::string::npos)
        return *this;

    _raw.erase(pos, bufferSize - pos);
    return *this;
}

ALWAYS_INLINE const LibString &LibString::RemoveHeadZero()
{
    const Int64 bufferSize = static_cast<Int64>(_raw.size());
    if(bufferSize == 0)
        return *this;

    auto pos = _raw.find_first_not_of((const char)(0), 0);
    if(pos == std::string::npos)
        return *this;

    _raw.erase(0, pos);
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

// ALWAYS_INLINE LibString &LibString::AppendFormat(const Byte8 *fmt, ...)
// {
//     va_list ap;
//     va_start(ap, fmt);
//     StringUtil::snprintf(_raw, fmt, ap);
//     va_end(ap);
// 
// 	return *this;
// }

// ALWAYS_INLINE LibString &LibString::AppendFormat(const Byte8 *fmt, ...)
// {
//     // TODO:比较优化前后性能
//     // char *buf; int len;
//     // MemoryPool *pool = KernelGetTlsMemoryPool();
//     // __CRYSTAL_BuildFormatStr_(fmt, buf, len, pool, _Build::TL::Type);
//     // _raw.append(buf, len);
//     // pool->Free(buf);
//     va_list va;
//     va_start(va, fmt);
//     this->AppendFormatWithVaList(fmt, va);
//     va_end(va);

//     return *this;
// }

ALWAYS_INLINE UInt64 LibString::CheckFormatSize(const Byte8 *fmt, va_list va)
{
    if (UNLIKELY(!fmt))
        return 0;

    // try detach detach format require buffers and resize it.
    Int32 len = ::vsnprintf(nullptr, 0, fmt, va);
    if (len <= 0)
        return 0;

    return static_cast<UInt64>(len);
}

ALWAYS_INLINE LibString &LibString::AppendFormatWithVaList(UInt64 finalFormatStrLen, const Byte8 *fmt, va_list va)
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

ALWAYS_INLINE LibString::_ThisType &LibString::findreplace(const _Elem &dest, const _Elem &with, Int32 count)
{
    if (dest == with)
        return *this;

    for (size_type i = 0; i < this->size(); ++i)
    {
        if (_raw[i] == dest)
            _raw.replace(i, 1, 1, with);
    }

    return *this;
}

ALWAYS_INLINE LibString::_ThisType &LibString::findreplace(const _ThisType &dest, const _ThisType &with, Int32 count)
{
    if (dest == with)
        return *this;

    size_type found = 0;
    const std::string &destRaw = dest._raw;
    const std::string &withRaw = with._raw;
    while ((found = _raw.find(destRaw, found)) != std::string::npos)
    {
        _raw.replace(found, destRaw.size(), withRaw);
        found += withRaw.size();
    }

    return *this;
}

ALWAYS_INLINE LibString::_ThisType &LibString::EraseAnyOf(const _ThisType &dest)
{
    auto &destRaw = dest.GetRaw();
    const UInt64 len = destRaw.length();
    UInt64 curPos = 0;
    for (UInt64 idx = 0; idx < len; ++idx)
    {
        do 
        {
            curPos = _raw.find(destRaw[idx]);
            if (curPos != std::string::npos)
                _raw.erase(curPos, 1);
        } while (curPos != std::string::npos);
    }

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

ALWAYS_INLINE LibString::_These LibString::Split(const LibString &sep, std::string::size_type max_split, bool onlyLikely, bool enableEmptyPart) const
{
    _These substrs;
    if(sep.empty() || max_split == 0 || this->empty())
    {
        if(!this->empty() || enableEmptyPart)
          substrs.push_back(*this);

        return substrs;
    }

    size_type idx = 0;
    UInt32 splitTimes = 0;
    const std::string &sepRaw = sep._raw;
    const UInt64 stepSize = onlyLikely ? 1 : sepRaw.size();
    for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
    {
        size_type findIdx = std::string::npos;
        if(onlyLikely)
        {
            for(size_t i = 0; i < sepRaw.size(); i++)
            {
                findIdx = _raw.find(sepRaw[i], idx);
                if(findIdx != std::string::npos)
                    break;
            }
        }
        else
        {
            findIdx = _raw.find(sepRaw, idx);
        }

        if(findIdx == std::string::npos)
            break;
        
        if (findIdx == idx)
        {
            if(enableEmptyPart)
                substrs.push_back(_ThisType());

            if ((idx = findIdx + stepSize) == this->size())
            {
                if (enableEmptyPart)
                    substrs.push_back(_ThisType());
                break;
            }

            continue;
        }

        substrs.push_back(_raw.substr(idx, findIdx - idx));

        if((idx = findIdx + stepSize) == this->size())
        {
            if(enableEmptyPart)
               substrs.push_back(_ThisType());

            break;
        }
    }

    // 还有剩余
    if(idx != _raw.size())
    {
        const auto &subStr = _raw.substr(idx);
        if(!subStr.empty() || enableEmptyPart)
            substrs.push_back(subStr);
    }

    return substrs;
}

ALWAYS_INLINE LibString::_These LibString::Split(const _These &seps, size_type max_split, bool enableEmptyPart) const
{
   _These substrs;
    if(seps.empty() || max_split == 0 || this->empty())
    {
        if(!this->empty() || enableEmptyPart)
           substrs.push_back(*this);
        return substrs;
    }

    size_type idx = 0;
    UInt32 splitTimes = 0;
    std::set<size_type> minIdx;
    for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
    {
        size_type findIdx = std::string::npos;
        minIdx.clear();
        for(size_t i = 0; i < seps.size(); i++)
        {
            findIdx = _raw.find(seps[i]._raw, idx);
            if(findIdx != std::string::npos)
                minIdx.insert(findIdx);
        }

        if(!minIdx.empty())
            findIdx = *minIdx.begin();

        if(findIdx == std::string::npos)
            break;

        substrs.push_back(_raw.substr(idx, findIdx - idx));
        if((idx = findIdx + 1) == this->size())
        {
            if(enableEmptyPart)
               substrs.push_back(_ThisType());
            break;
        }
    }

    if(idx != this->size())
    {
        const auto &subStr = _raw.substr(idx);
        if(!subStr.empty() || enableEmptyPart)
            substrs.push_back(subStr);
    }

    return substrs;
}

ALWAYS_INLINE LibString &LibString::lstrip(const LibString &chars)
{
    _ThisType willStripChars = chars;
    if (chars.empty())
    {
        willStripChars._raw.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));
    }

    std::string &thisRaw = _raw;
    size_type stripTo = 0;
    std::string &willStripRaw = willStripChars._raw;
    const size_type thisSize = static_cast<size_type>(thisRaw.size());
    for (size_type i = 0; i < thisSize; ++i)
    {
        bool found = false;
        const _Elem &now = thisRaw[i];
        for (size_type j = 0; j < willStripRaw.size(); ++j)
        {
            if (now == willStripRaw[j])
            {
                found = true;
                break;
            }
        }

        if (found)
            stripTo = i + 1;
        else
            break;
    }

    if (stripTo != 0)
        thisRaw.erase(0, stripTo);

    return *this;
}

ALWAYS_INLINE LibString LibString::lstrip(const LibString &chars) const
{
    LibString copyThis(*this);
    return copyThis.lstrip(chars);
}

ALWAYS_INLINE LibString &LibString::lstripString(const LibString &str)
{
    if (str.empty())
    {
        return *this;
    }

    std::string &thisRaw = _raw;
    size_type stripTo = 0;
    const size_type thisSize = static_cast<size_type>(thisRaw.size());
    const size_type sliceSize = static_cast<size_type>(str.size());
    for (size_type i = 0; i < thisSize; i += 1)
    {
        if(sliceSize > (thisSize - i))
            break;

        auto pos = thisRaw.find(str._raw, i);
        if(pos == i)
        {
            stripTo = pos + sliceSize;
            i = stripTo - 1;
        }
        else
        {
            break;
        }
    }

    if (stripTo != 0)
        thisRaw.erase(0, stripTo);

    return *this;
}

ALWAYS_INLINE LibString LibString::lstripString(const LibString &str) const
{
    auto copyThis = *this;
    return copyThis.lstripString(str);
}

ALWAYS_INLINE LibString &LibString::rstrip(const LibString &chars)
{
    _ThisType willStripChars = chars;
    if (chars.empty())
        willStripChars._raw.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));

    std::string &thisRaw = _raw;
    std::string &willStripRaw = willStripChars._raw;
    const Int64 willStripRawSize = static_cast<Int64>(willStripRaw.size());
    const Int64 thisSize = static_cast<Int64>(thisRaw.size());

    Int64 stripFrom = thisSize;
    for (Int64 i = thisSize - 1; i >= 0; --i)
    {
        bool found = false;
        const _Elem &now = thisRaw[i];
        for (Int64 j = 0; j < willStripRawSize; ++j)
        {
            if (now == willStripRaw[j])
            {
                found = true;
                break;
            }
        }

        if (found)
            stripFrom = i;
        else
            break;
    }

    if (stripFrom != thisSize)
        thisRaw.erase(stripFrom);

    return *this;
}

ALWAYS_INLINE LibString LibString::rstrip(const LibString &chars) const
{
    _ThisType copyThis(*this);
    return copyThis.rstrip(chars);
}

ALWAYS_INLINE LibString &LibString::rstripString(const LibString &str)
{
    if (str.empty())
    {
        return *this;
    }

    std::string &thisRaw = _raw;
    const Int64 thisSize = static_cast<Int64>(thisRaw.size());
    Int64 stripTo = thisSize;
    const Int64 sliceSize = static_cast<Int64>(str.size());
    for (Int64 i = thisSize - 1; i >= 0; i -= 1)
    {
        if(sliceSize > i + 1)
            break;

        auto pos = thisRaw.rfind(str._raw, i);
        if(pos == static_cast<size_type>((i + 1) - sliceSize))
        {
            stripTo = static_cast<Int64>(pos);
            i = stripTo;
        }
        else
        {
            break;
        }
    }

    if (stripTo != thisSize)
        thisRaw.erase(stripTo);

    return *this;
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

ALWAYS_INLINE LibString LibString::DragAfter(const LibString &start) const
{
    auto pos =  _raw.find(start._raw);
    if(pos == std::string::npos)
        return LibString();
    
    return _raw.substr(pos + start._raw.size());
}

ALWAYS_INLINE LibString LibString::DragAfter(const LibString &start, size_t &startPos, size_t &endPos) const
{
    auto pos =  _raw.find(start._raw);
    if(pos == std::string::npos)
        return LibString();

    endPos = pos + start._raw.size() - 1;
    startPos = pos;
    const auto &subStr = _raw.substr(pos + start._raw.size());
    endPos += subStr.size();
    return subStr;
}


ALWAYS_INLINE LibString LibString::DragBefore(const LibString &start) const
{
    auto pos =  _raw.find(start._raw);
    if(pos == std::string::npos)
        return LibString();

    return _raw.substr(0, pos);
}

ALWAYS_INLINE LibString LibString::DragRange(const LibString &startStr, const LibString &endStr) const
{
    const auto &cache = DragAfter(startStr);
    return cache.DragBefore(endStr);
}

ALWAYS_INLINE LibString LibString::lsub(const LibString &flagStr) const
{
    auto pos = _raw.find_first_of(flagStr.GetRaw(), 0);
    if(pos == std::string::npos)
        return LibString();

    return _raw.substr(pos + flagStr.size());
}

ALWAYS_INLINE LibString LibString::rsub(const LibString &flagStr) const
{
    auto pos = _raw.rfind(flagStr.GetRaw());
    if(pos == std::string::npos)
        return LibString();

    return _raw.substr(0, pos);
}

ALWAYS_INLINE LibString LibString::sub(const LibString &leftStr, const LibString &rightStr) const
{
    return lsub(leftStr).rsub(rightStr);
}

ALWAYS_INLINE bool LibString::isalpha(const LibString &s)
{
    if(s.empty())
        return false;

    const std::string &sRaw = s._raw;
    const size_type sSize = sRaw.size();
    for(size_t i = 0; i < sSize; i++)
    {
        if(!isalpha(sRaw[i]))
            return false;
    }

    return true;
}

ALWAYS_INLINE bool LibString::isalpha() const
{
    return isalpha(*this);
}

ALWAYS_INLINE bool LibString::islower(const char &c)
{
    return 'a' <= c && c <= 'z';
}

ALWAYS_INLINE bool LibString::islower(const LibString &s)
{
    if(s.empty())
        return false;

    bool foundLower = false;
    const std::string &sRaw = s._raw;
    const size_type sSize = sRaw.size();
    for(size_type i = 0; i < sSize; i++)
    {
        if(isupper(sRaw[i]))
            return false;
        else if(islower(sRaw[i]))
            foundLower = true;
    }

    return foundLower;
}

ALWAYS_INLINE bool LibString::islower() const
{
    return islower(*this);
}

ALWAYS_INLINE bool LibString::isupper(const char &c)
{
    return 'A' <= c && c <= 'Z';
}

ALWAYS_INLINE bool LibString::isupper(const LibString &s)
{
    if(s.empty())
        return false;

    bool foundUpper = false;
    const std::string &sRaw = s._raw;
    const size_type sSize = sRaw.size();
    for(size_type i = 0; i < sSize; i++)
    {
        if(islower(sRaw[i]))
            return false;
        else if(isupper(sRaw[i]))
            foundUpper = true;
    }

    return foundUpper;
}

ALWAYS_INLINE bool LibString::isupper() const
{
    return isupper(*this);
}

ALWAYS_INLINE bool LibString::isdigit(const char &c)
{
    return '0' <= c && c <= '9';
}

ALWAYS_INLINE bool LibString::isdigit(const LibString &s)
{
    if(s.empty())
        return false;

    const std::string &sRaw = s._raw;
    const size_type sSize = sRaw.size();
    for(size_type i = 0; i < sSize; ++i)
    {
        if(!isdigit(sRaw[i]))
            return false;
    }

    return true;
}

ALWAYS_INLINE bool LibString::isdigit() const
{
    return isdigit(*this);
}

ALWAYS_INLINE bool LibString::isspace(const char &c)
{
    return  c == ' ' || c == '\t' || c == '\v' || c == '\r' || c == '\n' || c == '\f';
}

ALWAYS_INLINE bool LibString::isspace(const LibString &s)
{
    if(s.empty())
        return false;

    const std::string &sRaw = s._raw;
    const size_type sSize = sRaw.size();
    for(size_type i = 0; i < sSize; i++)
    {
        if(!isspace(sRaw[i]))
            return false;
    }

    return false;
}

ALWAYS_INLINE bool LibString::isspace() const
{
    return isspace(*this);
}

ALWAYS_INLINE bool LibString::IsStartsWith(const LibString &s) const
{
    if (s.empty())
        return true;

    const std::string &sRaw = s._raw;
    return (_raw.size() >= sRaw.size() && memcmp(sRaw.data(), _raw.data(), sRaw.size() * sizeof(_Elem)) == 0);
}

ALWAYS_INLINE bool LibString::IsEndsWith(const LibString &s) const
{
    if (s.empty())
        return true;

    const std::string &sRaw = s._raw;
    return (_raw.size() >= sRaw.size() && 
        memcmp(sRaw.data(), _raw.data() + (_raw.size() - sRaw.size()) * sizeof(_Elem), sRaw.size() * sizeof(_Elem)) == 0);
}

// tolower/toupper operations. 请确保是英文字符串 isalpha
ALWAYS_INLINE LibString LibString::tolower() const
{
    const _Elem *buf = _raw.data();
    const size_type size = this->size();

    _ThisType lower;
    std::string &lowerRaw = lower._raw;
    lowerRaw.resize(size);
    for (size_type i = 0; i < size; ++i)
    {
        if (buf[i] >= 0x41 && buf[i] <= 0x5A)
            lowerRaw[i] = buf[i] + 0x20;
        else
            lowerRaw[i] = buf[i];
    }

    return lower;
}

ALWAYS_INLINE LibString LibString::toupper() const
{
    const _Elem *buf = this->data();
    const size_type size = this->size();

    _ThisType upper;
    std::string &upperRaw = upper._raw;
    upperRaw.resize(size);
    for (size_type i = 0; i < size; ++i)
        if (buf[i] >= 0x61 && buf[i] <= 0x7a)
            upperRaw[i] = buf[i] - 0x20;
        else
            upperRaw[i] = buf[i];

    return upper;
}

ALWAYS_INLINE LibString LibString::FirstCharToUpper() const
{
    const _Elem *buf = this->data();
    const size_type size = this->size();
    if(size == 0)
        return *this;

    _ThisType upper;
    std::string &upperRaw = upper._raw;
    upper = *this;

    if (buf[0] >= 0x61 && buf[0] <= 0x7a)
        upperRaw[0] = buf[0] - 0x20;

    return upper;
}

ALWAYS_INLINE LibString LibString::FirstCharToLower() const
{
    const _Elem *buf = this->data();
    const size_type size = this->size();
    if(size == 0)
        return *this;

    _ThisType lower;
    std::string &lowerRaw = lower._raw;
    lower = *this;

    if (buf[0] >= 0x41 && buf[0] <= 0x5A)
        lowerRaw[0] = buf[0] + 0x20;

    return lower;   
}

// escape support: escape string
ALWAYS_INLINE LibString::_ThisType &LibString::escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar)
{
    if (this->empty())
        return *this;

    const long len = static_cast<long>(this->size());
    std::string &thisRaw = _raw;
    const std::string &willbeEscapeRaw = willbeEscapeChars._raw;
    for (long i = len - 1; i >= 0; --i)
    {
        const _Elem &ch = thisRaw[i];
        if (ch == escapeChar ||
            willbeEscapeRaw.find(ch) != std::string::npos)
            thisRaw.insert(i, 1, escapeChar);
    }

    return *this;
}

ALWAYS_INLINE LibString::_ThisType LibString::escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar) const
{
    if (_raw.empty())
        return *this;

    return _ThisType(*this).escape(willbeEscapeChars, escapeChar);
}

// escape support: unescape string
ALWAYS_INLINE LibString::_ThisType &LibString::unescape(const _Elem &escapeChar)
{
    if (_raw.empty())
        return *this;

    std::string &thisRaw = _raw;
    const Int64 len = static_cast<Int64>(thisRaw.size());
    for (Int64 i = len - 1; i >= 0; --i)
    {
        const _Elem &ch = thisRaw[i];
        if (ch == escapeChar)
        {
            if (i > 0 && thisRaw[i - 1] == escapeChar)
                thisRaw.erase(i--, 1);
            else
                thisRaw.erase(i, 1);
        }
    }

    return *this;
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

ALWAYS_INLINE LibString::_ThisType LibString::substr_with_utf8(std::string::size_type pos, std::string::size_type n) const
{
    size_type utf8Len = this->length_with_utf8();
    if (pos >= utf8Len || n == 0)
        return _ThisType();

    _These substrs;
    this->split_utf8_string(pos, substrs);
    if (substrs.empty())
        return _ThisType();

    _ThisType str1 = *substrs.rbegin();
    utf8Len = str1.length_with_utf8();
    pos = (n == std::string::npos || n > utf8Len) ? utf8Len : n;

    substrs.clear();
    str1.split_utf8_string(pos, substrs);
    if (substrs.empty())
        return _ThisType();

    return substrs[0];
}

ALWAYS_INLINE void LibString::split_utf8_string(size_type charIndex, _These &strs) const
{
    strs.clear();
    if (charIndex == 0)
    {
        strs.push_back(*this);
        return;
    }

    size_type utf8Count = _ThisType::length_with_utf8();
    if (UNLIKELY(utf8Count == std::string::npos))
    {
        strs.push_back(*this);
        return;
    }

    charIndex = (charIndex < 0) ? 
        static_cast<size_type>(utf8Count) + charIndex : charIndex;
    if (charIndex <= 0 || charIndex >= static_cast<size_type>(utf8Count))
    {
        strs.push_back(*this);
        return;
    }

    size_type bytePos = 0;
    size_type charPos = 0;
    while (charPos != charIndex)
    {
        bytePos = _ThisType::_next_utf8_char_pos(bytePos);
        ++charPos;
    }

    strs.push_back(_raw.substr(0, bytePos));
    strs.push_back(_raw.substr(bytePos));
}

ALWAYS_INLINE void LibString::scatter_utf8_string(_These &chars, std::string::size_type scatterCount) const
{
    chars.clear();

    if (scatterCount == 0)
        scatterCount = std::string::npos;
    else if (scatterCount != std::string::npos)
        scatterCount -= 1;

    if (scatterCount == 0)
    {
        chars.push_back(*this);
        return;
    }

    size_type curPos = 0;
    size_type prevPos = 0;
    size_type curScatterCount = 0;
    while ((curPos = this->_next_utf8_char_pos(prevPos)) != std::string::npos)
    {
        chars.push_back(_raw.substr(prevPos, curPos - prevPos));

        if (scatterCount != std::string::npos && ++curScatterCount >= scatterCount)
        {
            chars.push_back(_raw.substr(curPos));
            break;
        }

        prevPos = curPos;
    }
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

ALWAYS_INLINE std::string::size_type LibString::_next_utf8_char_pos(std::string::size_type &beginBytePos) const
{
    if (beginBytePos == 0 && this->has_utf8_bomb())
        beginBytePos += 3;

    if (beginBytePos == std::string::npos || beginBytePos >= _raw.size())
        return std::string::npos;

    size_type waitCheckCount = std::string::npos;

    // 0xxx xxxx
    // Encoding len: 1 byte.
    U8 ch = static_cast<U8>(_raw.at(beginBytePos));
    if ((ch & 0x80) == 0x00)
        waitCheckCount = 0;
    // 110x xxxx
    // Encoding len: 2 bytes.
    else if ((ch & 0xe0) == 0xc0)
        waitCheckCount = 1;
    // 1110 xxxx
    // Encoding len: 3 bytes.
    else if ((ch & 0xf0) == 0xe0)
        waitCheckCount = 2;
    // 1111 0xxx
    // Encoding len: 4 bytes.
    else if ((ch & 0xf8) == 0xf0)
        waitCheckCount = 3;
    // 1111 10xx
    // Encoding len: 5 bytes.
    else if ((ch & 0xfc) == 0xf8)
        waitCheckCount = 4;
    // 1111 110x
    // Encoding len: 6 bytes.
    else if ((ch & 0xfe) == 0xfc)
        waitCheckCount = 5;

    if (waitCheckCount == std::string::npos)
        return std::string::npos;

    size_type curPos = beginBytePos + 1;
    size_type endPos = curPos + waitCheckCount;
    if (endPos > _raw.size())
        return std::string::npos;

    for (; curPos != endPos; ++curPos)
    {
        ch = static_cast<U8>(_raw.at(curPos));
        if ((ch & 0xc0) != 0x80)
            return std::string::npos;
    }

    return endPos;
}

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

template<typename T>
ALWAYS_INLINE KERNEL_NS::LibStream<T> &operator <<(KERNEL_NS::LibStream<T> &o, const KERNEL_NS::LibString &str)
{
    o.Write(str);
    return o;
}

#endif
