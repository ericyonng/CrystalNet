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
 * Date: 2020-10-11 23:01:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Utils/SystemUtil.h>

KERNEL_BEGIN

bool DirectoryUtil::CreateDir(const LibString &path)
{
    if(path.empty())
        return false;

    LibString rootDir, subDir;
    auto &pathRaw = path.GetRaw();
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
    auto startPos = path.GetRaw().find(':', 0);    // judge if include path
    if(startPos == std::string::npos)
    {
        // 没有盘符
        rootDir = "";
        subDir << "\\";
        subDir << pathRaw;
    }
    else
    {
        // 截取盘符
        rootDir = pathRaw.substr(startPos - 1, 1);
        rootDir << ":\\";

        // 盘符之后的子目录
        startPos = pathRaw.find("\\", startPos + 1);
        if(startPos == std::string::npos)
            return false;

        // 子目录路径
        subDir = pathRaw.substr(startPos, path.length() - startPos);
    }
#else
    if(pathRaw.at(0) == '/')
    {
        rootDir = "/";
        subDir = path;
    }
    else
    {
        auto startPos = pathRaw.find_first_of('/', 0);
        if(startPos == std::string::npos)
            return false;

        subDir = pathRaw.substr(startPos, path.length() - startPos);
        rootDir = pathRaw.substr(0, startPos);
    }
#endif

    return _CreateRecursiveDir(rootDir, subDir);
}

LibString DirectoryUtil::GetFileNameInPath(const LibString &path)
{
    if(!(path.GetRaw().length() > 0))
        return "";

    auto &pathRaw = path.GetRaw();
    Byte8 c = 0;
    Int32 i = 0;
    const Int32 len = static_cast<Int32>(pathRaw.length());
    for(i = len - 1; i >= 0; --i)
    {
        c = pathRaw.at(i);
        if(c == '\\' || c == '/')
        {
            ++i;
            break;
        }
    }

    if(UNLIKELY(i < 0))
        return path;


    return pathRaw.substr(i, len - i);
}

LibString DirectoryUtil::GetFileDirInPath(const LibString &path)
{
    if(path.GetRaw().length() <= 0)
        return "";

    char c = 0;
    int i = 0;
    auto &pathRaw = path.GetRaw();
    const Int32 len = static_cast<Int32>(pathRaw.length());
    for(i = len - 1; i >= 0; --i)
    {
        c = pathRaw.at(i);
        if(c == '\\' || c == '/')
        {
            ++i;
            break;
        }
    }

    if(i < 0)
        return "";

    return pathRaw.substr(0, i);
}

bool DirectoryUtil::IsDirExists(const LibString &dir)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS    
    if(::_access(dir.c_str(), 0) == -1)
        return false;
#else
    // CRYSTAL_TRACE("create sub dir %s", subDir.c_str());
    if(UNLIKELY(access(dir.c_str(), 0) == -1))
        return false;
#endif

    return true;
}

