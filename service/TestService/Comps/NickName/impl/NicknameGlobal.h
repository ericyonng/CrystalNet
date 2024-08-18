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
 * Date: 2023-09-04 13:32:11
 * Author: Eric Yonng
 * Description: 
*/

#pragma once

#include <Comps/NickName/interface/INicknameGlobal.h>
#include <kernel/comp/LibStream.h>
#include <set>
#include <map>

SERVICE_BEGIN

class NicknameGlobal : public INicknameGlobal
{
    POOL_CREATE_OBJ_DEFAULT_P1(INicknameGlobal, NicknameGlobal);

public:
    NicknameGlobal();
    ~NicknameGlobal();
    void Release() override;
    void OnRegisterComps() override;

    Int32 OnLoaded(UInt64 key, const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) override;
    Int32 OnSave(UInt64 key, KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &db) const override;
    
    // 检查昵称
    virtual bool CheckNickname(const KERNEL_NS::LibString &nickname) const override;

    // 随机生成昵称:nick + 唯一id
    virtual void GenRandNickname(KERNEL_NS::LibString &newName) override;

    virtual void AddUsedNickname(const KERNEL_NS::LibString &nickname) override;

    OBJ_GET_OBJ_TYPEID_DECLARE();

protected:
    void _AddName(UInt64 id, const KERNEL_NS::LibString &name);
    bool _IsExists(const KERNEL_NS::LibString &name) const;

private:
    std::set<KERNEL_NS::LibString> _historyNicknames;
    std::map<UInt64, KERNEL_NS::LibString> _idRefNickname;
};

SERVICE_END
