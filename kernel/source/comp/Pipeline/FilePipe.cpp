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
 * Date: 2022-12-25 15:59:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/File/FileCursorOffsetType.h>
#include <kernel/comp/Utils/FileUtil.h>
#include <kernel/comp/Log/log.h>
#include <kernel/comp/Pipline/PipeType.h>
#include <kernel/comp/Delegate/LibDelegate.h>
#include <kernel/comp/Pipline/FilePipe.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(FilePipe);

FilePipe::FilePipe()
:IPipe(PipeType::FILE)
,_fp(NULL)
,_writePos(0)
,_readPos(0)
,_release(NULL)
{

}

FilePipe::~FilePipe()
{
    Close();

    if(LIKELY(_release))
        CRYSTAL_RELEASE_SAFE(_release);
}

void FilePipe::Release()
{
    if(LIKELY(_release))
        _release->Invoke();
}

void FilePipe::SetRelease(std::function<void()> &&func)
{
    if(UNLIKELY(_release))
        CRYSTAL_RELEASE_SAFE(_release);
    
    _release = KERNEL_CREATE_CLOSURE_DELEGATE(func, void);
}

bool FilePipe::Init(const LibString &file)
{
    Close();
    
    _file = file;
    _writePos = 0;
    _readPos = 0;
    _fp = NULL;

    return true;
}

void FilePipe::SetPipeName(const LibString &name)
{
    _file = name;
}

LibString FilePipe::GetPipeName() const
{
    return _file;
}

Int32 FilePipe::Open()
{
    _fp = FileUtil::OpenFile(_file.c_str(), true);
    if(!_fp)
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("open file fail file:%s"), _file.c_str());
        return Status::Failed;
    }

    _writePos = 0;
    _readPos = 0;

    return Status::Success;
}

bool FilePipe::Write(const Byte8 *buffer, Int64 &sz)
{
    if(!FileUtil::SetFileCursor(*_fp, FileCursorOffsetType::FILE_CURSOR_POS_SET, static_cast<Long>(_writePos)))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("set file cusour fail write pos:%lld, file:%s"), _writePos, _file.c_str());
        return false;
    }

    sz = FileUtil::WriteFile(*_fp, buffer, sz);
    _writePos += sz;

    return true;
}

bool FilePipe::Read(Byte8 *buffer, Int64 &count)
{
    if(!FileUtil::SetFileCursor(*_fp, FileCursorOffsetType::FILE_CURSOR_POS_SET, static_cast<Long>(_readPos)))
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("set file cusour fail read pos:%lld, file:%s"), _readPos, _file.c_str());
        return false;
    }

    count = static_cast<Int64>(FileUtil::ReadFile(*_fp, static_cast<UInt64>(count), buffer));
    _readPos += count;
    
    return true;
}

void FilePipe::Flush()
{
    FileUtil::FlushFile(*_fp);
}

void FilePipe::Close()
{
    if(UNLIKELY(!_fp))
        return;

    Flush();
    FileUtil::CloseFile(*_fp);
    _fp = NULL;
    _writePos = 0;
    _readPos = 0;
}

KERNEL_END