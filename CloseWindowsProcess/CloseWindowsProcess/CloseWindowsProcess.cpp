/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2023-06-30 15:03:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <CloseWindowsProcess/CloseWindowsProcess.h>
#include <service_common/ServiceCommon.h>
#include <CloseWindowsProcess/CloseWindowsProcessIni.h>

Int32 CloseWindowsProcess::Run(int argc, char const *argv[])
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    std::vector<KERNEL_NS::LibString> params;
    Int32 count = 0;
    KERNEL_NS::LibString killProcessName;
    bool isLikely = true;
    bool isMachPath = false;
    SERVICE_COMMON_NS::ParamsHandler::GetStandardParams(argc, argv, [&killProcessName, &isLikely, &isMachPath, &count](const KERNEL_NS::LibString &param, std::vector<KERNEL_NS::LibString> &leftParam) ->bool{
        
        bool ret = true;
        do
        {
            if(count == 0)
            {
                ret = false;
                break;
            }
            else if(count == 1)
            {
                killProcessName = param.strip();
                ret = true;
                break;
            }
            else
            {
                if(param.GetRaw().find("is_likely") != std::string::npos)
                {
                    const auto &parts = param.Split("=");
                    if(static_cast<Int32>(parts.size()) != 2)
                    {
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseWindowsProcess, "is_likely param error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    const auto &likelyName = parts[0].strip();
                    if(likelyName != "is_likely")
                    {
                        ret = false;
                        break;
                    }

                    const auto &value = parts[1].strip();
                    if(!value.isdigit())
                    {
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseWindowsProcess, "is_likely value not number error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    isLikely = KERNEL_NS::StringUtil::StringToInt32(value.c_str()) != 0;
                    ret = false;
                    break;
                }

                if(param.GetRaw().find("is_match_path") != std::string::npos)
                {
                    const auto &parts = param.Split("=");
                    if(static_cast<Int32>(parts.size()) != 2)
                    {
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseWindowsProcess, "is_match_path param error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    const auto &isMatchPath = parts[0].strip();
                    if(isMatchPath != "is_match_path")
                    {
                        ret = false;
                        break;
                    }

                    const auto &value = parts[1].strip();
                    if(!value.isdigit())
                    {
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseWindowsProcess, "is_match_path value not number error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    isMachPath = KERNEL_NS::StringUtil::StringToInt32(value.c_str()) != 0;
                    ret = true;
                    break;
                }
            }

        }while(false);
       
        count++;

        return ret;
    });

    killProcessName.findreplace("//", "\\");
    killProcessName.findreplace("/", "\\");
    g_Log->Custom("killProcessName:%s, isLikely:%d, isMachPath:%d", killProcessName.c_str(), isLikely, isMachPath);
    
    std::vector<UInt64> processIds;
    std::vector<KERNEL_NS::LibString> processNames;
    std::map<UInt64, KERNEL_NS::LibString> processIdRefNames;
    if(!killProcessName.empty() && KERNEL_NS::SystemUtil::GetProcessIdList(killProcessName, processIdRefNames, isLikely, isMachPath))
    {
        for(auto iter : processIdRefNames)
        {
            ULong lastErr = 0;
            auto ret = KERNEL_NS::SystemUtil::SendCloseMsgToProcess(iter.first, &lastErr);
            // auto ret = KERNEL_NS::SystemUtil::CloseProcess(processId, &lastErr);
            if(ret != Status::Success)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseWindowsProcess, "close process:%s, pid:%llu fail lastError:%lu"), iter.second.c_str(), iter.first, lastErr);
            }
            else
            {
                g_Log->Custom("close process:%s, pid:%llu success.", iter.second.c_str(), iter.first);
            }
        }
    }
    else
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseWindowsProcess, "cant find process:%s"), killProcessName.c_str());
    }

#endif

    return Status::Success;
}
