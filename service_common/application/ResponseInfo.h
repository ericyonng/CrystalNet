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
 * Date: 2022-12-04 00:14:42
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_RESPONSE_INFO_H__
#define __CRYSTAL_NET_SERVICE_COMMON_APPLICATION_RESPONSE_INFO_H__

#pragma once

#include <kernel/kernel.h>
#include <service_common/common/common.h>

SERVICE_COMMON_BEGIN

struct ResponseInfo
{
    POOL_CREATE_OBJ_DEFAULT(ResponseInfo);

    UInt64 _id;
    UInt64 _resMicroSeconds;
};

class ResponseInfoCompare
{
public:
    bool operator()(const ResponseInfo *l, const ResponseInfo *r) const;
};

struct StatisticsInfo
{
    POOL_CREATE_OBJ_DEFAULT(StatisticsInfo);

    StatisticsInfo()
        :_minResNs(0)
        ,_maxResNs(0)
        ,_resCount(0)
        ,_resTotalNs(0)
    {

    }

    UInt64 _minResNs;
    UInt64 _maxResNs;
    UInt64 _resCount;
    UInt64 _resTotalNs;
};

SERVICE_COMMON_END

#endif
