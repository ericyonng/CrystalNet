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
 * Date: 2023-02-19 22:12:07
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <kernel/comp/CompObject/CompObject.h>
#include <kernel/comp/LibString.h>
#include <map>

class IXlsxExporterMgr : public KERNEL_NS::CompObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IXlsxExporterMgr);

public:
    IXlsxExporterMgr(UInt64 objTypeId) : KERNEL_NS::CompObject(objTypeId){}

    virtual Int32 ExportConfigs(const std::map<KERNEL_NS::LibString, KERNEL_NS::LibString> &params) = 0;
};
