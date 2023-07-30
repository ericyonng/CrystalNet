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
 * Date: 2023-07-16 14:37:39
 * Author: Eric Yonng
 * Description: 
*/

#include <service/TestService/ServiceCompHeader.h>
#include <OptionComp/storage/mysql/impl/SqlBuilder.h>
#include <OptionComp/storage/mysql/impl/Field.h>
#include <OptionComp/storage/mysql/impl/Record.h>
#include <OptionComp/storage/mysql/impl/MysqlMsg.h>

SERVICE_BEGIN

class IMysqlMgr : public IGlobalSys
{
    POOL_CREATE_OBJ_DEFAULT_P1(IGlobalSys, IMysqlMgr);

public:
    // 外部依赖注册
    virtual void RegisterDependence(ILogicSys *obj) = 0;
    virtual void UnRegisterDependence(const ILogicSys *obj) = 0;

    // 数据库请求 builders, fields, var无论失败成功都在方法内部帮忙释放
    virtual Int32 NewRequest(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields, bool isDestroyHandler, KERNEL_NS::IDelegate<void, KERNEL_NS::MysqlResponse *> *cb, KERNEL_NS::Variant **var = NULL) = 0;
    template<typename ObjType>
    Int32 NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields, ObjType *obj, void(ObjType::*handler)(KERNEL_NS::MysqlResponse *), KERNEL_NS::Variant **var = NULL);
    template<typename ObjType>
    Int32 NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &&fields, ObjType *obj, void(ObjType::*handler)(KERNEL_NS::MysqlResponse *), KERNEL_NS::Variant **var = NULL);
   
    template<typename CallbackType>
    Int32 NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields, CallbackType &&cb, KERNEL_NS::Variant **var = NULL);
    template<typename CallbackType>
    Int32 NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &&fields, CallbackType &&cb, KERNEL_NS::Variant **var = NULL);

    Int32 NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields);
    Int32 NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &&fields);

    // isRightRow:立即持久化
   virtual void MaskLogicNumberKeyAddDirty(const ILogicSys *logic, UInt64 key, bool isRightRow = false) = 0;
   virtual void MaskLogicNumberKeyModifyDirty(const ILogicSys *logic, UInt64 key, bool isRightRow = false) = 0;
   virtual void MaskLogicNumberKeyDeleteDirty(const ILogicSys *logic, UInt64 key, bool isRightRow = false) = 0;
   virtual void MaskLogicStringKeyAddDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow = false) = 0;
   virtual void MaskLogicStringKeyModifyDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow = false) = 0;
   virtual void MaskLogicStringKeyDeleteDirty(const ILogicSys *logic, const KERNEL_NS::LibString &key, bool isRightRow = false) = 0;

   // 获取系统的数据库操作id operator id是用来做负载均衡的, 每个用户一个, 系统操作的一个等
   virtual Int32 GetSystemDBOperatorId() const = 0;
   // 每个用户或者系统模块操作数据库都需要持有操作id
   virtual Int32 NewDbOperatorId() = 0;

   // 获取当前数据库名
   virtual const KERNEL_NS::LibString &GetCurrentServiceDbOption() const = 0;
   virtual const KERNEL_NS::LibString &GetCurrentServiceDbName() const = 0;
};

template<typename ObjType>
ALWAYS_INLINE Int32 IMysqlMgr::NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields, ObjType *obj, void(ObjType::*handler)(KERNEL_NS::MysqlResponse *), KERNEL_NS::Variant **var)
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    auto err = NewRequest(stub, dbName, dbOperatorId, builders, fields, true, delg, var);
    if(err != Status::Success)
    {
        delg->Release();
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequest fail err:%d"), err);
        return err;
    }

    return Status::Success;
}

template<typename ObjType>
ALWAYS_INLINE Int32 IMysqlMgr::NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &&fields, ObjType *obj, void(ObjType::*handler)(KERNEL_NS::MysqlResponse *), KERNEL_NS::Variant **var)
{
    auto delg = KERNEL_NS::DelegateFactory::Create(obj, handler);
    auto err = NewRequest(stub, dbName, dbOperatorId, builders, fields, true, delg, var);
    if(err != Status::Success)
    {
        delg->Release();
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequest fail err:%d"), err);
        return err;
    }

    return Status::Success;
}

template<typename CallbackType>
ALWAYS_INLINE Int32 IMysqlMgr::NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields, CallbackType &&cb, KERNEL_NS::Variant **var)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void , KERNEL_NS::MysqlResponse *);
    auto err = NewRequest(stub, dbName, dbOperatorId, builders, fields, true, delg, var);
    if(err != Status::Success)
    {
        delg->Release();
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequest fail err:%d"), err);
        return err;
    }

    return Status::Success;
}

template<typename CallbackType>
ALWAYS_INLINE Int32 IMysqlMgr::NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &&fields, CallbackType &&cb, KERNEL_NS::Variant **var)
{
    auto delg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void , KERNEL_NS::MysqlResponse *);
    auto err = NewRequest(stub, dbName, dbOperatorId, builders, fields, true, delg, var);
    if(err != Status::Success)
    {
        delg->Release();
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequest fail err:%d"), err);
        return err;
    }

    return Status::Success;
}

ALWAYS_INLINE Int32 IMysqlMgr::NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &fields)
{
    auto err = NewRequest(stub, dbName, dbOperatorId, builders, fields, true, NULL, NULL);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequest fail err:%d"), err);
        return err;
    }

    return Status::Success;
}

ALWAYS_INLINE Int32 IMysqlMgr::NewRequestBy(UInt64 &stub, const KERNEL_NS::LibString &dbName, Int32 dbOperatorId, std::vector<KERNEL_NS::SqlBuilder *> &builders, std::vector<KERNEL_NS::Field *> &&fields)
{
    auto err = NewRequest(stub, dbName, dbOperatorId, builders, fields, true, NULL, NULL);
    if(err != Status::Success)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("NewRequest fail err:%d"), err);
        return err;
    }

    return Status::Success;
}

SERVICE_END
