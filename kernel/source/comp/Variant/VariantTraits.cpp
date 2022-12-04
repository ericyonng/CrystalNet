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
 * Date: 2022-08-21 23:39:54
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Variant/Variant.h>

#include <kernel/comp/Variant/VariantTraits.h>

KERNEL_BEGIN

Variant VariantTraits::add(const Variant &left, const Variant &right)
{
    Variant ret;
    VariantTraits::assign(ret, left);
    VariantTraits::add_equal(ret, right);
    return ret;
}

Variant VariantTraits::sub(const Variant &left, const Variant &right)
{
    Variant ret;
    VariantTraits::assign(ret, left);
    VariantTraits::sub_equal(ret, right);
    return ret;
}

Variant VariantTraits::mul(const Variant &left, const Variant &right)
{
    Variant ret;
    VariantTraits::assign(ret, left);
    VariantTraits::mul_equal(ret, right);
    return ret;
}

Variant VariantTraits::div(const Variant &left, const Variant &right)
{
    Variant ret;
    VariantTraits::assign(ret, left);
    VariantTraits::div_equal(ret, right);
    return ret;
}

void VariantTraits::add_equal(Variant &left, const Variant &right)
{
    // &left == &right
    if(&left == &right)
    {
        Variant clone;
        assign(clone, left);
        add_equal(left, clone);
        return;
    }

    // left is Nil rules:
    // > left[Nil] + Right[Any] = Right
    if(left.IsNil())
    {
        assign(left, right);
        return;
    }

    // Left is Dict rules:
    // > Left[Dict] + Right[Dict] = Dict[Left join Right]
    // > Left[Dict] + Right[Non Dict] = Left
    if (left.IsDict())
    {
        if (right.IsDict())
        {
            Variant::Dict *&lDict = left._GetRaw()._obj._dictData;
            const Variant::Dict *rDict = right.GetRaw()._obj._dictData;
            if (!rDict || rDict->empty())
                return;

            if (lDict)
                lDict->insert(rDict->begin(), rDict->end());
            else
                lDict = CRYSTAL_NEW(Variant::Dict)(*rDict);
        }

        return;
    }

    // Left is Seq rules:
    // > Left[Seq] + Right[Dict] = Seq[Left join Right.Keys]
    // > Left[Seq] + Right[Seq] = Seq[Left join Right]
    // > Left[Seq] + Right[Non Seq/Dict] = Left Append Right
    if (left.IsSeq())
    {
        Variant::Sequence *&lSeq = left._GetRaw()._obj._sequenceData;
        if (right.IsDict())
        {
            const Variant::Dict *rDict = right.GetRaw()._obj._dictData;
            if (!rDict || rDict->empty())
                return;

            if (!lSeq)
                lSeq = CRYSTAL_NEW(Variant::Sequence);
            if (lSeq->capacity() < lSeq->size() + rDict->size())
                lSeq->reserve(lSeq->size() + rDict->size());

            Variant::DictConstIter rEndIt = rDict->end();
            for (Variant::DictConstIter rIt = rDict->begin(); rIt != rEndIt; ++rIt)
                lSeq->push_back(rIt->second);
        }
        else if (right.IsSeq())
        {
            const Variant::Sequence *rSeq = right.GetRaw()._obj._sequenceData;
            if (!rSeq || rSeq->empty())
                return;

            if (lSeq)
                lSeq->insert(lSeq->end(), rSeq->begin(), rSeq->end());
            else
                lSeq = CRYSTAL_NEW(Variant::Sequence)(*rSeq);
        }
        else
        {
            left.SeqPushBack(right);
        }

        return;
    }

    // Left is Str rules:
    // Left[Str] + Right[Dict/Seq] = Left
    // Left[Str] + Right[Str] = Left[left str + right str]
    // Left[Str] + Right[Raw/Nil] = Left + Right.AsStr()
    if (left.IsStr())
    {
        LibString *&lStr = left._GetRaw()._obj._strData;
        if (right.IsDict() || right.IsSeq())
        {
            return;
        }
        else if (right.IsStr())
        {
            LibString *rStr = right.GetRaw()._obj._strData;
            if (!rStr || rStr->empty())
                return;

            if (lStr)
                *lStr += *rStr;
            else
                lStr = CRYSTAL_NEW(LibString)(*rStr);
        }
        else
        {
            if (lStr)
                *lStr += right.AsStr();
            else
                lStr = CRYSTAL_NEW(LibString)(right.AsStr());
        }

        return;
    }

    // Left is Raw rules:
    // > Left[Raw] + Right[Dict/Seq/Nil] = Left
    // > Left[Raw] + Right[Str] = Left.AsStr() + Right
    // > Left[Raw] + Right[Raw] = VariantArithmetic::Performs(l, r, op)
    if (left.IsBriefData())
    {
        if (right.IsStr())
        {
            left = left.AsStr();
            add_equal(left, right);
            return;
        }
        else if (right.IsBriefData())
        {
            VariantArithmetic::Performs(left, right, VariantArithmetic::VT_ARITHMETIC_ADD);
        }

        return;
    }
}

