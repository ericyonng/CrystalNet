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
 * Date: 2022-01-29 14:21:44
 * Author: Eric Yonng
 * Description: 
 *              // 必要执行时序:Init/Start/Close, 其他可选
 *              时序:OnCreated-Init-Start-WillClose-Close
 *              派生类需要重写的函数：_OnInit/Clear/Release
 *              派生类可选重写的函数：
 *                          OnUpdate/ToString/TurnFocusToString/
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_OBJECT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_OBJECT_H__

#pragma once

#include <kernel/comp/CompObject/IObject.h>

KERNEL_BEGIN

class KERNEL_EXPORT CompObject : public IObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(IObject, CompObject);

public:
    CompObject();
    virtual ~CompObject();

    // // api
public:
    virtual LibString ToString() const override;
    virtual void Clear() override;
    virtual void OnUpdate() override;
    // 从宿主中被弹出
    virtual void OnPop(CompObject *oldOwner) {}
    // 更换了宿主
    virtual void OnBindNewHost(CompObject *oldOwner) {}

    // 组件接口资源
protected:
    virtual Int32 _OnCreated() override;
    virtual Int32 _OnInit() override;
    // start 可以启动线程，再此之前都不可以启动线程
    virtual Int32 _OnStart() override;
    virtual void _OnWillClose() override;
    virtual void _OnClose() override;

private:
    void _Clear();
};


KERNEL_END

#endif
