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
 * Date: 2026-06-02 14:26:22
 * Author: Eric Yonng
 * Description: 默认使用Ascii码,打印日志时候应使用Utf8编码,提供ascii码转utf8接口
 *              有别于标准库,内存分配由用户指定分配器,带码点，数据包括一颗树和一个数组，支持utf8，支持append_format接口等，编码不同直接报错处理
 *              默认使用tls memmory pool
 *              _Base::data只有在_Base内容大小使得capacity发生变化的时候才会失效
*/
#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_BASIC_STRING_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LIB_BASIC_STRING_H__

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
#include <kernel/comp/BasicStringHelper.h>


// template<typename ObjType>
// extern KERNEL_EXPORT KERNEL_NS::_This &operator <<(KERNEL_NS::_This &dest, const ObjType &obj);

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


template <typename _Elem,
          typename _Traits = std::char_traits<_Elem>,
          typename _Ax = std::allocator<_Elem> >
class LibBasicString : public ::std::basic_string<_Elem, _Traits, _Ax>
{
    static_assert(sizeof(_Elem) <= 2, "LibBasicString not support sizeof(_Elem) > 2 element type!");

    typedef LibBasicString<_Elem, _Traits, _Ax> _This;
    typedef std::basic_string<_Elem, _Traits, _Ax> _Base;

    typedef std::vector<LibBasicString<_Elem, _Traits, _Ax> > _These;

public:
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    static constexpr const Byte8 *endl = "\n";
#else
    static constexpr const Byte8 *endl = "\r\n";
#endif

    // 默认需要剔除的符号
    static constexpr const Byte8 *_defStripChars = DEF_STRIP_CHARS;
    
public:
    typedef typename _Base::size_type size_type;
    typedef typename _Base::value_type value_type;

    typedef typename _Base::iterator iterator;
    typedef typename _Base::const_iterator const_iterator;

    typedef typename _Base::pointer pointer;
    typedef typename _Base::const_pointer const_pointer;

    typedef _These These;

public:
    static constexpr size_type npos = _Base::npos;
    
public:
    LibBasicString()
    {
        
    }

    // 销毁自己
    void Release() const
    {
        delete this;
    }

    LibBasicString(_This &&other)
        : _Base(std::forward<_Base>(other))
    {
    }
    
    LibBasicString(const _This &other)
        : _Base(other)
    {
    }

    LibBasicString(const Byte8 *str)
    : _Base(str)
    {
        
    }


    LibBasicString(const _Base &other)
    : _Base(other)
    {
    }

    LibBasicString(_Base &&other)
        : _Base(std::forward<_Base>(other))
    {
    }

    LibBasicString(const Byte8 *other, UInt64 cacheSize)
        : _Base(other, cacheSize)
    {
    }

    LibBasicString(Byte8 other)
        : _Base(&other, 1)
    {
    }

    // 重载 operator new - 控制 _This 对象本身的内存分配
    static void* operator new(size_t size)
    {
        return KERNEL_ALLOC_MEMORY_TL(size);
    }
    
    static void* operator new[](size_t size)
    {
        return KERNEL_ALLOC_MEMORY_TL(size);
    }
    
    // 重载 operator delete - 控制 _This 对象本身的内存释放
    static void operator delete(void* ptr) noexcept
    {
        KERNEL_FREE_MEMORY_TL(ptr);
    }
    static void operator delete[](void* ptr) noexcept
    {
        KERNEL_FREE_MEMORY_TL(ptr);
    }

    _This &operator = (_This &&other)
    {
        _Base::operator = (std::forward<_Base>(other));
        return *this;
    }

    _This &operator = (const _This &other)
    {
        _Base::operator = (other);
        return *this;
    }

    _This operator + (_This &&other) const
    {
        _This copyThis = *this;
        copyThis.append(std::forward<_Base>(other));
        return copyThis;
    }

    _This operator + (const _This &other) const
    {
        _This copyThis = *this;
        copyThis.append(other);
        return copyThis;
    }

    _This operator + (const Byte8 *other) const
    {
        if(UNLIKELY(!other))
            return *this;
    
        _This copyThis = *this;
        copyThis.append(other);
        return copyThis;
    }

    // _This operator + (const _Base &other) const
    // {
    //     _This copyThis = *this;
    //     copyThis.append(other);
    //     return copyThis;
    // }

    _This &operator += (const _This &other)
    {
        this->append(other);
        return *this;
    }

    _This &operator += (const Byte8 *other)
    {
        if(LIKELY(other))
            this->append(other, ::strlen(other));
        
        return *this;
    }

    // _This &operator += (const _Base &other)
    // {
    //     append(other);
    //     return *this;
    // }

    _This &operator *= (size_t right)
    {
        if (this->empty() || right == 1)
            return *this;
		
        if (right == 0)
        {
            this->clear();
            return *this;
        }

        _This unitStr(*this);
        const _Elem *unitStrBuf = unitStr.data();
        size_type unitStrSize = unitStr.size();

        this->resize(unitStrSize * right);
        _Elem *buf = const_cast<_Elem *>(this->data());
        for (size_type i = 1; i < right; ++i)
            ::memcpy(buf + i * unitStrSize, unitStrBuf, unitStrSize * sizeof(_Elem));

        return *this;
    }

