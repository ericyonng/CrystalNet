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
 * Date: 2023-09-29 17:35:33
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <OptionComp/CodeAnalyze/impl/CodeUnit.h>

KERNEL_BEGIN

CodeUnit::CodeUnit()
{

}

CodeUnit::~CodeUnit()
{

}

void CodeUnit::Release()
{
    CodeUnit::DeleteThreadLocal_CodeUnit(this);
}

SmartPtr<CodeUnit, AutoDelMethods::Release> CodeUnit::CreateCodeUnit()
{
    return CodeUnit::NewThreadLocal_CodeUnit();
}

SmartPtr<CodeUnit, AutoDelMethods::Release> CodeUnitStack::_nullUnit;

CodeUnitStack::CodeUnitStack()
{

}

CodeUnitStack::~CodeUnitStack()
{

}

void CodeUnitStack::Release()
{
    CodeUnitStack::DeleteThreadLocal_CodeUnitStack(this);
}

SmartPtr<CodeUnitStack, AutoDelMethods::Release> CodeUnitStack::Create()
{
    return CodeUnitStack::NewThreadLocal_CodeUnitStack();
}


KERNEL_END

