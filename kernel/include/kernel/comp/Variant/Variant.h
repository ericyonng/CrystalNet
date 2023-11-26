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
 * Author: Eric Yonng
 * Date: 2021-03-16 11:23:02
 * Description: 
 *              1.注意variant作为dict时，请保证key的类型是一样的，否则会出现异常情况，导致数据错乱
 *              2.TODO:支持类拷贝(使用对象构造器，挂在Variant Dict身上，然后从目标（多个进程）进程取出来
 *              3.variant 的原则是数据是由构造器与释放器创建与释放的
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_VARIANT_VARIANT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_VARIANT_VARIANT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

// 运行时类型识别
class KERNEL_EXPORT VariantRtti
{
public:
    // 类型识别信息32bit 每个数据类型都有独立的mask 互斥
    // [high forty bits][middle 23 bits][first bit]
    // [    类型信息   ][ 具体类型枚举 ][  符号位 ]
    enum RttiType:UInt64
    {
        // 初始化类型
        VT_NIL = 0x0LLU,

        // 符号标记
        VT_SIGHED           = 0x01LLU,             // 有符号
        VT_UNSIGHED         = 0x0LLU,              // 无符号

        // 类型信息
        VT_BRIEF_DATA       = 0x01000000LLU,                // 基本数据类型
        VT_STRING           = VT_BRIEF_DATA << 1,           // 字符串类型
        VT_DICTIONARY       = VT_STRING << 1,               // map字典类型
        VT_SEQUENCE         = VT_DICTIONARY << 1,           // sequence类型

        // 简单类型的具体类型枚举
        VT_BRIEF_SIGHED_DATA        = VT_BRIEF_DATA     | VT_SIGHED,                        // 有符号简单数据类型
        VT_BRIEF_UNSIGHED_DATA      = VT_BRIEF_DATA     | VT_UNSIGHED,                      // 无符号简单数据类型
        VT_BRIEF_BOOL       = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 1),                     // bool类型
        VT_BRIEF_BYTE8      = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 2),                     // Byte8
        VT_BRIEF_UINT8      = VT_BRIEF_UNSIGHED_DATA    | (0x1LLU << 3),                     // U8
        VT_BRIEF_INT16      = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 4),                     // INT16
        VT_BRIEF_UINT16     = VT_BRIEF_UNSIGHED_DATA    | (0x1LLU << 5),                     // UINT16
        VT_BRIEF_INT32      = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 6),                     // INT32
        VT_BRIEF_UINT32     = VT_BRIEF_UNSIGHED_DATA    | (0x1LLU << 7),                     // UINT32
        VT_BRIEF_LONG       = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 8),                     // LONG
        VT_BRIEF_ULONG      = VT_BRIEF_UNSIGHED_DATA    | (0x1LLU << 9),                     // ULONG
        VT_BRIEF_PTR        = VT_BRIEF_UNSIGHED_DATA    | (0x1LLU << 10),                    // PTR
        VT_BRIEF_INT64      = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 11),                    // INT64
        VT_BRIEF_UINT64     = VT_BRIEF_UNSIGHED_DATA    | (0x1LLU << 12),                    // UINT64
        VT_BRIEF_FLOAT      = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 13),                    // float
        VT_BRIEF_DOUBLE     = VT_BRIEF_SIGHED_DATA      | (0x1LLU << 14),                    // double

        // 字符串类型
        VT_STRING_DEF       = VT_STRING | VT_SIGHED,        // 默认的字符串类型
        // 字典类型
        VT_DICTIONARY_DEF   = VT_DICTIONARY | VT_SIGHED,    // 默认的字典类型
        // 序列类型
        VT_SEQUENCE_DEF     = VT_SEQUENCE | VT_SIGHED,      // 默认的序列

        // 掩码
        VT_TYPEINFO_MASK    = ~(0x0FFFFFFLLU),                   // 高40位为类型信息掩码
        VT_SIGNED_MASK      = 0x01LLU,                           // 符号位
    };

    static const LibString &GetTypeName(UInt64 rttiType);
    static void InitRttiTypeNames();

private:
    static std::unordered_map<UInt64, LibString> _rttiTypeRefString;   // 类型字符串
    static const LibString _nullString;
};

/**
 * Pre-declare Variant class.
 */
class Variant;

KERNEL_END

