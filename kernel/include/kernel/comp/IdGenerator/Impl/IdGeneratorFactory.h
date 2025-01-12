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
 * Date: 2025-01-05 16:56:13
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IMPL_IDGENERATOR_FACTORY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IMPL_IDGENERATOR_FACTORY_H__

#pragma once

#include <kernel/comp/CompObject/CompFactory.h>

KERNEL_BEGIN

class KERNEL_EXPORT IdGeneratorFactory : public CompFactory
{
public:
 static constexpr _Build::MT _buildType{};
    
 static CompFactory *FactoryCreate();
 virtual void Release() override;
    
 virtual CompObject *Create() const override;
 // IdGenerator 属于中等优先级
 virtual Int32 GetPriorityLevel() const override { return CompPriorityLevel::MIDDLE; }
};

KERNEL_END

#endif