void VariantTraits::sub_equal(Variant &left, const Variant &right)
{

// &Left == &Right rules:
    if (&left == &right)
    {
        Variant clone;
        assign(clone, left);
        sub_equal(left, clone);
        return;
    }

    // Left is Nil rules:
    // > Left[Nil] - Right[Any] = Left
    if (left.IsNil())
        return;

    // Left is Dict rules:
    // > Left[Dict] - Right[Dict/Seq] = Dict[Left - Right]
    // > Left[Dict] - Right[Non Dict/Seq] = Dict[Left -Right(as key)]
    if (left.IsDict())
    {
        Variant ::Dict *&lDict = left._GetRaw()._obj._dictData;
        if (!lDict || lDict->empty())
            return;

        if (right.IsDict())
        {
            const Variant::Dict *rDict = right.GetRaw()._obj._dictData;
            if (!rDict || rDict->empty())
                return;

            const Variant::DictConstIter rEndIt = rDict->end();
            for (Variant::DictConstIter rIt = rDict->begin(); rIt != rEndIt; ++rIt)
                lDict->erase(rIt->first);
        }
        else if (right.IsSeq())
        {
            const Variant::Sequence *rSeq = right.GetRaw()._obj._sequenceData;
            if (!rSeq || rSeq->empty())
                return;

            const Variant::SequenceConstIter rEndIt = rSeq->end();
            for (Variant::SequenceConstIter rIt = rSeq->begin(); rIt != rEndIt; ++rIt)
            {
                lDict->erase(*rIt);
                if (lDict->empty())
                    return;
            }
        }
        else
        {
            lDict->erase(right);
        }

        return;
    }

    // Left is Seq rules:
    // > Left[Seq] - Right[Dict] = Seq[Left - Right.Keys]
    // > Left[Seq] - Right[Seq] = Seq[Left - Right]
    // > Left[Seq] - Right[Non Seq/Dict] = Left - Right(as left element)
    if (left.IsSeq())
    {
        Variant::Sequence *&lSeq = left._GetRaw()._obj._sequenceData;
        if (!lSeq || lSeq->empty())
            return;

        if (right.IsDict())
        {
            const Variant::Dict *rDict = right.GetRaw()._obj._dictData;
            if (!rDict || rDict->empty())
                return;

            Variant::DictConstIter rEndIt = rDict->end();
            for (Variant::DictConstIter rIt = rDict->begin(); rEndIt != rDict->end(); ++rIt)
            {
                lSeq->erase(std::remove_if(lSeq->begin(), lSeq->end(), 
                            [&rIt](const Variant &elem) { return elem == rIt->second; }), lSeq->end());
                if (lSeq->empty())
                    return;
            }
        }
        else if (right.IsSeq())
        {
            const Variant::Sequence *rSeq = right.GetRaw()._obj._sequenceData;
            if (!rSeq || rSeq->empty())
                return;

            Variant::SequenceConstIter rItEnd = rSeq->end();
            for (Variant::SequenceConstIter rIt = rSeq->begin(); rIt != rItEnd; ++rIt)
            {
                lSeq->erase(std::remove_if(lSeq->begin(), lSeq->end(), 
                            [&rIt](const Variant &elem) { return elem == *rIt; }), lSeq->end());
                if (lSeq->empty())
                    return;
            }
        }
        else
        {
            lSeq->erase(std::remove_if(lSeq->begin(), lSeq->end(),
                        [&right](const Variant &elem) { return elem == right; }), lSeq->end());
        }

        return;
    }

    // Left is Str rules:
    // > Left[Str] - Right[Nil/Seq/Dict] = Left
    // Left[Str] - Right[Str] = Left str - Right str
    // Left[Str] - Right[Raw] = Left - Right.AsStr()
    if (left.IsStr())
    {
        LibString *&lStr = left._GetRaw()._obj._strData;
        if (!lStr || lStr->empty())
            return;

        if (right.IsStr())
        {
            const LibString *rStr = right.GetRaw()._obj._strData;
            if (!rStr || rStr->empty())
                return;

            auto &lStrRaw = lStr->GetRaw();
            auto &rStrRaw = rStr->GetRaw();
            LibString::size_type pos = lStrRaw.find(rStrRaw);
            while (pos != std::string::npos)
            {
                lStrRaw.erase(pos, rStrRaw.size());
                pos = lStrRaw.find(rStrRaw);
            }
        }
        else if (right.IsBriefData())
        {
            const LibString rStr = right.AsStr();
            auto &lStrRaw = lStr->GetRaw();
            auto &rStrRaw = rStr.GetRaw();

            LibString::size_type pos = lStrRaw.find(rStrRaw);
            while (pos != std::string::npos)
            {
                lStrRaw.erase(pos, rStrRaw.size());
                pos = lStrRaw.find(rStrRaw);
            }
        }

        return;
    }

    // Left is Raw rules:
    // Left[Raw] - Right[Dict/Seq/Nil] = Left
    // Left[Raw] - Right[Str] = Left.AsStr() - Right
    // Left[Raw] - Right[Raw] = VariantArithmetic::Performs(l, r, op)
    if (left.IsBriefData())
    {
        if (right.IsStr())
        {
            left = left.AsStr();
            sub_equal(left, right);
        }
        else if (right.IsBriefData())
        {
            VariantArithmetic::Performs(left, right, VariantArithmetic::VT_ARITHMETIC_SUB);
        }

        return;
    }
}