/**
 * \brief Variant stream output function.
 */
extern KERNEL_EXPORT std::ostream &operator <<(std::ostream &o, const KERNEL_NS::Variant &var);

extern KERNEL_EXPORT std::string &operator <<(std::string &o, const KERNEL_NS::Variant &var);

extern KERNEL_EXPORT KERNEL_NS::LibString &operator <<(KERNEL_NS::LibString &o, const KERNEL_NS::Variant &var);

KERNEL_BEGIN

class KERNEL_EXPORT VariantAssist
{
public:
    static const LibString __g_nullStr;   // 前提必须是str类型
    static const LibString __g_nilStr;   // nil 类型
    static const LibString __g_trueToOneStr;
    static const LibString __g_falseToZeroStr;
    static const LibString __g_trueStr;
    static const LibString __g_falseStr;
    static const LibString __g_emptySeqStr;
    static const LibString __g_emptyDictStr;

    template<typename Var>
    static ALWAYS_INLINE const Var &GetNil()
    {
        static const Var nil;
        return nil;
    }

    template<typename Var>
    static ALWAYS_INLINE const std::map<Var, Var> &GetNullDict()
    {
        static const std::map<Var, Var> __g_nullDict;
        return __g_nullDict;
    }

    template<typename Var>
    static ALWAYS_INLINE const std::vector<Var> &GetNullSequence()
    {
        static const std::vector<Var> __g_nullSequence;
        return __g_nullSequence;
    }
};

class KERNEL_EXPORT Variant
{
    POOL_CREATE_OBJ_DEFAULT(Variant);
    
public:
    using Dict = typename std::map<Variant, Variant>;
    // typedef std::map<Variant<BuildType>, Variant<BuildType>> Dict;
    using Sequence = typename std::vector<Variant>;

	using DictIter = typename Dict::iterator;
	using DictConstIter = typename Dict::const_iterator;
	using DictReverseIter = typename Dict::reverse_iterator;
	using DictConstReverseIter = typename Dict::const_reverse_iterator;
    using DictSizeType = typename Dict::size_type;
    using DictKeyType = typename Dict::key_type;
    using DictValueType = typename Dict::value_type;
    using DictMappedType = typename Dict::mapped_type;

	using SequenceIter = typename Sequence::iterator;
	using SequenceConstIter = typename Sequence::const_iterator;
	using SequenceReverseIter = typename Sequence::reverse_iterator;
	using SequenceConstReverseIter = typename Sequence::const_reverse_iterator;
    using SequenceReference = typename Sequence::reference;
    using SequenceConstReference = typename Sequence::const_reference;
    using SequenceValueType = typename Sequence::value_type;
    using SequenceSizeType = typename Sequence::size_type;

public:
struct Raw
{
    Raw():_type(VariantRtti::VT_NIL)
    {
        ::memset(&_briefData, 0, sizeof(_briefData));
        ::memset(&_obj, 0, sizeof(_obj));
    }
    ~Raw()
    {
        Reset();
    }

    Raw(const Raw &other)
    {
        _Copy(other);
    }

    Raw(Raw &&other)
    {
        _Move(other);
    }

 // methods
    void Reset()
    {
        if(_type == VariantRtti::VT_STRING_DEF)
        {
            CRYSTAL_DELETE_SAFE(_obj._strData);
        }
        else if(_type == VariantRtti::VT_DICTIONARY_DEF)
        {
            CRYSTAL_DELETE_SAFE(_obj._dictData);
        }
        else if(_type == VariantRtti::VT_SEQUENCE_DEF)
        {
            CRYSTAL_DELETE_SAFE(_obj._sequenceData);
        }

        ::memset(&_briefData, 0, sizeof(_briefData));
        ::memset(&_obj, 0, sizeof(_obj));

        _type = VariantRtti::VT_NIL;
    }

    Variant::Raw &operator = (const Variant::Raw &other)
    {
        _CopyInAssign(other);

        return *this;
    }
    
    Variant::Raw &operator = (Variant::Raw &&other)
    {
        _MoveInAssign(other);

        return *this;
    }

    

private:
    void _CopyInAssign(const Variant::Raw &other)
    {
        if(UNLIKELY(this == &other))
            return;

        Reset();
        _Copy(other);
    }

