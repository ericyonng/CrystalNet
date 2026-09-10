/*
 * @Author: ericyonng 120453674@qq.com
 * @Date: 2026-09-10 00:37:54
 * @LastEditors: ericyonng 120453674@qq.com
 * @LastEditTime: 2026-09-11 01:10:41
 * @FilePath: \XProject\service\LogicService\Comps\PassTime\impl\PassTimeGlobalFactory.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
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
 * Date: 2023-09-17 20:24:11
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <Comps/PassTime/impl/PassTimeGlobalFactory.h>
#include <Comps/PassTime/impl/PassTimeGlobal.h>

#include "kernel/comp/memory/ObjPoolWrap.h"

SERVICE_BEGIN
    KERNEL_NS::CompFactory *PassTimeGlobalFactory::FactoryCreate()
{
    return kernel::ObjPoolWrap<PassTimeGlobalFactory>::NewByAdapter(_buildType.V);
}

void PassTimeGlobalFactory::Release()
{
    KERNEL_NS::ObjPoolWrap<PassTimeGlobalFactory>::DeleteByAdapter(_buildType.V, this);
}

KERNEL_NS::CompObject *PassTimeGlobalFactory::Create() const
{
    CREATE_CRYSTAL_COMP(comp, PassTimeGlobal);
    return comp;
}



SERVICE_END