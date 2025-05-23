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
* Author: Eric Yonng
* Description: Generated By protogentool, Dont Modify This File!!!
*/


#ifndef __PROTOCOLS_ORM_OUT_BORROWORDERINFOORMDATA_H__
#define __PROTOCOLS_ORM_OUT_BORROWORDERINFOORMDATA_H__

#pragma once
#include <kernel/kernel.h>
#include <service_common/protocol/ORM/IOrmData.h>

#include <vector>
#include <string>


namespace CRYSTAL_NET {
namespace service {

class BorrowOrderInfo;
class BorrowBookInfo;
class CancelOrderReason;

}
}

namespace google {
    namespace protobuf {
template <typename Element>
class RepeatedPtrField;


    }
}

SERVICE_COMMON_BEGIN

class BorrowBookInfoOrmData;
class CancelOrderReasonOrmData;

SERVICE_COMMON_END

SERVICE_COMMON_BEGIN

class BorrowOrderInfoOrmData : public SERVICE_COMMON_NS::IOrmData
{
    POOL_CREATE_OBJ_DEFAULT_P1(IOrmData, BorrowOrderInfoOrmData)

public:
    BorrowOrderInfoOrmData();
    BorrowOrderInfoOrmData(::CRYSTAL_NET::service::BorrowOrderInfo *pb);
    BorrowOrderInfoOrmData(const BorrowOrderInfoOrmData &other);
    BorrowOrderInfoOrmData(BorrowOrderInfoOrmData &&other);
    BorrowOrderInfoOrmData(const ::CRYSTAL_NET::service::BorrowOrderInfo &pb);
    ~BorrowOrderInfoOrmData();

    virtual void Release() override;

    BorrowOrderInfoOrmData &operator =(const ::CRYSTAL_NET::service::BorrowOrderInfo &pb);

    BorrowOrderInfoOrmData &operator =(const BorrowOrderInfoOrmData &other);

    BorrowOrderInfoOrmData &operator =(BorrowOrderInfoOrmData &&other);

    virtual KERNEL_NS::LibString ToJsonString() const override;

    virtual bool ToJsonString(std::string *data) const override;

    virtual bool FromJsonString(const Byte8 *data, size_t len) override;

    virtual Int64 GetOrmId() const override{ return 11; }

    void Clear();
    const ::CRYSTAL_NET::service::BorrowOrderInfo *GetPbRawData() const;

    void clear_orderid();

    uint64_t orderid() const;

    void set_orderid(uint64_t value);

    Int32 borrowbooklist_size() const;

    KERNEL_NS::SmartPtr<BorrowBookInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &mutable_borrowbooklist(Int32 idx);

    void DeleteArray_borrowbooklist(Int32 idx, Int32 count = 1);

    const std::vector<KERNEL_NS::SmartPtr<BorrowBookInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>> &borrowbooklist_OrmDataArray() const;

    const ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::BorrowBookInfo> &borrowbooklist() const;

    const KERNEL_NS::SmartPtr<BorrowBookInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &borrowbooklist_OrmDataArray(Int32 idx) const;

    const ::CRYSTAL_NET::service::BorrowBookInfo &borrowbooklist(Int32 idx) const;

    KERNEL_NS::SmartPtr<BorrowBookInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &add_borrowbooklist();

    void clear_borrowbooklist();

    void clear_createordertime();

    uint64_t createordertime() const;

    void set_createordertime(uint64_t value);

    void clear_orderstate();

    int32_t orderstate() const;

    void set_orderstate(int32_t value);

    KERNEL_NS::SmartPtr<CancelOrderReasonOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &mutable_cancelreason();

    const ::CRYSTAL_NET::service::CancelOrderReason &cancelreason() const;

    const KERNEL_NS::SmartPtr<CancelOrderReasonOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &cancelreason_OrmData() const;

    bool has_cancelreason() const;

    void clear_cancelreason();

    void clear_getovertime();

    int64_t getovertime() const;

    void set_getovertime(int64_t value);

    void clear_remark();

    const std::string &remark() const;

    void set_remark(const std::string &value);

    std::string *mutable_remark();

    void clear_userid();

    uint64_t userid() const;

    void set_userid(uint64_t value);


protected:

    virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override;
    virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override;

    virtual bool _OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override;
    virtual bool _OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override;

    virtual void _AttachPb(void *pb) override;

private:

    ::CRYSTAL_NET::service::BorrowOrderInfo *_ormRawPbData;

    std::vector<KERNEL_NS::SmartPtr<BorrowBookInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>> _borrowbooklist;

    KERNEL_NS::SmartPtr<CancelOrderReasonOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> _cancelreason;

};

class BorrowOrderInfoOrmDataFactory : public IOrmDataFactory
{
    POOL_CREATE_OBJ_DEFAULT_P1(IOrmDataFactory, BorrowOrderInfoOrmDataFactory);
public:
    BorrowOrderInfoOrmDataFactory(){}
    ~BorrowOrderInfoOrmDataFactory(){}

    virtual void Release() override { BorrowOrderInfoOrmDataFactory::DeleteThreadLocal_BorrowOrderInfoOrmDataFactory(this);}

    virtual IOrmData *Create() const override;
    virtual Int64 GetOrmId() const override { return 11; }
};

SERVICE_COMMON_END
#endif // __PROTOCOLS_ORM_OUT_BORROWORDERINFOORMDATA_H__