    void _Copy(const Variant::Raw &other)
    {
        if(UNLIKELY(this == &other))
            return;

        _type = other._type;
        _briefData = other._briefData;
        ::memset(&_obj, 0, sizeof(_obj));

        if(_type == VariantRtti::VT_STRING_DEF)
        {
            if(!_obj._strData)
                _obj._strData = CRYSTAL_NEW(LibString);
            *_obj._strData = *other._obj._strData;
        }
        else if(_type == VariantRtti::VT_DICTIONARY_DEF)
        {
            if(!_obj._dictData)
                _obj._dictData = CRYSTAL_NEW(Dict);
            *_obj._dictData = *other._obj._dictData;
        }
        else if(_type == VariantRtti::VT_SEQUENCE_DEF)
        {
            if(!_obj._sequenceData)
                _obj._sequenceData = CRYSTAL_NEW(Sequence);
            *_obj._sequenceData = *other._obj._sequenceData;
        }
    }

    void _MoveInAssign(Variant::Raw &other)
    {
        if(UNLIKELY(this == &other))
            return;

        Reset();
        _Move(other);
    }

    void _Move(Variant::Raw &other)
    {
        if(UNLIKELY(this == &other))
            return;

        _type = other._type;
        _briefData = other._briefData;
        _obj = other._obj;

        other._type = VariantRtti::VT_NIL;
        ::memset(&other._briefData, 0, sizeof(other._briefData));
        ::memset(&other._obj, 0, sizeof(other._obj));
    }



public:
    friend class Variant;

    UInt64 _type;
    union ObjType
    {
        LibString *_strData;
        Dict *_dictData;
        Sequence *_sequenceData;
    }_obj;

    union BriefDataType
    {
        Int64 _int64Data;
        UInt64 _uint64Data;
        Double _doubleData;
    }_briefData;
};

public:
    Variant();

    // Constructors(all parameter constructors is explicit, copy constructor is non-explicit).
    explicit Variant(const bool &boolVal);
    explicit Variant(const Byte8 &byte8Val);
    explicit Variant(const U8 &uint8Val);
    explicit Variant(const Int16 &int16Val);
    explicit Variant(const UInt16 &uint16Val);
    explicit Variant(const Int32 &int32Val);
    explicit Variant(const UInt32 &uint32Val);
    explicit Variant(const Long &longVal);
    explicit Variant(const ULong &ulongVal);
    template <typename _T>
    explicit Variant(const _T * const &ptrVal);
    explicit Variant(const Int64 &int64Val);
    explicit Variant(const UInt64 &uint64Val);
    explicit Variant(const Float &floatVal);
    explicit Variant(const Double &doubleVal);
    explicit Variant(const Byte8 *cstrVal);
    explicit Variant(const std::string &strVal);
    explicit Variant(const LibString &strVal);
    explicit Variant(const Variant::Dict &dictVal);
    Variant(const Variant &varVal);
    Variant(Variant &&varVal);

    // TODO: 以下接口需要实现
    template<typename _Key, typename _Val>
    explicit Variant(const std::map<_Key, _Val> &dictVal);
    template<typename _Key, typename _Val>
    explicit Variant(const std::unordered_map<_Key, _Val> &unorderedDictVal);
    template<typename _Val>
    explicit Variant(const std::set<_Val> &setVal);
    template<typename _Val>
    explicit Variant(const std::unordered_set<_Val> &unorderedSetVal);
    template<typename _Val>
    explicit Variant(const std::vector<_Val> &seqVal);
    template<typename _Val>
    explicit Variant(const std::list<_Val> &listVal);
    template<typename _Val>
    explicit Variant(const std::deque<_Val> &dequeVal);
    template<typename _Val>
    explicit Variant(const std::queue<_Val> &queueVal);

    // Fetch Variant data type and raw data.
    UInt64 GetType() const;
    UInt64 GetMainType() const;
    const struct Raw &GetRaw() const;
    void Reset();
    void Clear();
    bool IsEmpty() const;
    UInt64 GetCount() const;
    UInt64 GetCapacity() const;
    Variant &MoveFrom(Variant &other);

