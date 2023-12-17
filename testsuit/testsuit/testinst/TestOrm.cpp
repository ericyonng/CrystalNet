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
 * Date: 2023-12-17 14:45:19
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestOrm.h>
#include <protocols/orm_out/AllOrmDatas.h>

void TestOrm::Run()
{
    KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::IOrmDataFactory, KERNEL_NS::AutoDelMethods::Release> ormFactory = SERVICE_COMMON_NS::TestCustomDataOrmDataFactory::NewThreadLocal_TestCustomDataOrmDataFactory();

    KERNEL_NS::SmartPtr<SERVICE_COMMON_NS::IOrmData, KERNEL_NS::AutoDelMethods::Release> ormData = ormFactory->Create();

    ormData->SetMaskDirtyCallback([](SERVICE_COMMON_NS::IOrmData *ptr){
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "TestCustomDataOrmData is dirty pb data:%s, orm id:%lld")
        , ptr->ToJsonString().c_str(), ptr->GetOrmId());
    });

    auto orm = ormData->CastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>();
    orm->set_testint(100);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "orm int:%d"), orm->testint());
    orm->clear_testint();
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "orm int:%d after clear"), orm->testint());
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "orm :%s"), orm->ToJsonString().c_str());

    ormFactory = SERVICE_COMMON_NS::TestOrmOrmDataFactory::NewThreadLocal_TestOrmOrmDataFactory();
    ormData = ormFactory->Create();
    ormData->SetMaskDirtyCallback([](SERVICE_COMMON_NS::IOrmData *ptr){
        g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "TestOrmOrmData is dirty pb data:%s, orm id:%lld")
        , ptr->ToJsonString().c_str(), ptr->GetOrmId());
    });

    auto testOrmOrmData = ormData->CastTo<SERVICE_COMMON_NS::TestOrmOrmData>();
    testOrmOrmData->set_testint(120);
    testOrmOrmData->clear_testint();
    testOrmOrmData->set_teststring("hello test string");
    auto testString = testOrmOrmData->mutable_teststring();
    *testString = "hello test string 2";

    testOrmOrmData->add_testintarray(100);
    testOrmOrmData->add_testintarray(200);
    testOrmOrmData->add_testintarray(300);
    testOrmOrmData->add_testintarray(400);
    testOrmOrmData->add_testintarray(500);
    testOrmOrmData->DeleteArray_testintarray(1, 1);
    testOrmOrmData->DeleteArray_testintarray(1, 2);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "testintarray size:%d"), testOrmOrmData->testintarray_size());

    testOrmOrmData->DeleteArray_testintarray(0, 2);
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "testintarray size:%d"), testOrmOrmData->testintarray_size());

    testOrmOrmData->add_teststringarray("test string array hello world 0");
    testOrmOrmData->add_teststringarray("test string array hello world 1");
    testOrmOrmData->add_teststringarray("test string array hello world 2");
    testOrmOrmData->add_teststringarray("test string array hello world 3");
    testOrmOrmData->add_teststringarray("test string array hello world 4");

    *testOrmOrmData->mutable_teststringarray(1) = "test string array hello world replace";
    testOrmOrmData->set_teststringarray(2, "set teststring array 2");
    testOrmOrmData->DeleteArray_teststringarray(3);
    testOrmOrmData->mutable_testcustom()->set_testint(99);
    testOrmOrmData->clear_testcustom();
    testOrmOrmData->mutable_testcustom()->set_testint(1099);

    testOrmOrmData->add_testcustomarray()->set_testint(9966);
    testOrmOrmData->add_testcustomarray()->set_testint(99661);
    testOrmOrmData->add_testcustomarray()->set_testint(99662);
    testOrmOrmData->add_testcustomarray()->set_testint(99663);
    testOrmOrmData->mutable_testcustomarray(0)->set_testint(1669);
    testOrmOrmData->DeleteArray_testcustomarray(2, 1);
    // testOrmOrmData->clear_testcustomarray();

    testOrmOrmData->set_testoneofint(9966);
    if(testOrmOrmData->has_testoneofint())
        testOrmOrmData->set_testoneofint(65);
    
    testOrmOrmData->set_testoneofstring("dafas");

    *testOrmOrmData->mutable_testoneofstring() = "aaaaa";

    testOrmOrmData->mutable_testoneofcustom()->set_testint(1220563);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "hello world testOrmOrmData :%s"), testOrmOrmData->ToJsonString().c_str());
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "hello world testOrmOrmData :%s"), testOrmOrmData->ToJsonString().c_str());


    KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::CustomDelete> stream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
    stream.SetClosureDelegate([](void *p){
        KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(KERNEL_NS::KernelCastTo<KERNEL_NS::LibStreamTL>(p));
    });
    KERNEL_NS::SmartPtr<KERNEL_NS::LibStreamTL, KERNEL_NS::AutoDelMethods::CustomDelete> pbStream = KERNEL_NS::LibStreamTL::NewThreadLocal_LibStream();
    pbStream.SetClosureDelegate([](void *p){
        KERNEL_NS::LibStreamTL::DeleteThreadLocal_LibStream(KERNEL_NS::KernelCastTo<KERNEL_NS::LibStreamTL>(p));
    });

    testOrmOrmData->Encode(*stream);
    testOrmOrmData->PbEncode(*pbStream);

    testOrmOrmData->AfterPurge();

    auto testOrmOrmData2 = ormFactory->Create();
    testOrmOrmData2->Decode(*stream);

    auto testOrm3 = ormFactory->Create();
    testOrm3->PbDecode(*pbStream);

    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "testOrmOrmData2:%s"), testOrmOrmData2->ToJsonString().c_str());
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestOrm, "testOrm3:%s"), testOrm3->ToJsonString().c_str());

    {// 测试拷贝构造(pb,和同类型)/移动构造, 赋值(pb, 和同类型), 移动赋值 
        auto testOrmData6 = SERVICE_COMMON_NS::TestOrmOrmData::NewThreadLocal_TestOrmOrmData(*testOrmOrmData);
        auto testOrmData7 = SERVICE_COMMON_NS::TestOrmOrmData::NewThreadLocal_TestOrmOrmData(*testOrmOrmData->GetPbRawData());
        auto testOrmData8 = SERVICE_COMMON_NS::TestOrmOrmData::NewThreadLocal_TestOrmOrmData(std::move(*testOrmData7));

        *testOrmData8 = *testOrmOrmData->GetPbRawData();
        *testOrmData8 = *testOrmOrmData;
        *testOrmData8 = std::move(*testOrmData6);

        testOrmData8->Clear();
    }
}
