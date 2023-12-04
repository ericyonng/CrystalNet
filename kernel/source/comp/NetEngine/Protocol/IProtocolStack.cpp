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
 * Date: 2023-11-26 14:51:40
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Protocol/IProtocolStack.h>
#include <kernel/comp/LibTime.h>
#include <kernel/comp/Utils/CypherGeneratorUtil.h>
#include <kernel/comp/Coder/coder.h>

KERNEL_BEGIN

bool IProtocolStack::UpdateKey()
{
    const auto nowTime = LibTime::NowMilliTimestamp();
    if(LIKELY(_expireTime > nowTime))
        return true;

    _expireTime = nowTime + _expireIntervalMs;

    _key.clear();
    _base64Key.clear();
    _cypherKey.clear();
    KERNEL_NS::CypherGeneratorUtil::SpeedGen<KERNEL_NS::_Build::TL>(_key, KERNEL_NS::CypherGeneratorUtil::CYPHER_128BIT);
    
    if(_packetToBinRsa.IsPubEncryptPrivDecrypt())
    {
        _packetToBinRsa.PubKeyEncrypt(_key, _cypherKey);
    }
    else
    {
        _packetToBinRsa.PrivateKeyEncrypt(_key, _cypherKey);
    }

    if(UNLIKELY(_cypherKey.empty()))
    {
        _expireTime = nowTime;
        _key.clear();
        _base64Key.clear();
        return false;
    }

    KERNEL_NS::LibBase64::Encode(_cypherKey.data(), static_cast<UInt64>(_cypherKey.size()), _base64Key);

    return true;
}

KERNEL_END