    // Type diagnose.
    bool IsNil() const;
    bool IsBriefData() const;
    bool IsSignedBriefData() const;
    bool IsUnsignedBriefData() const;
    bool IsBool() const;
    bool IsByte8() const;
    bool IsUInt8() const;
    bool IsInt16() const;
    bool IsUInt16() const;
    bool IsInt32() const;
    bool IsUInt32() const;
    bool IsLong() const;
    bool IsULong() const;
    bool IsPtr() const;
    bool IsInt64() const;
    bool IsUInt64() const;
    bool IsFloat() const;
    bool IsDouble() const;
    bool IsStr() const;
    bool IsDict() const;
    bool IsSeq() const;
    bool IsVariantType(UInt64 type) const;
    bool IsMainType(UInt64 type) const;

    // Type convert.
    Variant &BecomeNil();
    Variant &BecomeBool();
    Variant &BecomeByte8();
    Variant &BecomeUInt8();
    Variant &BecomeInt16();
    Variant &BecomeUInt16();
    Variant &BecomeInt32();
    Variant &BecomeUInt32();
    Variant &BecomeLong();
    Variant &BecomeULong();
    Variant &BecomePtr();
    Variant &BecomeInt64();
    Variant &BecomeUInt64();
    Variant &BecomeFloat();
    Variant &BecomeDouble();
    Variant &BecomeStr();
    Variant &BecomeDict();
    Variant &BecomeSeq();
    template<VariantRtti::RttiType _Ty>
    Variant &Become();

    // Real data fetch.
    bool    AsBool() const;
    Byte8   AsByte8() const;
    U8      AsUInt8() const;
    Int16   AsInt16() const;
    UInt16  AsUInt16() const;
    Int32   AsInt32() const;
    UInt32  AsUInt32() const;
    Long    AsLong() const;
    ULong   AsULong() const;
    template <typename _T>
    _T *    AsPtr() const;
    Int64   AsInt64() const;
    UInt64  AsUInt64() const;
    Float   AsFloat() const;
    Double  AsDouble() const;
    LibString AsStr() const;
    const Dict &AsDict() const;
    const Sequence &AsSequence() const;

    operator bool() const;
    operator Byte8 () const;
    operator U8 () const;
    operator Int16 () const;
    operator UInt16 () const;
    operator Int32 () const;
    operator UInt32 () const;
    operator Long() const;
    operator ULong () const;
    template <typename _T>
    operator _T * () const;
    operator Int64 () const;
    operator UInt64 () const;
    operator Float() const;
    operator Double() const;
    operator LibString () const;
    operator std::string () const;
    operator const Variant::Dict &() const;

    template<typename _Key, typename _Val>
    operator std::map<_Key, _Val> () const;
    template<typename _Key, typename _Val>
    operator std::unordered_map<_Key, _Val> () const;
    operator const Sequence &() const;

    template<typename _Val>
    operator std::vector<_Val> () const;
    template <typename _Ty>
    operator std::set<_Ty>() const;
    template <typename _Ty>
    operator std::unordered_set<_Ty>() const;
    template <typename _Ty>
    operator std::queue<_Ty>() const;
    template <typename _Ty>
    operator std::deque<_Ty>() const;
    template<typename _Ty>
    operator std::list<_Ty>() const;

     // Dictionary methods
    // Dictionary type variant object specify operate methods.
    DictIter BeginDict();
    DictIter EndDict();
    DictConstIter BeginDict() const;
    DictConstIter EndDict() const;
    DictReverseIter ReverseBeginDict();
    DictReverseIter ReverseEndDict();
    DictConstReverseIter ReverseBeginDict() const;
    DictConstReverseIter ReverseEndDict() const;

    std::pair<DictIter, bool> InsertDict(const Variant::DictKeyType &key, const Variant::DictMappedType &val);
    std::pair<DictIter, bool> InsertDict(const Variant::DictValueType &val);
    template <typename _Kty, typename _Ty>
    std::pair<DictIter, bool> InsertDict(const _Kty &key, const _Ty &val);

    DictIter FindDict(const Variant &key);
    DictConstIter FindDict(const Variant &key) const;

    template <typename _Kty>
    DictIter FindDict(const _Kty &key);
    template <typename _Kty>
    DictConstIter FindDict(const _Kty &key) const;

    Variant::DictIter EraseDict(DictIter it);
    Variant::DictSizeType EraseDict(const Variant::DictKeyType &key);
    Variant::DictIter EraseDict(DictIter first, DictIter last);
    template <typename _Kty>
    Variant::DictSizeType EraseDict(const _Kty &key);
    

