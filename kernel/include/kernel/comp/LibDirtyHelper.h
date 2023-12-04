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
 * Date: 2021-08-12 00:05:34
 * Author: Eric Yonng
 * Description: 
 *              TODO:需要测试
 * 标脏仅限于单线程,不可跨线程使用
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_DIRTY_HELPER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_DIRTY_HELPER_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <kernel/comp/Utils/BitUtil.h>
#include <kernel/comp/Variant/variant_inc.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

template<typename KeyType, typename MaskValue>
struct DirtyMask
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(DirtyMask, KeyType, MaskValue);

    DirtyMask(KeyType key, Int32 dirtyTypeAmount)
        :_mask(0)
    {
        _dirtyTypeRefVariant.resize(dirtyTypeAmount);
        _key = key;
    }

    ~DirtyMask()
    {
        ContainerUtil::DelContainer(_dirtyTypeRefVariant, [](Variant * &elem)->void{
            if(!elem)
                return;

            Variant::DeleteThreadLocal_Variant(elem);
            elem = NULL;
        });
    }

    void ClearFlag(Int32 dirtyType)
    {
        _mask = BitUtil::Clear(_mask, dirtyType);
        auto variant = _dirtyTypeRefVariant[dirtyType];
        if(UNLIKELY(variant))
        {
            Variant::DeleteThreadLocal_Variant(variant);
            _dirtyTypeRefVariant[dirtyType] = NULL;
        }
    }

    void ClearAll()
    {
        _mask = 0;
        const Int32 amount = static_cast<Int32>(_dirtyTypeRefVariant.size());
        for(Int32 idx = 0; idx < amount; ++idx)
        {
            if(_dirtyTypeRefVariant[idx])
            {
                Variant::DeleteThreadLocal_Variant(_dirtyTypeRefVariant[idx]);
                _dirtyTypeRefVariant[idx] = NULL;
            }
        }
    }

    Variant *MaskDirty(Int32 dirtyType, bool fillParams = false)
    {
        _mask = BitUtil::Set(_mask, dirtyType);

        Variant *params = NULL;
        if(UNLIKELY(fillParams))
        {
            if(!_dirtyTypeRefVariant[dirtyType])
                _dirtyTypeRefVariant[dirtyType] = Variant::NewThreadLocal_Variant();

            params = _dirtyTypeRefVariant[dirtyType];
        }

        return params;
    }

    Variant *GetVar(Int32 dirtyType)
    {
        return _dirtyTypeRefVariant[dirtyType];
    }

    const Variant *GetVar(Int32 dirtyType) const
    {
        return _dirtyTypeRefVariant[dirtyType];
    }

    Variant *AddVar(Int32 dirtyType)
    {
        if(UNLIKELY(_dirtyTypeRefVariant[dirtyType]))
            return _dirtyTypeRefVariant[dirtyType];

        auto var = Variant::NewThreadLocal_Variant();
        _dirtyTypeRefVariant[dirtyType] = var;
        return var;
    }

    bool IsDirty(Int32 dirtyType) const
    {
        return BitUtil::IsSet(_mask, dirtyType);
    }

    LibString ToString() const
    {
        LibString info;
        info << "key = ";
        info << _key;
        info.AppendFormat(", mask = [%llu]", static_cast<UInt64>(_mask));

        return info;
    }

    MaskValue _mask;                                // 脏标记
    std::vector<Variant *> _dirtyTypeRefVariant;    // 标脏所携带的参数
    KeyType _key;
};

template<typename KeyType, typename MaskValue>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(DirtyMask, KeyType, MaskValue);

template<typename KeyType, typename MaskValue>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(DirtyMask, KeyType, MaskValue);

template<typename KeyType, typename MaskValue>
struct DirtyHelperDelayOp
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(DirtyHelperDelayOp, KeyType, MaskValue);

    DirtyHelperDelayOp()
        :_mask(NULL)
    {

    }

    ~DirtyHelperDelayOp()
    {
        if(_mask)
        {
            DirtyMask<KeyType, MaskValue>::DeleteThreadLocal_DirtyMask(_mask);
            _mask = NULL;
        }
    }

    DirtyMask<KeyType, MaskValue> *_mask;
    KeyType _key;
};