void VariantTraits::mul_equal(Variant &left, const Variant &right)
{
    // &Left == &Right rules:
    if (&left == &right)
    {
        Variant clone;
        assign(clone, left);
        mul_equal(left, clone);

        return;
    }

    // Left or Right is Nil rules:
    // > Left[Nil] or Right[Nil] mul = Nil
    if (left.IsNil() || right.IsNil())
    {
        left.BecomeNil();
        return;
    }

    // Left is Dict rules:
    // > Left[Dict] * Right[Dict/Seq] = Intersection Of Left and Right
    // > Left[Dict] * Right[Str/BriefData] = Left[Dict]
    else if (left.IsDict())
    {
        Variant::Dict *lDict = left._GetRaw()._obj._dictData;
        if (!lDict || lDict->empty())
            return;

        if (right.IsSeq() || right.IsDict())
        {
            if (right.IsEmpty())
            {
                lDict->clear();
                return;
            }

            for (Variant::DictIter lIt = lDict->begin(); lIt != lDict->end(); )
            {
                if ((right.IsDict() && right.FindDict(lIt->first) == right.EndDict()) ||
                    (right.IsSeq() && std::find(right.SeqBegin(), right.SeqEnd(), lIt->first) == right.SeqEnd()))
                    lDict->erase(lIt++);
                else
                    ++lIt;
            }
        }

        return;
    }

    // Left is Seq rules:
    // > Left[Seq] * Right[Dict/Seq] = Intersection On Left and Right
    // > Left[Seq] * Right[Str] = Left
    // > Left[Seq] * Right[BriefData] = Left[Seq] repeat right.AsInt32() times
    else if (left.IsSeq())
    {
        Variant::Sequence *lSeq = left._GetRaw()._obj._sequenceData;
        if (!lSeq || lSeq->empty())
            return;

        if (right.IsSeq() || right.IsDict())
        {
            if (right.IsEmpty())
            {
                lSeq->clear();
                return;
            }

            Int64 lSeqSize = static_cast<Int64>(lSeq->size());
            for (Int64 lIdx = lSeqSize - 1; lIdx >= 0; --lIdx)
            {
                if ((right.IsDict() && right.FindDict(lSeq->at(lIdx)) == right.EndDict()) ||
                    (right.IsSeq() && std::find(right.SeqBegin(), right.SeqEnd(), lSeq->at(lIdx)) == right.SeqEnd()))
                    lSeq->erase(lSeq->begin() + lIdx);
            }
        }
        else if (right.IsBriefData())
        {
            Int32 rRaw = right.AsInt32();
            if (rRaw <= 0)
            {
                lSeq->clear();
                return;
            }
            else if (rRaw == 1)
            {
                return;
            }

            size_t lSeqReserve = lSeq->size() * rRaw;
            if (lSeq->capacity() < lSeqReserve)
                lSeq->reserve(lSeqReserve);

            for (Int32 lIdx = 1; lIdx < rRaw; ++lIdx)
                lSeq->insert(lSeq->end(), lSeq->begin(), lSeq->end());
        }

        return;
    }

    // Left is Str rules:
    // > Left[Str] * Right[Dict/Seq] = Right[Dict/Seq] * Left[Str]
    // > Left[Str] * Right[Str] = Left
    // > Left[Str] * Right[Str] = Left[Str] repeat Right.AsInt32() times
    else if (left.IsStr())
    {
        if (right.IsDict() || right.IsSeq())
        {
            left = mul(right, left);
        }
        else if (right.IsBriefData())
        {
            LibString *lStr = left._GetRaw()._obj._strData;
            if (!lStr || lStr->empty())
                return;

            *lStr *= right.AsInt32();
        }

        return;
    }

    // Left is BriefData rules:
    // > Left[BriefData] * Right[Dict/Seq/Str] = Right[Dict/Seq/Str] * Left[BriefData]
    // > Left[BriefData] * Right[BriefData] = Left[left BriefData * right BriefData]
    else if (left.IsBriefData())
    {
        if (right.IsDict() || right.IsSeq() || right.IsStr())
        {
            left = mul(right, left);
        }
        else if (right.IsBriefData())
        {
            VariantArithmetic::Performs(left, right, VariantArithmetic::VT_ARITHMETIC_MUL);
        }

        return;
    }
}

