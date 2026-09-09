/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2026-09-09 11:57:25
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Log/log.h>
#include <LogicServer/LogicServerApp.h>
#include <OptionComp/Command/Command.h>
#include <OptionComp/GlobalParam/GlobalParam.h>
#include <OptionComp/GlobalId/GlobalId.h>
#include "OptionComp/GlobalParam/Impl/GlobalParamMgr.h"
#include "OptionComp/storage/MongoDB/Impl/MongoDbMgrFactory.h"
#include "OptionComp/storage/MongoDB/Impl/MongodbProxyFactory.h"
#include "OptionComp/storage/MongoDB/Interface/IMongoDbMgr.h"

void LogicServerApp::Release()
{
    LogicServerApp::Delete_LogicServerApp(this);
}

void LogicServerApp::OnRegisterComps()
{
    // 先注册数据库, 让数据库先初始化好
    RegisterComp<KERNEL_NS::MongoDbMgrFactory>();
    // 全局参数
    RegisterComp<KERNEL_NS::GlobalParamMgrFactory>();
    // 全球id
    RegisterComp<KERNEL_NS::GlobalIdMgrFactory>();

    SERVICE_COMMON_NS::Application::OnRegisterComps();

    // 注册命令行工具
    RegisterComp<KERNEL_NS::CommandMgrFactory>();
}

Int32 LogicServerApp::_OnCompsCreated()
{
    auto err = SERVICE_COMMON_NS::Application::_OnCompsCreated();
    if(err != Status::Success)
    {
        CLOG_WARN("comps created fail err:%d", err);
        return err;
    }  

    // 注册关服命令
    auto commandMgr = GetComp<KERNEL_NS::ICommandMgr>();
    commandMgr->AddCommand("quit", [this]()
    {
        SinalFinish(Status::Success);
    });

    auto mongodbMgr = GetComp<KERNEL_NS::IMongoDbMgr>();
    mongodbMgr->SetConfigSource(*GetSourceWrap());
    mongodbMgr->SetConfigKeyName("MongoOptions");

    // 设置GlobalParam参数
    auto globalParamMgr = GetComp<KERNEL_NS::IGlobalParamMgr>();
    globalParamMgr->SetMongodbMgr(mongodbMgr);

    // 设置GlobalId参数
    auto globalIdMgr = GetComp<KERNEL_NS::IGlobalIdMgr>();
    globalIdMgr->SetMongodbMgr(mongodbMgr);
    globalIdMgr->SetGlobalParamMgr(globalParamMgr);

    return Status::Success;
}


Int32 LogicServerApp::_OnHostWillStart()
{
    auto moduleId = GetAppModuleId();
    auto err = SERVICE_COMMON_NS::Application::_OnHostWillStart();
    if(err != Status::Success)
    {
        CLOG_WARN("_OnHostWillStart err:%d moduleId:%llu", err, moduleId);
        return err;
    }

    CLOG_INFO("app start success moduleId:%llu", moduleId);

    return Status::Success;
}
