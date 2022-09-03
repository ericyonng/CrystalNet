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
 * Date: 2021-01-05 23:44:29
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/StringUtil.h>

KERNEL_BEGIN


const Byte8 * FileUtil::GenRandFileNameNoDir(Byte8 randName[L_tmpnam])
{
#if CRYSTAL_TARGET_PLATFORM_LINUX
    // mkstemp(文件名)/mkdtemp(目录)
    ::memset(randName, 0, L_tmpnam);
    ::strcpy(randName, "tmp_XXXXXX");
    int fd = ::mkstemp(randName);
    if(fd == -1)
        return NULL;
    
    ::unlink(randName);
    const auto &fileName = DirectoryUtil::GetFileNameInPath(randName);
    if(fileName.GetRaw().length() > 0)
    {
        randName[0] = 0;
        auto len = sprintf(randName, "%s", fileName.c_str());
        len = ((len < L_tmpnam) ? std::max<Int32>(len, 0) : (L_tmpnam - 1));
        randName[len] = 0;
        return randName;
    }

    return NULL;
#endif

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    if(tmpnam(randName))
    {
        const auto &fileName = DirectoryUtil::GetFileNameInPath(randName);
        if(fileName.GetRaw().length() > 0)
        {
            randName[0] = 0;
            auto len = sprintf(randName, "%s", fileName.c_str());
            len = ((len < L_tmpnam) ? std::max<Int32>(len, 0) : (L_tmpnam - 1));
            randName[len] = 0;
            return randName;
        }
    }

    return NULL;
#endif
}

UInt64 FileUtil::ReadUtf8OneLine(FILE &fp, LibString &outBuffer, UInt64 *utf8CharCount)
{
    // 读取单字节字符时候判断是否\n
    clearerr(&fp);
    U8 get_c = 0;
    UInt64 cnt = 0;
    while(!feof(&fp))
    {
        get_c = 0;
        auto bytes = fread(&get_c, sizeof(get_c), 1, &fp);
        if(bytes == 1)
        {
            // 该utf8字符总字节数
            auto totalBytes = StringUtil::CalcUtf8CharBytes(get_c);
            UInt64 leftBytes = totalBytes - 1;
            if(!leftBytes)
            {// 单字节字符

                #if CRYSTAL_TARGET_PLATFORM_WINDOWS
                            
                    if(get_c == '\r')
                        continue;

                    if(get_c != '\n')
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;

                        if(UNLIKELY(utf8CharCount))
                            ++(*utf8CharCount);
                    }
                    else
                    {
                        break;
                    }
                #else
                    if(get_c != '\n')
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;
                        
                        if(UNLIKELY(utf8CharCount))
                            ++(*utf8CharCount);
                    }
                    else
                    {
                        //fread(&get_c, sizeof(get_c), 1, fpOutCache);
                        break;
                    }
                #endif
            }
            else
            {// 多字节字符

                outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                ++cnt;
                
                if(UNLIKELY(utf8CharCount))
                    ++(*utf8CharCount);

                // 读取剩下字节
                do
                {
                    if(fread(&get_c, sizeof(get_c), 1, &fp) == 1)
                    {
                        outBuffer.AppendData(reinterpret_cast<const char *>(&get_c), 1);
                        ++cnt;
                    }
                    else
                    {// 错误字符
                        break;
                    }
                } while (--leftBytes);

                // 错误字符
                if(UNLIKELY(leftBytes))
                {
                    CRYSTAL_TRACE("ERROR UTF8 Char totalBytes[%llu], leftBytes[%llu], fread fail", totalBytes, leftBytes);
                    break;
                }
            }
        }
        else
        {
            // 文件结束
            break;
        }
    }

    return cnt;
}

KERNEL_END
