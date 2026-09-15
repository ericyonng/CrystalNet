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
 * Date: 2026-09-15 10:00:00
 * Author: Eric Yonng
 * Description: windows toast通知组件工厂类实现
*/

#include <pch.h>
#include <OptionComp/WinToast/Impl/WinToastMgrFactory.h>
#include <OptionComp/WinToast/Impl/WinToastMgr.h>

#include "kernel/comp/memory/ObjPoolWrap.h"

KERNEL_BEGIN

KERNEL_NS::CompFactory *WinToastMgrFactory::FactoryCreate()
{
    return ObjPoolWrap<WinToastMgrFactory>::NewByAdapter(_buildType.V);
}

void WinToastMgrFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<WinToastMgrFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *WinToastMgrFactory::Create() const
{
    CREATE_CRYSTAL_COMP(var, WinToastMgr);
    return var;
}

KERNEL_END
