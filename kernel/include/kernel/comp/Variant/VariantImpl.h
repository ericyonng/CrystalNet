/*!
 *  MIT License
 *  
 *  Copyright (c) 2020 ericyonng<120453674@qq.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 * 
 * Date: 2022-01-29 16:59:30
 * Author: Eric Yonng
 * Description: 
*/

#ifdef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_VARIANT_VARIANT_H__

#pragma once

#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/Variant/VariantArithmetic.h>
#include <kernel/comp/Variant/VariantTraits.h>
#include <kernel/comp/LibStream.h>


ALWAYS_INLINE std::ostream &operator <<(std::ostream &o, const KERNEL_NS::Variant &var)
{
    const KERNEL_NS::LibString &str = var.ToString();
    o.write(str.c_str(), str.length());
    return o;
}

ALWAYS_INLINE std::string &operator <<(std::string &o, const KERNEL_NS::Variant &var)
{
    const KERNEL_NS::LibString &str = var.ToString();
    o.append(str.GetRaw());
    return o;
}

ALWAYS_INLINE KERNEL_NS::LibString &operator <<(KERNEL_NS::LibString &o, const KERNEL_NS::Variant &var)
{
    o << var.ToString();
    return o;
}

KERNEL_BEGIN


ALWAYS_INLINE Variant::Variant()
{

}

ALWAYS_INLINE Variant::Variant(const Byte8 *cstrVal)
{
    _raw._type = VariantRtti::VT_STRING_DEF;
    if(LIKELY(cstrVal != NULL))
    {
        auto strLen = ::strlen(cstrVal);
        if(LIKELY(strLen != 0))
        {
            if(!_raw._obj._strData)
                _raw._obj._strData = CRYSTAL_NEW(LibString);

            *_raw._obj._strData = cstrVal;
        }
    }
}

ALWAYS_INLINE Variant::Variant(const bool &boolVal)
{
    _raw._type = VariantRtti::VT_BRIEF_BOOL;
    _raw._briefData._int64Data = boolVal ? 1 : 0;
}

ALWAYS_INLINE Variant::Variant(const Byte8 &byte8Val)
{
    _raw._type = VariantRtti::VT_BRIEF_BYTE8;
    _raw._briefData._int64Data = static_cast<Int64>(byte8Val);
}

ALWAYS_INLINE Variant::Variant(const U8 &uint8Val)
{
    _raw._type = VariantRtti::VT_BRIEF_UINT8;
    _raw._briefData._uint64Data = static_cast<UInt64>(uint8Val);
}

ALWAYS_INLINE Variant::Variant(const Int16 &int16Val)
{
    _raw._type = VariantRtti::VT_BRIEF_INT16;
    _raw._briefData._int64Data = static_cast<Int64>(int16Val);
}

ALWAYS_INLINE Variant::Variant(const UInt16 &uint16Val)
{
    _raw._type = VariantRtti::VT_BRIEF_UINT16;
    _raw._briefData._uint64Data = static_cast<UInt64>(uint16Val);
}

ALWAYS_INLINE Variant::Variant(const Int32 &int32Val)
{
    _raw._type = VariantRtti::VT_BRIEF_INT32;
    _raw._briefData._int64Data = static_cast<Int64>(int32Val);
}

ALWAYS_INLINE Variant::Variant(const UInt32 &uint32Val)
{
    _raw._type = VariantRtti::VT_BRIEF_UINT32;
    _raw._briefData._uint64Data = static_cast<UInt64>(uint32Val);
}

ALWAYS_INLINE Variant::Variant(const Long &longVal)
{
    _raw._type = VariantRtti::VT_BRIEF_LONG;
    _raw._briefData._int64Data = static_cast<Int64>(longVal);
}

ALWAYS_INLINE Variant::Variant(const ULong &ulongVal)
{
    _raw._type = VariantRtti::VT_BRIEF_ULONG;
    _raw._briefData._uint64Data = static_cast<UInt64>(ulongVal);
}

template <typename _T>
ALWAYS_INLINE Variant::Variant(const _T * const &ptrVal)
{
    _raw._type = VariantRtti::VT_BRIEF_PTR;
    _raw._briefData._uint64Data = 0;
    ::memcpy(&_raw._briefData._uint64Data, &ptrVal, sizeof(_T *));
}

ALWAYS_INLINE Variant::Variant(const Int64 &int64Val)
{
    _raw._type = VariantRtti::VT_BRIEF_INT64;
    _raw._briefData._int64Data = static_cast<Int64>(int64Val);
}

ALWAYS_INLINE Variant::Variant(const UInt64 &uint64Val)
{
    _raw._type = VariantRtti::VT_BRIEF_UINT64;
    _raw._briefData._uint64Data = static_cast<UInt64>(uint64Val);
}

ALWAYS_INLINE Variant::Variant(const Float &floatVal)
{
    _raw._type = VariantRtti::VT_BRIEF_FLOAT;
    _raw._briefData._doubleData = static_cast<Double>(floatVal);
}

ALWAYS_INLINE Variant::Variant(const Double &doubleVal)
{
    _raw._type = VariantRtti::VT_BRIEF_DOUBLE;
    _raw._briefData._doubleData = static_cast<Double>(doubleVal);
}

ALWAYS_INLINE Variant::Variant(const std::string &strVal)
{
    _raw._type = VariantRtti::VT_STRING_DEF;
    _raw._obj._strData = CRYSTAL_NEW(LibString)(strVal);
}

ALWAYS_INLINE Variant::Variant(const LibString &strVal)
{
    _raw._type = VariantRtti::VT_STRING_DEF;
    _raw._obj._strData = CRYSTAL_NEW(LibString)(strVal);
}

ALWAYS_INLINE Variant::Variant(const Variant::Dict &dictVal)
{
    _raw._type = VariantRtti::VT_DICTIONARY_DEF;
    _raw._obj._dictData = CRYSTAL_NEW(Variant::Dict);
    auto &dictData = *_raw._obj._dictData;
    for(auto iter = dictVal.begin(); iter != dictVal.end(); ++iter)
        dictData.emplace(Variant(iter->first), Variant(iter->second));
}

ALWAYS_INLINE Variant::Variant(const Variant &varVal)
:_raw(varVal._raw)
{

}

ALWAYS_INLINE Variant::Variant(Variant &&varVal)
:_raw(std::move(varVal._raw))
{
    // varVal._raw._type = VariantRtti::VT_NIL;
    // varVal._raw._obj._strData = NULL;
    // varVal._raw._briefData._uint64Data = 0;
}

template<typename _Key, typename _Val>
ALWAYS_INLINE Variant::Variant(const std::map<_Key, _Val> &dictVal)
{
    _raw._type = VariantRtti::VT_DICTIONARY_DEF;
    _raw._obj._dictData = CRYSTAL_NEW(Dict);
    if(LIKELY(!dictVal.empty()))
    {
        auto dictData = _raw._obj._dictData;
        for(auto iter = dictVal.begin(); iter != dictVal.end(); ++iter)
            dictData->emplace(Variant(iter->first), Variant(iter->second));
    }
}

template<typename _Key, typename _Val>
ALWAYS_INLINE Variant::Variant(const std::unordered_map<_Key, _Val> &unorderedDictVal)
{
    _raw._type = VariantRtti::VT_DICTIONARY_DEF;
    auto newDict = CRYSTAL_NEW(Dict);
    _raw._obj._dictData = newDict;
    if(LIKELY(!unorderedDictVal.empty()))
    {
        for(auto iter = unorderedDictVal.begin(); iter != unorderedDictVal.end(); ++iter)
            newDict->emplace(Variant(iter->first), Variant(iter->second));
    }
}

