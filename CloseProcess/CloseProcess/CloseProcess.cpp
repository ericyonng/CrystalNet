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
#include <CloseProcess/CloseProcess.h>
#include <service_common/ServiceCommon.h>
#include <CloseProcess/CloseProcessIni.h>

Int32 CloseProcess::Run(int argc, char const *argv[])
{
    std::vector<KERNEL_NS::LibString> params;
    Int32 count = 0;
    KERNEL_NS::LibString killProcessName;
    bool isLikely = true;
    bool isMatchPath = false;
    bool isWaitingClose = false;
    KERNEL_NS::ParamsHandler::GetStandardParams(argc, argv, [&killProcessName, &isLikely, &isMatchPath, &isWaitingClose, &count](const KERNEL_NS::LibString &param, std::vector<KERNEL_NS::LibString> &leftParam) ->bool{
        
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
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "is_likely param error param:%s"), param.c_str());
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
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "is_likely value not number error param:%s"), param.c_str());
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
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "is_match_path param error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    const auto &valueStr = parts[0].strip();
                    if(valueStr != "is_match_path")
                    {
                        ret = false;
                        break;
                    }

                    const auto &value = parts[1].strip();
                    if(!value.isdigit())
                    {
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "is_match_path value not number error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    isMatchPath = KERNEL_NS::StringUtil::StringToInt32(value.c_str()) != 0;
                    ret = true;
                    break;
                }

                if(param.GetRaw().find("is_waiting_close") != std::string::npos)
                {
                    const auto &parts = param.Split("=");
                    if(static_cast<Int32>(parts.size()) != 2)
                    {
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "is_waiting_close param error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    const auto &valueStr = parts[0].strip();
                    if(valueStr != "is_waiting_close")
                    {
                        ret = false;
                        break;
                    }

                    const auto &value = parts[1].strip();
                    if(!value.isdigit())
                    {
                        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "is_waiting_close value not number error param:%s"), param.c_str());
                        ret = false;
                        break;
                    }

                    isWaitingClose = KERNEL_NS::StringUtil::StringToInt32(value.c_str()) != 0;
                    ret = true;
                    break;
                }
            }

        }while(false);
       
        count++;

        return ret;
    });

    #if CRYSTAL_TARGET_PLATFORM_WINDOWS
     killProcessName.findreplace("//", "\\");
     killProcessName.findreplace("/", "\\");
    #else
     killProcessName.findreplace("\\", "/");
     killProcessName.findreplace("//", "/");
    #endif

    g_Log->Custom("killProcessName:%s, isLikely:%d, isMachPath:%d, isWaitingClose:%d", killProcessName.c_str(), isLikely, isMatchPath, isWaitingClose);
    
    std::vector<UInt64> processIds;
    std::vector<KERNEL_NS::LibString> processNames;
    std::map<UInt64, KERNEL_NS::LibString> processIdRefNames;
    if(!killProcessName.empty() && KERNEL_NS::SystemUtil::GetProcessIdList(killProcessName, processIdRefNames, isLikely, isMatchPath))
    {
        for(auto iter : processIdRefNames)
        {
            // 创建关闭文件:xxx.kill_processId
            KERNEL_NS::LibString killFile;
            killFile.AppendFormat("%s.kill_%llu", iter.second.c_str(), iter.first);
            auto fp = KERNEL_NS::FileUtil::OpenFile(killFile.c_str(), true, "rb");
            if(!fp)
            {
                g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "create kill file fail. file:%s"), killFile.c_str());
            }
            else
            {
                KERNEL_NS::FileUtil::CloseFile(*fp);
            }

            // 等待关闭
            if(isWaitingClose)
            {
                while (true)
                {
                    if(KERNEL_NS::SystemUtil::IsProcessExist(iter.first))
                        g_Log->Custom("waiting process quit process:%s, process id:%llu", iter.second.c_str(), iter.first);
                    else
                        break;

                    KERNEL_NS::SystemUtil::ThreadSleep(1000);
                }

                g_Log->Custom("process is killed process:%s, process id:%llu.", iter.second.c_str(), iter.first);
            }
            else
            {
                g_Log->Custom("just notify process close, process name:%s, process id:%llu.", iter.second.c_str(), iter.first);
            }
        }
    }
    else
    {
        g_Log->Warn(LOGFMT_NON_OBJ_TAG(CloseProcess, "cant find process:%s"), killProcessName.c_str());
    }

    return Status::Success;
}
