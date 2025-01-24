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
 * Date: 2025-01-23 14:40:06
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <service_common/LibraryHotfixMonitor/Impl/LibraryHotfixMonitor.h>
#include <service_common/LibraryHotfixMonitor/Impl/LibraryHotfixMonitorFactory.h>
#include <kernel/comp/Timer/Timer.h>

#include "service_common/LibraryHotfixMonitor/Impl/HotFixDefine.h"

namespace 
{
    // 临时的参数规整
    struct HotFixParams
    {
        std::vector<KERNEL_NS::LibString> _params;
    };
    
}

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibraryHotfixMonitor);

LibraryHotfixMonitor::LibraryHotfixMonitor()
:ILibraryHotfixMonitor(KERNEL_NS::RttiUtil::GetTypeId<LibraryHotfixMonitor>())
,_tick(KERNEL_NS::LibTimer::NewThreadLocal_LibTimer())
,_enableDetection(false)
,_detectionInterval(KERNEL_NS::TimeSlice::FromSeconds(2))
{

}

LibraryHotfixMonitor::~LibraryHotfixMonitor()
{
    _Clear();
}

void LibraryHotfixMonitor::Release()
{
    LibraryHotfixMonitor::DeleteByAdapter_LibraryHotfixMonitor(LibraryHotfixMonitorFactory::_buildType.V, this);
}

void LibraryHotfixMonitor::SetDetectionFile(const KERNEL_NS::LibString &detectionFile)
{
    _deadthDetectionFile = detectionFile;

    _detectionFileNameWithoutDir = KERNEL_NS::DirectoryUtil::GetFileNameInPath(_deadthDetectionFile.c_str());
}

void LibraryHotfixMonitor::SetDetectionTimeInterval(Int64 seconds)
{
    _detectionInterval = KERNEL_NS::TimeSlice::FromSeconds(seconds);

    if(_tick && _tick->IsScheduling())
        _tick->Schedule(_detectionInterval);
}

void LibraryHotfixMonitor::SetRootPath(const KERNEL_NS::LibString &rootPath)
{
    _currentRootPath = rootPath;
    if((!_currentRootPath.IsEndsWith("/")) && (!_currentRootPath.IsEndsWith("\\")))
        _currentRootPath.AppendFormat("/");
}

void LibraryHotfixMonitor::AddHotFixListener(const KERNEL_NS::LibString &hotfixKey, KERNEL_NS::IDelegate<void, HotFixContainerElemType &> *cb)
{
    if(UNLIKELY(!cb))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("add a nil callback, hotfixkey:%s"), hotfixKey.c_str());
        return;
    }
    
    auto iter = _hotfixKeyRefHotfixDeleg.find(hotfixKey);
    if(iter == _hotfixKeyRefHotfixDeleg.end())
        iter = _hotfixKeyRefHotfixDeleg.insert(std::make_pair(hotfixKey, std::vector<KERNEL_NS::IDelegate<void, HotFixContainerElemType &> *>())).first;

    iter->second.push_back(cb);
}

void LibraryHotfixMonitor::AddHotFixCompleteCallback(KERNEL_NS::IDelegate<void, const std::set<KERNEL_NS::LibString> &> *cb)
{
    if(UNLIKELY(!cb))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("add a nil callback"));
        return;
    }

    _hotfixCompleteCallback.push_back(cb);
}



Int32 LibraryHotfixMonitor::_OnInit()
{
    _tick->SetTimeOutHandler(this, &LibraryHotfixMonitor::_OnDetectionTimerOut);

    if(!_detectionFileNameWithoutDir.empty() && _detectionInterval)
    {
        _enableDetection = true;
        _tick->Schedule(_detectionInterval);
    }

    g_Log->Info(LOGFMT_OBJ_TAG("enableDetection:%d _detectionInterval:%lld ms will detect hotfix file:%s, name without dir:%s")
        , _enableDetection, _detectionInterval.GetTotalMilliSeconds(), _deadthDetectionFile.c_str(), _detectionFileNameWithoutDir.c_str());

    return Status::Success;
}

void LibraryHotfixMonitor::_OnClose()
{
    _Clear();
}

void LibraryHotfixMonitor::_Clear()
{
    if(_enableDetection)
        _DelDetectionFile();

    _enableDetection = false;
    
    if(_tick)
    {
        KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_tick);
        _tick = NULL;
    }

    KERNEL_NS::ContainerUtil::DelContainer(_hotfixKeyRefHotfixDeleg, [](std::vector<KERNEL_NS::IDelegate<void, HotFixContainerElemType &> *> &container)
    {
        KERNEL_NS::ContainerUtil::DelContainer2(container);
    });
    KERNEL_NS::ContainerUtil::DelContainer2(_hotfixCompleteCallback);
}

