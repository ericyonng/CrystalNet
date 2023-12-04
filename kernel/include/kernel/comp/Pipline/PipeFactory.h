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
 * Date: 2022-12-25 19:30:49
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_PIPE_FACTORY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_PIPELINE_PIPE_FACTORY_H__

#pragma once

#include <kernel/comp/Pipline/PipeType.h>
#include <kernel/comp/Pipline/MemoryPipe.h>
#include <kernel/comp/Pipline/FilePipe.h>

KERNEL_BEGIN

class IPipe;

class KERNEL_EXPORT PipeFactory
{
public:
    template<typename BuildType>
    static IPipe *Create(Int32 pipeType, std::function<bool(IPipe *pipe)> &&initFunc)
    {
        IPipe *pipe = NULL;
        switch (pipeType)
        {
        case PipeType::MEMORY:
        {
            pipe = MemoryPipe<BuildType>::NewByAdapter_MemoryPipe(BuildType::V);
        }
        break;
        case PipeType::FILE:
        {
            pipe = FilePipe::NewByAdapter_FilePipe(BuildType::V);
        }
        break;
        default:
            break;
        }

        if(LIKELY(pipe))
        {
            if(UNLIKELY(!initFunc(pipe)))
            {
                pipe->Release();
                return NULL;
            }
        }

        return pipe;
    }
};

KERNEL_END

#endif

