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
 * Date: 2022-06-26 19:19:38
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <service/common/macro.h>

SERVICE_BEGIN

class MyServiceComp : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, MyServiceComp);

public:
    MyServiceComp();
    ~MyServiceComp();
    void Release() final;

    // 除非有线程的初始化,否则按照默认,不重写
    // virtual void DefaultMaskReady(bool isReady);

    virtual KERNEL_NS::LibString ToString() const override;
    virtual void Clear() override;
    
    // OnUpdate默认不会调度到,必须在_OnCreated/构造函数两个地方设置SetFocus才可以调度到
    virtual void OnUpdate() override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

    // 组件接口资源
protected:
    virtual Int32 _OnCreated() override;
    // 
    virtual Int32 _OnInit() override;
    virtual Int32 _OnStart() override;
    virtual void _OnWillClose() override;
    virtual void _OnClose() override;

private:
    void _Clear();

};

SERVICE_END