    _This operator - (const _This &other) const
    {
        return _This::operator-(static_cast<const _Base &>(other));
    }

    _This operator - (const Byte8 *other) const
    {
        if(UNLIKELY(!other))
            return *this;

        auto pos = this->find(other);
        if(pos == npos)
            return *this;

        auto cache = *this;
        cache.erase(pos, strlen(other));
        return cache;
    }

    _This operator - (const _Base &other) const
    {
        auto pos = this->find(other);
        if(pos == npos)
            return *this;

        auto cache = *this;
        cache.erase(pos, other.size());
        return cache;
    }

    _This &operator -= (const _This &other)
    {
        return  _This::operator -=(static_cast<const _Base &>(other));
    }

    _This &operator -= (const Byte8 *other)
    {
        if(UNLIKELY(!other))
            return *this;

        auto pos = this->find(other);
        if(pos == npos)
            return *this;

        this->erase(pos, strlen(other));
        return *this;
    }

    _This &operator -= (const _Base &other)
    {
        auto pos = this->find(other);
        if(pos == npos)
            return *this;

        this->erase(pos, other.size());
        return *this;
    }

    _This &operator << (const _This &str)
    {
        *this += str;
        return *this;
    }

    _This &operator << (_This &&str)
    {
        this->append(std::forward<_Base>(str));
        return *this;
    }

    _This &operator << (_This &str)
    {
        *this += str;
        return *this;
    }

    _This &operator << (const _Base &str)
    {
        this->append(str);
        return *this;
    }

    _This &operator << (_Base &&str)
    {
        this->append(std::forward<_Base>(str));
        return *this;
    }
    _This &operator << (const bool &val)
    {
        BUFFER4 cache;
        auto len = sprintf(cache, "%d", val);
        cache[len] = 0;
        *this += cache;
        return *this;
    }

    _This &operator << (const Byte8 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%d", val);
        cache[len] = 0;
        *this += cache;
        return *this;
    }

    _This &operator << (const U8 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%u", val);
        cache[len] = 0;
        *this += cache;
        return *this;
    }

    _This &operator << (const Int16 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%hd", val);
        cache[len] = 0;
        *this += cache;
        return *this;
    }

    _This &operator << (const UInt16 &val)
    {
        BUFFER8 cache;
        auto len = sprintf(cache, "%hu", val);
        cache[len] = 0;
        *this += cache;
        return *this;
    }

    _This &operator << (const Int32 &val)
    {
        return AppendFormat("%d", val);
    }

    _This &operator << (const UInt32 &val)
    {
        return AppendFormat("%u", val);
    }

    _This &operator << (const Long &val)
    {
        return AppendFormat("%ld", val);
    }

    _This &operator << (const ULong &val)
    {
        return AppendFormat("%lu", val);
    }

    _This &operator << (const Int64 &val)
    {
        return AppendFormat("%lld", val);
    }

    _This &operator << (const UInt64 &val)
    {
        return AppendFormat("%llu", val);
    }

    _This &operator << (const Float &val)
    {
        return AppendFormat("%f", val);
    }

    _This &operator << (const Double &val)
    {
        return AppendFormat("%lf", val);
    }

    _This &operator << (const Byte8 *val)
    {
        if(UNLIKELY(!val))
            return *this;

        *this += val;
        return *this;
    }

    _This &operator << (const void *addr)
    {
        return AppendFormat("%p", addr);
    }

    _This &operator << (void *&&addr)
    {
        return AppendFormat("%p", addr);
    }

    template<typename T>
    _This &operator <<(const T &value)
    {
        return StringOutAdapter<T>::output(*this, value);
    }

    Byte8 &operator [] (UInt64 index)
    {
        return _Base::operator[](index);
    }

    const Byte8 &operator [] (UInt64 index) const
    {
        return _Base::operator[](index);
    }

    Byte8 &at (UInt64 index)
    {
        return _Base::at(index);
    }

    const Byte8 &at (UInt64 index) const
    {
        return _Base::at(index);
    }
    
    bool operator == (const _This &other) const
    {
        return this->_Equal(other);
    }
    
    bool operator == (const _Base &other) const
    {
        return  this->_Equal(other);
    }

    bool operator == (const Byte8 *other) const
    {
        if(UNLIKELY(!other))
            return false;

        return this->_Equal(other);
    }

    bool operator != (const Byte8 *other) const
    {
        return !operator ==(other);
    }

    bool operator != (const _This &other) const
    {
        return !operator ==(other);
    }

    bool operator < (const _This &right) const
    {
        return this->compare(right) < 0;
    }

    friend std::basic_ostream<_Elem, _Traits> &operator <<(std::basic_ostream<_Elem, _Traits> &o, const _This &str)
    {
        o.write(str.data(), str.size());
        return o;
    }

    size_t CopyTo(char *destData, UInt64 destSize, UInt64 cntToCopy, UInt64 srcOffset = 0) const
    {
        if(UNLIKELY(!destData))
            return 0;

        const UInt64 maxLen = std::min<UInt64>(destSize, cntToCopy);
        ::memcpy(destData, this->data() + srcOffset, maxLen);

        return maxLen;
    }
    
