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
 * Date: 2020-11-10 01:31:12
 * Author: Eric Yonng
 * Description: 
*/

#include<pch.h>
#include<kernel/comp/memory/MemoryBuffer.h>
#include<kernel/comp/memory/MemoryBlock.h>

KERNEL_BEGIN

// void MemoryBuffer::Init()
// {
//     /**
//     *    死亡区
//     */
//     _deadArea = reinterpret_cast<LibStack<MemoryBlock *, MemoryBuffer> *>(_buffer);
//     MemoryBlock **deadStackPtr = reinterpret_cast<MemoryBlock **>(_buffer + _stackAlignSize);
//     _deadArea->Init(this, deadStackPtr, _blockCnt);

//     /**
//     *     可用内存区
//     */
//    _usableBuffer = _buffer + _deadAreaSizeAfterAlign;

//     /**
//     *   链表头和尾部指向同一位置
//     */
//     _head = reinterpret_cast<MemoryBlock*>(_usableBuffer);
//     _head->_blockSize = _blockSize;
//     _head->_usableBytes = _usableBytesPerBlock;
//     _head->_objBytes = 0;
//     _head->_ref = 0;
//     _head->_buffer = this;
//     _head->_isInAlloctor = true;
//     _head->_next = NULL;
//     _curInitBlockIdx = 1;

//     // 出于性能考虑使用惰性初始化
//     // MemoryBlock *temp = _head;

//     // // 构建内存块链表
//     // for(size_t i = 1; i < _blockCnt; ++i)
//     // {
//     //     Byte8 *cache = (_usableBuffer + _blockSize * i);
//     //     MemoryBlock *block = reinterpret_cast<MemoryBlock*>(cache);
//     //     block->_blockSize = _blockSize;
//     //     block->_usableBytes = _usableBytesPerBlock;
//     //     block->_objBytes = 0;
//     //     block->_ref = 0;
//     //     block->_buffer = this;
//     //     block->_isInAlloctor = true;
//     //     block->_next = NULL;
        
//     //     temp->_next = block;
//     //     temp = block;
//     // }
// }

KERNEL_END
