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
 * Date: 2023-08-02 22:38:00
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <Common/StorageHelper.h>
#include <service/common/common.h>

SERVICE_BEGIN

bool StorageHelper::AddMysqlStorageInfoWithPb(IStorageInfo *ownerStorageInfo, const ::google::protobuf::Descriptor *descriptor, const KERNEL_NS::LibString *primaryKey, const std::set<KERNEL_NS::LibString> &uniqueKeys, const std::set<KERNEL_NS::LibString> &indexs)
{
    for(Int32 idx = 0; idx < descriptor->field_count(); ++idx)
    {
        auto field = descriptor->field(idx);

        auto p = IStorageInfo::NewThreadLocal_IStorageInfo(field->name());
        KERNEL_NS::SmartPtr<IStorageInfo, KERNEL_NS::AutoDelMethods::Release> newStorageInfo = p;
        newStorageInfo->SetRelease([p](){
            p->WillClose();
            p->Close();
            
            IStorageInfo::DeleteThreadLocal_IStorageInfo(p);
        });
        newStorageInfo->AddFlags(StorageFlagType::MYSQL_FLAG);

        // 主键
        if(primaryKey && (field->name() == primaryKey->GetRaw()))
            newStorageInfo->AddFlags(StorageFlagType::PRIMARY_FIELD_FLAG);
        else if(uniqueKeys.find(newStorageInfo->GetFieldName()) != uniqueKeys.end())
        {
            newStorageInfo->AddFlags(StorageFlagType::AS_UNIQUE_KEY_FIELD_FLAG);
        }

        if(indexs.find(newStorageInfo->GetFieldName()) != indexs.end())
        {
            newStorageInfo->AddFlags(StorageFlagType::AS_INDEX_KEY_FIELD_FLAG);
        }

        // map/repeated使用text
        if(field->is_map() || field->is_repeated())
        {
            newStorageInfo->AddFlags(StorageFlagType::TEXT_STRING_FIELD_FLAG);
            if(!ownerStorageInfo->AddStorageInfo(newStorageInfo.AsSelf()))
            {
                g_Log->Error(LOGFMT_NON_OBJ_TAG(StorageHelper, "add storage info fail owner storage info system name:%s, new storage info system name:%s")
                        , ownerStorageInfo->GetSystemName().c_str(), newStorageInfo->GetSystemName().c_str());

                return false;
            }

            newStorageInfo.pop();

            continue;
        }

        switch (field->cpp_type())
        {
        case ::google::protobuf::FieldDescriptor::CPPTYPE_BOOL: 
        {
            newStorageInfo->AddFlags(StorageFlagType::INT32_NUMBER_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: 
        {
            newStorageInfo->AddFlags(StorageFlagType::DOUBLE_NUMBER_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_ENUM: 
        {
            newStorageInfo->AddFlags(StorageFlagType::INT32_NUMBER_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_FLOAT: 
        {
            newStorageInfo->AddFlags(StorageFlagType::FLOAT_NUMBER_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_INT32: 
        {
            newStorageInfo->AddFlags(StorageFlagType::INT32_NUMBER_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_INT64: 
        {
            newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
        {// message使用text
            newStorageInfo->AddFlags(StorageFlagType::TEXT_STRING_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_STRING: 
        {
            newStorageInfo->AddFlags(StorageFlagType::NORMAL_STRING_FIELD_FLAG);
            newStorageInfo->SetCapacitySize(StorageCapacityType::Cap64);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
        {
            newStorageInfo->AddFlags(StorageFlagType::INT32_NUMBER_FIELD_FLAG | StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG);
        }break;
        case ::google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
        {
            newStorageInfo->AddFlags(StorageFlagType::INT64_NUMBER_FIELD_FLAG | StorageFlagType::UNSIGNED_NUMBER_FIELD_FLAG);
        }break;
        default:
        {
            g_Log->Warn(LOGFMT_NON_OBJ_TAG(StorageHelper, "unknown pb cpp type:%d, field name:%s, message name:%s"), field->cpp_type(), field->name().c_str(), descriptor->name().c_str());
            continue;
        }break;
        };

        if(!ownerStorageInfo->AddStorageInfo(newStorageInfo.AsSelf()))
        {
            g_Log->Error(LOGFMT_NON_OBJ_TAG(StorageHelper, "add storage info fail owner storage info system name:%s, new storage info system name:%s")
                    , ownerStorageInfo->GetSystemName().c_str(), newStorageInfo->GetSystemName().c_str());

            return false;
        }

        newStorageInfo.pop();
    }

    return true;
}

SERVICE_END