template<typename KeyType, typename MaskValue>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(DirtyHelperDelayOp, KeyType, MaskValue);

template<typename KeyType, typename MaskValue>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(DirtyHelperDelayOp, KeyType, MaskValue);


template<typename KeyType, typename MaskValue>
class LibDirtyHelper
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(LibDirtyHelper, KeyType, MaskValue);
public:
    LibDirtyHelper();
    ~LibDirtyHelper();

    void Init(Int32 dirtyTypeAmount);
    void Destroy();
    void SetHandler(Int32 dirtyType, IDelegate<void, LibDirtyHelper<KeyType, MaskValue> *, KeyType &, Variant *> *handler);
    void SetHandler(Int32 dirtyType, const IDelegate<void, LibDirtyHelper<KeyType, MaskValue> *, KeyType &, Variant *> *handler);

    // // 操作
    Variant *MaskDirty(KeyType k, Int32 dirtyType, bool fillParams = false);
    void Clear(KeyType k, Int32 dirtyType);
    void Clear(KeyType k);
    bool IsDirty(KeyType k, Int32 dirtyType) const;
    const DirtyMask<KeyType, MaskValue> *GetMask(KeyType k) const;
    DirtyMask<KeyType, MaskValue> *GetMask(KeyType k);

    std::map<KeyType, DirtyMask<KeyType, MaskValue> *> &GetAllMasks();
    

    bool HasDirty() const;
    bool HasDirty(const KeyType &k) const;
    bool IsInPurge() const;
    UInt64 GetLoaded() const;
    // 脏标记需要在回调中移除,否则一直在
    Int64 Purge(LibString *errorLog = NULL);

private:
    void _AfterPurge();

private:
    Int32 _dirtyTypeAmount;
    Int32 _inPurge;
    // key => mask
    std::map<KeyType, DirtyMask<KeyType, MaskValue> *> _keyRefMask;
    // DirtyType => handler
    std::vector<IDelegate<void, LibDirtyHelper<KeyType, MaskValue> *, KeyType &, Variant *> *> _dirtyTypeRefHandler;      // dirtyType 按顺序执行

    // 延迟执行 只有_keyRefMask 中没有才会添加到delay队列
    std::vector<DirtyHelperDelayOp<KeyType, MaskValue> *> _delayOperations;
    std::map<KeyType, DirtyHelperDelayOp<KeyType, MaskValue> *> _delayOpDict;
};

template<typename KeyType, typename MaskValue>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(LibDirtyHelper, KeyType, MaskValue);

template<typename KeyType, typename MaskValue>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_TL_IMPL(LibDirtyHelper, KeyType, MaskValue);


