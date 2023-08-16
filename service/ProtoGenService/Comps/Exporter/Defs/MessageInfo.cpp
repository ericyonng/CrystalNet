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
 * Date: 2022-10-19 23:39:06
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/MessageInfo.h>
#include <service/ProtoGenService/Comps/Exporter/Defs/Defs.h>
SERVICE_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(MessageInfo);

MessageInfo::MessageInfo()
:_opcode(0)
,_noLog(false)
{

}

void MessageInfo::Release()
{
    MessageInfo::Delete_MessageInfo(this);
}

void MessageInfo::FieldsFromAnnotations(Int32 &maxOpcode)
{
    for(auto &kv:_annotationParamNameRefValue)
    {
        if(kv.first == ProtobufMessageParam::Opcode)
        {
            if(kv.second.length() != 0)
            {
                _opcode = KERNEL_NS::StringUtil::StringToInt32(kv.second.c_str());
            }

            if(_opcode == 0)
                _opcode = ++maxOpcode;
            
            maxOpcode = std::max<Int32>(_opcode, maxOpcode);
        }

        if(kv.first == ProtobufMessageParam::NoLog)
        {
            if(kv.second.length() != 0)
            {
                _noLog = (kv.second.strip().tolower()) == "true";
            } 
        }
    }
}

PbCaheInfo MessageInfo::ToPbCache(const KERNEL_NS::LibString &protoName, const KERNEL_NS::LibString &protoPath) const
{
    PbCaheInfo info;
    info._messageName = _messageName;
    info._protoName = protoName;
    info._protoPath = protoPath;
    info._opcode = _opcode;
    info._noLog = _noLog;

    return info;
}

SERVICE_END
