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
 * Date: 2023-01-28 21:00:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestXlsx.h>

class MyWorksheet : public KERNEL_NS::XlsxSheet
{
    POOL_CREATE_OBJ_DEFAULT_P1(XlsxSheet, MyWorksheet);

public:
    MyWorksheet(KERNEL_NS::XlsxWorkbook<MyWorksheet> *workbook, KERNEL_NS::LibString sheetName, UInt64 sheetId, const std::vector<std::tuple<UInt64, UInt64, UInt64>> &cells)
    :KERNEL_NS::XlsxSheet(workbook, sheetName, sheetId, cells)
    {

    }

    static MyWorksheet *FactoryCreate(KERNEL_NS::XlsxWorkbook<MyWorksheet> *workbook, KERNEL_NS::LibString sheetName, UInt64 sheetId, const std::vector<std::tuple<UInt64, UInt64, UInt64>> &cells)
    {
        return MyWorksheet::NewThreadLocal_MyWorksheet(workbook, sheetName, sheetId, cells);
    }

    static void FactoryRelease(MyWorksheet *sheet)
    {
        MyWorksheet::DeleteThreadLocal_MyWorksheet(sheet);
    }
};

POOL_CREATE_OBJ_DEFAULT_IMPL(MyWorksheet);

void TestXlsx::Run()
{
    g_Log->Info(LOGFMT_NON_OBJ_TAG(TestXlsx, "CUR dir:%s"), KERNEL_NS::SystemUtil::GetCurProgRootPath().c_str());

    auto path = KERNEL_NS::SystemUtil::GetCurProgRootPath() + "config.xlsx";
    auto workbook = KERNEL_NS::XlsxWorkbook<MyWorksheet>::NewThreadLocal_XlsxWorkbook(true);
    workbook->Parse(path);
    KERNEL_NS::XlsxWorkbook<MyWorksheet>::DeleteThreadLocal_XlsxWorkbook(workbook);
}
