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
 * Date: 2024-02-23 20:37:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include "TestCheckAdapter.h"

void TestCheckAdapter::Run()
{
    auto checkMtLibStream = KERNEL_NS::CheckTypeSerializable<KERNEL_NS::LibStream<KERNEL_NS::_Build::MT>, KERNEL_NS::_Build::MT>::value;
    auto checkTlLibStream = KERNEL_NS::CheckTypeSerializable<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>, KERNEL_NS::_Build::TL>::value;
    auto checkCurrentType = KERNEL_NS::CheckTypeSerializable<TestCheckAdapter, KERNEL_NS::_Build::MT>::value;
    auto checkCurrentType2 = KERNEL_NS::CheckTypeSerializable<TestCheckAdapter, KERNEL_NS::_Build::TL>::value;
    auto checkInt32 = KERNEL_NS::CheckTypeSerializable<Int32, KERNEL_NS::_Build::MT>::value;
    auto checkInt32TL = KERNEL_NS::CheckTypeSerializable<Int32, KERNEL_NS::_Build::TL>::value;
    auto checkInt32Ptr = KERNEL_NS::CheckTypeSerializable<Int32 *, KERNEL_NS::_Build::MT>::value;
    auto checkInt32PtrTL = KERNEL_NS::CheckTypeSerializable<Int32 *, KERNEL_NS::_Build::TL>::value;
    auto checkInt32ArrTL = KERNEL_NS::CheckTypeSerializable<Int32 [], KERNEL_NS::_Build::TL>::value;
    auto checkInt32ArrMT = KERNEL_NS::CheckTypeSerializable<Int32 [], KERNEL_NS::_Build::MT>::value;

    auto checkDeserializeMtLibStream = KERNEL_NS::CheckTypeDeserializable<KERNEL_NS::LibStream<KERNEL_NS::_Build::MT>, KERNEL_NS::_Build::MT>::value;
    auto checkDeserializeTlLibStream = KERNEL_NS::CheckTypeDeserializable<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>, KERNEL_NS::_Build::TL>::value;
    auto checkDeserializeCurrentType = KERNEL_NS::CheckTypeDeserializable<TestCheckAdapter, KERNEL_NS::_Build::MT>::value;
    auto checkDeserializeCurrentType2 = KERNEL_NS::CheckTypeDeserializable<TestCheckAdapter, KERNEL_NS::_Build::TL>::value;
    auto checkDeserializeInt32 = KERNEL_NS::CheckTypeDeserializable<Int32, KERNEL_NS::_Build::MT>::value;
    auto checkDeserializeInt32TL = KERNEL_NS::CheckTypeDeserializable<Int32, KERNEL_NS::_Build::TL>::value;
    auto checkDeserializeInt32Ptr = KERNEL_NS::CheckTypeDeserializable<Int32 *, KERNEL_NS::_Build::MT>::value;
    auto checkDeserializeInt32PtrTL = KERNEL_NS::CheckTypeDeserializable<Int32 *, KERNEL_NS::_Build::TL>::value;
    auto checkDeserializeInt32ArrTL = KERNEL_NS::CheckTypeDeserializable<Int32 [], KERNEL_NS::_Build::TL>::value;
    auto checkDeserializeInt32ArrMT = KERNEL_NS::CheckTypeDeserializable<Int32 [], KERNEL_NS::_Build::MT>::value;

    auto checkSerializableMtLibStream = KERNEL_NS::CheckSerializable<KERNEL_NS::LibStream<KERNEL_NS::_Build::MT>>::value;
    auto checkSerializableTLLibStream = KERNEL_NS::CheckSerializable<KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>>::value;
    auto checkSerializableCurrentType = KERNEL_NS::CheckSerializable<TestCheckAdapter>::value;
    auto checkSerializableInt32 = KERNEL_NS::CheckSerializable<Int32>::value;
    auto checkSerializableInt32Ptr = KERNEL_NS::CheckSerializable<Int32 *>::value;
    auto checkSerializableInt32Arr = KERNEL_NS::CheckSerializable<Int32 []>::value;

    auto isMultiCheckSerializable = KERNEL_NS::CheckAdapterRule::GetAndResult(KERNEL_NS::CheckSerializable<Int32>::value, KERNEL_NS::CheckSerializable<Int32 *>::value);
    isMultiCheckSerializable = KERNEL_NS::CheckAdapterRule::GetAndResult(KERNEL_NS::CheckSerializable<Int32>::value, KERNEL_NS::CheckSerializable<Int32 []>::value);
    isMultiCheckSerializable = KERNEL_NS::CheckAdapterRule::GetOrResult(KERNEL_NS::CheckSerializable<Int32>::value, KERNEL_NS::CheckSerializable<Int32>::value);
    isMultiCheckSerializable = KERNEL_NS::CheckAdapterRule::GetOrResult(KERNEL_NS::CheckSerializable<TestCheckAdapter>::value, KERNEL_NS::CheckSerializable<Int32[]>::value);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestCheckAdapter, "checkMtLibStream:%d, checkTlLibStream:%d")
    , checkMtLibStream, checkTlLibStream);
}