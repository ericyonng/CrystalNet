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
 * Date: 2022-02-24 13:53:05
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCK_ADAPTER_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOCK_IMPL_LOCK_ADAPTER_H__

#pragma once

#include <kernel/comp/Lock/Impl/LockType.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/Lock/Impl/Locker.h>
#include <kernel/comp/Lock/Impl/ConditionLocker.h>
#include <kernel/common/status.h>

KERNEL_BEGIN

template<LockType::ENUMS SpecifyLockType>
class LockAdapter
{
};

// 适配spinlock
template<>
class KERNEL_EXPORT LockAdapter<LockType::SpinLock>
{
public:
    ALWAYS_INLINE void Lock() {  _raw.Lock();}
    ALWAYS_INLINE void Unlock() { _raw.Unlock();}
    ALWAYS_INLINE bool TryLock() { return _raw.TryLock();}

    ALWAYS_INLINE Int32 Wait() { return Status::Failed;}
    ALWAYS_INLINE Int32 TimeWait(UInt64 second, UInt64 microSec) { return Status::Failed;}
    ALWAYS_INLINE Int32 TimeWait(UInt64 milliSecond) { return Status::Failed;}

    ALWAYS_INLINE bool Sinal() { return false;}
    ALWAYS_INLINE bool HasWaiter() { return false;}
    ALWAYS_INLINE void Broadcast() { }
    ALWAYS_INLINE void ResetSinal() { }
    ALWAYS_INLINE void ResetSinalFlag()  { }
    ALWAYS_INLINE bool IsSinal() const { return false;}

private:
    SpinLock _raw;
};


// 适配 MutexLock
template<>
class KERNEL_EXPORT LockAdapter<LockType::MutexLock>
{
public:
     ALWAYS_INLINE void Lock() { _raw.Lock();}
     ALWAYS_INLINE void Unlock() { _raw.Unlock();}
     ALWAYS_INLINE bool TryLock() { return _raw.TryLock();}

     ALWAYS_INLINE Int32 Wait() { return Status::Failed;}
     ALWAYS_INLINE Int32 TimeWait(UInt64 second, UInt64 microSec) { return Status::Failed;}
     ALWAYS_INLINE Int32 TimeWait(UInt64 milliSecond) { return Status::Failed;}

     ALWAYS_INLINE bool Sinal() { return false;}
     ALWAYS_INLINE bool HasWaiter() { return false;}
     ALWAYS_INLINE void Broadcast() { }
     ALWAYS_INLINE void ResetSinal() { }
     ALWAYS_INLINE void ResetSinalFlag() { }
     ALWAYS_INLINE bool IsSinal() const { return false; }
 
private:
    Locker _raw;
};

// 适配 ConditionLock
template<>
class KERNEL_EXPORT LockAdapter<LockType::ConditionLock>
{
public:
    ALWAYS_INLINE void Lock() { _raw.Lock();}
    ALWAYS_INLINE void Unlock()  { _raw.Unlock();}
    ALWAYS_INLINE bool TryLock() { return _raw.TryLock();}

    ALWAYS_INLINE Int32 Wait() { return _raw.Wait();}
    ALWAYS_INLINE Int32 TimeWait(UInt64 second, UInt64 microSec) { return _raw.TimeWait(second, microSec);}
    ALWAYS_INLINE Int32 TimeWait(UInt64 milliSecond) { return _raw.TimeWait(milliSecond); }

    ALWAYS_INLINE bool Sinal() { return _raw.Sinal();}
    ALWAYS_INLINE bool HasWaiter() { return _raw.HasWaiter();}
    ALWAYS_INLINE void Broadcast() { _raw.Broadcast();}
    ALWAYS_INLINE void ResetSinal() {  _raw.ResetSinal(); }
    ALWAYS_INLINE void ResetSinalFlag() { _raw.ResetSinalFlag(); }
    ALWAYS_INLINE bool IsSinal() const { return _raw.IsSinal();}

private:
    ConditionLocker _raw;
};

// 适配 dummy
template<>
class KERNEL_EXPORT LockAdapter<LockType::DummyLock>
{
public:
    ALWAYS_INLINE void Lock()  { }
    ALWAYS_INLINE void Unlock()  { }
    ALWAYS_INLINE bool TryLock()  { return true;}

    ALWAYS_INLINE Int32 Wait() { return Status::DummyHandle;}
    ALWAYS_INLINE Int32 TimeWait(UInt64 second, UInt64 microSec) { return Status::DummyHandle;}
    ALWAYS_INLINE Int32 TimeWait(UInt64 milliSecond) { return Status::DummyHandle; }

    ALWAYS_INLINE bool Sinal() { return true;}
    ALWAYS_INLINE bool HasWaiter() { return false;}
    ALWAYS_INLINE void Broadcast() { }
    ALWAYS_INLINE void ResetSinal() { }
    ALWAYS_INLINE void ResetSinalFlag()  { }
    ALWAYS_INLINE bool IsSinal() const { return false;}
    
private:
};

KERNEL_END

#endif
