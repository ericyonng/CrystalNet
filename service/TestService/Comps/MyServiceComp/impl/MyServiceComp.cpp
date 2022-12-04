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
 * Date: 2022-06-26 19:19:45
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/TestService/Comps/MyServiceComp/impl/MyServiceComp.h>
#include <service/TestService/Comps/MyServiceComp/impl/MyServiceCompFactory.h>
#include <service/TestService/ServiceFactory.h>

SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MyServiceComp);

MyServiceComp::MyServiceComp()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("%s constructor."), GetObjName().c_str());
}

MyServiceComp::~MyServiceComp()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("%s destructor."), GetObjName().c_str());
    _Clear();
}

void MyServiceComp::Release()
{
    g_Log->Debug(LOGFMT_OBJ_TAG("%s release."), GetObjName().c_str());
    // delete this;
    MyServiceComp::DeleteByAdapter_MyServiceComp(MyServiceCompFactory::_buildType.V, this);
}

KERNEL_NS::LibString MyServiceComp::ToString() const 
{
    g_Log->Debug(LOGFMT_OBJ_TAG("%s to string."), GetObjName().c_str());

    return KERNEL_NS::CompObject::ToString();
}

void MyServiceComp::Clear() 
{
    g_Log->Debug(LOGFMT_OBJ_TAG("%s clear."), GetObjName().c_str());
    _Clear();
    KERNEL_NS::CompObject::Clear();
}

void MyServiceComp::OnUpdate()
{
    // g_Log->Debug(LOGFMT_OBJ_TAG("%s on update."), GetObjName().c_str());
}

Int32 MyServiceComp::_OnCreated()
{
    // 关注update接口,才会被更新到（必须在OnCreated或者构造中设置否则太迟）:
    SetFocus(KERNEL_NS::ObjFocusInterfaceFlag::ON_UPDATE);

    return Status::Success;
}

Int32 MyServiceComp::_OnInit()
{
    // TODO:
    return Status::Success;
}

Int32 MyServiceComp::_OnStart()
{
    // TODO:
    // Int32 *ii = NULL;
    // *ii = 12;

    return Status::Success;
}

void MyServiceComp::_OnWillClose()
{
    // TODO:
}

void MyServiceComp::_OnClose()
{
    // TODO:
}

void MyServiceComp::_Clear()
{
    // TODO:
}

SERVICE_END
