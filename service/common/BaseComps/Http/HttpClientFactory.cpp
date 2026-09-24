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
 * Date: 2026-09-24 10:00:00
 * Author: CrystalNet
 * Description: http客户端组件工厂实现
*/

#include <pch.h>
#include <kernel/comp/Http/HttpClient.h>
#include <kernel/comp/Http/HttpClientFactory.h>
#include <kernel/comp/memory/ObjPoolWrap.h>

KERNEL_BEGIN

CompFactory *HttpClientFactory::FactoryCreate()
{
    return KERNEL_NS::ObjPoolWrap<HttpClientFactory>::NewByAdapter(_buildType.V);
}

void HttpClientFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<HttpClientFactory>::DeleteByAdapter(_buildType.V, this);
}

CompObject *HttpClientFactory::Create() const
{
    CREATE_CRYSTAL_COMP(comp, HttpClient);
    return comp;
}

KERNEL_END
