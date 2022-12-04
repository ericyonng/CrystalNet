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
 * Date: 2022-07-25 18:10:20
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENDIAN_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_ENDIAN_H__

#pragma once

#include <kernel/kernel_inc.h>

KERNEL_BEGIN

class KERNEL_EXPORT LibEndianType
{
public:
    enum ENUMS : Int32
    {
        LittleEndian = 0,       // 小端存储
        BigEndian = 1,          // 大端存储

        NetOrder = BigEndian,   // 网络存储是大端存储

        UnknownEndian = 2,      // 未知存储
    };

    static const char *ToString(Int32 endianType)
    {
        switch (endianType)
        {
        case LibEndianType::LittleEndian: return "LittleEndian";
        case LibEndianType::BigEndian: return "BigEndian";
        default:
            break;
        }

        return "UnknownEndian";
    }
};

class KERNEL_EXPORT LibEndian
{
public:
    static void Init();

    // 获取本地字节序
    static Int32 GetLocalMachineEndianType();
    // 获取字节序
    static Int32 GetEndianType(Int32 detectData, Byte8 sucDetectFlag, Byte8 flagOffset);
    // 判断小端
    static bool IsLittleEndian(Int32 detectData, Byte8 sucDetectFlag, Byte8 flagOffset);
    // 判断大端
    static bool IsBigEndian(Int32 detectData, Byte8 sucDetectFlag, Byte8 flagOffset);

private:
    static Int32 _GetLocalMachineEndianType();

    static Int32 _machineEndianType;
};

ALWAYS_INLINE Int32 LibEndian::GetLocalMachineEndianType()
{
    return _machineEndianType;
}

ALWAYS_INLINE Int32 LibEndian::GetEndianType(Int32 detectData, Byte8 sucDetectFlag, Byte8 flagOffset)
{
    // 是否小端
    if(IsLittleEndian(detectData, sucDetectFlag, flagOffset))
        return LibEndianType::LittleEndian;

    // 是否大端
    if(IsBigEndian(detectData, sucDetectFlag, flagOffset))
        return LibEndianType::BigEndian;

    return LibEndianType::UnknownEndian;
}

ALWAYS_INLINE bool LibEndian::IsLittleEndian(Int32 detectData, Byte8 sucDetectFlag, Byte8 flagOffset)
{
    // 是否小端
    if(static_cast<Byte8>(detectData >> flagOffset) == sucDetectFlag)
        return true;

    return false;
}

ALWAYS_INLINE bool LibEndian::IsBigEndian(Int32 detectData, Byte8 sucDetectFlag, Byte8 flagOffset)
{
    // 是否小端
    if(static_cast<Byte8>(detectData >> flagOffset) == sucDetectFlag)
        return true;

    return false;
}

ALWAYS_INLINE Int32 LibEndian::_GetLocalMachineEndianType()
{
    // 若是小端字节序，从低位开始存储，则顺序为l,?,?,b, 则endian_test.l的第一个字节为'l',若是大端则是'b'
    static union {
        char c[4];
        int l;
    } endian_test = {{'l', '?', '?', 'b'}};

    return GetEndianType(endian_test.l, '1', 0);
}

KERNEL_END

#endif
