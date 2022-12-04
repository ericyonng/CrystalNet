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
 * Date: 2022-03-10 13:02:00
 * Author: Eric Yonng
 * Description: 
 *              // 必要执行时序:OnCheck/Init/Start/Close, 其他可选
 *              时序:OnCheck-OnCreate-Init-BeforeStart-Start-AfterStart-BeforeClose-Close-CloseFinish
 * 1.IObject::Release需要派生类重写
 *  可选虚接口：    virtual LibString ToString() const;
 *                 virtual void Clear();
 * 
 * 2.HostObject需要重写 _OnInit/OnRegisterComps
 * 3.CompObject 需要重写 _OnInit
 * 4.Comp/HostObject可选虚接口:
 *  virtual Int32 _OnBeforeStart();
    virtual Int32 _OnStart();
    virtual Int32 _OnAfterStart();
    virtual void _OnBeforeClose();
    virtual void _OnClose();
    virtual void _OnCloseFinish();
    virtual void _OnUpdate();
    virtual void Clear() override;

    attention：ecs模型的前提是不管任何组件,宿主与子组件的创建与销毁必须在同一个线程,他们是一个整体,不可以跨线程操作,内部的多线程自己处理好相关的线程安全问题
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_OBJECT_INC_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_COMP_OBJECT_COMP_OBJECT_INC_H__

#pragma once

#include <kernel/comp/CompObject/IObject.h>
#include <kernel/comp/CompObject/ObjectFactory.h>
#include <kernel/comp/CompObject/CompFactory.h>
#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/CompObject/CompHostObject.h>

#endif
