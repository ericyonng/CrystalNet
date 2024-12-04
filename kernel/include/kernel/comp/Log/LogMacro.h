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
 * Date: 2024-12-05 00:24:51
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_MACRO_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_LOG_LOG_MACRO_H__

#pragma once

// 函数与行号便利宏
#undef _FUNC_LINE_ARGS_
#define _FUNC_LINE_ARGS_ __FILE__, __FUNCTION__, __LINE__

// // 获取tag

// 类实例获取tag
#undef LOG_OBJ_TAG
#define LOG_OBJ_TAG() KERNEL_NS::RttiUtil::GetByType<decltype(*this)>().c_str()
// 非类实例获取tag
#undef LOG_NON_OBJ_TAG
#define LOG_NON_OBJ_TAG(classType) KERNEL_NS::RttiUtil::GetByType<classType>().c_str()


// 带修饰的log宏
#undef LOGFMT_DETAIL
#define LOGFMT_DETAIL(tag, x) tag, _FUNC_LINE_ARGS_, x

// NO FMT
#undef LOGFMT_DETAIL_NO_FMT
#define LOGFMT_DETAIL_NO_FMT(tag) tag, _FUNC_LINE_ARGS_


// // 带tag宏,除了Net接口特殊外都可用

// 类实例tag
#undef LOGFMT_OBJ_TAG
#define LOGFMT_OBJ_TAG(x) LOGFMT_DETAIL(LOG_OBJ_TAG(), x)

// no fmt
#undef LOGFMT_OBJ_TAG_NO_FMT
#define LOGFMT_OBJ_TAG_NO_FMT() LOGFMT_DETAIL_NO_FMT(LOG_OBJ_TAG())

// 非类实例tag
#undef LOGFMT_NON_OBJ_TAG
#define LOGFMT_NON_OBJ_TAG(classType, x) LOGFMT_DETAIL(LOG_NON_OBJ_TAG(classType), x)

// no fmt
#undef LOGFMT_NON_OBJ_TAG_NO_FMT
#define LOGFMT_NON_OBJ_TAG_NO_FMT(classType) LOGFMT_DETAIL_NO_FMT(LOG_NON_OBJ_TAG(classType))

#endif
