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
 * Date: 2021-03-23 14:20:19
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_IBLACK_WHITE_LIST_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_NET_ENGINE_IBLACK_WHITE_LIST_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/BlackWhiteList/BlackWhiteDef.h>
#include <kernel/comp/Lock/Lock.h>

KERNEL_BEGIN

template<typename Elem>
class BlackWhiteList
{
public:
    BlackWhiteList();
    virtual ~BlackWhiteList();

public:
    void Lock();
    void Unlock();
    bool SetMode(UInt32 flag);
    bool Check(const Elem &e) const;

    void PushWhite(Elem &e);
    void PushWhite(const Elem &e);
    void EraseWhite(const Elem &e);
    void EraseWhite(Elem &e);

    void PushBlack(Elem &e);
    void PushBlack(const Elem &e);
    void EraseBlack(const Elem &e);
    void EraseBlack(Elem &e);

    const std::set<Elem> &GetBlackList() const;
    const std::set<Elem> &GetWhiteList() const;

    void Clear();

    // 策略接口
private:
    bool _AllowAll(const Elem &e) const;
    bool _CheckWhite(const Elem &e) const;
    bool _CheckBlack(const Elem &e) const;
    bool _CheckWhiteBlack(const Elem &e) const;
    bool _CheckWhiteBlackAllowUnknown(const Elem &e) const;

private:
    SpinLock _lck;
    std::set<Elem> _black;
    std::set<Elem> _white;

    typedef bool (BlackWhiteList<Elem>::*cb)(const Elem &) const;
    cb _check;
    std::vector<cb> _checkArray;
};

template<typename Elem>
ALWAYS_INLINE BlackWhiteList<Elem>::BlackWhiteList()
{
    // 所欲掩码组合
    const UInt32 allowAllMask = BlackWhiteFlag::AllowAllFlag;  // 全通过
    const UInt32 whiteMask = BlackWhiteFlag::CheckWhiteFlag;   // 只允许白名单通过
    const UInt32 blackMask = BlackWhiteFlag::CheckBlackFlag;   // 只过滤黑名单通过
    const UInt32 whiteBlackMask = BlackWhiteFlag::CheckBlackFlag | BlackWhiteFlag::CheckWhiteFlag;    // 过滤黑名单且在白名单内
    const UInt32 whiteBlackAllowUnknownMask = BlackWhiteFlag::CheckBlackFlag | BlackWhiteFlag::CheckWhiteFlag | BlackWhiteFlag::AllowUnknownFlag;    // 过滤黑名单白名单允许未知通过

    // 初始化接口
    _checkArray.resize(BlackWhiteFlag::AllowAllFlag | BlackWhiteFlag::CheckWhiteFlag | BlackWhiteFlag::CheckBlackFlag | BlackWhiteFlag::AllowUnknownFlag);
    _checkArray[allowAllMask] = &BlackWhiteList<Elem>::_AllowAll;
    _checkArray[whiteMask] = &BlackWhiteList<Elem>::_CheckWhite;
    _checkArray[blackMask] = &BlackWhiteList<Elem>::_CheckBlack;
    _checkArray[whiteBlackMask] = &BlackWhiteList<Elem>::_CheckWhiteBlack;
    _checkArray[whiteBlackAllowUnknownMask] = &BlackWhiteList<Elem>::_CheckWhiteBlackAllowUnknown;

    // 默认全通过
    _check = _checkArray[allowAllMask];
}

template<typename Elem>
inline BlackWhiteList<Elem>::~BlackWhiteList()
{

}

template<typename Elem>
inline void BlackWhiteList<Elem>::Lock()
{
    _lck.Lock();
}

template<typename Elem>
inline void BlackWhiteList<Elem>::Unlock()
{
    _lck.Unlock();
}

template<typename Elem>
inline bool BlackWhiteList<Elem>::SetMode(UInt32 flag)
{
    auto cb = _checkArray[flag];
    if(UNLIKELY(!cb))
        return false;

    _check = cb;
    return true;
}

template<typename Elem>
inline bool BlackWhiteList<Elem>::Check(const Elem &e) const
{
    return (this->*_check)(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::PushWhite(Elem &e)
{
    _black.erase(e);
    _white.insert(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::PushWhite(const Elem &e)
{
    _black.erase(e);
    _white.insert(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::EraseWhite(const Elem &e)
{
    _white.erase(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::EraseWhite(Elem &e)
{
    _white.erase(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::PushBlack(Elem &e)
{
    _white.erase(e);
    _black.insert(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::PushBlack(const Elem &e)
{
    _white.erase(e);
    _black.insert(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::EraseBlack(const Elem &e)
{
    _black.erase(e);
}

template<typename Elem>
inline void BlackWhiteList<Elem>::EraseBlack(Elem &e)
{
    _black.erase(e);
}

template<typename Elem>
inline const std::set<Elem> &BlackWhiteList<Elem>::GetBlackList() const
{
    return _black;
}

template<typename Elem>
inline const std::set<Elem> &BlackWhiteList<Elem>::GetWhiteList() const
{
    return _white;
}

template<typename Elem>
inline void BlackWhiteList<Elem>::Clear()
{
    _black.clear();
    _white.clear();
}

template<typename Elem>
inline bool BlackWhiteList<Elem>::_AllowAll(const Elem &e) const
{
    return true;
}

template<typename Elem>
inline bool BlackWhiteList<Elem>::_CheckWhite(const Elem &e) const
{
    return _white.find(e) != _white.end();
}

template<typename Elem>
inline bool BlackWhiteList<Elem>::_CheckBlack(const Elem &e) const
{
    return _black.find(e) == _black.end();
}

template<typename Elem>
inline bool BlackWhiteList<Elem>::_CheckWhiteBlack(const Elem &e) const
{
    // 黑名单禁止
    if(_black.find(e) != _black.end())
        return false;

    // 白名单通过
    return _white.find(e) != _white.end();
}

template<typename Elem>
inline bool BlackWhiteList<Elem>::_CheckWhiteBlackAllowUnknown(const Elem &e) const
{
    // 黑名单禁止
    if(_black.find(e) != _black.end())
        return false;

    // 白名单通过
     if(_white.find(e) != _white.end())
        return true;

    // 未知通过
    return true;   
}


KERNEL_END

#endif