void VariantTraits::assign(Variant &left, const Variant &right)
{
    // TODO
    if(&left == &right)
        return;

    const UInt64 oldLeftType = left.GetType();

    // Execute assignment.
    if(right.IsNil())// Do NIL type data assignment.
    {
        left.BecomeNil();
    }
    else if(right.IsBriefData()) // Do RAW type data assignment.
    {
        // If left type is STR/DICT type, clean first.
        if(!left.IsMainType(VariantRtti::VT_BRIEF_DATA))
            left._CleanTypeData(oldLeftType);

        // Assignment.
        left._GetRaw() = right.GetRaw();
    }
    else if(right.IsStr()) // Do STR type data assignment.
    {
        if(!left.IsMainType(VariantRtti::VT_STRING))
            left._CleanTypeData(oldLeftType);

        auto &lRaw = left._GetRaw();
        left._SetType(right.GetType());
        if(oldLeftType != VariantRtti::VT_STRING_DEF)
            lRaw._obj._strData = CRYSTAL_NEW(LibString);

        // Assignment.
        const auto &rRaw = right.GetRaw();
        if(rRaw._obj._strData == NULL || rRaw._obj._strData->empty())
        {
            lRaw._obj._strData->clear();
        }
        else
        {
            if(lRaw._obj._strData != rRaw._obj._strData)
                *lRaw._obj._strData = *rRaw._obj._strData;
        }
    }
    else if(right.IsDict()) // Do DICT type data assignment.
    {
        // If left type is RAW/STR type, clean first.
        if(!left.IsMainType(VariantRtti::VT_DICTIONARY))
            left._CleanTypeData(oldLeftType);

        left._SetType(right.GetType());
        auto &lRaw = left._GetRaw();
        if(oldLeftType != VariantRtti::VT_DICTIONARY_DEF)
            lRaw._obj._dictData = CRYSTAL_NEW(Variant::Dict);

        const auto &rRaw = right.GetRaw();
        if(rRaw._obj._dictData == NULL || rRaw._obj._dictData->empty())
        {
            lRaw._obj._dictData->clear();
        }
        else
        {
            if(lRaw._obj._dictData != rRaw._obj._dictData)
                *lRaw._obj._dictData = *rRaw._obj._dictData;
        }
    }
    else if(right.IsSeq())
    {
        // If left type is RAW/STR type, clean first.
        if(!left.IsMainType(VariantRtti::VT_SEQUENCE))
            left._CleanTypeData(oldLeftType);

        left._SetType(right.GetType());
        auto &lRaw = left._GetRaw();
        if(oldLeftType != VariantRtti::VT_SEQUENCE_DEF)
            lRaw._obj._sequenceData = CRYSTAL_NEW(Variant::Sequence);

        const auto &rRaw = right.GetRaw();
        if(rRaw._obj._sequenceData == NULL || rRaw._obj._sequenceData->empty())
        {
            lRaw._obj._sequenceData->clear();
        }
        else
        {
            if(lRaw._obj._sequenceData != rRaw._obj._sequenceData)
                *lRaw._obj._sequenceData = *rRaw._obj._sequenceData;
        }
    }
    else
    {
        CRYSTAL_TRACE("VariantTraits::assign unknown right variant type:%llu", right.GetType());
    }
}

