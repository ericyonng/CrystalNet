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
 * Date: 2026-07-10 09:46:12
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_PARAM_INTERFRACE_IGLOBAL_PARAM_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_PARAM_INTERFRACE_IGLOBAL_PARAM_MGR_H__

#pragma once

#include <kernel/comp/CompObject/CompHostObject.h>

KERNEL_BEGIN

class IMongoDbMgr;

class IGlobalParamMgr : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IGlobalParamMgr);
    
public:
   IGlobalParamMgr(UInt64 objTypeId) : CompHostObject(objTypeId) {}

    // 设置mongodb
    virtual void SetMongodbMgr(IMongoDbMgr *mongodbMgr) = 0;

    // 添加参数
    virtual bool UpdateParam(const KERNEL_NS::LibString &paramName, std::map<LibString, Variant> *keyRefValue) = 0;
    
    // 原子更新参数
    virtual bool AtomicUpdateParam(const KERNEL_NS::LibString &paramName, std::map<LibString, Variant> *keyRefValue, void *condition) = 0;

    // 原子自增
    virtual bool AtomicUpdateParam(const KERNEL_NS::LibString &paramName, std::map<LibString, Variant> *keyRefValue, void *condition) = 0;

};

KERNEL_END


#endif