template<typename _Val>
ALWAYS_INLINE Variant::Variant(const std::set<_Val> &setVal)
{
    _raw._type = VariantRtti::VT_SEQUENCE_DEF;
    auto newData = CRYSTAL_NEW(Sequence);
    _raw._obj._sequenceData = newData;
    if(LIKELY(!setVal.empty()))
    {
        newData->reserve(setVal.size());
        for(auto iter = setVal.begin(); iter != setVal.end(); ++iter)
            newData->emplace_back(Variant(*iter));
    }
}

template<typename _Val>
ALWAYS_INLINE Variant::Variant(const std::unordered_set<_Val> &unorderedSetVal)
{
    _raw._type = VariantRtti::VT_SEQUENCE_DEF;
    auto newData = CRYSTAL_NEW(Sequence);
    _raw._obj._sequenceData = newData;
    if(LIKELY(!unorderedSetVal.empty()))
    {
        newData->reserve(unorderedSetVal.size());
        for(auto iter = unorderedSetVal.begin(); iter != unorderedSetVal.end(); ++iter)
            newData->emplace_back(Variant(*iter));
    }
}

template<typename _Val>
ALWAYS_INLINE Variant::Variant(const std::vector<_Val> &seqVal)
{
    _raw._type = VariantRtti::VT_SEQUENCE_DEF;
    auto newData = CRYSTAL_NEW(Sequence);
    _raw._obj._sequenceData = newData;
    if(LIKELY(!seqVal.empty()))
    {
        newData->reserve(seqVal.size());
        for(auto iter = seqVal.begin(); iter != seqVal.end(); ++iter)
            newData->emplace_back(Variant(*iter));
    }
}

template<typename _Val>
ALWAYS_INLINE Variant::Variant(const std::list<_Val> &listVal)
{
    _raw._type = VariantRtti::VT_SEQUENCE_DEF;
    auto newData = CRYSTAL_NEW(Sequence);
    _raw._obj._sequenceData = newData;
    if(LIKELY(!listVal.empty()))
    {
        newData->reserve(listVal.size());
        for(auto iter = listVal.begin(); iter != listVal.end(); ++iter)
            newData->emplace_back(Variant(*iter));
    }
}

template<typename _Val>
ALWAYS_INLINE Variant::Variant(const std::deque<_Val> &dequeVal)
{
    _raw._type = VariantRtti::VT_SEQUENCE_DEF;
    auto newData = CRYSTAL_NEW(Sequence);
    _raw._obj._sequenceData = newData;
    if(LIKELY(!dequeVal.empty()))
    {
        newData->reserve(dequeVal.size());
        for(auto iter = dequeVal.begin(); iter != dequeVal.end(); ++iter)
            newData->emplace_back(Variant(*iter));
    }
}

template<typename _Val>
ALWAYS_INLINE Variant::Variant(const std::queue<_Val> &queueVal)
{
    _raw._type = VariantRtti::VT_SEQUENCE_DEF;
    auto newData = CRYSTAL_NEW(Sequence);
    _raw._obj._sequenceData = newData;
    if(LIKELY(!queueVal.empty()))
    {
        newData->reserve(queueVal.size());
        for(auto iter = queueVal.begin(); iter != queueVal.end(); ++iter)
            newData->emplace_back(Variant(*iter));
    }
}

// Fetch Variant data type and raw data.
ALWAYS_INLINE UInt64 Variant::GetType() const
{
    return _raw._type;
}

ALWAYS_INLINE UInt64 Variant::GetMainType() const
{
    return (_raw._type & VariantRtti::VT_TYPEINFO_MASK);
}

ALWAYS_INLINE const typename Variant::Raw &Variant::GetRaw() const
{
    return _raw;
}

ALWAYS_INLINE void Variant::Reset()
{
    _raw.Reset();
}

ALWAYS_INLINE void Variant::Clear()
{
    if(IsStr())
        _raw._obj._strData->clear();
    else if(IsDict())
        _raw._obj._dictData->clear();
    else if(IsSeq())
        _raw._obj._sequenceData->clear();
    else if(IsBriefData())
        _raw._briefData._uint64Data = 0;
}

ALWAYS_INLINE bool Variant::IsEmpty() const
{
    if(IsStr())
        return _raw._obj._strData->empty();
    else if(IsDict())
        return _raw._obj._dictData->empty();
    else if(IsSeq())
        return _raw._obj._sequenceData->empty();
    else if(IsBriefData())
        return false;

    return true;
}

ALWAYS_INLINE UInt64 Variant::GetCount() const
{
    if(IsStr())
        return _raw._obj._strData->size();
    else if(IsDict())
        return _raw._obj._dictData->size();
    else if(IsSeq())
        return _raw._obj._sequenceData->size();
    else if(IsBriefData())
        return 1;

    return 0;
}

ALWAYS_INLINE UInt64 Variant::GetCapacity() const
{
    if(IsStr())
        return _raw._obj._strData->GetRaw().capacity();
    else if(IsDict())
        return _raw._obj._dictData->size();
    else if(IsSeq())
        return _raw._obj._sequenceData->capacity();
    else if(IsBriefData())
        return 1;

    return 0;  
}

ALWAYS_INLINE Variant &Variant::MoveFrom(Variant &other)
{
    _raw._MoveInAssign(other._raw);

    return *this;
}

ALWAYS_INLINE bool Variant::IsNil() const
{
    return _raw._type == VariantRtti::VT_NIL;
}

ALWAYS_INLINE bool Variant::IsBriefData() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_DATA) == VariantRtti::VT_BRIEF_DATA;
}

ALWAYS_INLINE bool Variant::IsSignedBriefData() const
{
    return (IsBriefData() && ((_raw._type & VariantRtti::VT_SIGHED) == VariantRtti::VT_SIGHED));
}

ALWAYS_INLINE bool Variant::IsUnsignedBriefData() const
{
    return (IsBriefData() && ((_raw._type & VariantRtti::VT_UNSIGHED) == VariantRtti::VT_UNSIGHED));
}

ALWAYS_INLINE bool Variant::IsBool() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_BOOL) == VariantRtti::VT_BRIEF_BOOL;
}

ALWAYS_INLINE bool Variant::IsByte8() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_BYTE8) == VariantRtti::VT_BRIEF_BYTE8;
}

ALWAYS_INLINE bool Variant::IsUInt8() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_UINT8) == VariantRtti::VT_BRIEF_UINT8;
}

ALWAYS_INLINE bool Variant::IsInt16() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_INT16) == VariantRtti::VT_BRIEF_INT16;
}

ALWAYS_INLINE bool Variant::IsUInt16() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_UINT16) == VariantRtti::VT_BRIEF_UINT16;
}

ALWAYS_INLINE bool Variant::IsInt32() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_INT32) == VariantRtti::VT_BRIEF_INT32;
}

ALWAYS_INLINE bool Variant::IsUInt32() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_UINT32) == VariantRtti::VT_BRIEF_UINT32;
}

ALWAYS_INLINE bool Variant::IsLong() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_LONG) == VariantRtti::VT_BRIEF_LONG;
}

ALWAYS_INLINE bool Variant::IsULong() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_ULONG) == VariantRtti::VT_BRIEF_ULONG;
}

ALWAYS_INLINE bool Variant::IsPtr() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_PTR) == VariantRtti::VT_BRIEF_PTR;
}

ALWAYS_INLINE bool Variant::IsInt64() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_INT64) == VariantRtti::VT_BRIEF_INT64;
}