    // c_str()'\0'结尾
    // data()没有'\0'结尾
    _Base &GetRaw();
    const _Base &GetRaw() const;
    bool Contain(const _This &piece) const;

    _This ToHexString() const;
    _This ToString() const;    

    void ToHexString(_This &target) const
    {
        const Int64 bufferSize = static_cast<Int64>(this->size());
        if(bufferSize == 0)
            return;

        static const Byte8 ChToHexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

        _This info;
        char cache[4] = {0};
        info.reserve(bufferSize * 2);
        for(Int64 i = 0; i < bufferSize; ++i)
        {
            auto &ch = (*this)[i];
            cache[0] = ChToHexChars[U8(ch) >> 4];
            cache[1] = ChToHexChars[ch & 0X0F];
            cache[2] = 0;

            info.append(cache, 2);
        }

        target += info;
    }

    void ToHexView(_This &target) const
    {
        const Int64 bufferSize = static_cast<Int64>(this->size());
        if(bufferSize == 0)
            return;

        target.reserve(target.size() + (bufferSize * 3));

        // 每16字节一行
        Byte8 cache[8] = { 0 };
        Int32 cacheLen = 0;
        for (Int64 i = 0; i < bufferSize; ++i)
        {
            cacheLen = ::sprintf(cache, "%02x%s"
                , static_cast<U8>((*this)[i]), ((i + 1) % 16 == 0) ? "\n" : " ");

            cache[cacheLen] = 0;
            target.append(cache, cacheLen);
        }
    }

    bool FromHexString(const _This &hexString)
    {
        const UInt64 hexLen = hexString.size();
        if(UNLIKELY(hexLen == 0 || ((hexLen % 2) != 0) ))
            return false;

        static const Byte8 *ChHexToDecimalValues = InitHexToDecimalValues();

        this->reserve(this->size() + hexLen / 2);
        for(UInt64 idx = 0; idx < hexLen; idx += 2)
        {
            const Int32 hiIdx = static_cast<Int32>(hexString[idx]);
            const Int32 loIdx = static_cast<Int32>(hexString[idx + 1]);
            auto &hi = ChHexToDecimalValues[hiIdx];
            auto &lo = ChHexToDecimalValues[loIdx];
            U8 decimalNumber = (hi << 4) | lo;
            this->append(reinterpret_cast<Byte8 *>(&decimalNumber), 1);
        }

        return true;
    }

    void Swap(_Base &str)
    {
        if(UNLIKELY(this == &str))
        {
            return;
        }

        this->swap(str);
    }

    void Swap(_This &&str)
    {
        if (UNLIKELY(this == &str))
            return;
            
        this->swap(std::forward<_Base>(str));
    }

    void Swap(_This &str)
    {
        if (UNLIKELY(this == &str))
            return;

        this->swap(str);
    }

    // 去除字符串末尾的\0压缩空间只对0结尾的字符串有效
    void CompressString()
    {
        const auto strSize = this->size();
        if(strSize == 0)
            return;

        auto len = strlen(this->c_str());
        if(strSize > len)
        {
            this->erase(len + 1, strSize - len - 1);
        }
    }

     // 移除容器中尾部的0
    const _This &RemoveZeroTail()
    {
        const Int64 bufferSize = static_cast<Int64>(this->size());
        if(bufferSize == 0)
            return *this;

        auto pos = this->find_first_of((const char)(0), 0);
        if(pos == npos)
            return *this;

        this->erase(pos, bufferSize - pos);
        return *this;
    }

    // 移除容器中首部的0
    const _This &RemoveHeadZero()
    {
        const Int64 bufferSize = static_cast<Int64>(this->size());
        if(bufferSize == 0)
            return *this;

        auto pos = this->find_first_not_of((const char)(0), 0);
        if(pos == npos)
            return *this;

        this->erase(0, pos);
        return *this;
    }

    // 如果是字符串的话会带很多的\0, 如果bitData有的话，这会造成再次Append的时候之后追加的字符串打印不出来
    _This &AppendData(const Byte8 *bitData, Int64 dataSize);

    _This &AppendData(const Byte8 *str)
    {
        return AppendData(str, static_cast<Int64>(strlen(str)));
    }
    
