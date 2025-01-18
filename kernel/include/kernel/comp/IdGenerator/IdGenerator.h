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
 * Date: 2025-01-05 15:30:13
 * Author: Eric Yonng
 * Description: 
 * TODO: 
 * 1. 搭建id生成服务, 10bit机器码, 41bit 毫秒时间戳(从指定时间开始), 13bit 序列号: 采用批量获取id的模式, 从id生成器中获取到本地, id生成服务, 一定要保证时间同步, 
 * 2. 把id生成逻辑嵌入到本地: 限制:总的业务节点不能超过1024, 可以本地灵活的分配id, 无需额外支持, 一定要保证时间同步, 本地的时间戳是由远程id服务中心同步的, 如果没同步则一直等待
 * 3. 当当前时间戳下id耗尽, 需要等待时间同步
 * 4. 毫秒时间戳下可以使用69年
 * 5. 机器id并不是给业务服务分配的id, 而是id生成服务的节点分配的
 * 6. 举例子， 比如设计[35bit 秒级时间位(其实仍然是序列号满就增加1)] + [10bit 机器id位] + [23bit 序列号位] + 控速(令牌桶算法), 按照8,388,608 qps控速, 剩余id清零时, 比较下时间戳, 比上次的时间戳大则高位增加1, 如果没有了则结束生成
 *    当请求批量获取id来临时, 可以设置限制一次取的最大数量(设置成一个配置)和批量取的id数量取最小值, 来扣减当前剩余id数量,并返回, 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IDGENERATOR_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IDGENERATOR_IDGENERATOR_H__

#pragma once

#include <kernel/comp/IdGenerator/Impl/IdGenerator.h>
#include <kernel/comp/IdGenerator/Impl/IdGeneratorFactory.h>

#endif