void LibraryHotfixMonitor::_DelDetectionFile() const
{
    if(!_deadthDetectionFile.empty())
        KERNEL_NS::FileUtil::DelFileCStyle(_deadthDetectionFile.c_str());
}

void LibraryHotfixMonitor::_OnDetectionTimerOut(KERNEL_NS::LibTimer *timer)
{    
    // 文件检测到说明需要热更, 那么先读取文件内容再删除文件, 然后再执行热更
    if(!KERNEL_NS::FileUtil::IsFileExist(_deadthDetectionFile.c_str()))
        return;

    g_Log->Info(LOGFMT_OBJ_TAG("will scan hotfix file:%s"), _deadthDetectionFile.c_str());

    // 1.热更参数粗筛
    std::vector<HotFixParams> params;
    std::vector<KERNEL_NS::LibString> lines;
    {
        KERNEL_NS::SmartPtr<FILE, KERNEL_NS::AutoDelMethods::CustomDelete> fp = KERNEL_NS::FileUtil::OpenFile(_deadthDetectionFile.c_str());
        fp.SetClosureDelegate([](void *p)
        {
            if(p)
                KERNEL_NS::FileUtil::CloseFile(*KERNEL_NS::KernelCastTo<FILE>(p));
        });

        if(fp)
        {
            KERNEL_NS::FileUtil::ReadUtf8File(*fp, lines);
            const Int32 count = static_cast<Int32>(lines.size());
            Int32 idx = 0;
            while (idx < count)
            {
                // 扫描直到非空行
                auto &content = lines[idx];
                content.strip();
                if(content.empty())
                {
                    ++idx;
                    continue;
                }

                HotFixParams param;
                while (idx < count)
                {
                    auto &lineData = lines[idx];
                    lineData.strip();
                    
                    // 一个热更的参数粗筛结束
                    if(lineData.empty())
                    {
                        // 下一个参数
                        ++idx;
                        break;
                    }

                    // 粗筛有效参数内容
                    param._params.emplace_back(lineData);
                    ++idx;
                }

                // 当前是有参数的
                if(!param._params.empty())
                    params.emplace_back(param);
            }
        }
    }// 粗筛参数

    // 2.删除文件
    _DelDetectionFile();

    // 3.分析参数并执行热更, 热更要么全热更, 要么一个都不热更
    const Int32 count = static_cast<Int32>(params.size());

    // hotfixkey => 加载的热更初步结果
    std::map<KERNEL_NS::LibString, std::vector<HotFixContainerElemType>> hotfixKeyRefHotfixCaches;

    // 便利生成可热更的初步结果
    for(Int32 idx = 0; idx < count; ++idx)
    {
        auto &hotfixParam = params[idx];
        if(hotfixParam._params.empty())
            continue;

        if(hotfixParam._params.size() < 2)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("bad hotfix params:%s"), KERNEL_NS::StringUtil::ToString(hotfixParam._params, ",").c_str());
            continue;
        }
        
        // FilePath
        const Int32 paramCount = static_cast<Int32>(hotfixParam._params.size());
        bool isSuc = true;
        KERNEL_NS::LibString filePath;
        KERNEL_NS::LibString hotfixKey;
        for(Int32 idx = 0; idx < paramCount; ++idx)
        {
            auto &param = hotfixParam._params[idx];
            if(param.empty())
            {
                g_Log->Error(LOGFMT_OBJ_TAG("param is empty, hotfix params:%s, lines:%s")
                , KERNEL_NS::StringUtil::ToString(hotfixParam._params, ",").c_str()
                , KERNEL_NS::StringUtil::ToString(lines, ",").c_str());
                isSuc = false;
                break;
            }
            auto pos = param.GetRaw().find_first_of(":");
            if(pos == std::string::npos)
            {
                g_Log->Error(LOGFMT_OBJ_TAG("not found : hotfix params:%s, lines:%s")
                , KERNEL_NS::StringUtil::ToString(hotfixParam._params, ",").c_str()
                , KERNEL_NS::StringUtil::ToString(lines, ",").c_str());
                isSuc = false;
                break;
            }
            
            KERNEL_NS::LibString key = param.GetRaw().substr(0, pos);
            KERNEL_NS::LibString value = param.GetRaw().substr(pos + 1);
            key.strip();
            value.strip();

            if(key == HotfixParamName::FILE_PATH)
            {
                filePath = value;
                // 相对路径需要构建出一个绝对路径
                if(filePath.IsStartsWith("./") || filePath.IsStartsWith("../") || filePath.IsStartsWith(".\\") || filePath.IsStartsWith("..\\"))
                {
                    filePath = _currentRootPath + filePath;
                }
            }
            else if(key == HotfixParamName::HOTFIX_KEY)
            {
                hotfixKey = value;
            }
        }
        if(!isSuc)
        {
            g_Log->Error(LOGFMT_OBJ_TAG("resoleve hotfix param fail params:%s, lines:%s")
            , KERNEL_NS::StringUtil::ToString(hotfixParam._params, ",").c_str()
            , KERNEL_NS::StringUtil::ToString(lines, ",").c_str());
            continue;
        }
        
        // 文件是否存在
        if(!KERNEL_NS::FileUtil::IsFileExist(filePath.c_str()))
        {
            const auto &handledInfo = KERNEL_NS::StringUtil::ToStringBy(hotfixKeyRefHotfixCaches, ","
                , [](const KERNEL_NS::LibString &hotfixKeyStr, const std::vector<HotFixContainerElemType> &elems) -> KERNEL_NS::LibString
            {
                    const auto &elemStr = KERNEL_NS::StringUtil::ToStringBy(elems, ","
                        , [](const HotFixContainerElemType &elem) -> KERNEL_NS::LibString
                    {
                            return elem ? elem->ToString() : "";
                    });

                    return KERNEL_NS::LibString().AppendFormat("hotfix key:%s:[%s]", hotfixKeyStr.c_str(), elemStr.c_str());
            });
            
            g_Log->Error(LOGFMT_OBJ_TAG("file path:%s, not exists, handled hotfix will release:%s, hotfix file contents:%s")
                , filePath.c_str(), handledInfo.c_str(), KERNEL_NS::StringUtil::ToString(lines, ",").c_str());
            
            return;
        }

        // owner检查
        if(hotfixKey.empty())
        {
            const auto &handledInfo = KERNEL_NS::StringUtil::ToStringBy(hotfixKeyRefHotfixCaches, ","
                , [](const KERNEL_NS::LibString &hotfixKeyStr, const std::vector<HotFixContainerElemType> &elems) -> KERNEL_NS::LibString
            {
                    const auto &elemStr = KERNEL_NS::StringUtil::ToStringBy(elems, ","
                        , [](const HotFixContainerElemType &elem) -> KERNEL_NS::LibString
                    {
                            return elem ? elem->ToString() : "";
                    });

                    return KERNEL_NS::LibString().AppendFormat("hotfix key:%s:[%s]", hotfixKeyStr.c_str(), elemStr.c_str());
            });
            
            g_Log->Error(LOGFMT_OBJ_TAG("owner is empty, file path:%s, not exists, handled hotfix will release:%s, hotfix file contents:%s")
                , filePath.c_str(), handledInfo.c_str(), KERNEL_NS::StringUtil::ToString(lines, ",").c_str());
            
            return;
        }

        // 为有关注该热更的Listener加载动态库
        auto iter = _hotfixKeyRefHotfixDeleg.find(hotfixKey);
        if(iter == _hotfixKeyRefHotfixDeleg.end())
            continue;

        auto &listeners = iter->second;
        const Int32 totalCount = static_cast<Int32>(listeners.size());
        auto iterCaches = hotfixKeyRefHotfixCaches.find(hotfixKey);
        if(iterCaches ==  hotfixKeyRefHotfixCaches.end())
            iterCaches = hotfixKeyRefHotfixCaches.insert(std::make_pair(hotfixKey, std::vector<HotFixContainerElemType>())).first;
        
        auto &caches = iterCaches->second;
        if(caches.size() < static_cast<size_t>(totalCount))
            caches.resize(static_cast<size_t>(totalCount));
        
        for(Int32 offsetIdx = 0; offsetIdx < totalCount; ++offsetIdx)
        {
            auto cb = listeners[offsetIdx];
            if(UNLIKELY(!cb))
                continue;

            // 创建hotfixElem
            HotFixContainerElemType hotfixElem = SERVICE_COMMON_NS::HotFixCommonParam::NewThreadLocal_HotFixCommonParam();
            hotfixElem.SetClosureDelegate([](void *p)
            {
                SERVICE_COMMON_NS::HotFixCommonParam::DeleteThreadLocal_HotFixCommonParam(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::HotFixCommonParam>(p));
            });
            hotfixElem->_shareLib = KERNEL_NS::ShareLibraryLoaderFactory().Create()->CastTo<KERNEL_NS::ShareLibraryLoader>();
            hotfixElem->_hotfixKey = hotfixKey;

            // 加载动态库
            auto &shareLib = hotfixElem->_shareLib;
            shareLib->SetLibraryPath(filePath);
            auto err = shareLib->Init();
            if(err != Status::Success)
            {
                const auto &handledInfo = KERNEL_NS::StringUtil::ToStringBy(hotfixKeyRefHotfixCaches, ","
                    , [](const KERNEL_NS::LibString &hotfixKeyStr, const std::vector<HotFixContainerElemType> &elems) -> KERNEL_NS::LibString
                {
                        const auto &elemStr = KERNEL_NS::StringUtil::ToStringBy(elems, ","
                            , [](const HotFixContainerElemType &elem) -> KERNEL_NS::LibString
                        {
                                return elem ? elem->ToString() : "";
                        });

                        return KERNEL_NS::LibString().AppendFormat("hotfix key:%s:[%s]", hotfixKeyStr.c_str(), elemStr.c_str());
                });
                
                g_Log->Error(LOGFMT_OBJ_TAG("share library init fail err:%d, share library:%s, hot fix elem:%s, handled hotfix will release:%s, hotfix file contents:%s")
                    , err, shareLib->ToString().c_str(), hotfixElem->ToString().c_str(), handledInfo.c_str(), KERNEL_NS::StringUtil::ToString(lines, ",").c_str());
                
                return;
            }

            err = shareLib->Start();
            if(err != Status::Success)
            {
                const auto &handledInfo = KERNEL_NS::StringUtil::ToStringBy(hotfixKeyRefHotfixCaches, ","
                    , [](const KERNEL_NS::LibString &hotfixKeyStr, const std::vector<HotFixContainerElemType> &elems) -> KERNEL_NS::LibString
                {
                        const auto &elemStr = KERNEL_NS::StringUtil::ToStringBy(elems, ","
                            , [](const HotFixContainerElemType &elem) -> KERNEL_NS::LibString
                        {
                                return elem ? elem->ToString() : "";
                        });

                        return KERNEL_NS::LibString().AppendFormat("hotfix key:%s:[%s]", hotfixKeyStr.c_str(), elemStr.c_str());
                });
                
                g_Log->Error(LOGFMT_OBJ_TAG("share library start fail err:%d, share library:%s, hot fix elem:%s, handled hotfix will release:%s, hotfix file contents:%s")
                    , err, shareLib->ToString().c_str(), hotfixElem->ToString().c_str(), handledInfo.c_str(), KERNEL_NS::StringUtil::ToString(lines, ",").c_str());

                return;
            }

            caches[offsetIdx] = hotfixElem;
        }
    }

    // 便利成功的缓存, 并回调
    std::set<KERNEL_NS::LibString> completeHotfixKeys;
    for(auto &iter : hotfixKeyRefHotfixCaches)
    {
        auto &caches = iter.second;
        auto &hotfixKey = iter.first;
        auto iterCb = _hotfixKeyRefHotfixDeleg.find(hotfixKey);
        if(iterCb == _hotfixKeyRefHotfixDeleg.end())
            continue;

        // 完成的热更键
        completeHotfixKeys.insert(hotfixKey);

        auto &listeners = iterCb->second;
        const Int32 totalCount = static_cast<Int32>(listeners.size());
        for(Int32 offsetIdx = 0; offsetIdx < totalCount; ++offsetIdx)
        {
            auto cb = listeners[offsetIdx];
            if(UNLIKELY(!cb))
            {
                const auto &handledInfo = KERNEL_NS::StringUtil::ToStringBy(caches, ","
                    , [](const HotFixContainerElemType &elem) -> KERNEL_NS::LibString
                {
                        return elem ? elem->ToString() : "";
                });
                
                g_Log->Warn(LOGFMT_OBJ_TAG("register a nil callback hot key:%s, handledInfo:%s"), hotfixKey.c_str(), handledInfo.c_str());
                continue;
            }

            auto &cache = caches[offsetIdx];
            g_Log->Info(LOGFMT_OBJ_TAG("hotfix library is loaded, and will callback to hotfix, hotfix key:%s, hotfix info:%s"), hotfixKey.c_str(), cache->ToString().c_str());
            cb->Invoke(cache);
        }
    }
    
    // 4.空告警
    if(hotfixKeyRefHotfixCaches.empty())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("have no any valid hotfix info, hotfix contents:%s"), KERNEL_NS::StringUtil::ToString(lines, ",").c_str());
    }

    {
        const Int32 completeCount = static_cast<Int32>(_hotfixCompleteCallback.size());
        for(Int32 idx = 0; idx < completeCount; ++idx)
        {
            auto completeCb = _hotfixCompleteCallback[idx];
            if(UNLIKELY(!completeCb))
                continue;

            completeCb->Invoke(completeHotfixKeys);
            g_Log->Info(LOGFMT_OBJ_TAG("hotfix complete file:%s hotfix keys:%s"), _deadthDetectionFile.c_str(), KERNEL_NS::StringUtil::ToString(completeHotfixKeys, ",").c_str());
        }
    }
}

SERVICE_COMMON_END