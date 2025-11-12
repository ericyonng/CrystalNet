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
 * Date: 2022-05-05 00:57:32
 * Author: Eric Yonng
 * Description: 使用宏调用内存分配与释放,记录内存分配的位置信息,以便定位内存泄漏位置, 底层不建议使用，因为会强行串行化导致性能下降
 * 
 *              1.记录对象类型对应的new总数, delete总数, BuildType区分的new总数,delete总数，活跃的对象总数, buildType区分的活跃的总数
 *              2.记录new位置对应的总数, buildType区分的总数,活跃的对象总数，buildType区分的活跃的总数,
 *              3. CRYSTAL_MA_NEW/CRYSTAL_MA_DELETE 建议业务层使用且BuildType必须是_Build::TL 这个版本是无锁的
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_ASSIST_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_MEMMORY_MEMORY_ASSIST_H__

#pragma once

#include <memory>

#include <kernel/comp/memory/ObjPoolMacro.h>
#include <kernel/comp/memory/ObjPoolWrap.h>
#include <kernel/comp/LibString.h>
#include <kernel/common/LibObject.h>
#include <kernel/comp/Lock/Impl/LockWrap.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/Utils/RttiUtil.h>
#include <kernel/comp/MemoryMonitor/memorymonitor_inc.h>
#include <kernel/comp/Utils/AllocUtil.h>
#include <memory>