    _This &AppendData(const _This &data);
    _This &AppendFormat(const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3)
    {
        // if fmt args is null, return.
        if (UNLIKELY(!fmt))
            return *this;

        // try detach detach format require buffers and resize it.
        va_list va;
        const size_type oldSize = this->size();
        va_start(va, fmt);
        Int32 len =::vsnprintf(nullptr, 0, fmt, va);
        va_end(va);
        if (len <= 0)
            return *this;

        // exec format.
        this->resize(oldSize + len);
        va_start(va, fmt);
        len = ::vsnprintf(const_cast<Byte8 *>(this->data() + oldSize),
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

    _This &AppendFormatWithVaList(UInt64 finalFormatStrLen, const Byte8 *fmt, va_list va)
    {
        // if fmt args is null, return.
        if (UNLIKELY(!fmt || (finalFormatStrLen == 0)))
            return *this;

        // try detach detach format require buffers and resize it.
        const Int64 oldSize = static_cast<Int64>(this->size());

        // exec format.
        this->resize(static_cast<UInt64>(oldSize) + finalFormatStrLen);
        Int32 len = static_cast<Int32>(finalFormatStrLen);
        len = ::vsnprintf(const_cast<Byte8 *>(this->data() + oldSize),
                            len + 1,
                            fmt,
                            va);

        // len < 0 then _raw.size() - len > oldSize back to old string
        if (UNLIKELY(oldSize != static_cast<Int64>((this->size() - len))))
        {
            CRYSTAL_TRACE("wrong apend format and back to old string, oldSize:%llu, len:%d, new size:%llu", static_cast<UInt64>(oldSize), len, static_cast<UInt64>(this->size()));

            throw std::logic_error("rong apend format and back to old string");
            this->resize(oldSize);
        }

        return *this;
    }

    _This &AppendEnd();

    // 支持任意类型的追加只要对象重载<<即可
    template< typename... Args>
    _This &Append(Args&&... rest);
    // 当参数为0时会匹配非泛型版本
    _This &Append();

    _This &findreplace(const _Elem &dest, const _Elem &with, Int32 count = -1)
    {
        if (dest == with)
            return *this;

        Int32 founded = 0;
        for (size_type i = 0; i < this->size(); ++i)
        {
            if ((*this)[i] == dest)
            {
                ++founded;
                this->replace(i, 1, 1, with);

                if((count > 0) && (founded >= count))
                    break;
            }
        }

        return *this;
    }

    _This &findreplace(const _This &dest, const _This &with, Int32 count = -1)
    {
        if (dest == with)
            return *this;

        size_type found = 0;
        Int32 foundCount = 0;
        while ((found = this->find(dest, found)) != npos)
        {
            this->replace(found, dest.size(), with);
            found += with.size();
            ++foundCount;
            if((count > 0) && (foundCount >= count))
                break; 
        }

        return *this;
    }

    _This &findFirstAppendFormat(const _This &dest, const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(3, 4)
    {
        auto pos = this->find(dest);
        if(pos == npos)
            return *this;

        // try detach detach format require buffers and resize it.
        va_list va;
        va_start(va, fmt);
        Int32 len =::vsnprintf(nullptr, 0, fmt, va);
        va_end(va);
        if (len <= 0)
            return *this;

        _This cache;
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

        if(pos + 1 == this->size())
        {
            this->append(cache);
        }
        else
        {
            this->insert(pos + 1, cache);
        }

        return *this;
    }

    _This &EraseAnyOf(const _This &dest)
    {
        const UInt64 len = dest.length();
        UInt64 curPos = 0;
        for (UInt64 idx = 0; idx < len; ++idx)
        {
            do 
            {
                curPos = this->find(dest[idx]);
                if (curPos != npos)
                    this->erase(curPos, 1);
            } while (curPos != npos);
        }

        return *this;
    }

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
    _These Split(const _This &sep, size_type max_split = -1, bool onlyLikely = false, bool enableEmptyPart = true) const
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
        const UInt64 stepSize = onlyLikely ? 1 : sep.size();
        for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
        {
            size_type findIdx = npos;
            if(onlyLikely)
            {
                for(size_t i = 0; i < sep.size(); i++)
                {
                    findIdx = this->find(sep[i], idx);
                    if(findIdx != npos)
                        break;
                }
            }
            else
            {
                findIdx = this->find(sep, idx);
            }

            if(findIdx == npos)
                break;
		
            if (findIdx == idx)
            {
                if(enableEmptyPart)
                    substrs.push_back(_This());

                if ((idx = findIdx + stepSize) == this->size())
                {
                    if (enableEmptyPart)
                        substrs.push_back(_This());
                    break;
                }

                continue;
            }

            substrs.push_back(this->substr(idx, findIdx - idx));

            if((idx = findIdx + stepSize) == this->size())
            {
                if(enableEmptyPart)
                    substrs.push_back(_This());

                break;
            }
        }

        // 还有剩余
        if(idx != this->size())
        {
            const auto &subStr = this->substr(idx);
            if(!subStr.empty() || enableEmptyPart)
                substrs.push_back(subStr);
        }

        return substrs;
    }
   
    // @param(seps):切割关键字符串， 完全匹配这组字符串中的一个即可
    // @param(max_split):切割成多少份
    // @param(enableEmptyPart):允许空字符串
    _These Split(const _These &seps, size_type max_split = -1, bool enableEmptyPart = true) const
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
            size_type findIdx = npos;
            minIdx.clear();
            for(size_t i = 0; i < seps.size(); i++)
            {
                findIdx = this->find(seps[i], idx);
                if(findIdx != npos)
                    minIdx.insert(findIdx);
            }

            if(!minIdx.empty())
                findIdx = *minIdx.begin();

            if(findIdx == npos)
                break;

            substrs.push_back(substr(idx, findIdx - idx));
            if((idx = findIdx + 1) == this->size())
            {
                if(enableEmptyPart)
                    substrs.push_back(_This());
                break;
            }
        }

        if(idx != this->size())
        {
            const auto &subStr = this->substr(idx);
            if(!subStr.empty() || enableEmptyPart)
                substrs.push_back(subStr);
        }

        return substrs;
    }

