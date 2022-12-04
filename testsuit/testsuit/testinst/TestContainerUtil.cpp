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
 * Date: 2021-01-31 18:51:07
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestContainerUtil.h>


void TestContainerUtil::Run() 
{
    // std::deque<Int32 *> vsss;
    // const Int64 sizeV = vsss.size();
    // for(Int64 idx = sizeV - 1; idx >= 0 ;)
    // {
    //     auto iter = vsss.begin() + idx;
    //     vsss.erase(vsss.begin() + idx);
    // }

    // std::stack<Int32 *> stackss;

    std::map<Int32, Int32 *> mapContainer;
    mapContainer.insert(std::make_pair(1, new Int32));
    mapContainer.insert(std::make_pair(2, new Int32));
    mapContainer.insert(std::make_pair(5, new Int32));
    mapContainer.insert(std::make_pair(4, new Int32));
    KERNEL_NS::ContainerUtil::DelMapContainer(mapContainer);
    KERNEL_NS::ContainerUtil::DelContainer(mapContainer);

    std::set<Int32 *> setContainer;
    setContainer.insert(new Int32);
    setContainer.insert(new Int32);
    setContainer.insert(new Int32);
    setContainer.insert(new Int32);
    setContainer.insert(new Int32);
    KERNEL_NS::ContainerUtil::DelSetContainer(setContainer);
    KERNEL_NS::ContainerUtil::DelContainer(setContainer);

    std::list<Int32 *> listContainer;
    listContainer.push_back(new Int32);
    listContainer.push_back(new Int32);
    listContainer.push_back(new Int32);
    listContainer.push_back(new Int32);
    listContainer.push_back(new Int32);
    KERNEL_NS::ContainerUtil::DelListContainer(listContainer);
    KERNEL_NS::ContainerUtil::DelContainer(listContainer);

    std::vector<Int32 *> vecContainer;
    vecContainer.push_back(new Int32);
    vecContainer.push_back(new Int32);
    vecContainer.push_back(new Int32);
    vecContainer.push_back(new Int32);
    vecContainer.push_back(new Int32);
    KERNEL_NS::ContainerUtil::DelVectorContainer(vecContainer);
    KERNEL_NS::ContainerUtil::DelContainer(vecContainer);

    Int32 *arrContainer[]= {new Int32, new Int32, new Int32
    , new Int32, new Int32};
    KERNEL_NS::ContainerUtil::DelArray(arrContainer, 5);
    KERNEL_NS::ContainerUtil::DelContainer(arrContainer, 5);

    std::deque<Int32 *> dequeContainer;
    dequeContainer.push_back(new Int32);
    dequeContainer.push_back(new Int32);
    dequeContainer.push_back(new Int32);
    dequeContainer.push_back(new Int32);
    dequeContainer.push_back(new Int32);
    KERNEL_NS::ContainerUtil::DelDeque(dequeContainer);
    KERNEL_NS::ContainerUtil::DelContainer(dequeContainer);

    std::stack<Int32 *> stackContainer;
    stackContainer.push(new Int32);
    stackContainer.push(new Int32);
    stackContainer.push(new Int32);
    stackContainer.push(new Int32);
    stackContainer.push(new Int32);
    KERNEL_NS::ContainerUtil::DelStack(stackContainer);
    KERNEL_NS::ContainerUtil::DelContainer(stackContainer);

    std::queue<Int32 *> queueContainer;
    queueContainer.push(new Int32);
    queueContainer.push(new Int32);
    queueContainer.push(new Int32);
    queueContainer.push(new Int32);
    queueContainer.push(new Int32);
    KERNEL_NS::ContainerUtil::DelQueue(queueContainer);
    KERNEL_NS::ContainerUtil::DelContainer(queueContainer);

}
