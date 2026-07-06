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
 * Date: 2026-07-06 16:59:12
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_INTERFRACE_GLOBAL_ID_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_ID_INTERFRACE_GLOBAL_ID_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>

KERNEL_BEGIN

class IMongodbProxy;

class IGlobalIdMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IGlobalIdMgr);
    
public:
    IGlobalIdMgr(UInt64 objTypeId) : CompHostObject(objTypeId){}

    // [63bit] - [32      -      62bit] - [18 - 31 bit] - [0 - 17]
    //   [0]      时间bit(相对基准时间)      machine id      seq id
    virtual Int64 NewId() = 0;

    // 基于mongodb
    virtual void SetMongoProxy(IMongodbProxy *mongoProxy) = 0;
};

KERNEL_END

#endif