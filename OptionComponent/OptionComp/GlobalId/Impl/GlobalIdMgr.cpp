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
 * Date: 2026-07-06 16:59:12
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgr.h>
#include <OptionComp/GlobalId/Impl/GlobalIdMgrFactory.h>
#include <OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h>
#include <kernel/comp/Log/log.h>

KERNEL_BEGIN

GlobalIdMgr::GlobalIdMgr()
 :IGlobalIdMgr(KERNEL_NS::RttiUtil::GetTypeId<GlobalIdMgr>())
,_lastId{0}
,_mongodbMgr(NULL)
{
 
}

GlobalIdMgr::~GlobalIdMgr()
{
 
}

void GlobalIdMgr::Release()
{
    GlobalIdMgr::DeleteByAdapter_GlobalIdMgr(GlobalIdMgrFactory::_buildType.V, this);
}

void GlobalIdMgr::OnRegisterComps()
{
    
}

Int64 GlobalIdMgr::NewId()
{
    return 0;
}

void GlobalIdMgr::SetMongodbMgr(IMongoDbMgr *mongodbMgr)
{
    _mongodbMgr = mongodbMgr;
}

Int32 GlobalIdMgr::_OnHostInit()
{
    if (!_mongodbMgr)
    {
        CLOG_ERROR("have no mongodb mgr");
        return Status::Failed;
    }

    
    return Status::Success;
}

Int32 GlobalIdMgr::_OnHostWillStart()
{
    return Status::Success;
}

void GlobalIdMgr::_OnHostClose()
{
    
}



KERNEL_END
