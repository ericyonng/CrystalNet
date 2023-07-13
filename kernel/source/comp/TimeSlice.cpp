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
 * Date: 2021-01-02 00:54:09
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/TimeSlice.h>
#include <kernel/comp/Utils/StringUtil.h>

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(TimeSlice);

TimeSlice::TimeSlice(const LibString &fmtSlice)
{
    // Ensure the slice string is time format, not datetime format.
    LibString sliceRepr = fmtSlice;
    const auto &fmtRaw = fmtSlice.GetRaw();
    std::string::size_type spaceIdx = fmtRaw.find(' ');
    if(spaceIdx != std::string::npos)
        sliceRepr = fmtRaw.substr(spaceIdx + 1);

    // If the slice string is empty, set slice value to 0.
    if(sliceRepr.empty())
    {
        _slice = 0;
        return;
    }

    // Split by ':', fetch hour,minute,second, nanosecond parts.
    auto sliceParts = fmtSlice.Split(':');
    if(sliceParts.size() == 1) // Only has second part.
    {
        sliceParts.insert(sliceParts.begin(), "0");
        sliceParts.insert(sliceParts.begin(), "0");
    }
    else if(sliceParts.size() == 2) // Only has second and minute parts.
    {
        sliceParts.insert(sliceParts.begin(), "0");
    }

    _slice = StringUtil::StringToInt32(sliceParts[0].c_str()) * TimeDefs::NANO_SECOND_PER_HOUR +
        StringUtil::StringToInt32(sliceParts[1].c_str()) * TimeDefs::NANO_SECOND_PER_MINUTE;

    auto secParts = sliceParts[2].Split('.', 1);
    _slice += StringUtil::StringToInt32(secParts[0].c_str()) * TimeDefs::NANO_SECOND_PER_SECOND;
    if(secParts.size() == 2)
        _slice += StringUtil::StringToInt32(secParts[1].c_str());
}

KERNEL_END
