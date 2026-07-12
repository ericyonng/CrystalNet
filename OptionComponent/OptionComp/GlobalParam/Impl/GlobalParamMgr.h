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

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_PARAM_IMPL_GLOBAL_PARAM_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_OPTIONCOMP_GLOBAL_PARAM_IMPL_GLOBAL_PARAM_MGR_H__

#pragma once

#include <OptionComponent/OptionComp/GlobalParam/Interface/IGlobalParamMgr.h>

KERNEL_BEGIN

class GlobalParamMgr : public IGlobalParamMgr
{
  POOL_CREATE_OBJ_DEFAULT_P1(IGlobalParamMgr, GlobalParamMgr);

public:
    GlobalParamMgr();
    virtual ~GlobalParamMgr() override;

    void Release() override;

    virtual void SetMongodbMgr(IMongoDbMgr *mongodbMgr) override;
    
    // 添加参数
    virtual CoTask<bool> UpdateParam(const KERNEL_NS::LibString &paramName, std::map<LibString, Variant> *keyRefValue) override;
    
    // 原子更新参数
    virtual CoTask<bool> AtomicUpdateParam(const KERNEL_NS::LibString &paramName, std::map<LibString, Variant> *keyRefValue, void *condition, LibString *jsonBackOrigin) override;

    virtual const KERNEL_NS::LibString &GetUniqueKeyFieldName() const override;

protected:
    Int32 _OnHostInit() override;
    Int32 _OnHostStart() override;
    void _OnHostClose() override;

    void _Clear();

    // db名
    const LibString _db;
    // 表名
    const LibString _collectionName;
    // 唯一key字段名
    const LibString _uniqueIndexFieldName;
    // 索引名
    const LibString _indexName;

    IMongoDbMgr *_mongodbMgr;
};

KERNEL_END


#endif