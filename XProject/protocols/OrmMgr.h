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
 * Date: 2023-12-16 22:09:37
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __PROTOCOLS_ORM_MGR_H__
#define __PROTOCOLS_ORM_MGR_H__

#pragma once

#include <protocols/IOrmMgr.h>
#include <service_common/common/macro.h>
#include <unordered_map>

SERVICE_COMMON_BEGIN

class OrmMgr : public IOrmMgr
{
    POOL_CREATE_OBJ_DEFAULT_P1(IOrmMgr, OrmMgr);

public:
    OrmMgr();
    ~OrmMgr();
    virtual void Release() override;

    // 获取所有ORM工厂
    virtual const std::unordered_map<Int64, IOrmDataFactory *> &GetAllOrmFactorys() const override;

    // 创建ORM实例
    virtual IOrmData *CreateOrmData(Int64 ormId) const override;

    virtual void AddOrmFactory(IOrmDataFactory *factory) override;

protected:
    virtual Int32 _OnInit() override;
    virtual void _OnClose() override;

private:
    void _Clear();

    std::unordered_map<Int64, IOrmDataFactory *> _ormIdRefOrmFactory;
};

SERVICE_COMMON_END

#endif