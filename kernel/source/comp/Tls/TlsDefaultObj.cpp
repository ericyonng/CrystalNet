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
 * Date: 2021-01-17 22:12:49
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Tls/TlsDefaultObj.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/Lock/Impl/SpinLock.h>
#include <kernel/comp/Tls/TlsCompsOwner.h>

KERNEL_BEGIN

TlsDefaultObj::TlsDefaultObj()
    :_objTypeName("TlsDefaultObj")
    ,rtti{0}
    ,stackArray{NULL}
    ,_threadId(0)
    ,_threadGlobalId(0)
    ,_thread(NULL)
    ,_threadPool(NULL)
    ,_timerWheel(NULL)
    ,_lck(new SpinLock)
    ,_durtyList(new std::set<MemoryAlloctor *>)
    ,_durtyListSwap(new std::set<MemoryAlloctor *>)
    ,_isForceFreeIdleBuffer(false)
    ,_alloctorTotalBytes(0)
    , _tlsComps(new TlsCompsOwner())
{

}

TlsDefaultObj::~TlsDefaultObj()
{
    OnDestroy();
}

void TlsDefaultObj::OnDestroy()
{
    if(LIKELY(_tlsComps))
    {
        _tlsComps->WillClose();
        _tlsComps->Close();
    }
    CRYSTAL_DELETE_SAFE(_tlsComps);

    CRYSTAL_DELETE_SAFE(_durtyList);
    CRYSTAL_DELETE_SAFE(_durtyListSwap);
    CRYSTAL_DELETE_SAFE(_lck);
}


KERNEL_END