ALWAYS_INLINE bool Variant::IsUInt64() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_UINT64) == VariantRtti::VT_BRIEF_UINT64;
}

ALWAYS_INLINE bool Variant::IsFloat() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_FLOAT) == VariantRtti::VT_BRIEF_FLOAT;
}

ALWAYS_INLINE bool Variant::IsDouble() const
{
    return (_raw._type & VariantRtti::VT_BRIEF_DOUBLE) == VariantRtti::VT_BRIEF_DOUBLE;
}

ALWAYS_INLINE bool Variant::IsStr() const
{
    return (_raw._type & VariantRtti::VT_STRING_DEF) == VariantRtti::VT_STRING_DEF;
}

ALWAYS_INLINE bool Variant::IsDict() const
{
    return (_raw._type & VariantRtti::VT_DICTIONARY_DEF) == VariantRtti::VT_DICTIONARY_DEF;
}

ALWAYS_INLINE bool Variant::IsSeq() const
{
    return (_raw._type & VariantRtti::VT_SEQUENCE_DEF) == VariantRtti::VT_SEQUENCE_DEF;
}

ALWAYS_INLINE bool Variant::IsVariantType(UInt64 type) const
{
    return (_raw._type & type) == type;
}

ALWAYS_INLINE bool Variant::IsMainType(UInt64 type) const
{
    return GetMainType() == type;
}

ALWAYS_INLINE Variant &Variant::BecomeNil()
{
    return Become<VariantRtti::VT_NIL>();
}

ALWAYS_INLINE Variant &Variant::BecomeBool()
{
    return Become<VariantRtti::VT_BRIEF_BOOL>();
}

ALWAYS_INLINE Variant &Variant::BecomeByte8()
{
    return Become<VariantRtti::VT_BRIEF_BYTE8>();
}

ALWAYS_INLINE Variant &Variant::BecomeUInt8()
{
    return Become<VariantRtti::VT_BRIEF_UINT8>();
}

ALWAYS_INLINE Variant &Variant::BecomeInt16()
{
    return Become<VariantRtti::VT_BRIEF_INT16>();
}

ALWAYS_INLINE Variant &Variant::BecomeUInt16()
{
    return Become<VariantRtti::VT_BRIEF_UINT16>();
}

ALWAYS_INLINE Variant &Variant::BecomeInt32()
{
    return Become<VariantRtti::VT_BRIEF_INT32>();
}

ALWAYS_INLINE Variant &Variant::BecomeUInt32()
{
    return Become<VariantRtti::VT_BRIEF_UINT32>();
}

ALWAYS_INLINE Variant &Variant::BecomeLong()
{
    return Become<VariantRtti::VT_BRIEF_LONG>();
}

ALWAYS_INLINE Variant &Variant::BecomeULong()
{
    return Become<VariantRtti::VT_BRIEF_ULONG>();
}

ALWAYS_INLINE Variant &Variant::BecomePtr()
{
    return Become<VariantRtti::VT_BRIEF_PTR>();
}

ALWAYS_INLINE Variant &Variant::BecomeInt64()
{
    return Become<VariantRtti::VT_BRIEF_INT64>();
}

ALWAYS_INLINE Variant &Variant::BecomeUInt64()
{
    return Become<VariantRtti::VT_BRIEF_UINT64>();
}

ALWAYS_INLINE Variant &Variant::BecomeFloat()
{
    return Become<VariantRtti::VT_BRIEF_FLOAT>();
}

ALWAYS_INLINE Variant &Variant::BecomeDouble()
{
    return Become<VariantRtti::VT_BRIEF_DOUBLE>();
}

ALWAYS_INLINE Variant &Variant::BecomeStr()
{
    return Become<VariantRtti::VT_STRING_DEF>();
}

ALWAYS_INLINE Variant &Variant::BecomeDict()
{
    return Become<VariantRtti::VT_DICTIONARY_DEF>();
}

ALWAYS_INLINE Variant &Variant::BecomeSeq()
{
    return Become<VariantRtti::VT_SEQUENCE_DEF>();
}

template<VariantRtti::RttiType _Ty>
ALWAYS_INLINE Variant &Variant::Become()
{
    if(!IsVariantType(_Ty))
    {
        _CleanTypeData(_raw._type);
        _BecomeSpecify<_Ty>();
    }

    return *this;
}

ALWAYS_INLINE bool Variant::AsBool() const
{
    if(IsBriefData())
    {
        // 0值比较
        if(IsFloat() || IsDouble())
            return std::fabs(_raw._briefData._doubleData) > DBL_EPSILON;

        return _raw._briefData._uint64Data != 0;
    }

    if(IsNil())
        return false;

    if(IsDict())
        return !(_raw._obj._dictData->empty());

    if(IsSeq())
        return !(_raw._obj._sequenceData->empty());

    if(IsStr())
    {// 数值型,与true false需要进行转换
        return !(_raw._obj._strData->empty());
    }

    return false;
}

ALWAYS_INLINE Byte8 Variant::AsByte8() const
{
    return static_cast<Byte8>(AsInt64());
}

ALWAYS_INLINE U8 Variant::AsUInt8() const
{
    return static_cast<U8>(AsUInt64());
}

ALWAYS_INLINE Int16 Variant::AsInt16() const
{
    return static_cast<Int16>(AsInt64());
}

ALWAYS_INLINE UInt16 Variant::AsUInt16() const
{
    return static_cast<UInt16>(AsUInt64());
}

ALWAYS_INLINE Int32 Variant::AsInt32() const
{
    return static_cast<Int32>(AsInt64());
}

ALWAYS_INLINE UInt32 Variant::AsUInt32() const
{
    return static_cast<UInt32>(AsUInt64());
}

ALWAYS_INLINE Long Variant::AsLong() const
{
    return static_cast<Long>(AsInt64());
}

ALWAYS_INLINE ULong Variant::AsULong() const
{
    return static_cast<ULong>(AsUInt64());
}

template <typename _T>
ALWAYS_INLINE _T *Variant::AsPtr() const
{
    return reinterpret_cast<_T *>(AsUInt64());
}

ALWAYS_INLINE Float Variant::AsFloat() const
{
    return static_cast<Float>(AsDouble());
}

ALWAYS_INLINE const typename Variant::Dict &Variant::AsDict() const
{
    if(IsDict() && _raw._obj._dictData)
        return *_raw._obj._dictData;

    return VariantAssist::GetNullDict<Variant>();
}

ALWAYS_INLINE const typename Variant::Sequence &Variant::AsSequence() const
{
    if(IsSeq() && _raw._obj._sequenceData)
        return *_raw._obj._sequenceData;

    return VariantAssist::GetNullSequence<Variant>();
}

ALWAYS_INLINE Variant::operator bool() const
{
    return AsBool();
}

ALWAYS_INLINE Variant::operator Byte8 () const
{
    return AsByte8();
}

ALWAYS_INLINE Variant::operator U8 () const
{
    return AsUInt8();
}

ALWAYS_INLINE Variant::operator Int16 () const
{
    return AsInt16();
}

ALWAYS_INLINE Variant::operator UInt16 () const
{
    return AsUInt16();
}

ALWAYS_INLINE Variant::operator Int32 () const
{
    return AsInt32();
}

ALWAYS_INLINE Variant::operator UInt32 () const
{
    return AsUInt32();
}

ALWAYS_INLINE Variant::operator Long() const
{
    return AsLong();
}

ALWAYS_INLINE Variant::operator ULong () const
{
    return AsULong();
}

