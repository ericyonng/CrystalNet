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
 * Date: 2022-02-15 02:36:32
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/MemoryMonitor/Statistics.h>
#include <kernel/comp/Utils/ContainerUtil.h>
#include <kernel/comp/Delegate/IDelegate.h>
#include <kernel/common/macro.h>

KERNEL_BEGIN

Statistics::~Statistics()
{
    ContainerUtil::DelContainer<IDelegate<UInt64, LibString &> *, AutoDelMethods::Release>(_addrRefDeleg);
}

void Statistics::Remove(IDelegate<UInt64, LibString &> *delg)
{
    if(_addrRefDeleg.empty())
    {
        CRYSTAL_TRACE("Statistics _addrRefDeleg empty perhaps have released before");
        return;
    }

    const Int64 delgSize = static_cast<Int64>(_addrRefDeleg.size());
    for(Int64 idx = delgSize - 1; idx >= 0; --idx)
    {
        if(_addrRefDeleg[idx] == delg)
        {
            _addrRefDeleg.erase(_addrRefDeleg.begin() + idx);
            CRYSTAL_RELEASE_SAFE(delg);
            // CRYSTAL_TRACE("Statistics remove a delegate idx = [%lld], left count=[%llu]", idx, static_cast<UInt64>(_addrRefDeleg.size()));
            break;
        }
    }
}

KERNEL_END
