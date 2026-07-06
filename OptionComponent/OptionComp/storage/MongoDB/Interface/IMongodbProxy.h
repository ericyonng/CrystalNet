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
 * Date: 2026-06-16 14:49:39
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_IMONGODB_PROXY_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_STORAGE_MONGODB_IMPL_IMONGODB_PROXY_H__

#pragma once

#include <kernel/comp/Coroutines/CoTask.h>
#include <OptionComp/storage/MongoDB/Impl/IMongodbStorageInfo.h>
#include <OptionComp/storage/MongoDB/Impl/MongoSerializeInfo.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Delegate/IDelegate.h>

KERNEL_BEGIN

class IMongoDbMgr;
struct SourceWrap;
class EventManager;

// 每个系统提供的持久化信息

// 每个线程一个, 不支持跨线程
// mongodb代理
class IMongodbProxy : public CompHostObject
{
    POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IMongodbProxy);

public:
    IMongodbProxy(UInt64 objTypeId) : CompHostObject(objTypeId){}

    virtual void SetMongodbMgr(KERNEL_NS::IMongoDbMgr *mongodbMgr) = 0;
    virtual KERNEL_NS::IMongoDbMgr *GetMongodbMgr() = 0;
    virtual const KERNEL_NS::IMongoDbMgr *GetMongodbMgr() const = 0;
    
    // 设置检查依赖是否退出
    template<typename LambadaType>
    requires requires(LambadaType l, const KERNEL_NS::CompHostObject *host)
    {
        {l(host)}->std::same_as<bool>;
    }
    void SetCheckDependenceQuit(LambadaType &&cb);
    // 设置检查依赖是否退出
    virtual void SetCheckDependenceQuit(KERNEL_NS::IDelegate<bool, const KERNEL_NS::CompHostObject *> *cb) = 0;

    // 设置自己完成回调(告知自己完成了, 可以退出,因为proxy要等待所有依赖都退出后才可退出)
    template<typename LambadaType>
    requires requires(LambadaType l, const KERNEL_NS::CompHostObject *host)
    {
        {l(host)}->std::same_as<void>;
    }
    void SetMongoProxyMaskQuit(LambadaType &&cb);
    // 设置自己完成回调(告知自己完成了, 可以退出,因为proxy要等待所有依赖都退出后才可退出)
    virtual void SetMongoProxyMaskQuit(KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> *cb) = 0;

    // 设置关注的组件回调, 以便Host在退出的时候检查是否Proxy要退出, 只有等待Proxy也退出, Host才可以退出, 避免数据没完全落地Host就退出了
    template<typename LambadaType>
    requires requires(LambadaType l, const KERNEL_NS::CompHostObject *host)
    {
        {l(host)}->std::same_as<void>;
    }
    void SetRegisterFocus(LambadaType &&cb);
    // 设置关注的组件回调, 以便Host在退出的时候检查是否Proxy要退出, 只有等待Proxy也退出, Host才可以退出, 避免数据没完全落地Host就退出了
    virtual void SetRegisterFocus(KERNEL_NS::IDelegate<void, const KERNEL_NS::CompHostObject *> *cb) = 0;
    // 需要监听关闭
    virtual void ListenClose(KERNEL_NS::EventManager *eventMgr, Int32 eventType) = 0;

    // TODO:提供load数据接口直接外部调用Load接口(协程)

    // 标脏 mongodb只支持int64
    virtual void MaskLogicNumberKeyAddDirty(const CompHostObject *logic, Int64 key) = 0;
    virtual void MaskLogicNumberKeyModifyDirty(const CompHostObject *logic, Int64 key) = 0;
    virtual void MaskLogicNumberKeyDeleteDirty(const CompHostObject *logic, Int64 key) = 0;
    
    virtual void MaskLogicStringKeyAddDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key) = 0;
    virtual void MaskLogicStringKeyModifyDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key) = 0;
    virtual void MaskLogicStringKeyDeleteDirty(const CompHostObject *logic, const KERNEL_NS::LibString &key) = 0;

    // 等待落库完成
    virtual KERNEL_NS::CoTask<> Purge() = 0;
    // 等待logic落库完成
    virtual KERNEL_NS::CoTask<> Purge(const CompHostObject *logic) = 0;

    // 查Logic数据 只需要外部传fieldNameRefData即可,用于接收结果, 内部不会释放, 外部自己手动释放结果
    virtual KERNEL_NS::CoTask<bool> Query(const CompHostObject *logic, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) = 0;
    // fieldNameRefDataResult:指定字段名, 则会查询指定字段名的数据, 不指定, 则查询全部数据
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, Int64 key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) = 0;
    virtual KERNEL_NS::CoTask<bool> Query(KERNEL_NS::LibString dbName, KERNEL_NS::LibString collectionName, KERNEL_NS::LibString keyName, KERNEL_NS::LibString key, std::map<KERNEL_NS::LibString, KERNEL_NS::MongoSerializeInfo> *fieldNameRefDataResult) = 0;

    // template<typename DataType, typename LambdaType>
    // requires requires(LambdaType lambda, KERNEL_NS::SmartMongoSerializeInfoWrapper wrapper, DataType *data)
    // {
    //     {lambda(wrapper.Ptr.AsSelf(), data)} -> std::same_as<bool>;
    // }
    // KERNEL_NS::CoTask<bool> Query(const CompHostObject *logic, Int64 key, DataType *data, LambdaType &&parseData);

};

template<typename LambadaType>
requires requires(LambadaType l, const KERNEL_NS::CompHostObject *host)
{
    {l(host)}->std::same_as<bool>;
}
ALWAYS_INLINE void IMongodbProxy::SetCheckDependenceQuit(LambadaType &&cb)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, bool, const KERNEL_NS::CompHostObject *);
    SetCheckDependenceQuit(deleg);
}


template<typename LambadaType>
requires requires(LambadaType l, const KERNEL_NS::CompHostObject *host)
{
    {l(host)}->std::same_as<void>;
}
ALWAYS_INLINE void IMongodbProxy::SetMongoProxyMaskQuit(LambadaType &&cb)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, const KERNEL_NS::CompHostObject *);
    SetMongoProxyMaskQuit(deleg);
}

template<typename LambadaType>
requires requires(LambadaType l, const KERNEL_NS::CompHostObject *host)
{
    {l(host)}->std::same_as<void>;
}
ALWAYS_INLINE void IMongodbProxy::SetRegisterFocus(LambadaType &&cb)
{
    auto deleg = KERNEL_CREATE_CLOSURE_DELEGATE(cb, void, const KERNEL_NS::CompHostObject *);
    SetRegisterFocus(deleg);
}

// template<typename DataType, typename LambdaType>
// requires requires(LambdaType lambda, KERNEL_NS::SmartMongoSerializeInfoWrapper wrapper, DataType *data)
// {
//     {lambda(wrapper.Ptr.AsSelf(), data)} -> std::same_as<bool>;
// }
// ALWAYS_INLINE KERNEL_NS::CoTask<bool> IMongodbProxy::Query(const CompHostObject *logic, Int64 key, DataType *data, LambdaType &&parseData)
// {
//     KERNEL_NS::SmartMongoSerializeInfoWrapper wrapper;
//     auto ret = co_await Query(logic, key, wrapper.Ptr.AsSelf());
//     if(!ret)
//     {
//         co_return false;
//     }
//
//     co_return parseData(wrapper.Ptr.AsSelf(), data);
// }


KERNEL_END

#endif