template <typename _T>
ALWAYS_INLINE Variant::operator _T * () const
{
    return AsPtr<_T>();
}

ALWAYS_INLINE Variant::operator Int64 () const
{
    return AsInt64();
}

ALWAYS_INLINE Variant::operator UInt64 () const
{
    return AsUInt64();
}

ALWAYS_INLINE Variant::operator Float() const
{
    return AsFloat();
}

ALWAYS_INLINE Variant::operator Double() const
{
    return AsDouble();
}

ALWAYS_INLINE Variant::operator LibString () const
{
    return AsStr();
}

ALWAYS_INLINE Variant::operator std::string() const
{
    return AsStr().GetRaw();
}

ALWAYS_INLINE Variant::operator const typename Variant::Dict &() const
{
    return AsDict();
}

template<typename _Key, typename _Val>
ALWAYS_INLINE Variant::operator std::map<_Key, _Val> () const
{
    std::map<_Key, _Val> m;
    if(IsDict())
    {
        auto &dictData = _raw._obj._dictData;
        auto iterEnd = dictData->end();
        for(auto iter = dictData->begin(); iter != iterEnd; ++iter)
            m.emplace(Variant(iter->first), Variant(iter->second));
    }
    
    return m;   
}

template<typename _Key, typename _Val>
ALWAYS_INLINE Variant::operator std::unordered_map<_Key, _Val> () const
{
    std::unordered_map<_Key, _Val> m;
    if(IsDict())
    {
        auto &dictData = _raw._obj._dictData;
        auto iterEnd = dictData->end();
        for(auto iter = dictData->begin(); iter != iterEnd; ++iter)
            m.emplace(Variant(iter->first), Variant(iter->second));
    }
    
    return m;   
}

ALWAYS_INLINE Variant::operator const Sequence &() const
{
    return AsSequence(); 
}

template<typename _Val>
ALWAYS_INLINE Variant::operator std::vector<_Val> () const
{
    std::vector<_Val> seq;
    if(IsSeq() && !(_raw._obj._sequenceData->empty()))
    {
        auto &seqData = *(_raw._obj._sequenceData);
        seq.reserve(seqData.size());
        for(auto iter = seqData.begin(); iter != seqData.end(); ++iter)
            seq.emplace_back(*iter);
    }

    return seq;
}

template <typename _Ty>
ALWAYS_INLINE Variant::operator std::set<_Ty> () const
{
    std::set<_Ty> data;
    if(IsSeq() && !(_raw._obj._sequenceData->empty()))
    {
        auto &seqData = *(_raw._obj._sequenceData);
        data.reserve(seqData.size());
        for(auto &iter : seqData)
            data.emplace(iter);
    }
    else if(IsDict() && !(_raw._obj._dictData->empty()))
    {
        auto &dictData = *(_raw._obj._dictData);
        data.reserve(dictData.size());
        for(auto iter:dictData)
            data.emplace(iter.first);
    }

    return data;
}

template <typename _Ty>
ALWAYS_INLINE Variant::operator std::unordered_set<_Ty> () const
{
    std::unordered_set<_Ty> data;
    if(IsSeq() && !(_raw._obj._sequenceData->empty()))
    {
        auto &seqData = *(_raw._obj._sequenceData);
        data.reserve(seqData.size());
        for(auto &iter:seqData)
            data.emplace(iter);
    }
    else if(IsDict() && !(_raw._obj._dictData->empty()))
    {
        auto &dictData = *(_raw._obj._dictData);
        data.reserve(dictData.size());
        for(auto iter:dictData)
            data.emplace(iter.first);
    }

    return data;
}

template <typename _Ty>
ALWAYS_INLINE Variant::operator std::queue<_Ty> () const
{
    std::queue<_Ty> data;
    if(IsSeq() && !(_raw._obj._sequenceData->empty()))
    {
        auto &seqData = *(_raw._obj._sequenceData);
        data.reserve(seqData.size());
        for(auto &iter:seqData)
            data.emplace_back(iter);
    }
    else if(IsDict() && !(_raw._obj._dictData->empty()))
    {
        auto &dictData = *(_raw._obj._dictData);
        data.reserve(dictData.size());
        for(auto iter:dictData)
            data.emplace_back(iter.second);
    }

    return data;
}

template <typename _Ty>
ALWAYS_INLINE Variant::operator std::deque<_Ty> () const
{
    std::deque<_Ty> data;
    if(IsSeq() && !(_raw._obj._sequenceData->empty()))
    {
        auto &seqData = *(_raw._obj._sequenceData);
        data.reserve(seqData.size());
        for(auto &iter:seqData)
            data.emplace_back(iter);
    }
    else if(IsDict() && !(_raw._obj._dictData->empty()))
    {
        auto &dictData = *(_raw._obj._dictData);
        data.reserve(dictData.size());
        for(auto iter:dictData)
            data.emplace_back(iter.second);
    }

    return data;
}

ALWAYS_INLINE typename Variant::DictIter Variant::BeginDict()
{
    if(IsDict())
        return _raw._obj._dictData->begin();
    else
        return const_cast<Dict &>(VariantAssist::GetNullDict<Variant>()).begin();
}

ALWAYS_INLINE typename Variant::DictIter Variant::EndDict()
{
    if(IsDict())
        return  _raw._obj._dictData->end();
    else
        return const_cast<Dict &>(VariantAssist::GetNullDict<Variant>()).end();
}

ALWAYS_INLINE typename Variant::DictConstIter Variant::BeginDict() const
{
    if(IsDict())
        return _raw._obj._dictData->begin();
    else
        return const_cast<Dict &>(VariantAssist::GetNullDict<Variant>()).begin();
}

ALWAYS_INLINE typename Variant::DictConstIter Variant::EndDict() const
{
    if(IsDict())
        return  _raw._obj._dictData->end();
    else
        return VariantAssist::GetNullDict<Variant>().end();
}

ALWAYS_INLINE typename Variant::DictReverseIter Variant::ReverseBeginDict()
{
    if(IsDict())
        return _raw._obj._dictData->rbegin();
    else
        return const_cast<Dict &>(VariantAssist::GetNullDict<Variant>()).rbegin();
}

ALWAYS_INLINE typename Variant::DictReverseIter Variant::ReverseEndDict()
{
    if(IsDict())
        return _raw._obj._dictData->rend();
    else
        return const_cast<Dict &>(VariantAssist::GetNullDict<Variant>()).rend();
}

ALWAYS_INLINE typename Variant::DictConstReverseIter Variant::ReverseBeginDict() const
{
    if(IsDict() && _raw._obj._dictData)
        return _raw._obj._dictData->rbegin();
    else
        return VariantAssist::GetNullDict<Variant>().rbegin();
}

ALWAYS_INLINE typename Variant::DictConstReverseIter Variant::ReverseEndDict() const
{
    if(IsDict())
        return _raw._obj._dictData->rend();
    else
        return VariantAssist::GetNullDict<Variant>().rend();
}

ALWAYS_INLINE std::pair<typename Variant::DictIter, bool> Variant::InsertDict(const Variant::DictKeyType &key, const Variant::DictMappedType &val)
{
    _BecomeAndAllocDict();
    return _raw._obj._dictData->emplace(key, val);
}

ALWAYS_INLINE std::pair<typename Variant::DictIter, bool> Variant::InsertDict(const Variant::DictValueType &val)
{
    _BecomeAndAllocDict();
    return _raw._obj._dictData->insert(val);
}