bool VariantTraits::eq(const Variant &left, const Variant &right)
{
    if(&left == &right)
        return true;

    const auto &lRaw = left.GetRaw();
    const auto &rRaw = right.GetRaw();
    if(left.IsStr())
    {
        if(!right.IsStr())
            return false;

        if(lRaw._obj._strData)
        {
            if(rRaw._obj._strData)
                return *lRaw._obj._strData == *rRaw._obj._strData;
            else
                return lRaw._obj._strData->empty();
        }
        else
        {
            if(rRaw._obj._strData)
                return rRaw._obj._strData->empty();
            else
                return true;
        }
    }
    else if(left.IsDict())
    {
        if(!right.IsDict())
            return false;

        const auto *lDict = lRaw._obj._dictData;
        const auto *rDict = rRaw._obj._dictData;
        if(lDict)
        {
            if(rDict)
                return *lDict == *rDict;
            else
                return lDict->empty();
        }
        else
        {
            if(rDict)
                return rDict->empty();
            else
                return true;
        }
    }
    else if(left.IsBriefData())
    {
        if(!right.IsBriefData())
            return false;

        if((left.IsDouble() || left.IsFloat()) ||
            (right.IsDouble() || right.IsFloat()))
        {
            // TODO:浮点数精度问题
            return  left.AsDouble() == right.AsDouble();
        }

        return (left.GetRaw()._briefData._uint64Data ==
                right.GetRaw()._briefData._uint64Data);
    }
    else if(left.IsSeq())
    {
        if(!right.IsSeq())
            return false;

        const auto *lSeq = lRaw._obj._sequenceData;
        const auto *rSeq = rRaw._obj._sequenceData;
        if(lSeq)
        {
            if(rSeq)
                return *lSeq == *rSeq;
            else
                return lSeq->empty();
        }
        else
        {
            if(rSeq)
                return rSeq->empty();
            else
                return true;
        }
    }

    return (left.IsNil() && right.IsNil());
}

