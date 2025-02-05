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
 * Date: 2024.08.01 22:57:00
 * Author: Eric Yonng
 * Description: 
*/
module;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <pch.h>
#endif

#include <kernel/common/macro.h>
#include <kernel/common/os_libs.h>
#include <kernel/common/statics.h>
#include <kernel/common/status.h>

#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
#include <3rd/3rd.h>

#include <type_traits>  
#include <concepts> 

export module HelloFoo;

// 接口玩别的基类
template<typename T>
concept BaseClassInterface = requires(T target)
{
    // 需要满足Init接口 返回值是Int32
    T::A;
    
    static_cast<Int32>(target.Init());

    // // 需要有Start接口
    // target.Start();

    // // 需要有Close接口
    // target.Close();

    // // 必须有Release接口
    // target.Release();
};

template<typename T>
requires BaseClassInterface<T>
class FooBaseClass
{
public:
};

class TempObj
{
public:
    Int32 Init()
    {
        return 0;
    }

    Int32 A;
};

export class HelloFooSun : public FooBaseClass<TempObj>
{
public:
    Int32 Init();

    virtual Int32 Start();

    virtual void Close(){}

    void Release() {}

    void Print();

    Int32 A;

};

Int32 HelloFooSun::Init()
{
    return 0;
}

Int32 HelloFooSun::Start()
{
    return 0;
}

void HelloFooSun::Print()
{
    g_Log->Info(LOGFMT_OBJ_TAG("this type:%s"), KERNEL_NS::RttiUtil::GetByObj(this).c_str());
}