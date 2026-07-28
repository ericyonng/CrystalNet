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
 * Date: 2026-07-28 15:49:18
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/GlobalLifeCtrl.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

GlobalLifeCtrl::GlobalLifeCtrl()
{
 
}

GlobalLifeCtrl::~GlobalLifeCtrl()
{
 for (auto &deleg : _cbs)
 {
    if (g_Log)
    {
      CLOG_DEBUG("global life ctrl deleg:%s", deleg->GetCallbackRtti().c_str());
    }

  deleg->Invoke();
   CRYSTAL_RELEASE_SAFE(deleg);
 }
}

void GlobalLifeCtrl::Register(IDelegate<void> *deleg)
{
 _lck.Lock();
 _cbs.push_back(deleg);
 _lck.Unlock();
}

KERNEL_END