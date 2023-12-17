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
 * Date: 2022-09-30 13:04:09
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/NetEngine/Protocol/ICoder.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(ICoder);

POOL_CREATE_OBJ_DEFAULT_IMPL(ICoderFactory);

// bool ICoder::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
// {
//     auto attachStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
//     attachStream->Attach(stream);
//     auto ret = Decode(*attachStream);

//     KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(attachStream);
//     return ret;
// }

// bool ICoder::Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
// {
//     auto attachStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
//     attachStream->Attach(stream);
//     auto ret = Decode(*attachStream);
//     KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(attachStream);
//     return ret;
// }

KERNEL_END
