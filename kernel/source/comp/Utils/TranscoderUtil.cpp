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
 * Date: 2023-06-10 17:13:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibString.h>

// iconv lib header file.
#if CRYSTAL_TARGET_PLATFORM_WINDOWS
 #include <kernel/comp/Utils/iconv.h>
#else // Non-Win32
 #include <iconv.h>
#endif // Win32

#include <kernel/comp/Utils/TranscoderUtil.h>

KERNEL_BEGIN

Int32 TranscoderUtil::MultiByteToWideChar(const LibString &fromCode, const LibString &src, LibString &dest)
{
    if (fromCode.empty())
    {
        return Status::ParamError;
    }

    dest.clear();
    if (src.empty())
    {
        return Status::Success;
    }

    iconv_t cd = iconv_open("UTF-16LE", fromCode.c_str());
    if (cd == reinterpret_cast<iconv_t>(-1))
    {
        return Status::LibraryError;
    }

    size_t inLen = src.length();
    char *in = const_cast<char *>(src.c_str());

    size_t outLen = (inLen + 1) * sizeof(wchar);
    dest.resize(outLen);

    char *out = const_cast<char *>(reinterpret_cast<const char *>(dest.c_str()));
    if (iconv(cd, &in, &inLen, &out, &outLen) == (size_t)(-1))
    {
        dest.clear();
        iconv_close(cd);

        return Status::LibraryError;
    }

    iconv_close(cd);
    dest.resize(out - reinterpret_cast<const char *>(dest.c_str()));

    return Status::Success;
}

Int32 TranscoderUtil::WideCharToMultiByte(const LibString &toCode, const LibString &src, LibString &dest)
{
    if (toCode.empty())
    {
        return Status::Failed;
    }

    dest.clear();
    if (src.empty())
    {
        return Status::Success;
    }

    iconv_t cd = iconv_open(toCode.c_str(), "UTF-16LE");
    if (cd == (iconv_t)(-1))
        return Status::LibraryError;

    size_t inLen = src.length();
    char *in = const_cast<char *>(src.c_str());

    size_t outLen = inLen * 2 + 4;
    dest.resize(outLen);
    char *out = const_cast<char *>(dest.c_str());

    if (iconv(cd, &in, &inLen, &out, &outLen) == (size_t)(-1))
    {
        dest.clear();
        iconv_close(cd);

        return Status::LibraryError;
    }

    iconv_close(cd);
    dest.resize(out - dest.c_str());

    return Status::Success;
}

Int32 TranscoderUtil::MultiByteToMultiByte(const LibString &fromCode,
                                const LibString &src,
                                const LibString &toCode,
                                LibString &dest)
{
    if (fromCode == toCode)
    {
        dest.clear();
        dest.AppendData(src.c_str(), src.length());

        return Status::Success;
    }

    LibString wStr;
    const Int32 err = MultiByteToWideChar(fromCode, src, wStr);
    if (err != Status::Success)
    {
        return err;
    }

    return WideCharToMultiByte(toCode, wStr, dest);
}

KERNEL_END
