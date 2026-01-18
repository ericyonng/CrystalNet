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
 * Date: 2023-07-17 00:40:00
 * Author: Eric Yonng
 * Description: TestService logic sys 接口扩展
*/

#include <pch.h>
#include <service/common/BaseComps/LogicSys/LogicSys.h>

// 存储
#ifdef CRYSTAL_STORAGE_ENABLE

#include <service/TestService/Comps/DB/db.h>

#endif

SERVICE_BEGIN

void ILogicSys::MaskDirty()
{

}

void ILogicSys::MaskNumberKeyAddDirty(UInt64 key)
{
#ifdef CRYSTAL_STORAGE_ENABLE
    auto mysqlMgr = GetService()->GetComp<IMysqlMgr>();
    if(mysqlMgr)
        mysqlMgr->MaskLogicNumberKeyAddDirty(this, key);
#endif
}

void ILogicSys::MaskNumberKeyModifyDirty(UInt64 key)
{
#ifdef CRYSTAL_STORAGE_ENABLE
    auto mysqlMgr = GetService()->GetComp<IMysqlMgr>();
    if(mysqlMgr)
        mysqlMgr->MaskLogicNumberKeyModifyDirty(this, key);
#endif
}

void ILogicSys::MaskNumberKeyDeleteDirty(UInt64 key)
{
#ifdef CRYSTAL_STORAGE_ENABLE
    auto mysqlMgr = GetService()->GetComp<IMysqlMgr>();
    if(mysqlMgr)
        mysqlMgr->MaskLogicNumberKeyDeleteDirty(this, key);
#endif
    
}

void ILogicSys::MaskStringKeyAddDirty(const KERNEL_NS::LibString &key)
{
#ifdef CRYSTAL_STORAGE_ENABLE
    auto mysqlMgr = GetService()->GetComp<IMysqlMgr>();
    if(mysqlMgr)
        mysqlMgr->MaskLogicStringKeyAddDirty(this, key);
#endif
}

void ILogicSys::MaskStringKeyModifyDirty(const KERNEL_NS::LibString &key)
{
#ifdef CRYSTAL_STORAGE_ENABLE
    auto mysqlMgr = GetService()->GetComp<IMysqlMgr>();
    if(mysqlMgr)
        mysqlMgr->MaskLogicStringKeyModifyDirty(this, key);
#endif
}

void ILogicSys::MaskStringKeyDeleteDirty(const KERNEL_NS::LibString &key)
{
#ifdef CRYSTAL_STORAGE_ENABLE
    auto mysqlMgr = GetService()->GetComp<IMysqlMgr>();
    if(mysqlMgr)
        mysqlMgr->MaskLogicStringKeyDeleteDirty(this, key);
#endif
}

SERVICE_END