template<typename KeyType, typename MaskValue>
ALWAYS_INLINE LibDirtyHelper<KeyType, MaskValue>::LibDirtyHelper()
    :_dirtyTypeAmount(0)
    ,_inPurge(0)
{
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE LibDirtyHelper<KeyType, MaskValue>::~LibDirtyHelper()
{
    Destroy();
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE void LibDirtyHelper<KeyType, MaskValue>::Init(Int32 dirtyTypeAmount)
{
    _dirtyTypeAmount = dirtyTypeAmount;
    _dirtyTypeRefHandler.resize(_dirtyTypeAmount);
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE void LibDirtyHelper<KeyType, MaskValue>::Destroy()
{
    if(!_keyRefMask.empty())
    {
        // g_Log->Warn(LOGFMT_OBJ_TAG( "has dirty elem not purge count = [%llu]"), static_cast<UInt64>(_keyRefMask.size()));

        ContainerUtil::DelContainer(_keyRefMask, [this](DirtyMask<KeyType, MaskValue> *&mask)->void{
            // g_Log->Warn(LOGFMT_OBJ_TAG("dirty key = [%s] not purge"), mask->ToString().c_str());

            DirtyMask<KeyType, MaskValue>::DeleteThreadLocal_DirtyMask(mask);
            mask = NULL;
        });
    }

    ContainerUtil::DelContainer(_dirtyTypeRefHandler, [](IDelegate<void, LibDirtyHelper<KeyType, MaskValue> *, KeyType &, Variant *> *delg)->void{
        CRYSTAL_RELEASE_SAFE(delg);
    });

    if(!_delayOperations.empty())
    {
        ContainerUtil::DelContainer(_delayOperations, [this](DirtyHelperDelayOp<KeyType, MaskValue> *delay)->void{
            // g_Log->Warn(LOGFMT_OBJ_TAG("delay dirty mask = [%s] not purge"), delay->_mask->ToString().c_str());

            DirtyHelperDelayOp<KeyType, MaskValue>::DeleteThreadLocal_DirtyHelperDelayOp(delay);
            delay = NULL;
        });
    }
    _delayOpDict.clear();
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE void LibDirtyHelper<KeyType, MaskValue>::SetHandler(Int32 dirtyType, IDelegate<void, LibDirtyHelper<KeyType, MaskValue> *, KeyType &, Variant *> *handler)
{
    auto h = _dirtyTypeRefHandler[dirtyType];
    if(UNLIKELY(h))
        CRYSTAL_DELETE_SAFE(h);
    _dirtyTypeRefHandler[dirtyType] = handler;
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE void LibDirtyHelper<KeyType, MaskValue>::SetHandler(Int32 dirtyType, const IDelegate<void, LibDirtyHelper<KeyType, MaskValue> *, KeyType &, Variant *> *handler)
{
    auto h = _dirtyTypeRefHandler[dirtyType];
    if(UNLIKELY(h))
        CRYSTAL_DELETE_SAFE(h);
    _dirtyTypeRefHandler[dirtyType] = const_cast<IDelegate<void, LibDirtyHelper<KeyType, MaskValue> *, KeyType &, Variant *> *>(handler);
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE Variant *LibDirtyHelper<KeyType, MaskValue>::MaskDirty(KeyType k, Int32 dirtyType, bool fillParams)
{
    auto iter = _keyRefMask.find(k);
    DirtyMask<KeyType, MaskValue> *mask = NULL;
    if(UNLIKELY(iter == _keyRefMask.end()))
    {// 初次标脏
        if(UNLIKELY(IsInPurge()))
        {// purge中maskDirty
            auto iterDelay = _delayOpDict.find(k);
            if(iterDelay == _delayOpDict.end())
            {// 新增
                auto newDelayInfo = DirtyHelperDelayOp<KeyType, MaskValue>::NewThreadLocal_DirtyHelperDelayOp();
                newDelayInfo->_key = k;
                mask = DirtyMask<KeyType, MaskValue>::NewThreadLocal_DirtyMask(k, _dirtyTypeAmount);
                newDelayInfo->_mask = mask;

                // 映射
                _delayOperations.push_back(newDelayInfo);
                _delayOpDict.insert(std::make_pair(k, newDelayInfo));
            }
            else
            {
                mask = iterDelay->second->_mask;
            }
        }
        else
        {
            mask = DirtyMask<KeyType, MaskValue>::NewThreadLocal_DirtyMask(k, _dirtyTypeAmount);
            _keyRefMask.insert(std::make_pair(k, mask));
        }
    }
    else
    {
        mask = iter->second;
    }

    return mask->MaskDirty(dirtyType, fillParams);
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE void LibDirtyHelper<KeyType, MaskValue>::Clear(KeyType k, Int32 dirtyType)
{
    DirtyMask<KeyType, MaskValue> *mask = GetMask(k);
    if(LIKELY(mask))
        mask->ClearFlag(dirtyType);
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE void LibDirtyHelper<KeyType, MaskValue>::Clear(KeyType k)
{
    DirtyMask<KeyType, MaskValue> *mask = GetMask(k);
    if(LIKELY(mask))
        mask->ClearAll();
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE bool LibDirtyHelper<KeyType, MaskValue>::IsDirty(KeyType k, Int32 dirtyType) const
{
    auto mask = GetMask(k);
    if(UNLIKELY(!mask))
       return false;

    return mask->IsDirty(dirtyType);
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE const DirtyMask<KeyType, MaskValue> *LibDirtyHelper<KeyType, MaskValue>::GetMask(KeyType k) const
{
    auto iter = _keyRefMask.find(k);
    DirtyMask<KeyType, MaskValue> *mask = NULL;
    if(UNLIKELY(iter == _keyRefMask.end()))
    {
        auto iterDelay = _delayOpDict.find(k);
        if(iterDelay != _delayOpDict.end())
            mask = iterDelay->second->_mask;
    }
    else
    {
        mask = iter->second;
    }

    return mask;
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE DirtyMask<KeyType, MaskValue> *LibDirtyHelper<KeyType, MaskValue>::GetMask(KeyType k)
{
    auto iter = _keyRefMask.find(k);
    DirtyMask<KeyType, MaskValue> *mask = NULL;
    if (UNLIKELY(iter == _keyRefMask.end()))
    {
        auto iterDelay = _delayOpDict.find(k);
        if (iterDelay != _delayOpDict.end())
            mask = iterDelay->second->_mask;
    }
    else
    {
        mask = iter->second;
    }

    return mask;
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE std::map<KeyType, DirtyMask<KeyType, MaskValue> *> &LibDirtyHelper<KeyType, MaskValue>::GetAllMasks()
{
    return _keyRefMask;
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE bool LibDirtyHelper<KeyType, MaskValue>::IsInPurge() const
{
    return _inPurge > 0;
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE UInt64 LibDirtyHelper<KeyType, MaskValue>::GetLoaded() const
{
    return _keyRefMask.size();
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE bool LibDirtyHelper<KeyType, MaskValue>::HasDirty() const
{
    return !_keyRefMask.empty();
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE bool LibDirtyHelper<KeyType, MaskValue>::HasDirty(const KeyType &k) const
{
    auto iter = _keyRefMask.find(k);
    return iter != _keyRefMask.end();
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE Int64 LibDirtyHelper<KeyType, MaskValue>::Purge(LibString *errorLog)
{
    Int64 handled = 0;
    ++_inPurge;
    for(auto iter = _keyRefMask.begin(); iter != _keyRefMask.end();)
    {
        auto mask = iter->second;
        if(UNLIKELY(!mask))
        {
            if(errorLog)
                errorLog->AppendFormat("zero mask in dirty queue key:").Append(iter->first);
            iter = _keyRefMask.erase(iter);
            continue;
        }

        for(Int32 i = 0; i < _dirtyTypeAmount; ++i)
        {
            if(!BitUtil::IsSet(mask->_mask, i))
                continue;

            auto handler = _dirtyTypeRefHandler[i];
            if(LIKELY(handler))
            {
                auto &variantDict = mask->_dirtyTypeRefVariant;
                auto &k = (KeyType &)(iter->first);
                handler->Invoke(this, k, variantDict[i]);
                ++handled;
            }
            else
            {
                mask->ClearFlag(i);
            }
        }

        // 没有脏之后移除, 还有脏表示下次还需要继续
        if(LIKELY(!mask->_mask))
        {
            DirtyMask<KeyType, MaskValue>::DeleteThreadLocal_DirtyMask(mask);
            iter = _keyRefMask.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    _AfterPurge();

    return handled;
}

template<typename KeyType, typename MaskValue>
ALWAYS_INLINE void LibDirtyHelper<KeyType, MaskValue>::_AfterPurge()
{
    --_inPurge;
    if(_inPurge <= 0)
    {
        _inPurge = 0;
        const Int32 delayAmount = static_cast<Int32>(_delayOperations.size());
        for(Int32 i = 0; i < delayAmount; ++i)
        {
            auto op = _delayOperations[i];
            _keyRefMask.insert(std::make_pair(op->_key, op->_mask));
            op->_mask = NULL;
            DirtyHelperDelayOp<KeyType, MaskValue>::DeleteThreadLocal_DirtyHelperDelayOp(op);
        }
        _delayOperations.clear();
        _delayOpDict.clear();
    }
}

KERNEL_END

#endif
