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
 * Date: 2021-01-11 02:01:35
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestTemplateObjPool.h>

template<typename ObjType, typename ObjType2>
class TestTemplateObj
{
    // POOL_CREATE_TEMPLATE_OBJ_DEFAULT(TestTemplateObj, ObjType, ObjType2);

public:
    std::vector<ObjType2> _eles;
};

// template<typename ObjType, typename ObjType2>
// POOL_CREATE_TEMPLATE_ANCESTOR_IMPL(10, TestTemplateObj, ObjType, ObjType2);

template <typename T1, typename T2, typename T3>
class TestTemplateDeriveObj : public TestTemplateObj<T1, T2>
{
public:
    // POOL_CREATE_TEMPLATE_OBJ_P1(true, 10, TestTemplateObj, TestTemplateDeriveObj, T1, T2, T3)

public:
    std::vector<T3> _eles;
};

// template <typename T1, typename T2, typename T3>
// POOL_CREATE_TEMPLATE_DERIVE_IMPL(10, TestTemplateDeriveObj, T1, T2, T3);

template <typename T1, typename T2, typename T3>
class Testtemddj : public TestTemplateDeriveObj<T1, T2, T3>
{
public:
    // POOL_CREATE_TEMPLATE_OBJ_P1(true, 10, TestTemplateDeriveObj, Testtemddj, T1, T2, T3)

public:
    std::vector<T3> _errd;
};

// template<typename T1, typename T2, typename T3>
// POOL_CREATE_TEMPLATE_DERIVE_IMPL(10, Testtemddj, T1, T2, T3);


class TestStaticObj
{
public:
    explicit TestStaticObj( ) {}
    
    void Detect() {}

    bool sssss;
};

class testddklsjdfldk;

class testStatic
{
public:
    testStatic()
    {
    }

    void Dddd()
    {
        hi.Detect();

    };
    static testddklsjdfldk dkk;
    static TestStaticObj hi;
};

TestStaticObj testStatic::hi;


class testddklsjdfldk
{
public:
    testddklsjdfldk()
    {
        testStatic::hi.Detect();
    }
};

testddklsjdfldk testStatic::dkk;


//TestStaticObj testStatic::hi;


// template <typename T1, typename T2, typename T3>
// POOL_CREATE_TEMPLATE_DERIVE_DETECT_FUNC_NAME(TestTemplateDeriveObj, T1, T2, T3)
// POOL_CREATE_TEMPLATE_DERIVE_DETECT_FUNC_NAME_IMPL(TestTemplateDeriveObj, T1, T2)

void TestTemplateObjPool::Run()
{
    // TestTemplateObj<std::string, Int32> testTemplateObjPool;
   // TestTemplateObj<std::string, Int32>::_TestTemplateObj_ObjPoolDetect;
    // testTemplateObjPool.AddRef_TestTemplateObj();

    // TestTemplateDeriveObj<std::string, Int32, std::string> *deriveObj = new TestTemplateDeriveObj<std::string, Int32, std::string>;
    // TestTemplateDeriveObj<std::string, Int32, std::string> deriveObj;
     //deriveObj._TestTemplateDeriveObjobjAlloctor;
    //deriveObj._TestTemplateDeriveObj_ObjPoolDetect.Detect(1);
    //deriveObj._TestTemplateDeriveObj_ObjPoolDetect.Detect();

    //  std::string name = "hello dk?|dkdk\t(dk{";
    //  std::string pattern = "[`,#'$%%;:\"?|/\t{}\r\n]";
    //  std::regex express(pattern);
    //  std::string result;
    //  result.resize(name.size());
    //  *std::regex_replace(result.begin(), name.begin(), name.end(), express, "") = '\0';

    // testStatic dddd;
    // Testtemddj<Int32, Int32, Int32> *dddff = new Testtemddj<Int32, Int32, Int32>;
    //dddff->TesttemddjobjpoolDetect();
    KERNEL_NS::AlloctorInfoCollector collector;
   // dddff->ObjPoolCollect_Testtemddj(collector);
   // dddff->ObjPoolCollectThreadLocal_Testtemddj(collector);
   // TestTemplateObj<Int32, Int32>::NewByAdapter_TestTemplateObj(KERNEL_NS::_Build::MT::V);
   // std::cout << "GetAlloctor__TesttemddjobjAlloctor to string" << dddff->GetAlloctor__TesttemddjobjAlloctor().ToString().c_str() << std::endl;
   std::cout << "collector info :" << collector.Result() << std::endl;
   // deriveObj.TestTemplateDeriveObjTemplatePoolDetect();
}