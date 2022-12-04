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
 * Date: 2021-12-09 02:03:36
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_COMMON_MACRO_H__
#define __CRYSTAL_NET_SERVICE_COMMON_COMMON_MACRO_H__

#pragma once

#include <kernel/common/macro.h>

// service_common命名空间
#ifndef SERVICE_COMMON_BEGIN
    #define SERVICE_COMMON_BEGIN CRYSTAL_NET_BEGIN    \
    namespace service_common{ 
#endif
#ifndef SERVICE_COMMON_END
    #define SERVICE_COMMON_END CRYSTAL_NET_END    \
    }
#endif
#ifndef SERVICE_COMMON_NS
    #define SERVICE_COMMON_NS CRYSTAL_NET_NS::service_common
#endif

#endif
