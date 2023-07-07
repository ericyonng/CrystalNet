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
 * Date: 2022-09-27 19:16:18
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestList.h>

template<typename ObjType, typename BuildType>
class TestLibListObjType
{
public:
    TestLibListObjType()
        :_head(NULL)
        ,_release(NULL)
    {
        auto release = [this](){
            if(_head)
                KERNEL_NS::LibList<ObjType, BuildType>::DeleteByAdapter_LibList(BuildType::V, _head);

            _head = NULL;
        };
        _release = KERNEL_CREATE_CLOSURE_DELEGATE(release, void);
    }

    ~TestLibListObjType()
    {
        if(_release)
           _release->Invoke();

        CRYSTAL_RELEASE_SAFE(_release);
    }

    KERNEL_NS::LibList<ObjType, BuildType> *_head;
    BUFFER1024 _buffer;
    KERNEL_NS::IDelegate<void> *_release;
};

void TestList::Run()
{
    // KERNEL_NS::LibList<int *> *newIntList =  KERNEL_NS::LibList<int *>::New_LibList();
    // *(newIntList->PushBack(new int)->_data) = 15;
    // *(newIntList->PushBack(new int)->_data) = 20;

    // auto iter = newIntList->PushBack(new int);
    // newIntList->Erase(iter);

    auto testObj = new TestLibListObjType<BUFFER1024, KERNEL_NS::_Build::TL>();

    BUFFER1024 obj = {'a', 'b', 0};
    testObj->_head = KERNEL_NS::LibList<BUFFER1024, KERNEL_NS::_Build::TL>::NewByAdapter_LibList(KERNEL_NS::_Build::TL::V);
    testObj->_head->PushBack(obj);
    auto iter = testObj->_head->PushBack(obj);
    testObj->_head->PushBack(obj);
    testObj->_head->Erase(iter);

    delete testObj;
}