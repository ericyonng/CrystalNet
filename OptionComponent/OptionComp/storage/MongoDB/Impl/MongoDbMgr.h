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
 * Date: 2025-10-14 14:15:00
 * Author: Eric Yonng
 * Description: mongodb数据库管理
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_MGR_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_MONGODB_MGR_H__

#pragma once

#include <OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h>

KERNEL_BEGIN

class MongoDbMgr : public IMongoDbMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IMongoDbMgr, MongoDbMgr);

public:
    MongoDbMgr();
    ~MongoDbMgr() override;

    // 释放
    virtual void Release() override;
    // 需要手动设置ready
    virtual void DefaultMaskReady(bool isReady) override {}
    virtual void OnRegisterComps() override;

protected:
    virtual Int32 _OnHostInit() override;
    virtual Int32 _OnCompsCreated() override;
    virtual Int32 _OnHostStart() override;
    virtual void _OnHostBeforeCompsWillClose() override;
    virtual void _OnHostClose() override;
};

KERNEL_END

#endif