bool VariantTraits::lt(const Variant &left, const Variant &right)
{
    if(&left == &right)
        return false;

    if(left.IsDict())
    {
        if(right.IsDict())
        {
            const auto *lDict = left.GetRaw()._obj._dictData;
            const auto *rDict = right.GetRaw()._obj._dictData;

            if(lDict == rDict)
                return false;

            if(lDict == NULL || lDict->empty())
                return rDict && !rDict->empty();
            if(rDict == NULL || rDict->empty())
                return false;

            return *lDict < *rDict;
        }
       
        return false;
    }
    else if(left.IsSeq())
    {
        if(right.IsDict())  // seq < dict : true
            return true;
        if(right.IsSeq())
        {
            const auto *lSeq = left.GetRaw()._obj._sequenceData;
            const auto *rSeq = right.GetRaw()._obj._sequenceData;
            if(lSeq)
                return rSeq && (*lSeq < *rSeq);

            return rSeq && !rSeq->empty();
        }
        
        // str/ brief/nil :false
        return false;
    }
    else if(left.IsStr())
    {
        if(right.IsDict() || right.IsSeq()) // dict/seq: true
            return true;
        else if(right.IsStr())
        {
            const auto *lStr = left.GetRaw()._obj._strData;
            const auto *rStr = right.GetRaw()._obj._strData;
            if(lStr)
                return rStr && (*lStr < *rStr);

            return rStr && !rStr->empty();
        }
        
        return false;
    }
    else if(left.IsBriefData())
    {
        if(right.IsDict() || right.IsSeq() || right.IsStr())    // Dict/Seq/Str: true
            return true;
        else if(right.IsBriefData())
        {
            if((left.IsDouble() || left.IsFloat()) ||
                (right.IsDouble() || right.IsFloat()))
                return left.AsDouble() < right.AsDouble();
            
            if(left.IsUnsignedBriefData() && right.IsUnsignedBriefData())
                return left.AsUInt64() < right.AsUInt64();
            else if(left.IsSignedBriefData() && right.IsSignedBriefData())
            {
                return left.AsInt64() < right.AsInt64();
            }
            else if(left.IsSignedBriefData() && right.IsUnsignedBriefData())
            {
                const Int64 lNum = left.AsInt64();
                if(lNum < 0)
                    return true;
                
                return static_cast<UInt64>(lNum) < right.AsUInt64();
            }
            else if(left.IsUnsignedBriefData() && right.IsSignedBriefData())
            {
                const Int64 rNum = right.AsInt64();
                if(rNum < 0)
                    return true;
                
                return left.AsUInt64() < static_cast<UInt64>(rNum);
            }
        }
       
        return false;
    }
   
    return !right.IsNil();
}

void VariantTraits::div_equal(Variant &left, const Variant &right)
{
    // &Left == &Right rules:
    if (&left == &right)
    {
        Variant clone;
        assign(clone, left);
        div_equal(left, clone);

        return;
    }

    // Left[Nil] or Right[Nil] = Nil
    if (left.IsNil() || right.IsNil())
    {
        left.BecomeNil();
        return;
    }

    // Left[Dict/Seq/Str] / Right[Non Nil] = Left # undefined
    if (!left.IsBriefData())
        return;

    // Left[BriefData] / Right[Dict/Seq/Str] = Left[BriefData]
    if (!right.IsBriefData())
        return;

    // Left[BriefData] / Right[BriefData] = VariantArithmetic(l, r, op)
    VariantArithmetic::Performs(left, right, VariantArithmetic::VT_ARITHMETIC_DIV);
}

KERNEL_END