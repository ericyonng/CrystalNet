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
 * Date: 2021-01-24 15:45:49
 * Author: Eric Yonng
 * Description: 
 *              1.随机数发生器由随机数源和随机数分布器构成
 *              2.随机数源是产生随机数的根本
 *              3.随机数分布器是限制随机数产生的范围以及随机性
 *              4.随机数分布器重新构建不影响随机数源的序列，随机序列不会被重置
 *              5.提供了特化的LibInt64Random生成Int64范围内的随机整数
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_RANDOM_RANDOM_H__

#pragma once

#include <kernel/comp/Random/RandomDefs.h>
#include <kernel/comp/Random/RandomDistribute.h>
#include <kernel/comp/Random/RandomSource.h>
#include <kernel/comp/Random/LibRandom.h>
#include <kernel/comp/Random/LibInt64Random.h>

#endif