template <typename _Kty, typename _Ty>
ALWAYS_INLINE std::pair<typename Variant::DictIter, bool> Variant::InsertDict(const _Kty &key, const _Ty &val)
{
    return this->InsertDict(DictKeyType(key),
        DictMappedType(val));
}

ALWAYS_INLINE typename Variant::DictIter Variant::FindDict(const Variant &key)
{
    if(IsDict() && _raw._obj._dictData)
        return _raw._obj._dictData->find(key);
    else
        return const_cast<Dict &>(VariantAssist::GetNullDict<Variant>()).end();
}

ALWAYS_INLINE typename Variant::DictConstIter Variant::FindDict(const Variant &key) const
{
    if(IsDict() && _raw._obj._dictData)
        return _raw._obj._dictData->find(key);
    else
        return VariantAssist::GetNullDict<Variant>().end();
}

template <typename _Kty>
ALWAYS_INLINE typename Variant::DictIter Variant::FindDict(const _Kty &key)
{
    return this->FindDict(Variant(key));
}

template <typename _Kty>
ALWAYS_INLINE typename Variant::DictConstIter Variant::FindDict(const _Kty &key) const
{
    return this->FindDict(Variant(key));
}

ALWAYS_INLINE typename Variant::DictIter Variant::EraseDict(Variant::DictIter it)
{
    if(IsDict())
       return _raw._obj._dictData->erase(it);
    
    return (const_cast<Variant::Dict &>(VariantAssist::GetNullDict<Variant>())).end();
}

ALWAYS_INLINE typename Variant::DictSizeType Variant::EraseDict(const Variant::DictKeyType &key)
{
    if(IsDict() && _raw._obj._dictData)
        return _raw._obj._dictData->erase(key);
    else
        return 0;
}

ALWAYS_INLINE typename Variant::DictIter Variant::EraseDict(Variant::DictIter first, Variant::DictIter last)
{
    if(IsDict() && _raw._obj._dictData)
        return _raw._obj._dictData->erase(first, last);

    return (const_cast<Variant::Dict &>(VariantAssist::GetNullDict<Variant>())).end();
}

template <typename _Kty>
ALWAYS_INLINE typename Variant::DictSizeType Variant::EraseDict(const _Kty &key)
{
    return this->EraseDict(Variant::DictKeyType(key));
}

ALWAYS_INLINE typename Variant::SequenceIter Variant::SeqBegin()
{
    BecomeSeq();
    return _raw._obj._sequenceData->begin();
}

ALWAYS_INLINE typename Variant::SequenceIter Variant::SeqEnd()
{
    BecomeSeq();
    return _raw._obj._sequenceData->end();
}

ALWAYS_INLINE typename Variant::SequenceConstIter Variant::SeqBegin() const
{
    return AsSequence().begin();
}

ALWAYS_INLINE typename Variant::SequenceConstIter Variant::SeqEnd() const
{
    return AsSequence().end();
}

ALWAYS_INLINE typename Variant::SequenceReverseIter Variant::SeqReverseBegin()
{
    BecomeSeq();
    return _raw._obj._sequenceData->rbegin();
}

ALWAYS_INLINE typename Variant::SequenceReverseIter Variant::SeqReverseEnd()
{
    BecomeSeq();
    return _raw._obj._sequenceData->rend();
}

ALWAYS_INLINE typename Variant::SequenceConstReverseIter Variant::SeqReverseBegin() const
{
    return AsSequence().rbegin();
}

ALWAYS_INLINE typename Variant::SequenceConstReverseIter  Variant::SeqReverseEnd() const
{
    return AsSequence().rend();
}

ALWAYS_INLINE typename Variant::SequenceReference Variant::SeqFront()
{
    BecomeSeq();
    return _raw._obj._sequenceData->front();
}

ALWAYS_INLINE typename Variant::SequenceReference Variant::SeqBack()
{
    BecomeSeq();
    return _raw._obj._sequenceData->back();
}

ALWAYS_INLINE typename Variant::SequenceConstReference Variant::SeqFront() const
{
    return AsSequence().front();
}

ALWAYS_INLINE typename Variant::SequenceConstReference Variant::SeqBack() const
{
    return AsSequence().back();
}

ALWAYS_INLINE typename Variant::SequenceIter Variant::SeqInsert(SequenceIter it, const Variant::SequenceValueType &val)
{
    BecomeSeq();
    return _raw._obj._sequenceData->insert(it, val);
}

ALWAYS_INLINE void Variant::SequenceInsert(Variant::SequenceIter it, Variant::SequenceSizeType n, const Variant::SequenceValueType &val)
{
    BecomeSeq();
    _raw._obj._sequenceData->insert(it, n, val);
}

ALWAYS_INLINE void Variant::SequenceInsert(Variant::SequenceIter it, Variant::SequenceConstIter first, Variant::SequenceConstIter last)
{
    BecomeSeq();
    _raw._obj._sequenceData->insert(it, first, last);
}

template <typename _Ty>
ALWAYS_INLINE typename Variant::SequenceIter Variant::SeqInsert(Variant::SequenceIter it, const _Ty &val)
{
    return SeqInsert(it, Variant(val));
}

template <typename _Ty>
ALWAYS_INLINE void Variant::SeqInsert(Variant::SequenceIter it, Variant::SequenceSizeType n, const _Ty &val)
{
    SequenceInsert(it, n, Variant(val));
}

ALWAYS_INLINE void Variant::SeqPushBack(const Variant::SequenceValueType &val)
{
    BecomeSeq();
    _raw._obj._sequenceData->push_back(val);
}

template <typename _Ty>
ALWAYS_INLINE void Variant::SeqPushBack(const _Ty &val)
{
    SeqPushBack(Variant(val));
}

ALWAYS_INLINE void Variant::SeqPopBack()
{
    BecomeSeq();
    if(!_raw._obj._sequenceData->empty())
        _raw._obj._sequenceData->pop_back();
}

ALWAYS_INLINE void Variant::SeqResize(Variant::SequenceSizeType n, const Variant::SequenceValueType &val)
{
    BecomeSeq();
    _raw._obj._sequenceData->resize(n, val);
}

template <typename _Ty>
ALWAYS_INLINE void Variant::SeqResize(Variant::SequenceSizeType n, const _Ty &val)
{
    SeqResize(n, Variant(val));
}

ALWAYS_INLINE void Variant::SeqReserve(Variant::SequenceSizeType n)
{
    BecomeSeq();
    _raw._obj._sequenceData->reserve(n);
}

ALWAYS_INLINE typename Variant::SequenceIter Variant::SeqErase(Variant::SequenceIter it)
{
    BecomeSeq();
    return _raw._obj._sequenceData->erase(it);
}

ALWAYS_INLINE typename Variant::SequenceIter Variant::SeqErase(Variant::SequenceIter first, Variant::SequenceIter last)
{
    BecomeSeq();
    return _raw._obj._sequenceData->erase(first, last);
}

ALWAYS_INLINE void Variant::SeqErase(const Variant::SequenceValueType &val)
{
    BecomeSeq();
    if(_raw._obj._sequenceData->empty())
        return;

    SequenceIter iter;
    while ((iter = std::find(_raw._obj._sequenceData->begin(), _raw._obj._sequenceData->end(), val)) != _raw._obj._sequenceData->end())
        _raw._obj._sequenceData->erase(iter);
}

template <typename _Ty>
ALWAYS_INLINE void Variant::SeqErase(const _Ty &val)
{
    SeqErase(Variant(val));
}

template <typename _Kty>
ALWAYS_INLINE Variant & Variant::operator [](const _Kty &key)
{
    return this->operator [](Variant(key));
}

