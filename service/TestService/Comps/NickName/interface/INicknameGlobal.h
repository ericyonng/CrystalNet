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
 * Date: 2023-09-04 13:26:11
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <ServiceCompHeader.h>
#include <service/common/BaseComps/GlobalSys/GlobalSys.h>
#include <kernel/comp/LibString.h>

SERVICE_BEGIN

class INicknameGlobal : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, INicknameGlobal);

public:
    INicknameGlobal(UInt64 objTypeId) : IGlobalSys(objTypeId) {}
    
    // 检查昵称: 唯一性, 昵称规则
    virtual bool CheckNickname(const KERNEL_NS::LibString &nickname) const = 0;
    // 随机生成昵称:nick + 唯一id
    virtual void GenRandNickname(KERNEL_NS::LibString &newName) = 0;
    // 添加已使用的昵称
    virtual void AddUsedNickname(const KERNEL_NS::LibString &nickname) = 0;
};

SERVICE_END