// // 使用常规对象池创建对象
#undef __CRYSTAL_MA_NEW
#define __CRYSTAL_MA_NEW(ObjType, BuildType, FileName, FileLine, ...)       \
KERNEL_NS::MemoryAssist<ObjType, BuildType>::GetInstance()->WrapNew<BuildType>(ObjType::NewByAdapter_##ObjType(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_NEW
#define CRYSTAL_MA_NEW(ObjType, BuildType, ...)                             \
__CRYSTAL_MA_NEW(ObjType, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_DELETE
#define CRYSTAL_MA_DELETE(ObjType, BuildType, ptr)   \
KERNEL_NS::MemoryAssist<ObjType, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), ObjType::DeleteByAdapter_##ObjType(BuildType::V, ptr)


// // 使用ObjPoolWrap<ObjType>创建对象
#undef __CRYSTAL_MA_NEW_BY_POOL_WRAP
#define __CRYSTAL_MA_NEW_BY_POOL_WRAP(ObjType, BuildType, FileName, FileLine, ...)       \
KERNEL_NS::MemoryAssist<ObjType, BuildType>::GetInstance()->WrapNew<BuildType>(KERNEL_NS::ObjPoolWrap<ObjType>::NewByAdapter(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef OBJ_POOL_NEW
#define OBJ_POOL_NEW(ObjType, BuildType, ...)                             \
__CRYSTAL_MA_NEW_BY_POOL_WRAP(ObjType, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef OBJ_POOL_DEL
#define OBJ_POOL_DEL(ObjType, BuildType, ptr)   \
KERNEL_NS::MemoryAssist<ObjType, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), KERNEL_NS::ObjPoolWrap<ObjType>::DeleteByAdapter(BuildType::V, ptr)


// // 常规模版对象池创建对象

// 一个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P1
#define __CRYSTAL_MA_TEMPLATE_NEW_P1(cls, P1, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1>, BuildType>::GetInstance()->WrapNew<BuildType>(cls<P1>::NewByAdapter_##cls(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P1
#define CRYSTAL_MA_TEMPLATE_NEW_P1(cls, P1, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P1(cls, P1, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P1
#define CRYSTAL_MA_TEMPLATE_DELETE_P1(cls, P1, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), cls<P1>::DeleteByAdapter_##cls(BuildType::V, ptr)

// 两个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P2
#define __CRYSTAL_MA_TEMPLATE_NEW_P2(cls, P1, P2, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2>, BuildType>::GetInstance()->WrapNew<BuildType>(cls<P1, P2>::NewByAdapter_##cls(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P2
#define CRYSTAL_MA_TEMPLATE_NEW_P2(cls, P1, P2, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P2(cls, P1, P2, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P2
#define CRYSTAL_MA_TEMPLATE_DELETE_P2(cls, P1, P2, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), cls<P1, P2>::DeleteByAdapter_##cls(BuildType::V, ptr)

// 三个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P3
#define __CRYSTAL_MA_TEMPLATE_NEW_P3(cls, P1, P2, P3, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3>, BuildType>::GetInstance()->WrapNew<BuildType>(cls<P1, P2, P3>::NewByAdapter_##cls(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P3
#define CRYSTAL_MA_TEMPLATE_NEW_P3(cls, P1, P2, P3, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P3(cls, P1, P2, P3, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P3
#define CRYSTAL_MA_TEMPLATE_DELETE_P3(cls, P1, P2, P3, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), cls<P1, P2, P3>::DeleteByAdapter_##cls(BuildType::V, ptr)

// 四个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P4
#define __CRYSTAL_MA_TEMPLATE_NEW_P4(cls, P1, P2, P3, P4, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4>, BuildType>::GetInstance()->WrapNew<BuildType>(cls<P1, P2, P3, P4>::NewByAdapter_##cls(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P4
#define CRYSTAL_MA_TEMPLATE_NEW_P4(cls, P1, P2, P3, P4, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P4(cls, P1, P2, P3, P4, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P4
#define CRYSTAL_MA_TEMPLATE_DELETE_P4(cls, P1, P2, P3, P4, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), cls<P1, P2, P3, P4>::DeleteByAdapter_##cls(BuildType::V, ptr)

// 五个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P5
#define __CRYSTAL_MA_TEMPLATE_NEW_P5(cls, P1, P2, P3, P4, P5, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5>, BuildType>::GetInstance()->WrapNew<BuildType>(cls<P1, P2, P3, P4, P5>::NewByAdapter_##cls(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P5
#define CRYSTAL_MA_TEMPLATE_NEW_P5(cls, P1, P2, P3, P4, P5, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P5(cls, P1, P2, P3, P4, P5, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P5
#define CRYSTAL_MA_TEMPLATE_DELETE_P5(cls, P1, P2, P3, P4, P5, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), cls<P1, P2, P3, P4, P5>::DeleteByAdapter_##cls(BuildType::V, ptr)

// 六个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P6
#define __CRYSTAL_MA_TEMPLATE_NEW_P6(cls, P1, P2, P3, P4, P5, P6, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5, P6>, BuildType>::GetInstance()->WrapNew<BuildType>(cls<P1, P2, P3, P4, P5, P6>::NewByAdapter_##cls(BuildType::V, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P6
#define CRYSTAL_MA_TEMPLATE_NEW_P6(cls, P1, P2, P3, P4, P5, P6,BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P6(cls, P1, P2, P3, P4, P5, P6, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P6
#define CRYSTAL_MA_TEMPLATE_DELETE_P6(cls, P1, P2, P3, P4, P5, P6, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5, P6>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), cls<P1, P2, P3, P4, P5, P6>::DeleteByAdapter_##cls(BuildType::V, ptr)

// // 使用ObjPoolWrap 模版版本对象池创建对象

// 一个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P1_BY_POOL_WRAP
#define __CRYSTAL_MA_TEMPLATE_NEW_P1_BY_POOL_WRAP(cls, P1, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1>, BuildType>::GetInstance()->WrapNew<BuildType>(OBJ_POOL_WRAP_TEMPLATE_NEW_P1(cls, P1, BuildType, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P1_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_NEW_P1_BY_POOL_WRAP(cls, P1, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P1_BY_POOL_WRAP(cls, P1, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P1_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_DELETE_P1_BY_POOL_WRAP(cls, P1, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), OBJ_POOL_WRAP_TEMPLATE_DELETE_P1(cls, P1, BuildType, ptr)

// 两个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P2_BY_POOL_WRAP
#define __CRYSTAL_MA_TEMPLATE_NEW_P2_BY_POOL_WRAP(cls, P1, P2, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2>, BuildType>::GetInstance()->WrapNew<BuildType>(OBJ_POOL_WRAP_TEMPLATE_NEW_P2(cls, P1, P2, BuildType, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P2_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_NEW_P2_BY_POOL_WRAP(cls, P1, P2, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P2_BY_POOL_WRAP(cls, P1, P2, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P2_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_DELETE_P2_BY_POOL_WRAP(cls, P1, P2, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), OBJ_POOL_WRAP_TEMPLATE_DELETE_P2(cls, P1, P2, BuildType, ptr)

// 三个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P3_BY_POOL_WRAP
#define __CRYSTAL_MA_TEMPLATE_NEW_P3_BY_POOL_WRAP(cls, P1, P2, P3, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3>, BuildType>::GetInstance()->WrapNew<BuildType>(OBJ_POOL_WRAP_TEMPLATE_NEW_P3(cls, P1, P2, P3, BuildType, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P3_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_NEW_P3_BY_POOL_WRAP(cls, P1, P2, P3, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P3_BY_POOL_WRAP(cls, P1, P2, P3, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P3_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_DELETE_P3_BY_POOL_WRAP(cls, P1, P2, P3, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), OBJ_POOL_WRAP_TEMPLATE_DELETE_P3(cls, P1, P2, P3, BuildType, ptr)

// 四个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P4_BY_POOL_WRAP
#define __CRYSTAL_MA_TEMPLATE_NEW_P4_BY_POOL_WRAP(cls, P1, P2, P3, P4, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4>, BuildType>::GetInstance()->WrapNew<BuildType>(OBJ_POOL_WRAP_TEMPLATE_NEW_P4(cls, P1, P2, P3, P4, BuildType, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P4_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_NEW_P4_BY_POOL_WRAP(cls, P1, P2, P3, P4, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P4_BY_POOL_WRAP(cls, P1, P2, P3, P4, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P4_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_DELETE_P4_BY_POOL_WRAP(cls, P1, P2, P3, P4, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), OBJ_POOL_WRAP_TEMPLATE_DELETE_P4(cls, P1, P2, P3, P4, BuildType, ptr)

// 五个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P5_BY_POOL_WRAP
#define __CRYSTAL_MA_TEMPLATE_NEW_P5_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5>, BuildType>::GetInstance()->WrapNew<BuildType>(OBJ_POOL_WRAP_TEMPLATE_NEW_P5(cls, P1, P2, P3, P4, P5, BuildType, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P5_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_NEW_P5_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P5_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P5_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_DELETE_P5_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), OBJ_POOL_WRAP_TEMPLATE_DELETE_P5(cls, P1, P2, P3, P4, P5, BuildType, ptr)

// 六个模版参数
#undef __CRYSTAL_MA_TEMPLATE_NEW_P6_BY_POOL_WRAP
#define __CRYSTAL_MA_TEMPLATE_NEW_P6_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, P6, BuildType, FileName, FileLine, ...) \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5, P6>, BuildType>::GetInstance()->WrapNew<BuildType>(OBJ_POOL_WRAP_TEMPLATE_NEW_P6(cls, P1, P2, P3, P4, P5, P6, BuildType, ##__VA_ARGS__), FileName, FileLine)

#undef CRYSTAL_MA_TEMPLATE_NEW_P6_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_NEW_P6_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, P6,BuildType, ...)                             \
__CRYSTAL_MA_TEMPLATE_NEW_P6_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, P6, BuildType, __FILE__, __LINE__, ##__VA_ARGS__)

#undef CRYSTAL_MA_TEMPLATE_DELETE_P6_BY_POOL_WRAP
#define CRYSTAL_MA_TEMPLATE_DELETE_P6_BY_POOL_WRAP(cls, P1, P2, P3, P4, P5, P6, BuildType, ptr)      \
KERNEL_NS::MemoryAssist<cls<P1, P2, P3, P4, P5, P6>, BuildType>::GetInstance()->WrapDelete<BuildType>(ptr), OBJ_POOL_WRAP_TEMPLATE_DELETE_P6(cls, P1, P2, P3, P4, P5, P6, BuildType, ptr)



KERNEL_BEGIN

template<typename BuildType>
struct MemoryAssistInfoByType
{
    POOL_CREATE_TEMPLATE_OBJ_DEFAULT(MemoryAssistInfoByType, BuildType);

    MemoryAssistInfoByType(UInt64 typeSize, const LibString &type, const LibString &buildType)
    :_type(type)
    ,_buildType(buildType)
    ,_totalNewCount{0}
    ,_totalDeleteCount{0}
    ,_activeObjCount{0}
    ,_curVecSize{0}
    , _objSize(typeSize)
    {

    }
    ~MemoryAssistInfoByType()
    {
        _curVecSize.store(0, std::memory_order_release);
        _objPosInfoList.clear();
        for(auto iter = _newObjPosRefActiveCountInfo.begin(); iter != _newObjPosRefActiveCountInfo.end();)
        {
            OBJ_POOL_WRAP_TEMPLATE_DELETE_P2(std::pair, LibString, Int64, BuildType, iter->second);
            iter = _newObjPosRefActiveCountInfo.erase(iter);
        }
    }

    const LibString _type;
    const LibString _buildType;
    std::atomic<Int64> _totalNewCount;      // new总数
    std::atomic<Int64> _totalDeleteCount;   // delete总数
    std::atomic<Int64> _activeObjCount;     // 活跃数量

    // 位置相关
    std::vector<std::pair<LibString, Int64> *> _objPosInfoList;
    std::atomic<Int64> _curVecSize;
    const UInt64 _objSize;

    std::map<LibString, std::pair<LibString, Int64> *> _newObjPosRefActiveCountInfo;    // 各个位置活跃数量
    std::map<LibString, Int64> _newObjPosRefVecIdx;                                     // 创建对象的位置对应的vector下标索引

    // 某个位置创建出来的对象指针对应的位置映射
    std::map<void *, LibString> _newObjPtrRefPos;                                   // 指针与位置映射
};

template<typename BuildType>
POOL_CREATE_TEMPLATE_OBJ_DEFAULT_IMPL(MemoryAssistInfoByType, BuildType);

template<typename ObjType, typename BuildType, LockParticleType::ENUMS ParticleType = LockParticleType::Light>
class MemoryAssist
{
    DEF_AGAINST_TEMPLATE_LAZY();

public:
    MemoryAssist();
    virtual ~MemoryAssist();
    static MemoryAssist<ObjType, BuildType, ParticleType> *GetInstance();

    template<typename ObjBuildType>
    ObjType *WrapNew(ObjType *obj, const Byte8 *fileName, Int32 line)
    {
        const LibString objPosStr = LibString(fileName).AppendFormat(":%d", line);

        // 总数统计
        _maInfo._totalNewCount.fetch_add(1, std::memory_order_release);
        _maInfo._activeObjCount.fetch_add(1, std::memory_order_release);

        _lck.Lock();

        // 指针对应的位置
        _maInfo._newObjPtrRefPos[obj] = objPosStr;

        // 位置活跃对象信息
        bool isNewInfo = false;
        auto iterActivePos = _maInfo._newObjPosRefActiveCountInfo.find(objPosStr);
        if(iterActivePos == _maInfo._newObjPosRefActiveCountInfo.end())
        {
            auto newPosActiveInfo = OBJ_POOL_WRAP_TEMPLATE_NEW_P2(std::pair, LibString, Int64, BuildType);
            newPosActiveInfo->first = objPosStr;
            newPosActiveInfo->second = 0;
            isNewInfo = true;
            iterActivePos = _maInfo._newObjPosRefActiveCountInfo.insert(std::make_pair(objPosStr, newPosActiveInfo)).first;
        }
        auto posActiveInfo = iterActivePos->second;
        ++posActiveInfo->second;

        // 添加到打印队列中
        if(UNLIKELY(isNewInfo))
        {
            _maInfo._objPosInfoList.push_back(posActiveInfo);
            _maInfo._newObjPosRefVecIdx[objPosStr] = _maInfo._objPosInfoList.size() - 1;
            _maInfo._curVecSize.fetch_add(1, std::memory_order_release);
        }

        _lck.Unlock();

        return obj;
    }

    template<typename ObjBuildType>
    void WrapDelete(ObjType *obj)
    {
        _maInfo._totalDeleteCount.fetch_add(1, std::memory_order_release);
        _maInfo._activeObjCount.fetch_sub(1, std::memory_order_release);

        _lck.Lock();

        auto iterPtrPos = _maInfo._newObjPtrRefPos.find(obj);
        if(iterPtrPos != _maInfo._newObjPtrRefPos.end())
        {
            auto &posStr = iterPtrPos->second;

            auto iterActiveCountPair = _maInfo._newObjPosRefActiveCountInfo.find(posStr);
            if(iterActiveCountPair != _maInfo._newObjPosRefActiveCountInfo.end())
            {
                auto activieInfoPair = iterActiveCountPair->second;
                --activieInfoPair->second;
            }
        }

        _lck.Unlock();
    }

private:
    static MemoryAssist<ObjType, BuildType, ParticleType> *_GetInstance(_Build::MT::Type);
    static MemoryAssist<ObjType, BuildType, ParticleType> *_GetInstance(_Build::TL::Type);

    UInt64 _Collect(LibString &memoryInfo);

private:
    LockWrap<BuildType, ParticleType> _lck;
    MemoryAssistInfoByType<BuildType> _maInfo;
    IDelegate<UInt64, LibString &> *_objPoolPrintDelg;
};

template<typename ObjType, typename BuildType, LockParticleType::ENUMS ParticleType>
ALWAYS_INLINE MemoryAssist<ObjType, BuildType, ParticleType>::MemoryAssist()
:_maInfo(sizeof(ObjType), RttiUtil::GetByType<ObjType>(), RttiUtil::GetByType<BuildType>())
,_objPoolPrintDelg(NULL)
{
    auto staticstics = MemoryMonitor::GetStatistics();
    _objPoolPrintDelg = DelegateFactory::Create<MemoryAssist<ObjType, BuildType, ParticleType>, UInt64, LibString &>(this, &MemoryAssist<ObjType, BuildType, ParticleType>::_Collect);
    staticstics->Lock();
    staticstics->GetDict().push_back(_objPoolPrintDelg);
    staticstics->Unlock();
}

template<typename ObjType, typename BuildType, LockParticleType::ENUMS ParticleType>
ALWAYS_INLINE MemoryAssist<ObjType, BuildType, ParticleType>::~MemoryAssist()
{
    if(_objPoolPrintDelg)
    {
        auto staticstics = MemoryMonitor::GetStatistics();
        staticstics->Lock();
        staticstics->Remove(_objPoolPrintDelg);
        staticstics->Unlock();
        _objPoolPrintDelg = NULL;
    }
}

template<typename ObjType, typename BuildType, LockParticleType::ENUMS ParticleType>
ALWAYS_INLINE MemoryAssist<ObjType, BuildType, ParticleType> *MemoryAssist<ObjType, BuildType, ParticleType>::GetInstance()
{
    return _GetInstance(BuildType::V);
}


template<typename ObjType, typename BuildType, LockParticleType::ENUMS ParticleType>
ALWAYS_INLINE MemoryAssist<ObjType, BuildType, ParticleType> *MemoryAssist<ObjType, BuildType, ParticleType>::_GetInstance(_Build::MT::Type)
{
    static MemoryAssist<ObjType, BuildType, ParticleType> *staticAlloctor = new MemoryAssist<ObjType, BuildType, ParticleType>();                                         

    // staticAlloctor->_againstLazy = 0;                                                       
    return staticAlloctor;
}

template<typename ObjType, typename BuildType, LockParticleType::ENUMS ParticleType>
ALWAYS_INLINE MemoryAssist<ObjType, BuildType, ParticleType> *MemoryAssist<ObjType, BuildType, ParticleType>::_GetInstance(_Build::TL::Type)
{
    DEF_STATIC_THREAD_LOCAL_DECLEAR MemoryAssist<ObjType, BuildType, ParticleType> *staticAlloctor = NULL;
    if(UNLIKELY(!staticAlloctor))
    {
        staticAlloctor = AllocUtil::GetStaticThreadLocalTemplateObjNoFree<MemoryAssist<ObjType, BuildType, ParticleType>>([]() -> void * {
            return new MemoryAssist<ObjType, BuildType, ParticleType>();
        });
    }                                       

    // staticAlloctor->_againstLazy = 0;                                                                                                         
    return staticAlloctor;
}

template<typename ObjType, typename BuildType, LockParticleType::ENUMS ParticleType>
ALWAYS_INLINE UInt64 MemoryAssist<ObjType, BuildType, ParticleType>::_Collect(LibString &memoryInfo)
{
    const Int64 curVecSize = _maInfo._curVecSize.load(std::memory_order_acquire);
    const Int64 totalNewCount = _maInfo._totalNewCount.load(std::memory_order_acquire);
    const Int64 totalDeleteCount = _maInfo._totalDeleteCount.load(std::memory_order_acquire);
    const Int64 activeObjCount = _maInfo._activeObjCount.load(std::memory_order_acquire);

    memoryInfo.AppendFormat("\n[MEMORY ASSIST RECORD BEGIN %s-%s]\n", _maInfo._type.c_str(), _maInfo._buildType.c_str());
    memoryInfo.AppendFormat("obj size:%llu, total new count:%lld, total new count:%lld, total active count:%lld, total create obj pos:%lld\n"
                            , _maInfo._objSize, totalNewCount, totalDeleteCount, activeObjCount, curVecSize);

    for(Int64 idx = 0; idx < curVecSize; ++idx)
    {
        auto posActiveInfo = _maInfo._objPosInfoList[idx];
        memoryInfo.AppendFormat("obj create pos:%s, current active count:%lld\n"
                    , posActiveInfo->first.c_str(), posActiveInfo->second);
    }
    memoryInfo.AppendFormat("\n[MEMORY ASSIST RECORD END %s-%s]\n", _maInfo._type.c_str(), _maInfo._buildType.c_str());

    return activeObjCount * _maInfo._objSize;
}

KERNEL_END

#endif
