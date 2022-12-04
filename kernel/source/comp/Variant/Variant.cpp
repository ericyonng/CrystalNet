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
 * Date: 2021-03-16 11:39:34
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Variant/VariantTraits.h>
#include <kernel/comp/Variant/Variant.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(Variant);

const LibString VariantAssist::__g_nullStr;   // 前提必须是str类型
const LibString VariantAssist::__g_nilStr = "nil";   // nil 类型
const LibString VariantAssist::__g_trueToOneStr = "1";
const LibString VariantAssist::__g_falseToZeroStr = "0";
const LibString VariantAssist::__g_trueStr = "true";
const LibString VariantAssist::__g_falseStr = "false";
const LibString VariantAssist::__g_emptySeqStr = "[]";
const LibString VariantAssist::__g_emptyDictStr = "{}";

std::unordered_map<UInt64, LibString> VariantRtti::_rttiTypeRefString;
const LibString VariantRtti::_nullString;

const LibString &VariantRtti::GetTypeName(UInt64 rttiType)
{
    auto iterStr = _rttiTypeRefString.find(rttiType);
    return iterStr != _rttiTypeRefString.end() ? iterStr->second : _rttiTypeRefString[VT_NIL];
}

void VariantRtti::InitRttiTypeNames()
{
    if(LIKELY(_rttiTypeRefString.empty()))
    {
        _rttiTypeRefString.insert(std::make_pair(VT_NIL, "Nil"));

        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_BOOL, "bool"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_BYTE8, "Byte8"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_UINT8, "U8"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_INT16, "Int16"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_UINT16, "UInt16"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_INT32, "Int32"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_UINT32, "UInt32"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_LONG, "Long"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_ULONG, "ULong"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_PTR, "Pointer"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_INT64, "Int64"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_UINT64, "UInt64"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_FLOAT, "Float"));
        _rttiTypeRefString.insert(std::make_pair(VT_BRIEF_DOUBLE, "Double"));
        _rttiTypeRefString.insert(std::make_pair(VT_STRING_DEF, "string"));

        _rttiTypeRefString.insert(std::make_pair(VT_DICTIONARY_DEF, "dictionary"));
        _rttiTypeRefString.insert(std::make_pair(VT_SEQUENCE_DEF, "sequence"));
        _rttiTypeRefString.insert(std::make_pair(VT_NIL, "NULL"));
    }
}


Variant &Variant::operator =(const Variant &val)
{
    VariantTraits::assign(*this, val);
    return *this;
}

bool Variant::operator ==(const Variant &another) const
{
    return VariantTraits::eq(*this, another);
}

bool Variant::operator !=(const Variant &another) const
{
    return VariantTraits::ne(*this, another);
}

bool Variant::operator <(const Variant &another) const
{
    return VariantTraits::lt(*this, another);
}

bool Variant::operator >(const Variant &another) const
{
    return VariantTraits::gt(*this, another);
}

bool Variant::operator <=(const Variant &another) const
{
    return VariantTraits::le(*this, another);
}

bool Variant::operator >=(const Variant &another) const
{
    return VariantTraits::ge(*this, another);
}

Variant Variant::operator +(const Variant &another) const
{
    return VariantTraits::add(*this, another);
}

Variant Variant::operator -(const Variant &another) const
{
    return VariantTraits::sub(*this, another);
}

Variant Variant::operator *(const Variant &another) const
{
    return VariantTraits::mul(*this, another);
}

Variant Variant::operator /(const Variant &another) const
{
    return VariantTraits::div(*this, another);
}

Variant &Variant::operator +=(const Variant &another)
{
    VariantTraits::add_equal(*this, another);
    return *this;
}

Variant &Variant::operator -=(const Variant &another)
{
    VariantTraits::sub_equal(*this, another);
    return *this;
}

Variant &Variant::operator *=(const Variant &another)
{
    VariantTraits::mul_equal(*this, another);
    return *this;
}

Variant &Variant::operator /=(const Variant &another)
{
    VariantTraits::div_equal(*this, another);
    return *this;
}

LibString Variant::AsStr() const
{
    if(IsBriefData())
    {
        if(IsBool())
            return _raw._briefData._uint64Data ? VariantAssist::__g_trueStr : VariantAssist::__g_falseStr;
        else if(IsFloat() || IsDouble())
            return StringUtil::Num2Str(_raw._briefData._doubleData);
        else if(IsSignedBriefData())
            return StringUtil::Num2Str(_raw._briefData._int64Data);
        else if(IsPtr())
            return LibString().AppendFormat("0x%x", static_cast<UInt32>(_raw._briefData._uint64Data));
        return StringUtil::Num2Str(_raw._briefData._uint64Data);
    }
    
    if(IsStr())
        return _raw._obj._strData ? *_raw._obj._strData : VariantAssist::__g_nullStr;

    if(IsSeq())
    {
        if(_raw._obj._sequenceData->empty())
            return VariantAssist::__g_emptySeqStr;
        
        LibString content;
        std::string &contentRaw = content.GetRaw();
        // const auto elemCount = _raw._obj._sequenceData->size();
        contentRaw.reserve(64); // elemCount + elemCount - 1 + 2
        contentRaw.append(1, '[');
        auto &seqData = _raw._obj._sequenceData;
        for(auto iter = seqData->begin(); iter != seqData->end();)
        {
            auto &elem = *iter;
            contentRaw.append(elem.AsStr().GetRaw());
            if(++iter != seqData->end())
                contentRaw.append(1, ',');
        }
        contentRaw.append(1, ']');
        return content;
    }

    if(IsDict())
    {
        if(_raw._obj._dictData->empty())
            return VariantAssist::__g_emptyDictStr;

        auto &dictData = _raw._obj._dictData;
        LibString content;
        std::string &contentRaw = content.GetRaw();
        contentRaw.reserve(64);
        contentRaw.append(1, '{');
        for(auto iter = dictData->begin(); iter != dictData->end();)
        {
            contentRaw.append(iter->first.AsStr().GetRaw());
            contentRaw.append(1, ':');
            contentRaw.append(iter->second.AsStr().GetRaw());
            
            if(++iter != dictData->end())
                contentRaw.append(1, ',');
        }
        contentRaw.append(1, '}');
        return content;
    }

    return VariantAssist::__g_nilStr;
}

KERNEL_END

