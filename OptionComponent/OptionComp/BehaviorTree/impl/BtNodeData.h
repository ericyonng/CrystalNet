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
 * Date: 2023-09-26 14:17:00
 * Author: Eric Yonng
 * Description:
*/

#ifndef __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_BTNODE_DATA_H__
#define __CRYSTAL_NET_OPTION_COMPONENT_BEHAVIOR_TREE_IMPL_BTNODE_DATA_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/Poller/PollerInc.h>

KERNEL_BEGIN

class BtNodeType
{
public:
    enum ENUMS
    {
        UNKNOWN = 0,
        SEQUENCE,
        SELECT,
        LOOP,   // for
        PARALLEL,

        MAX_TYPE,
    };
};

class BtNodeState
{
public:
    enum ENUMS
    {
        CREATED = 0,
        INITED,
        RUNNING,
        SUCCESS,
        FAILURE,
    };

    static const Byte8 *GetStr(Int32 state)
    {
        switch (state)
        {
        case BtNodeState::CREATED: return "CREATED";
        case BtNodeState::INITED: return "INITED";
        case BtNodeState::RUNNING: return "RUNNING";
        case BtNodeState::SUCCESS: return "SUCCESS";
        case BtNodeState::FAILURE: return "FAILURE";
        default:
            break;
        }

        return "UNKNOWN";
    }
};

class BtState
{
public:
    enum ENUMS
    {
        CREATED = 0,
        INITED,
        RUNNING,
        FINISHED,
    };

    static const Byte8 *GetStr(Int32 state)
    {
        switch (state)
        {
        case BtState::CREATED: return "CREATED";
        case BtState::INITED: return "INITED";
        case BtState::RUNNING: return "RUNNING";
        case BtState::FINISHED: return "FINISHED";
        default:
            break;
        }

        return "UNKNOWN";
    }
};

struct BtNodeData
{
    POOL_CREATE_OBJ_DEFAULT(BtNodeData);

    UInt64 _id = 0;     // nodeId
    UInt64 _flags = 0;  // 特性
    Int32 _type = BtNodeType::UNKNOWN;    // BtNodeType
    Int32 _state = BtNodeState::CREATED;   // BtNodeState
};

KERNEL_END

#endif