     // Sequence
    // Dictionary type variant object specify operate methods.
    SequenceIter SeqBegin();
    SequenceIter SeqEnd();
    SequenceConstIter SeqBegin() const;
    SequenceConstIter SeqEnd() const;
    SequenceReverseIter SeqReverseBegin();
    SequenceReverseIter SeqReverseEnd();
    SequenceConstReverseIter SeqReverseBegin() const;
    SequenceConstReverseIter SeqReverseEnd() const;

    SequenceReference SeqFront();
    SequenceReference SeqBack();
    SequenceConstReference SeqFront() const;
    SequenceConstReference SeqBack() const;

    SequenceIter SeqInsert(SequenceIter it, const SequenceValueType &val);
    void SequenceInsert(SequenceIter it, SequenceSizeType n, const SequenceValueType &val);
    void SequenceInsert(SequenceIter it, SequenceConstIter first, SequenceConstIter last);

    template <typename _Ty>
    SequenceIter SeqInsert(SequenceIter it, const _Ty &val);
    template <typename _Ty>
    void SeqInsert(SequenceIter it, SequenceSizeType n, const _Ty &val);

    void SeqPushBack(const SequenceValueType &val);
    template <typename _Ty>
    void SeqPushBack(const _Ty &val);

    void SeqPopBack();

    void SeqResize(SequenceSizeType n, const SequenceValueType &val = VariantAssist::GetNil<Variant>());
    template <typename _Ty>
    void SeqResize(SequenceSizeType n, const _Ty &val = _Ty());

    void SeqReserve(SequenceSizeType n);

    SequenceIter SeqErase(SequenceIter it);
    SequenceIter SeqErase(SequenceIter first, SequenceIter last);
    void SeqErase(const SequenceValueType &val);
    template <typename _Ty>
    void SeqErase(const _Ty &val);
    

    template <typename _Kty>
    Variant &operator [](const _Kty &key);
    template <typename _Kty>
    const Variant &operator [](const _Kty &key) const;
    Variant &operator [](const Variant &key);
    const Variant &operator [](const Variant &key) const;

    // assignment operators.
    Variant &operator =(bool val);
    Variant &operator =(Byte8 val);
    Variant &operator =(U8 val);
    Variant &operator =(Int16 val);
    Variant &operator =(UInt16 val);
    Variant &operator =(Int32 val);
    Variant &operator =(UInt32 val);
    Variant &operator =(Long val);
    Variant &operator =(ULong val);
    template <typename _T>
    Variant &operator =(const _T * const &val);
    Variant &operator =(const Byte8 * const &val);
    Variant &operator =(Int64 val);
    Variant &operator =(UInt64 val);
    Variant &operator =(Float val);
    Variant &operator =(const Double &val);
    Variant &operator =(const LibString &val);

    Variant &operator =(const Variant::Sequence &val);
    template<typename _Ty>
    Variant &operator =(const std::vector<_Ty> &val);
    template<typename _Ty>
    Variant &operator =(const std::vector<const _Ty *> &val);
    template<typename _Ty>
    Variant &operator =(const std::vector<_Ty *> &val);
    template<typename _Ty>
    Variant &operator =(const std::list<_Ty> &val);
    template<typename _Ty>
    Variant &operator =(const std::list<_Ty *> &val);
    template<typename _Ty>
    Variant &operator =(const std::list<const _Ty *> &val);
    template<typename _Ty>
    Variant &operator =(const std::queue<_Ty> &val);
    template<typename _Ty>
    Variant &operator =(const std::set<_Ty> &val);
    template<typename _Ty>
    Variant &operator =(const std::unordered_set<_Ty> &val);
    template<typename _Ty>
    Variant &operator =(const std::deque<_Ty> &val);

    Variant &operator =(const Variant::Dict &val);
    
    template<typename _Key, typename _Val>
    Variant &operator =(const std::map<_Key, _Val> &val);
    template<typename _Key, typename _Val>
    Variant &operator =(const std::unordered_map<_Key, _Val> &val);
    
    Variant &operator =(const Variant &val);
    Variant &operator =(Variant &&val);

    // Relational operators.
    bool operator ==(const Variant &another) const;
    bool operator !=(const Variant &another) const;

