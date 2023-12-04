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
 * Date: 2023-09-29 17:35:20
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_CODE_ANALYZE_IMPL_CODE_UNIT_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_CODE_ANALYZE_IMPL_CODE_UNIT_H__

#pragma once

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/SmartPtr.h>

KERNEL_BEGIN

class CodeUnitFlags
{
public:
    enum POS_ENUMS : UInt64
    {
        // 数据类型定义
        DATA_TYPE_DEFINE_FLAG_POS = 0,

        // 枚举数据类型定义
        ENUM_DATA_TYPE_DEFINE_FLAG_POS,

        // 命名空间
        NAMESPACE_FLAG_POS,

        // 字段
        FIELD_FLAG_POS,

        // 数值字段
        NUMBER_FIELD_FLAG_POS,

        // 枚举字段
        ENUM_FIELD_FLAG_POS,

        // 布尔
        BOOL_FIELD_FLAG_POS,

        // 字符串字段
        STRING_FIELD_FLAG_POS,

        // 自定义类型字段
        CUSTOM_FIELD_FLAG_POS,

        // 数组类型字段
        ARRAY_FIELD_FLAG_POS,

        // 函数
        FUNCTION_FLAG_POS,

        // 静态特性
        STATIC_FLAG_POS,

        // 可选
        OPTION_FILED_FLAG_POS,

        // 最大
        MAX_POS,
    };

    enum FLAG_ENUMS : UInt64
    {
        // 
        DATA_TYPE_DEFINE_FLAG = (1LLU << DATA_TYPE_DEFINE_FLAG_POS),
        ENUM_DATA_TYPE_DEFINE_FLAG = DATA_TYPE_DEFINE_FLAG | (1LLU << ENUM_DATA_TYPE_DEFINE_FLAG_POS),

        NAMESPACE_FLAG = (1LLU << NAMESPACE_FLAG_POS),

        FIELD_FLAG = (1LLU << FIELD_FLAG_POS),
        NUMBER_FIELD_FLAG = FIELD_FLAG | (1LLU << NUMBER_FIELD_FLAG_POS),
        ENUM_FIELD_FLAG = FIELD_FLAG | (1LLU << ENUM_FIELD_FLAG_POS),
        BOOL_FIELD_FLAG = FIELD_FLAG | (1LLU << BOOL_FIELD_FLAG_POS),
        STRING_FIELD_FLAG = FIELD_FLAG | (1LLU << STRING_FIELD_FLAG_POS),
        CUSTOM_FIELD_FLAG = FIELD_FLAG | (1LLU << CUSTOM_FIELD_FLAG_POS),
        ARRAY_FIELD_FLAG = FIELD_FLAG | (1LLU << ARRAY_FIELD_FLAG_POS),


        FUNCTION_FLAG = FIELD_FLAG | (1LLU << FUNCTION_FLAG_POS),

        STATIC_FLAG = (1LLU << STATIC_FLAG_POS),
        OPTION_FILED_FLAG = FIELD_FLAG | (1LLU << OPTION_FILED_FLAG_POS),
    };

    static bool HasFlags(UInt64 flags, UInt64 someFlags)
    {
        return (flags & someFlags) == someFlags;
    }

    static void SetFlags(UInt64 &flags, UInt64 someFlags)
    {
        flags |= someFlags; 
    }

    static void ClearFlags(UInt64 &flags, UInt64 someFlags)
    {
        flags &= (~someFlags);
    }
};

class CodeUnit
{
    POOL_CREATE_OBJ_DEFAULT(CodeUnit);

public:
    CodeUnit();
    virtual ~CodeUnit();

public:
    virtual void Release() final;
    static SmartPtr<CodeUnit, AutoDelMethods::Release> CreateCodeUnit();

    LibString GetFullName() const;
    const LibString &GetUnitName() const;
    Int32 GetLine() const;
    const LibString &GetFileName() const;
    LibString GetBelongToArea() const;

    // 单元名
    LibString _unitName;

    // 单元所在行
    Int32 _line = 0;

    // 所在文件名
    LibString _fileName;

    // 所属的域
    std::vector<KERNEL_NS::LibString> _belongToArea;

    // 当前单元的注释(当前单元同行的, 或者在之前行的注释)
    std::vector<LibString> _comments;
    // 同行, 行末注释
    LibString _commentSameLine;

    // CodeUnitFlags
    UInt64 _flags = 0;

    // 子单元
    std::vector<SmartPtr<CodeUnit, AutoDelMethods::Release>> _subCodeUnits;

    // 参数, 如果是field, 那么params放的就是类型名(自定义类型的fullname),如果是枚举类型, 那么放的是枚举值
    std::vector<KERNEL_NS::LibString> _params;

    // 状态
    bool _isStarted = false;
    bool _isEnd = false;
};

ALWAYS_INLINE LibString CodeUnit::GetFullName() const
{
    KERNEL_NS::LibString &&fullName = GetBelongToArea();
    fullName.AppendData(_unitName);

    return fullName;    
}

ALWAYS_INLINE const LibString &CodeUnit::GetUnitName() const
{
    return _unitName;
}

ALWAYS_INLINE Int32 CodeUnit::GetLine() const
{
    return _line;
}

ALWAYS_INLINE const LibString &CodeUnit::GetFileName() const
{
    return _fileName;
}

ALWAYS_INLINE LibString CodeUnit::GetBelongToArea() const
{
    KERNEL_NS::LibString area;
    for(auto &item : _belongToArea)
        area.AppendData(item);

    return area;
}

class CodeUnitStack
{
    POOL_CREATE_OBJ_DEFAULT(CodeUnitStack);
public:
    CodeUnitStack();
    virtual ~CodeUnitStack();

public:
    virtual void Release() final;
    static SmartPtr<CodeUnitStack, AutoDelMethods::Release> Create();

    SmartPtr<CodeUnit, AutoDelMethods::Release> &GetCurrentCodeUnit();
    const SmartPtr<CodeUnit, AutoDelMethods::Release> &GetCurrentCodeUnit() const;

    std::vector<SmartPtr<CodeUnit, AutoDelMethods::Release>> _codeUnits;

    static SmartPtr<CodeUnit, AutoDelMethods::Release> _nullUnit;
};

ALWAYS_INLINE SmartPtr<CodeUnit, AutoDelMethods::Release> &CodeUnitStack::GetCurrentCodeUnit()
{
    return _codeUnits.empty() ? _nullUnit : _codeUnits[_codeUnits.size() - 1];
}

ALWAYS_INLINE const SmartPtr<CodeUnit, AutoDelMethods::Release> &CodeUnitStack::GetCurrentCodeUnit() const
{
    return _codeUnits.empty() ? _nullUnit : _codeUnits[_codeUnits.size() - 1];
}

KERNEL_END

#endif

