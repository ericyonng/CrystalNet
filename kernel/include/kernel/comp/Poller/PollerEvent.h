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
 * Date: 2022-03-23 11:59:46
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_EVENT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_POLLER_POLLER_EVENT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>

KERNEL_BEGIN

struct KERNEL_EXPORT PollerEvent
{
    POOL_CREATE_OBJ_DEFAULT(PollerEvent);
    PollerEvent(Int32 type);
    virtual ~PollerEvent();

    virtual void Release() = 0;
    template<typename TargetType>
    TargetType *CastTo()
    {
        return reinterpret_cast<TargetType *>(this);
    }
    template<typename TargetType>
    const TargetType *CastTo() const
    {
        return reinterpret_cast<const TargetType *>(this);
    }

    virtual LibString ToString() const;

    Int32 _type;
};

KERNEL_END

#endif