template <typename _Kty>
ALWAYS_INLINE const Variant &Variant::operator [](const _Kty &key) const
{
    return this->operator [](Variant(key));
}

ALWAYS_INLINE Variant &Variant::operator [](const Variant &key)
{
    if(IsSeq())
        return (*_raw._obj._sequenceData)[key];

    BecomeDict();
    return (*_raw._obj._dictData)[key];
}

ALWAYS_INLINE const Variant &Variant::operator [](const Variant &key) const
{
    if(IsSeq())
        return (*_raw._obj._sequenceData)[key];

    auto &nilVariant = VariantAssist::GetNil<Variant>();
    if(IsDict())
    {
        auto iter = _raw._obj._dictData->find(key);
        return iter == _raw._obj._dictData->end() ? nilVariant : iter->second;
    }

    return nilVariant;
}

ALWAYS_INLINE Variant &Variant::operator =(bool val)
{
    BecomeBool();
    _raw._briefData._int64Data = val ? 1 : 0;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(Byte8 val)
{
    BecomeByte8();
    _raw._briefData._int64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(U8 val)
{
    BecomeUInt8();
    _raw._briefData._uint64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(Int16 val)
{
    BecomeInt16();
    _raw._briefData._int64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(UInt16 val)
{
    BecomeUInt16();
    _raw._briefData._uint64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(Int32 val)
{
    BecomeInt32();
    _raw._briefData._int64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(UInt32 val)
{
    BecomeUInt32();
    _raw._briefData._uint64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(Long val)
{
    BecomeLong();
    _raw._briefData._int64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(ULong val)
{
    BecomeULong();
    _raw._briefData._uint64Data = val;

    return *this;
}

template <typename _T>
ALWAYS_INLINE Variant &Variant::operator =(const _T * const &val)
{
    BecomePtr();
    _raw._briefData._uint64Data = 0;
    ::memcpy(&_raw._briefData._uint64Data, &val, sizeof(_T *));

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(const Byte8 * const &val)
{
    BecomeStr();
    const size_t len = val ? ::strlen(val) : 0;
    if (len == 0)
    {
        _raw._obj._strData->clear();
    }
    else
    {
        _raw._obj._strData->GetRaw().assign(val, len);
    }

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(Int64 val)
{
    BecomeInt64();
    _raw._briefData._int64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(UInt64 val)
{
    BecomeUInt64();
    _raw._briefData._uint64Data = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(Float val)
{
    BecomeFloat();
    _raw._briefData._doubleData = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(const Double &val)
{
    BecomeDouble();
    _raw._briefData._doubleData = val;

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(const LibString &val)
{
    BecomeStr();
    auto &strData = _raw._obj._strData;
    if(val.empty())
    {
        strData->clear();
    }
    else
    {
        *strData = val;
    }

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(const Variant::Sequence &val)
{
    BecomeSeq();
    auto &seqData = _raw._obj._sequenceData;
    if(val.empty())
        seqData->clear();
    else
        *seqData = val;

    return *this;
}

template<typename _Ty>
ALWAYS_INLINE Variant &Variant::operator =(const std::vector<_Ty> &val)
{
    BecomeSeq();
    auto &seqData = _raw._obj._sequenceData;
    seqData->clear();
    if(LIKELY(!val.empty()))
    {
        if(seqData->capacity() < val.size())
            seqData->reserve(val.size());

        for(auto iter = val.begin(); iter != val.end(); ++iter)
           seqData->emplace_back(Variant(*iter));
    }

    return *this;
}

template<typename _Ty>
ALWAYS_INLINE Variant &Variant::operator =(const std::list<_Ty> &val)
{
    BecomeSeq();
    auto &seqData = _raw._obj._sequenceData;
    seqData->clear();
    if(LIKELY(!val.empty()))
    {
        if(seqData->capacity() < val.size())
            seqData->reserve(val.size());

        for(auto iter = val.begin(); iter != val.end(); ++iter)
           seqData->emplace_back(Variant(*iter));
    }

    return *this;
}

template<typename _Ty>
ALWAYS_INLINE Variant &Variant::operator =(const std::queue<_Ty> &val)
{
    BecomeSeq();
    auto &seqData = _raw._obj._sequenceData;
    seqData->clear();
    if(LIKELY(!val.empty()))
    {
        if(seqData->capacity() < val.size())
            seqData->reserve(val.size());

        for(auto iter = val.begin(); iter != val.end(); ++iter)
           seqData->emplace_back(Variant(*iter));
    }

    return *this;
}

template<typename _Ty>
ALWAYS_INLINE Variant &Variant::operator =(const std::set<_Ty> &val)
{
    BecomeSeq();
    auto &seqData = _raw._obj._sequenceData;
    seqData->clear();
    if(LIKELY(!val.empty()))
    {
        if(seqData->capacity() < val.size())
            seqData->reserve(val.size());
            
        for(auto iter = val.begin(); iter != val.end(); ++iter)
           seqData->emplace_back(Variant(*iter));
    } 

    return *this;
}

template<typename _Ty>
ALWAYS_INLINE Variant &Variant::operator =(const std::unordered_set<_Ty> &val)
{
    BecomeSeq();
    auto &seqData = _raw._obj._sequenceData;
    seqData->clear();
    if(LIKELY(!val.empty()))
    {
        if(seqData->capacity() < val.size())
            seqData->reserve(val.size());

        for(auto iter = val.begin(); iter != val.end(); ++iter)
           seqData->emplace_back(Variant(*iter));
    }  

    return *this;
}

ALWAYS_INLINE Variant &Variant::operator =(const Variant::Dict &val)
{
    BecomeDict();
    auto &dict = _raw._obj._dictData;
    if(val.empty())
    {
        dict->clear();
    }
    else
    {
        *dict = val;
    }

    return *this;
}

template<typename _Key, typename _Val>
ALWAYS_INLINE Variant &Variant::operator =(const std::map<_Key, _Val> &val)
{
    BecomeDict();
    auto &dict = _raw._obj._dictData;
    dict->clear();
    if(LIKELY(val.empty()))
    {
        for(auto iter = val.begin(); iter != val.end(); ++iter)
            dict->emplace(Variant(iter->first), Variant(iter->second));
    }

    return *this; 
}

template<typename _Key, typename _Val>
ALWAYS_INLINE Variant &Variant::operator =(const std::unordered_map<_Key, _Val> &val)
{
    BecomeDict();
    auto &dict = _raw._obj._dictData;
    dict->clear();
    if(LIKELY(val.empty()))
    {
        for(auto iter = val.begin(); iter != val.end(); ++iter)
            dict->emplace(Variant(iter->first), Variant(iter->second));
    }

    return *this;  
}

ALWAYS_INLINE Variant &Variant::operator =(Variant &&val)
{
    _raw = std::move(val._raw);
    return *this;
}

ALWAYS_INLINE Int64 Variant::AsInt64() const
{
    if(IsNil())
        return 0;
    else if(IsDict())
        return 0;
    else if(IsStr())
        return (_raw._obj._strData && !_raw._obj._strData->empty()) ? StringUtil::StringToInt64(_raw._obj._strData->c_str()) : 0;

    if(IsDouble() || IsFloat())
        return static_cast<Int64>(_raw._briefData._doubleData);

    return _raw._briefData._int64Data;
}

ALWAYS_INLINE UInt64 Variant::AsUInt64() const
{
    if(IsNil())
        return 0;
    else if(IsDict() || IsSeq())
        return 0;
    else if(IsStr())
        return (_raw._obj._strData && !_raw._obj._strData->empty()) ? StringUtil::StringToUInt64(_raw._obj._strData->c_str()) : 0;

    if(IsDouble() || IsFloat())
        return static_cast<UInt64>(_raw._briefData._doubleData);

    return _raw._briefData._uint64Data;
}

ALWAYS_INLINE Double Variant::AsDouble() const
{
    if(IsNil() || IsDict() || IsSeq())
        return 0.0;
    else if(IsStr())
        return (_raw._obj._strData && !_raw._obj._strData->empty()) ? StringUtil::StringToDouble(_raw._obj._strData->c_str()) : 0;

    if(IsBriefData())
    {
        if(IsDouble() || IsFloat())
            return _raw._briefData._doubleData;

        if(IsSignedBriefData())
            return static_cast<double>(_raw._briefData._int64Data);

        return static_cast<double>(_raw._briefData._uint64Data);
    }

    return 0.0;
}

template<typename _Ty>
ALWAYS_INLINE bool Variant::operator ==(const _Ty &another) const
{
    return operator ==(Variant(another));
}

template<typename _Ty>
ALWAYS_INLINE bool Variant::operator !=(const _Ty &another) const
{
    return operator !=(Variant(another));
}

template<typename _Ty>
ALWAYS_INLINE bool Variant::operator <(const _Ty &another) const
{
    return operator <(Variant(another));
}

template<typename _Ty>
ALWAYS_INLINE bool Variant::operator >(const _Ty &another) const
{
    return operator >(Variant(another));
}

template<typename _Ty>
ALWAYS_INLINE bool Variant::operator <=(const _Ty &another) const
{
    return operator <=(Variant(another));
}

template<typename _Ty>
ALWAYS_INLINE bool Variant::operator >=(const _Ty &another) const
{
    return operator >=(Variant(another));
}

ALWAYS_INLINE const LibString &Variant::TypeToString() const
{
    return VariantRtti::GetTypeName(_raw._type);
}

ALWAYS_INLINE LibString Variant::ValueToString() const
{
    return AsStr();
}

ALWAYS_INLINE LibString Variant::ToString() const
{
    LibString desc;
    std::string &descRaw = desc.GetRaw();
    descRaw.reserve(64);
    descRaw.append("Variant(", 8);

    descRaw.append(TypeToString().GetRaw());
    descRaw.append(1, '|');
    descRaw.append(ValueToString().GetRaw());
    descRaw.append(1, ')');
    return desc;
}

template<typename StreamBuildType>
ALWAYS_INLINE bool Variant::Serialize(LibStream<StreamBuildType> &stream) const
{
    const auto oldWrPos = stream.GetWriteBytes();
    if(UNLIKELY(!stream.Write(_raw._type)))
    {
        CRYSTAL_TRACE("Variant serialize fail write _type stream buffer not enough lastWrPos:%lld, oldWrPos:%lld, fail write bytes:%llu"
                    , stream.GetWriteBytes(), oldWrPos, static_cast<UInt64>(sizeof(_raw._type)));
        stream.SetWritePos(oldWrPos);
        return false;
    }

    if (IsBriefData())
    {
        if(UNLIKELY(!stream.Write(_raw._briefData._uint64Data)))
        {
            CRYSTAL_TRACE("Variant serialize fail write _uint64Data stream buffer not enough lastWrPos:%lld, oldWrPos:%lld, fail write bytes:%llu"
                    , stream.GetWriteBytes(), oldWrPos, static_cast<UInt64>(sizeof(_raw._briefData._uint64Data)));
            stream.SetWritePos(oldWrPos);
            return false; 
        }
    }
    else if (IsStr())
    {
        const LibString &toWrite = _raw._obj._strData ? (*_raw._obj._strData) : VariantAssist::__g_nullStr;
        if(UNLIKELY(!stream.Write(toWrite)))
        {
            CRYSTAL_TRACE("Variant serialize fail write _strData stream buffer not enough lastWrPos:%lld, oldWrPos:%lld, fail write bytes:%llu"
                    , stream.GetWriteBytes(), oldWrPos, static_cast<UInt64>(sizeof(UInt64) + toWrite.size()));
            stream.SetWritePos(oldWrPos);
            return false;
        }
    }
    else if(IsSeq())
    {
        if(UNLIKELY(!stream.Write(static_cast<UInt64>(_raw._obj._sequenceData->size()))))
        {
            CRYSTAL_TRACE("Variant serialize fail write _sequenceData size stream buffer not enough lastWrPos:%lld, oldWrPos:%lld, fail write bytes:%llu"
                    , stream.GetWriteBytes(), oldWrPos, static_cast<UInt64>(sizeof(UInt64)));
            stream.SetWritePos(oldWrPos);
            return false;
        }
        auto seq = _raw._obj._sequenceData;
        for(SequenceConstIter iter = seq->begin(); iter != seq->end(); ++iter)
        {
            if(UNLIKELY(!stream.Write(*iter)))
            {
                CRYSTAL_TRACE("Variant serialize fail write _sequenceData elem stream buffer not enough lastWrPos:%lld, oldWrPos:%lld"
                        , stream.GetWriteBytes(), oldWrPos);
                stream.SetWritePos(oldWrPos);
                return false;
            }
        }
    }
    else if (IsDict())
    {
        if(UNLIKELY(!stream.Write(static_cast<UInt64>(_raw._obj._dictData->size()))))
        {
            CRYSTAL_TRACE("Variant serialize fail write _dictData size stream buffer not enough lastWrPos:%lld, oldWrPos:%lld, fail write bytes:%llu"
                    , stream.GetWriteBytes(), oldWrPos, static_cast<UInt64>(sizeof(UInt64)));
            stream.SetWritePos(oldWrPos);
            return false;
        }

        for (DictConstIter it = _raw._obj._dictData->begin();
             it != _raw._obj._dictData->end();
             ++it)
        {
            if(UNLIKELY(!stream.Write(it->first)))
            {
                CRYSTAL_TRACE("Variant serialize fail write _dictData key stream buffer not enough lastWrPos:%lld, oldWrPos:%lld"
                    , stream.GetWriteBytes(), oldWrPos);
                stream.SetWritePos(oldWrPos);
                return false;
            }

            if(UNLIKELY(!stream.Write(it->second)))
            {
                CRYSTAL_TRACE("Variant serialize fail write _dictData value stream buffer not enough lastWrPos:%lld, oldWrPos:%lld"
                    , stream.GetWriteBytes(), oldWrPos);
                stream.SetWritePos(oldWrPos);
                return false;
            }
        }
    }

    return true;
}

template<typename StreamBuildType>
ALWAYS_INLINE bool Variant::DeSerialize(LibStream<StreamBuildType> &stream)
{
    decltype(_raw._type) rawType = VariantRtti::VT_NIL;
    const UInt64 oldReadPos = stream.GetReadBytes();
    if (!stream.Read(rawType))
    {
        CRYSTAL_TRACE("Variant DeSerialize fail read rawType stream buffer");
        stream.SetReadPos(oldReadPos);
        return false;
    }

    if(rawType != _raw._type)
        BecomeNil();

    _raw._type = rawType;
    if (IsNil())
        return true;

    if (IsBriefData())
    {
        if (!stream.Read(_raw._briefData._uint64Data))
        {
            CRYSTAL_TRACE("Variant DeSerialize fail read _uint64Data stream buffer");
            BecomeNil();
            stream.SetReadPos(oldReadPos);
            return false;
        }

        return true;
    }
    else if (IsStr())
    {
        if (!_raw._obj._strData)
            _raw._obj._strData = CRYSTAL_NEW(LibString);
        else
            _raw._obj._strData->clear();

        if (!stream.Read(*_raw._obj._strData))
        {
            CRYSTAL_TRACE("Variant DeSerialize fail read _strData stream buffer");
            BecomeNil();
            stream.SetReadPos(oldReadPos);
            return false;
        }

        return true;
    }
    else if(IsSeq())
    {
        UInt64 count = 0;
        if (!stream.Read(count))
        {
            CRYSTAL_TRACE("Variant DeSerialize fail read _sequenceData count stream buffer");
            BecomeNil();
            stream.SetReadPos(oldReadPos);
            return false;
        }

        if(_raw._obj._sequenceData)
            _raw._obj._sequenceData->clear();
        else
            _raw._obj._sequenceData = CRYSTAL_NEW(Sequence);

        if(count == 0)
            return true;
        
        for(UInt64 idx = 0; idx < count; ++idx)
        {
            Variant val;
            if(!stream.Read(val))
            {
                CRYSTAL_TRACE("Variant DeSerialize fail read _sequenceData elem stream buffer");
                BecomeNil();
                stream.SetReadPos(oldReadPos);
                return false;
            }

            _raw._obj._sequenceData->push_back(val);
        }

        return true;  
    }
    else if (IsDict())
    {
        UInt64 count = 0;
        if (!stream.Read(count))
        {
            CRYSTAL_TRACE("Variant DeSerialize fail read _dictData count stream buffer");
            BecomeNil();
            stream.SetReadPos(oldReadPos);
            return false;
        }

        if(_raw._obj._dictData)
            _raw._obj._dictData->clear();
        else
            _raw._obj._dictData = CRYSTAL_NEW(Dict);

        if (count == 0)
            return true;

        for (UInt64 i = 0; i < count; ++i)
        {
            Variant key;
            Variant val;
            if (!stream.Read(key) || !stream.Read(val))
            {
                CRYSTAL_TRACE("Variant DeSerialize fail read _dictData elem stream buffer");
                BecomeNil();
                stream.SetReadPos(oldReadPos);
                return false;
            }

            _raw._obj._dictData->insert(std::make_pair(key, val));
        }

        return true;
    }
    
    stream.SetReadPos(oldReadPos);
    return false;
}

ALWAYS_INLINE void Variant::_BecomeAndAllocDict()
{
    if(!IsDict())
        BecomeDict();
    if(!_raw._obj._dictData)
        _raw._obj._dictData = CRYSTAL_NEW(Dict);
}

ALWAYS_INLINE void Variant::_SetType(UInt64 type)
{
    _raw._type = static_cast<VariantRtti::RttiType>(type);
}

ALWAYS_INLINE typename Variant::Raw &Variant::_GetRaw()
{
    return _raw;
}

ALWAYS_INLINE void Variant::_CleanBriefData()
{
    _raw._briefData._uint64Data = 0;
    _raw._type = VariantRtti::VT_NIL;
}

ALWAYS_INLINE void Variant::_CleanStrData()
{
    if(LIKELY(_raw._obj._strData))
    {
        CRYSTAL_DELETE(_raw._obj._strData);
        _raw._obj._strData = NULL;
        _raw._type = VariantRtti::VT_NIL;
    }
}

ALWAYS_INLINE void Variant::_CleanDictData()
{
    if(LIKELY(_raw._obj._dictData))
    {
        CRYSTAL_DELETE(_raw._obj._dictData);
        _raw._obj._dictData = NULL;
        _raw._type = VariantRtti::VT_NIL;
    }
}

ALWAYS_INLINE void Variant::_CleanSeqData()
{
    if(LIKELY(_raw._obj._sequenceData))
    {
        CRYSTAL_DELETE(_raw._obj._sequenceData);
        _raw._obj._sequenceData = NULL;
        _raw._type = VariantRtti::VT_NIL;
    }
}

ALWAYS_INLINE void Variant::_CleanTypeData(UInt64 type)
{
    typedef VariantRtti _RttiType;

    const auto topType = (type & _RttiType::VT_TYPEINFO_MASK);
    switch(topType)
    {
        case _RttiType::VT_BRIEF_DATA:
            _CleanBriefData();
            break;
        case _RttiType::VT_STRING:
            _CleanStrData();
            break;
        case _RttiType::VT_DICTIONARY:
            _CleanDictData();
            break;
        case _RttiType::VT_SEQUENCE:
            _CleanSeqData();
            break;
        default:
            CRYSTAL_TRACE("unknown variant type, top type:%llu, type:%llu", topType, type);
            break;
    }
}

KERNEL_END

// Variant hash 特化
namespace std
{

/**
 * \brief The explicit specialization of std::hash<Variant<KERNEL_NS::_Build::MT>> impl.
 * 
 */
template <>
struct hash<KERNEL_NS::Variant>
{
    // gcc 或者clang下pure属性可以当作内联
    #if CRYSTAL_CUR_COMP == CRYSTAL_COMP_GCC || CRYSTAL_CUR_COMP == CRYSTAL_COMP_CLANG
     CRYSTAL_ATTRIBUTE_PURE
    #endif // Comp == Gcc or Comp == Clang
    size_t operator()(const KERNEL_NS::Variant &var) const noexcept
    {
        if (var.IsBriefData())
        {
            const KERNEL_NS::Variant::Raw &raw = var.GetRaw();
            if (var.IsFloat() || var.IsDouble())
                #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                return ::std::_Hash_representation(
                    raw._briefData._doubleData == 0.0f ? 0.0f : raw._briefData._doubleData);
                #else
                return ::std::_Hash_impl::hash(raw._briefData._doubleData);
                #endif
            else
                return static_cast<size_t>(raw._briefData._uint64Data);
        }
        else if (var.IsStr())
        {
            const KERNEL_NS::Variant::Raw &raw = var.GetRaw();
            const KERNEL_NS::LibString * const &str = raw._obj._strData;
            #if CRYSTAL_TARGET_PLATFORM_WINDOWS
            if (str && !str->empty())
                return ::std::_Hash_array_representation(str->data(), str->size());
            else
                return ::std::_Hash_representation(nullptr);
            #else
            if (str && !str->empty())
                return ::std::_Hash_impl::hash(str->data(), str->size());
            else
                return ::std::_Hash_impl::hash(nullptr, 0);
            #endif
        }
        else if (var.IsSeq())
        {
            size_t hashVal = 10000;
            const KERNEL_NS::Variant::Raw &raw = var.GetRaw();
            const KERNEL_NS::Variant::Sequence * const &seq = raw._obj._sequenceData;
            if (!seq || seq->empty())
                return hashVal;

            const KERNEL_NS::Variant::SequenceConstIter endIt = seq->end();
            for (KERNEL_NS::Variant::SequenceConstIter it = seq->begin();
                 it != endIt;
                 ++it)
                 hashVal += (*this)(*it);

            return hashVal;
        }
        else if (var.IsDict())
        {
            size_t hashVal = 20000;
            const KERNEL_NS::Variant::Raw &raw = var.GetRaw();
            const KERNEL_NS::Variant::Dict * const &dict = raw._obj._dictData;
            if (!dict || dict->empty())
                return hashVal;

            const KERNEL_NS::Variant::DictConstIter endIt = dict->end();
            for (KERNEL_NS::Variant::DictConstIter it = dict->begin();
                 it != endIt;
                 ++it)
                hashVal += (*this)(it->first);

            return hashVal;
        }
        else // Nil
        {
            return 0;
        }
    }
};

}

#endif
