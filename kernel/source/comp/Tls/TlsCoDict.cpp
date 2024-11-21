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
 * Date: 2024-11-16 19:18:38
 * Author: Eric Yonng
 * Description: 当前线程所有协程管理
*/
#include <pch.h>
#include <kernel/comp/Tls/TlsCoDict.h>
#include <kernel/comp/Coroutines/CoHandle.h>

#include "kernel/comp/Log/ILog.h"

KERNEL_BEGIN
TlsCoDict::TlsCoDict()
:_objTypeName("TlsCoDict")
{
 
}

TlsCoDict::~TlsCoDict()
{
    OnDestroy();
}

void TlsCoDict::OnDestroy()
{
  if(!_idRefHandle.empty())
  {
      // 打印还没销毁的协程
      for(auto iter : _idRefHandle)
      {
          auto handleId = iter.first;
          auto coHandle = iter.second;

          KERNEL_NS::LibString content;
          coHandle->GetBacktrace(content);
          g_Log->Warn(LOGFMT_OBJ_TAG("co not destroy when thread destroy co handle id:%llu, backtrace:\n%s")
              , handleId, content.c_str());
      }

      _idRefHandle.clear();
  }
}


KERNEL_END