    bool operator <(const Variant &another) const;
    bool operator >(const Variant &another) const;
    bool operator <=(const Variant &another) const;
    bool operator >=(const Variant &another) const;

    template<typename _Ty>
    bool operator ==(const _Ty &another) const;
    template<typename _Ty>
    bool operator !=(const _Ty &another) const;
    template<typename _Ty>
    bool operator <(const _Ty &another) const;
    template<typename _Ty>
    bool operator >(const _Ty &another) const;
    template<typename _Ty>
    bool operator <=(const _Ty &another) const;
    template<typename _Ty>
    bool operator >=(const _Ty &another) const;

    // Arithmetic operators.
    Variant operator +(const Variant &another) const;
    Variant operator -(const Variant &another) const;
    Variant operator *(const Variant &another) const;
    Variant operator /(const Variant &another) const;

    Variant &operator +=(const Variant &another);
    Variant &operator -=(const Variant &another);
    Variant &operator *=(const Variant &another);
    Variant &operator /=(const Variant &another);

    // Type to string.
    const LibString &TypeToString() const;
    // Value to string.
    LibString ValueToString() const;
    // To string.
    LibString ToString() const;

    // Serialize / DeSerialize support.
    template<typename StreamBuildType>
    bool Serialize(LibStream<StreamBuildType> &stream) const
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
    bool DeSerialize(LibStream<StreamBuildType> &stream)
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

private:
    void _BecomeAndAllocDict();
    
    template<VariantRtti::RttiType _Ty>
    void _BecomeSpecify();
    
private:
        friend class VariantTraits;

        void _SetType(UInt64 type);
        Raw &_GetRaw();

        void _CleanBriefData();
        void _CleanStrData();
        void _CleanDictData();
        void _CleanSeqData();
        void _CleanTypeData(UInt64 type);

private:
    Raw _raw;
};

// become specify
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_NIL>()
{
    _raw._type = VariantRtti::VT_NIL;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_BOOL>()
{
    _raw._type = VariantRtti::VT_BRIEF_BOOL;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_BYTE8>()
{
    _raw._type = VariantRtti::VT_BRIEF_BYTE8;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_UINT8>()
{
    _raw._type = VariantRtti::VT_BRIEF_UINT8;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_INT16>()
{
    _raw._type = VariantRtti::VT_BRIEF_INT16;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_UINT16>()
{
    _raw._type = VariantRtti::VT_BRIEF_UINT16;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_INT32>()
{
    _raw._type = VariantRtti::VT_BRIEF_INT32;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_UINT32>()
{
    _raw._type = VariantRtti::VT_BRIEF_UINT32;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_LONG>()
{
    _raw._type = VariantRtti::VT_BRIEF_LONG;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_ULONG>()
{
    _raw._type = VariantRtti::VT_BRIEF_ULONG;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_PTR>()
{
    _raw._type = VariantRtti::VT_BRIEF_PTR;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_INT64>()
{
    _raw._type = VariantRtti::VT_BRIEF_INT64;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_UINT64>()
{
    _raw._type = VariantRtti::VT_BRIEF_UINT64;
    _raw._briefData._uint64Data = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_FLOAT>()
{
    _raw._type = VariantRtti::VT_BRIEF_FLOAT;
    _raw._briefData._doubleData = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_BRIEF_DOUBLE>()
{
    _raw._type = VariantRtti::VT_BRIEF_DOUBLE;
    _raw._briefData._doubleData = 0;
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_STRING_DEF>()
{
    _raw._type = VariantRtti::VT_STRING_DEF;
    if(!_raw._obj._strData)
        _raw._obj._strData = CRYSTAL_NEW(LibString);
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_DICTIONARY_DEF>()
{
    _raw._type = VariantRtti::VT_DICTIONARY_DEF;
    if(!_raw._obj._dictData)
        _raw._obj._dictData = CRYSTAL_NEW(Dict);
}
template<>
ALWAYS_INLINE void Variant::_BecomeSpecify<VariantRtti::VT_SEQUENCE_DEF>()
{
    _raw._type = VariantRtti::VT_SEQUENCE_DEF;
    if(!_raw._obj._sequenceData)
        _raw._obj._sequenceData = CRYSTAL_NEW(Sequence);
}
    

KERNEL_END

#include <kernel/comp/Variant/VariantImpl.h>

#endif