bool DirectoryUtil::_TraverseDirRecursively(const LibString &dir
    , IDelegate<bool, const FindFileInfo &, bool &> *stepCallback, Int32 &currentDepth, Int32 depth)
{
#if CRYSTAL_TARGET_PLATFORM_WINDOWS

    if(UNLIKELY(dir.empty()))
    {
        CRYSTAL_TRACE("dir is empty");
        return true;
    }

    if(UNLIKELY(!IsDirExists(dir)))
    {
        CRYSTAL_TRACE("dir is not existes: %s", dir.c_str());
        return true;
    }

    _finddata_t findData;
    const auto &matchAll = dir + "/*";
    auto handle = ::_findfirst(matchAll.c_str(), &findData);
    if(handle == -1)
    {
        CRYSTAL_TRACE("dir first file cant found dir:%s", dir.c_str());
        return true;
    }
    
    bool isContinue = true;
    do
    {
        // 跳过. ..
        if(strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
            continue;

        FindFileInfo findFile;
        findFile._fileName = findData.name;
        findFile._extension = FileUtil::ExtractFileExtension(findFile._fileName);
        findFile._rootPath = dir;

        findFile._fullName = dir;
        if(findFile._fullName.length() > 0)
        {
            const auto &lastSymbol = findFile._fullName[findFile._fullName.length() - 1];
            if(lastSymbol != '/' && lastSymbol != '\\')
            {
                findFile._fullName += '/';
            }
        }
        findFile._fullName += findFile._fileName;

        findFile._modifyTime = static_cast<Int64>(findData.time_write);
        if(findData.attrib & _A_ARCH)
            findFile._fileAttr |= FindFileInfo::F_FILE;
        if(findData.attrib & _A_SUBDIR)
            findFile._fileAttr |= FindFileInfo::F_DIR;

        // 回调
        if(stepCallback)
        {
            if(!stepCallback->Invoke(findFile, isContinue))
                break;
        }

        // 判断若是目录则递归
        // if(FileUtil::IsDir(findData) && (LibString(findData.name) != DirectoryUtil::GetFileNameInPath(dir)))
        if(FileUtil::IsDir(findFile))
        {
            const auto &subDir = dir + "/" + findFile._fileName;
            if((depth < 0) || (currentDepth < depth))
            {
                ++currentDepth;
                if (!_TraverseDirRecursively(subDir, stepCallback, currentDepth, depth))
                {
                    isContinue = false;
                    break;
                }
            }
        }

    } while (_FindNextFile(handle, findData));
    
    _findclose(handle);

    return isContinue;
#else
    if(UNLIKELY(dir.empty()))
    {
        CRYSTAL_TRACE("dir is empty");
        return true;
    }

    if(UNLIKELY(!IsDirExists(dir)))
    {
        CRYSTAL_TRACE("dir is not existes: %s", dir.c_str());
        return true;
    }

    DIR *d; // 声明一个句柄
    struct dirent *file; // readdir函数的返回值就存放在这个结构体中
    struct stat sb;   

    if(!(d = ::opendir(dir.c_str())))
    {
        CRYSTAL_TRACE("error opendir: %s", dir.c_str());
        return true;
    }

    bool isContinue = true;
    while((file = ::readdir(d)) != NULL)
    {
        // 把当前目录.，上一级目录..及隐藏文件都去掉，避免死循环遍历目录
        if((strcmp(file->d_name, ".") == 0) || (strcmp(file->d_name, "..") == 0))
            continue;

        FindFileInfo findFile;
        findFile._fileName = file->d_name;
        findFile._rootPath = dir;
        
        const auto &filePath = dir + "/" + findFile._fileName;
        if(::stat(filePath.c_str(), &sb) < 0)
        {
            CRYSTAL_TRACE("stat fail errno:%d, %s file name:%s, dir:%s", errno, SystemUtil::GetErrString(errno).c_str(), file->d_name, dir.c_str());
            continue;
        }

        findFile._modifyTime = static_cast<Int64>(sb.st_mtime);
        if(S_ISREG(sb.st_mode))
            findFile._fileAttr |= FindFileInfo::F_FILE;
        if(S_ISDIR(sb.st_mode))
            findFile._fileAttr |= FindFileInfo::F_DIR;

        if(stepCallback)
        {
            if(!stepCallback->Invoke(findFile, isContinue))
                break;
        }

        // 判断该文件是否是目录，及是否已搜索了三层，这里我定义只搜索了三层目录.
        if(FileUtil::IsDir(findFile))
        {
            if((depth < 0) || (currentDepth < depth))
            {
                ++currentDepth;
                if (!_TraverseDirRecursively(filePath, stepCallback, currentDepth, depth))
                {
                    isContinue = false;
                    break;
                }
            }
        }
    }

    ::closedir(d);

    return isContinue;
#endif
}

bool DirectoryUtil::_CreateRecursiveDir(const LibString &masterDir, const LibString &subDir)
{
    std::string dir = "";
    std::string strtocreate = "";
    std::string strMasterPath = masterDir.GetRaw();
    std::string::size_type startPos = 0;
    std::string::size_type revStartPos = 0;
    std::string::size_type endPos = 0;
    std::string::size_type revEndPos = 0;

    std::string::size_type finalStartPos = 0, finalCount = 0;
    const int spritLen = static_cast<int>(strlen("\\"));
    const int revSpritLen = static_cast<int>(strlen("/"));
    const auto &subDirRaw = subDir.GetRaw();
    while(true)
    {
        startPos = subDirRaw.find("\\", startPos);
        revStartPos = subDirRaw.find("/", revStartPos);
        if(startPos == std::string::npos && 
           revStartPos == std::string::npos) 
            break;

        if(startPos == std::string::npos && revStartPos != std::string::npos)
        {
            revEndPos = subDirRaw.find("/", revStartPos + revSpritLen);
            if(revEndPos == std::string::npos) 
                break;

            finalStartPos = revStartPos + revSpritLen;
            finalCount = revEndPos - revStartPos - revSpritLen;
            revStartPos += revSpritLen;
        }
        else if(startPos != std::string::npos && revStartPos == std::string::npos)
        {
            endPos = subDirRaw.find("\\", startPos + spritLen);
            if(endPos == std::string::npos)
                break;

            finalStartPos = startPos + spritLen;
            finalCount = endPos - startPos - spritLen;
            startPos += spritLen;
        }
        else if(startPos < revStartPos)
        {
            //"\\"
            endPos = subDirRaw.find("\\", startPos + spritLen);
            revEndPos = subDirRaw.find("/", startPos + spritLen);
            if(endPos == std::string::npos && 
               revEndPos == std::string::npos) 
                break;

            if(endPos != std::string::npos && revEndPos == std::string::npos)
            {
                finalStartPos = startPos + spritLen;
                finalCount = endPos - startPos - spritLen;
            }
            else if(endPos == std::string::npos && revEndPos != std::string::npos)
            {
                finalStartPos = startPos + spritLen;
                finalCount = revEndPos - startPos - spritLen;
            }
            else if(endPos < revEndPos)
            {
                //"\\"
                finalStartPos = startPos + spritLen;
                finalCount = endPos - startPos - spritLen;
                //dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
            }
            else
            {
                //"/"
                finalStartPos = startPos + spritLen;
                finalCount = revEndPos - startPos - spritLen;
                //dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
            }

            startPos += spritLen;
        }
        else
        {
            //"/"
            finalStartPos = revStartPos + revSpritLen;
            endPos = subDirRaw.find("\\", revStartPos + revSpritLen);
            revEndPos = subDirRaw.find("/", revStartPos + revSpritLen);
            if(endPos == std::string::npos &&
               revEndPos == std::string::npos) 
                break;

            if(endPos != std::string::npos && revEndPos == std::string::npos)
            {
                finalCount = endPos - finalStartPos;
            }
            else if(endPos == std::string::npos && revEndPos != std::string::npos)
            {
                finalCount = revEndPos - finalStartPos;
            }
            else if(endPos < revEndPos)
            {
                //"\\"
                finalCount = endPos - finalStartPos;
                //dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
            }
            else
            {
                //"/"
                finalCount = revEndPos - finalStartPos;
                //dir = strMasterPath + strSubDir.substr(iStartPos + strlen("\\"), iEndPos - iStartPos - strlen("\\"));
            }

            revStartPos += revSpritLen;
        }

        strtocreate.clear();
        dir = strMasterPath;
        if(dir.length() != 0) 
            dir += "/";

        dir += subDirRaw.substr(finalStartPos, finalCount);
        strtocreate = dir;
        strtocreate += "/";
        // CRYSTAL_TRACE("WILL CREATE strtocreate %s", strtocreate.c_str());
        if(strcmp((char *)strtocreate.c_str(), "./") != 0 && 
           strcmp((char *)strtocreate.c_str(), ".\\") != 0)
        {
            _CreateSubDir(strtocreate);
        }

        strMasterPath.clear();
        strMasterPath = dir;
    }

    return true;
}

static inline LibString NextFileError(int err)
{
    switch (err)
    {
    case EINVAL:
        return LibString().AppendFormat("err:%d Invalid parameter: filespec or fileinfo was NULL. Or, the operating system returned an unexpected error", err);
    case ENOENT:
        return LibString().AppendFormat("err:%d File specification that could not be matched.", err);
    case ENOMEM:
        return LibString().AppendFormat("err:%d Insufficient memory.", err);
    default:
        break;
    }

    return LibString().AppendFormat("unknown find next error err:%d", err);
}

#if CRYSTAL_TARGET_PLATFORM_WINDOWS
bool DirectoryUtil::_FindNextFile(intptr_t handle, _finddata_t &fileData)
{
    if(_findnext(handle, &fileData) != 0)
    {
        // CRYSTAL_TRACE("find next file fail errno :%d, %s findData:%s"
        //     , errno, NextFileError(errno).c_str(),  fileData.name);
        return false;
    }

    return true;
}
#endif

bool DirectoryUtil::_CreateSubDir(const std::string &subDir)
{
    if(subDir.length() <= 0)
        return false;

#if CRYSTAL_TARGET_PLATFORM_WINDOWS    
    if(::_mkdir(subDir.c_str()) != 0)
    {
        if(::_access(subDir.c_str(), 0) == -1)
            return false;
    }
#else
    // CRYSTAL_TRACE("create sub dir %s", subDir.c_str());
    if(mkdir(subDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    {
        if(UNLIKELY(access(subDir.c_str(), 0) == -1))
        {
            CRYSTAL_TRACE("create sub dir fail %s", subDir.c_str());
            return false;
        }
    }
#endif

    // CRYSTAL_TRACE("create dir suc %s", subDir.c_str());

    return true;
}


KERNEL_END