    // strip operation: strip left. 去首部连续字符
    _This &lstrip(const _This &chars = _This())
    {
        _This willStripChars = chars;
        if (chars.empty())
        {
            willStripChars.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));
        }

        size_type stripTo = 0;
        const size_type thisSize = this->size();
        for (size_type i = 0; i < thisSize; ++i)
        {
            bool found = false;
            const _Elem &now = (*this)[i];
            for (size_type j = 0; j < willStripChars.size(); ++j)
            {
                if (now == willStripChars[j])
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
            this->erase(0, stripTo);

        return *this;
    }
   
    _This lstrip(const _This &chars = _This()) const;
    _This &lstripString(const _This &str)
    {
        if (str.empty())
        {
            return *this;
        }

        size_type stripTo = 0;
        const size_type thisSize = this->size();
        const size_type sliceSize = str.size();
        for (size_type i = 0; i < thisSize; i += 1)
        {
            if(sliceSize > (thisSize - i))
                break;

            auto pos = this->find(str, i);
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
            this->erase(0, stripTo);

        return *this;
    }
   
    _This lstripString(const _This &str) const;

    // strip operation: strip right. 去尾部连续字符
    _This &rstrip(const _This &chars = _This())
    {
        _This willStripChars = chars;
        if (chars.empty())
            willStripChars.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));

        const Int64 willStripRawSize = static_cast<Int64>(willStripChars.size());
        const Int64 thisSize = static_cast<Int64>(this->size());

        Int64 stripFrom = thisSize;
        for (Int64 i = thisSize - 1; i >= 0; --i)
        {
            bool found = false;
            const _Elem &now = (*this)[i];
            for (Int64 j = 0; j < willStripRawSize; ++j)
            {
                if (now == willStripChars[j])
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
            this->erase(stripFrom);

        return *this;
    }
    _This rstrip(const _This &chars = _This()) const;

    _This &rstripString(const _This &str)
    {
        if (str.empty())
        {
            return *this;
        }
	
        const Int64 thisSize = static_cast<Int64>(this->size());
        Int64 stripTo = thisSize;
        const Int64 sliceSize = static_cast<Int64>(str.size());
        for (Int64 i = thisSize - 1; i >= 0; i -= 1)
        {
            if(sliceSize > i + 1)
                break;

            auto pos = this->rfind(str, i);
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
            this->erase(stripTo);

        return *this;
    }
    
    _This rstripString(const _This &str) const;

    // strip operation: 去除首尾字符
    _This &strip(const _This &chars = _This());
    _This strip(const _This &chars = _This()) const;
    _This &stripString(const _This &str);
    _This stripString(const _This &str) const;

    _This DragAfter(const _This &start) const
    {
        auto pos =  this->find(start);
        if(pos == npos)
            return _This();
	
        return this->substr(pos + start.size());
    }
    
    _This DragAfter(const _This &start, size_t &startPos, size_t &endPos) const
    {
        auto pos =  this->find(start);
        if(pos == npos)
            return _This();

        endPos = pos + start.size() - 1;
        startPos = pos;
        const auto &subStr = this->substr(pos + start.size());
        endPos += subStr.size();
        return subStr;
    }

    _This DragBefore(const _This &start) const
    {
        auto pos =  this->find(start);
        if(pos == npos)
            return _This();

        return this->substr(0, pos);
    }
    
    _This DragRange(const _This &startStr, const _This &endStr) const;

    _This lsub(const _This &flagStr) const
    {
        auto pos = this->find(flagStr, 0);
        if(pos == npos)
            return _This();

        return this->substr(pos + flagStr.size());
    }
    
    _This rsub(const _This &flagStr) const
    {
        auto pos = this->rfind(flagStr);
        if(pos == npos)
            return _This();

        return this->substr(0, pos);
    }

    _This sub(const _This &leftStr, const _This &rightStr) const;

    // isalpha/isupper/islower 是否字母
    static bool isalpha(const char &c);
    static bool isalpha(const _This &s)
    {
        if(s.empty())
            return false;

        const size_type sSize = s.size();
        for(size_t i = 0; i < sSize; i++)
        {
            if(!isalpha(s[i]))
                return false;
        }

        return true;
    }
    
    bool isalpha() const;
    static bool islower(const char &c);
    static bool islower(const _This &s)
    {
        if(s.empty())
            return false;

        bool foundLower = false;
        const size_type sSize = s.size();
        for(size_type i = 0; i < sSize; ++i)
        {
            if(isupper(s[i]))
                return false;
            else if(islower(s[i]))
                foundLower = true;
        }

        return foundLower;
    }

    bool islower() const;
    static bool isupper(const char &c);
    static bool isupper(const _This &s)
    {
        if(s.empty())
            return false;

        bool foundUpper = false;
        const size_type sSize = s.size();
        for(size_type i = 0; i < sSize; ++i)
        {
            if(islower(s[i]))
                return false;
            else if(isupper(s[i]))
                foundUpper = true;
        }

        return foundUpper;
    }

    bool isupper() const;
    // 是否数值
    static bool isdigit(const char &c);
    static bool isdigit(const _This &s)
    {
        if(s.empty())
            return false;

        const size_type sSize = s.size();
        for(size_type i = 0; i < sSize; ++i)
        {
            if(!isdigit(s[i]))
                return false;
        }

        return true;
    }

    bool isdigit() const;
    // isspace: space[' '], carriage return['\r'], line feed['\n'], form feed['\f'], horizontal tab['\t'], vertical tab['\v']
    static bool isspace(const char &c);
    static bool isspace(const _This &s)
    {
        if(s.empty())
            return false;

        const size_type sSize = s.size();
        for(size_type i = 0; i < sSize; ++i)
        {
            if(!isspace(s[i]))
                return false;
        }

        return false;
    }
    bool isspace() const;

    // startswith/endswith
    bool IsStartsWith(const _This &s) const
    {
        if (s.empty())
            return true;

        return (this->size() >= s.size() && ::memcmp(s.data(), this->data(), s.size() * sizeof(_Elem)) == 0);
    }

    bool IsEndsWith(const _This &s) const
    {
        if (s.empty())
            return true;

        return (this->size() >= s.size() && 
            ::memcmp(s.data(), this->data() + (this->size() - s.size()) * sizeof(_Elem), s.size() * sizeof(_Elem)) == 0);
    }

    // start/end cut
    _This StartCut(const _This &startStr) const
    {
        auto pos = this->find(startStr, 0);
        if(pos == npos)
            return "";

        return this->substr(pos + startStr.size());
    }
    _This EndCut(const _This &endStr) const
    {
        auto pos = this->find(endStr, 0);
        if(pos == npos)
            return "";

        return this->substr(0, pos);
    }

    // tolower/toupper operations. 请确保是英文字符串 isalpha
    _This tolower() const
    {
        const _Elem *buf = this->data();
        const size_type size = this->size();
    
        _This lower;
        lower.resize(size);
        for (size_type i = 0; i < size; ++i)
        {
        	if (buf[i] >= 0x41 && buf[i] <= 0x5A)
        		lower[i] = buf[i] + 0x20;
        	else
        		lower[i] = buf[i];
        }
    
        return lower;
    }

    _This toupper() const
    {
        const _Elem *buf = this->data();
        const size_type size = this->size();

        _This upper;
        upper.resize(size);
        for (size_type i = 0; i < size; ++i)
            if (buf[i] >= 0x61 && buf[i] <= 0x7a)
                upper[i] = buf[i] - 0x20;
            else
                upper[i] = buf[i];

        return upper;
    }

    _This FirstCharToUpper() const
    {
        const _Elem *buf = this->data();
        const size_type size = this->size();
        if(size == 0)
            return *this;

        _This upper = *this;

        if (buf[0] >= 0x61 && buf[0] <= 0x7a)
            upper[0] = buf[0] - 0x20;

        return upper;
    }

    _This FirstCharToLower() const
    {
        const _Elem *buf = this->data();
        const size_type size = this->size();
        if(size == 0)
            return *this;

        _This lower = *this;

        if (buf[0] >= 0x41 && buf[0] <= 0x5A)
            lower[0] = buf[0] + 0x20;

        return lower; 
    }

    // escape support: escape string
    _This &escape(const _This &willbeEscapeChars, const _Elem &escapeChar)
    {
        if (this->empty())
            return *this;

        const long len = static_cast<long>(this->size());
        for (long i = len - 1; i >= 0; --i)
        {
            const _Elem &ch = (*this)[i];
            if (ch == escapeChar ||
                willbeEscapeChars.find(ch) != npos)
                insert(i, 1, escapeChar);
        }

        return *this;
    }

    _This escape(const _This &willbeEscapeChars, const _Elem &escapeChar) const
    {
        if (this->empty())
            return *this;

        return _This(*this).escape(willbeEscapeChars, escapeChar);
    }

    // escape support: unescape string
    _This &unescape(const _Elem &escapeChar)
    {
        if (this->empty())
            return *this;

        const Int64 len = static_cast<Int64>(this->size());
        for (Int64 i = len - 1; i >= 0; --i)
        {
            const _Elem &ch = (*this)[i];
            if (ch == escapeChar)
            {
                if (i > 0 && (*this)[i - 1] == escapeChar)
                    this->erase(i--, 1);
                else
                    this->erase(i, 1);
            }
        }

        return *this;
    }

    _This unescape(const _Elem &escapeChar) const;

    // UTF8 Surport
    // 是否utf8判断(只对不带bom utf8判断, 如果带bom请移除后判断)
    bool IsUtf8() const
    {
        UInt64 count = static_cast<UInt64>(this->length());
        UInt64 loop = 0;
        while(count > 0)
        {
            U8 ctrl = (*this)[loop];

            auto bytesNum = BasicStringHelper::CalcUtf8CharBytes(ctrl);
            if(bytesNum == 0)
                break;

            ++loop;
            --count;
            --bytesNum;

            // 校验除控制位以外的数据位开头必须是10
            for(UInt64 idx = 0; idx < bytesNum; ++idx)
            {
                U8 data = (*this)[loop + idx];
                if(((data >> 6) ^ 0x02) != 0)
                    return false;
            }
		
            loop += (bytesNum);
            count -= bytesNum;
        }

        return count == 0;
    }
    // 添加bomb
    void add_utf8_bomb();
    // utf8字符个数
    size_type length_with_utf8() const;
    // 截取utf8字符串pos是按照utf8字符算的（不是按照字节）
    _This substr_with_utf8(size_type pos = 0, size_type n = npos) const
    {
        size_type utf8Len = this->length_with_utf8();
        if (pos >= utf8Len || n == 0)
            return _This();

        _These substrs;
        this->split_utf8_string(pos, substrs);
        if (substrs.empty())
            return _This();

        _This str1 = *substrs.rbegin();
        utf8Len = str1.length_with_utf8();
        pos = (n == npos || n > utf8Len) ? utf8Len : n;

        substrs.clear();
        str1.split_utf8_string(pos, substrs);
        if (substrs.empty())
            return _This();

        return substrs[0];
    }

    // 从第charIndex个字符拆分成两个utf8字符串（前charIndex个utf8字符在strs的第一个）
    void split_utf8_string(size_type charIndex, _These &strs) const
    {
        strs.clear();
        if (charIndex == 0)
        {
            strs.push_back(*this);
            return;
        }

        size_type utf8Count = _This::length_with_utf8();
        if (UNLIKELY(utf8Count ==  npos))
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
            bytePos = _This::_next_utf8_char_pos(bytePos);
            ++charPos;
        }

        strs.push_back(this->substr(0, bytePos));
        strs.push_back(this->substr(bytePos));
    }

    // 打散ut8f字符串 scatterCount:按顺序打散n次，生产scatterCount个utf8字符串
    void scatter_utf8_string(_These &chars, size_type scatterCount = 0) const
    {
        chars.clear();

        if (scatterCount == 0)
            scatterCount = npos;
        else if (scatterCount != npos)
            scatterCount -= 1;

        if (scatterCount == 0)
        {
            chars.push_back(*this);
            return;
        }

        size_type curPos = 0;
        size_type prevPos = 0;
        size_type curScatterCount = 0;
        while ((curPos = this->_next_utf8_char_pos(prevPos)) != npos)
        {
            chars.push_back(this->substr(prevPos, curPos - prevPos));

            if (scatterCount != npos && ++curScatterCount >= scatterCount)
            {
                chars.push_back(this->substr(curPos));
                break;
            }

            prevPos = curPos;
        }
    }

    // utf8有没带bomb（windows下utf8识别带bomb（字符串前三个字节：\xef\xbb\xbf）一般在处理文件时用到）
    bool has_utf8_bomb() const;
    // 移除bomb
    void remove_utf8_bomb();


private:
    // 下一个utf8字符索引pos
    size_type _next_utf8_char_pos(size_type &beginBytePos) const
    {
        if (beginBytePos == 0 && this->has_utf8_bomb())
            beginBytePos += 3;

        if (beginBytePos == npos || beginBytePos >= this->size())
            return npos;

        size_type waitCheckCount = npos;

        // 0xxx xxxx
        // Encoding len: 1 byte.
        U8 ch = static_cast<U8>(at(beginBytePos));
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

        if (waitCheckCount == npos)
            return npos;

        size_type curPos = beginBytePos + 1;
        size_type endPos = curPos + waitCheckCount;
        if (endPos > this->size())
            return npos;

        for (; curPos != endPos; ++curPos)
        {
            ch = static_cast<U8>(this->at(curPos));
            if ((ch & 0xc0) != 0x80)
                return npos;
        }

        return endPos;
    }
    
    // hex=>decimal
    U8 _TurnDecimal(const Byte8 hexChar)
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
};

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE LibBasicString<_Elem, _Traits, _Ax>::_Base &LibBasicString<_Elem, _Traits, _Ax>::GetRaw()
{
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE const LibBasicString<_Elem, _Traits, _Ax>::_Base & LibBasicString<_Elem, _Traits, _Ax>::GetRaw() const
{
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  bool LibBasicString<_Elem, _Traits, _Ax>::Contain(const _This &piece) const
{
    return this->find(piece) != npos;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::ToHexString() const
{
    _This info;
    ToHexString(info);
    return info;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::ToString() const
{
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> &LibBasicString<_Elem, _Traits, _Ax>::AppendData(const Byte8 *bitData, Int64 dataSize)
{
    if(UNLIKELY(!bitData))
        return *this;

    this->append(bitData, dataSize);
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> &LibBasicString<_Elem, _Traits, _Ax>::AppendData(const _This &data)
{
    this->append(data);
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE LibBasicString<_Elem, _Traits, _Ax> &LibBasicString<_Elem, _Traits, _Ax>::AppendEnd()
{
    *this += endl;
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
template< typename... Args>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> &LibBasicString<_Elem, _Traits, _Ax>::Append(Args&&... rest)
{
    // 使用数组展开参数包，并同时调用<<
    char _[] = { ((*this) << rest, char(0))... };
    UNUSED(_);
    
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> &LibBasicString<_Elem, _Traits, _Ax>::Append()
{
    return *this;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax>::_These LibBasicString<_Elem, _Traits, _Ax>::Split(Byte8 sep, size_type max_split, bool enableEmptyPart) const
{
    return this->Split(_This(sep), max_split, false, enableEmptyPart);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE LibBasicString<_Elem, _Traits, _Ax>::_These LibBasicString<_Elem, _Traits, _Ax>::Split(const Byte8 *sep, size_type max_split, bool onlyLikely, bool enableEmptyPart) const
{
    return this->Split(_This(sep), max_split, onlyLikely, enableEmptyPart);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::lstrip(const _This &chars) const
{
    _This copyThis(*this);
    return copyThis.lstrip(chars);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::lstripString(const _This &str) const
{
    auto copyThis = *this;
    return copyThis.lstripString(str);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::rstrip(const _This &chars) const
{
    _This copyThis(*this);
    return copyThis.rstrip(chars);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::rstripString(const _This &str) const
{
    auto copyThis = *this;
    return copyThis.rstripString(str);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> &LibBasicString<_Elem, _Traits, _Ax>::strip(const _This &chars)
{
    return this->lstrip(chars).rstrip(chars);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::strip(const _This &chars) const
{
    _This copyThis(*this);
    return copyThis.lstrip(chars).rstrip(chars);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> &LibBasicString<_Elem, _Traits, _Ax>::stripString(const _This &str)
{
    return this->lstripString(str).rstripString(str);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::stripString(const _This &str) const
{
    _This copyThis(*this);
    return copyThis.lstripString(str).rstripString(str);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isalpha(const char &c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::DragRange(const _This &startStr, const _This &endStr) const
{
    const auto &cache = DragAfter(startStr);
    return cache.DragBefore(endStr);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::sub(const _This &leftStr, const _This &rightStr) const
{
    return lsub(leftStr).rsub(rightStr);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isalpha() const
{
    return isalpha(*this);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::islower(const char &c)
{
    return 'a' <= c && c <= 'z';
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::islower() const
{
    return islower(*this);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isupper(const char &c)
{
    return 'A' <= c && c <= 'Z';
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isupper() const
{
    return isupper(*this);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isdigit(const char &c)
{
    return '0' <= c && c <= '9';
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isdigit() const
{
    return isdigit(*this);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isspace(const char &c)
{
    return  c == ' ' || c == '\t' || c == '\v' || c == '\r' || c == '\n' || c == '\f';
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::isspace() const
{
    return isspace(*this);
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> LibBasicString<_Elem, _Traits, _Ax>::unescape(const _Elem &escapeChar) const
{
    return _This(*this).unescape(escapeChar);   
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE void LibBasicString<_Elem, _Traits, _Ax>::add_utf8_bomb()
{
    if (!this->has_utf8_bomb())
        insert(0, reinterpret_cast<const _Elem *>("\xef\xbb\xbf"));
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax>::size_type LibBasicString<_Elem, _Traits, _Ax>::length_with_utf8() const
{
    size_type count = 0;
    size_type bytePos = 0;
    while ((bytePos = _This::_next_utf8_char_pos(bytePos)) != _Base::npos)
        ++count;

    return count;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE bool LibBasicString<_Elem, _Traits, _Ax>::has_utf8_bomb() const
{
    if (this->size() < 3)
        return false;
    
    return (::memcmp(reinterpret_cast<const Byte8 *>(this->data()), 
        reinterpret_cast<const char *>("\xef\xbb\xbf"), 3) == 0) ? true : false;
}

template <typename _Elem,
          typename _Traits,
          typename _Ax>
ALWAYS_INLINE void LibBasicString<_Elem, _Traits, _Ax>::remove_utf8_bomb()
{
    if (this->has_utf8_bomb())
        this->erase(0, 3);
}

template <typename _Elem,
          typename _Traits = std::char_traits<_Elem>,
          typename _Ax = std::allocator<_Elem> >
ALWAYS_INLINE  LibBasicString<_Elem, _Traits, _Ax> &KernelAppendFormat(LibBasicString<_Elem, _Traits, _Ax> &o, const Byte8 *fmt, ...) LIB_KERNEL_FORMAT_CHECK(2, 3)
{
    va_list va;
    va_start(va, fmt);
    auto fmtSize = o.CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    o.AppendFormatWithVaList(fmtSize, fmt, va);
    va_end(va);
    
    return o;
}

KERNEL_END

// 重载运算符实现 _Base += LibBasicString
// extern KERNEL_EXPORT ALWAYS_INLINE _Base &operator +=(_Base &o, const KERNEL_NS::_This &input)
// {
//     o.append(input);
//     return o;
// }

// 重载运算符实现 _Base + LibBasicString
// extern KERNEL_EXPORT ALWAYS_INLINE _Base operator +(const _Base &o, const KERNEL_NS::_This &input)
// {
//     return o + input;
// }

// 重载运算符实现 _Base += LibBasicString
// extern KERNEL_EXPORT ALWAYS_INLINE _Base &operator +=(_Base &o, KERNEL_NS::_This &&input)
// {
//     o.append(input);
//     return o;
// }

// 重载运算符实现 _Base + LibBasicString
// extern KERNEL_EXPORT ALWAYS_INLINE _Base operator +(const _Base &o, KERNEL_NS::_This &&input)
// {
//     return o + input;
// }


#endif