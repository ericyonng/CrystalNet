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
 * Date: 2021-02-18 23:10:34
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/File/ConsoleConfigMgr.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/File/LibIniFile.h>
#include <kernel/comp/SmartPtr.h>
#include <kernel/comp/Utils/SystemUtil.h>
#include <kernel/comp/Utils/DirectoryUtil.h>
#include <kernel/comp/LibString.h>
#include <kernel/source/comp/File/ConsoleConfigTemplate.h>

KERNEL_BEGIN

ConsoleConfigMgr::ConsoleConfigMgr()
    :_ini(NULL)
    ,_front{0}
    ,_back{0}
{

}

ConsoleConfigMgr::~ConsoleConfigMgr()
{
    Close();
}

// 放在程序根目录ini子目录下
ConsoleConfigMgr *ConsoleConfigMgr::GetInstance()
{
    static SmartPtr<ConsoleConfigMgr> s_consoleMgr(new ConsoleConfigMgr());
    return s_consoleMgr.Cast<ConsoleConfigMgr>();
}

bool ConsoleConfigMgr::Init(const Byte8 *cfgFileName, const Byte8 *dirName, const LibString &content)
{
    if(UNLIKELY(_ini))
    {
        CRYSTAL_TRACE("file[%s], ConsoleConfigMgr already init", cfgFileName);
        return false;
    }

    LibString fileName = dirName;
    fileName += cfgFileName;

    // 文件是否存在 不存在则创建
    if(content.empty())
    {
        if(UNLIKELY(!FileUtil::IsFileExist(fileName.c_str())))
        {
            CRYSTAL_TRACE("file[%s], not exist when init console config mgr and will create", fileName.c_str());
            if(fileName.empty())
            {
                CRYSTAL_TRACE("log config file is empty, create fail");
                return false;
            }

            // 获取路径
            const auto &fileDir = DirectoryUtil::GetFileDirInPath(fileName);
            // 创建路径
            if(!DirectoryUtil::CreateDir(fileDir))
            {
                CRYSTAL_TRACE("log config dir[%s], create fail", fileDir.c_str());
                return false;
            }

            auto fp = FileUtil::OpenFile(fileName.c_str(), true);
            const auto &content = ConsoleConfigTemplate::GetConsoleIniContent();
            FileUtil::WriteFile(*fp, content.data(), content.size());
            FileUtil::FlushFile(*fp);
            FileUtil::CloseFile(*fp);
        }
    }

    _ini = LibIniFile::New_LibIniFile();
    if(!content.empty())
        _ini->SetMemoryIniContent(content);

    if(!_ini->Init(content.empty() ? fileName.c_str() : NULL, false))
    {
        CRYSTAL_TRACE("file[%s], init ini file fail when init console config mgr", fileName.c_str());
        return false;
    }

    // 初始化前景色
    Int64 result;
    LibString keyName = "Black";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._black = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);
    
    keyName = "Red";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._red = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Green";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", "Green", result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._green = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Blue";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._blue = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Yellow";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._yellow = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Purple";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._purple = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Cyan";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._cyan = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "White";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._white = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Gray";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._gray = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "LightYellow";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._lightYellow = static_cast<Int32>(result);
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Highlight";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._highLight = static_cast<Int32>(result);  
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "FrontDefault";
    if(!_ini->CheckReadInt("Win32FrontConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32FrontConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _front._frontDefault = static_cast<Int32>(result);  
    _frontNameRefColor[keyName] = static_cast<Int32>(result);

    // 初始化背景色
    keyName = "Black";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._black = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Red";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._red = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Green";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._green = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Blue";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._blue = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);


    keyName = "Yellow";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._yellow = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Purple";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._purple = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Cyan";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._cyan = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "White";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._white = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "Highlight";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._highLight = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    keyName = "BackDefault";
    if(!_ini->CheckReadInt("Win32BackConsoleColor", keyName.c_str(), result))
    {
        CRYSTAL_TRACE("CheckReadInt Win32BackConsoleColor %s FAIL", keyName.c_str());
        return false;
    }
    _back._backDefault = static_cast<Int32>(result); 
    _backNameRefColor[keyName] = static_cast<Int32>(result);

    return true;
}

void ConsoleConfigMgr::Close()
{
    if(_ini)
    {
      LibIniFile::Delete_LibIniFile(_ini);
    }

    _ini = NULL;
}

const Int32 ConsoleConfigMgr::GetFrontColor(const Byte8 *colorName) const
{
    return _frontNameRefColor.find(colorName)->second;
}

const Int32 ConsoleConfigMgr::GetBackColor(const Byte8 *colorName) const
{
    return _backNameRefColor.find(colorName)->second;
}

KERNEL_END