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
 * Date: 2023-09-25 02:01:48
 * Author: Eric Yonng
 * Description: 
*/

#include <service/ProtoGenService/ServiceCompHeader.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/PbCacheInfoFormat.h>

SERVICE_BEGIN

class ProtoBuffAttributeFlags
{
private:
    enum ENUMS_POS : UInt64
    {
        ENUM_FLAG_POS = 0,
        NUMBER_FLAG_POS,
        STRING_FLAG_POS,
        BOOLEAN_FLAG_POS,

        FLOAT_NUMBER_FLAG_POS,
        DOUBLE_NUMBER_FLAG_POS,

        INTEGER_NUMBER_FLAG_POS,
        FOUR_BYTE_NUMBER_FLAG_POS,
        EIGHT_BYTE_NUMBER_FLAG_POS,

        NORMAL_STRING_FLAG_POS,

        // 是base64 字符串
        BYTES_STRING_FLAG_POS,

        ARRAY_FLAG_POS,
        DICT_FLAG_POS,
    };
public:
    enum ENUMS : UInt64
    {
        ENUM_FLAG = (1LLU << ENUM_FLAG_POS),

        // 数值
        NUMBER_FLAG = (1LLU << NUMBER_FLAG_POS),
        // 字符串
        STRING_FLAG = (1LLU << STRING_FLAG_POS),
        // 布尔
        BOOLEAN_FLAG = (1LLU << BOOLEAN_FLAG_POS),

        // 浮点数
        FLOAT_NUMBER_FLAG = NUMBER_FLAG | (1LLU << FLOAT_NUMBER_FLAG_POS),
        DOUBLE_NUMBER_FLAG = NUMBER_FLAG | (1LLU << DOUBLE_NUMBER_FLAG_POS),
        
        // 整数
        INTEGER_NUMBER_FLAG = NUMBER_FLAG | (1LLU << INTEGER_NUMBER_FLAG_POS),
        FOUR_BYTE_NUMBER_FLAG = NUMBER_FLAG | (1LLU << FOUR_BYTE_NUMBER_FLAG_POS),
        EIGHT_BYTE_NUMBER_FLAG = NUMBER_FLAG | (1LLU << EIGHT_BYTE_NUMBER_FLAG_POS),
        
        // 字符串
        NORMAL_STRING_FLAG = STRING_FLAG | (1LLU << NORMAL_STRING_FLAG_POS),
        BYTES_STRING_FLAG = STRING_FLAG | (1LLU << BYTES_STRING_FLAG_POS),

        // 数组
        ARRAY_FLAG = (1LLU << ARRAY_FLAG_POS),
        // 字典
        DICT_FLAG = (1LLU << DICT_FLAG_POS),
    };
};

// 需要注意枚举类型 enum 跳过parser生成
// XXX = value
struct TsProtobufFieldRawInfo
{
    // protobuf 每个字段的id
    Int32 tagId = 0;
    // ProtoBuffAttributeFlags
    UInt64 flags = 0;
    // 字段名
    KERNEL_NS::LibString fieldName;
    // 数据类型
    KERNEL_NS::LibString fieldDataType;
};

struct TsProtobufMessageRawInfo
{
    std::map<Int32, TsProtobufFieldRawInfo> _tagIdRefRawInfo;
};

SERVICE_END