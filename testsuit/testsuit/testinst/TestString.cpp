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
 * Date: 2020-12-02 00:27:24
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestString.h>
#include <3rd/3rd.h>

class TestStringObj
{
public:
    friend KERNEL_NS::LibString &operator <<(KERNEL_NS::LibString &dest, const TestStringObj &obj)
    {
        dest << "hello test string obj";
        return dest;
    }

    KERNEL_NS::LibString ToString() const
    {
        return "TestStringObj";
    }
};

void TestString::Run()
{
    // 测试strip
    KERNEL_NS::LibString str = "123aaabbb789";
    // KERNEL_NS::LibString str = "123123aaabbb789";
    str.lstripString("123");
    str.lstripString("1234");
    str.rstripString("89");
    str.rstripString("bb");

    KERNEL_NS::LibString str2 = "123aaabb789789";
    str2.rstripString("789");
    KERNEL_NS::LibString str3 = "123aaabb789b789";
    str3.rstripString("789");

    KERNEL_NS::LibString str4 = "789789";
    str4.rstripString("789");

    KERNEL_NS::LibString str5 = "78789";
    str5.rstripString("789");

    KERNEL_NS::LibString str6 = "789789";
    str6.lstripString("789");

    KERNEL_NS::LibString str7 = "7897";
    str7.lstripString("789");

    KERNEL_NS::LibString str8 = "7897b0055789789";
    str8.stripString("789");

    std::string stringEx;
    stringEx = "hello";
    stringEx = std::string("big world");
    stringEx += std::string("5564");
    stringEx += std::string("123");

    KERNEL_NS::LibString testCut = "PlayerId=12233 TemplateId=545 Type=FsmT] dafsadf";
    auto startStr = testCut.StartCut("Type=").EndCut("]");
}


// void TestString::Run()
// {
//     KERNEL_NS::LibString str;

//     str.AppendFormat("hello world %d", 123)
//         .AppendFormat("bigger world %s", "haliluya")
//         .AppendFormat("dklakd %d %f %s %hu %x", 77, 1.5, "coder", 168, 169);

//     TestStringObj testObj;

//     str.Append(testObj, 125566, "dkafkajdklfj", true);

//     str.findreplace(KERNEL_NS::LibString("bigger"), KERNEL_NS::LibString("hello"));

//     const auto &results = str.Split("hello");
//     for(auto &piece:results)
//     {
//         std::cout <<piece << std::endl;
//     }

//     std::ifstream stream;
//     BUFFER512 utf8str = {0};
//     stream.open("./utf8.txt", std::ios_base::binary);
//     stream.read(utf8str, 512);

//     KERNEL_NS::LibString utf8LibStr;
//     utf8LibStr = utf8str;

//     // UInt64 len = utf8LibStr.length_with_utf8();
//     auto utf8Piece = utf8LibStr.substr_with_utf8(1, 2);
//     utf8Piece.GetRaw().push_back(0);
//     BUFFER512 utf82;
//     const Byte8 * ptr = utf8Piece.c_str();
//     UInt64 loopCnt = 0;
//     while (true)
//     {
//         ++loopCnt;
//         if(*ptr == 0)
//             break;
//         ++ptr;
//     }
//     KERNEL_NS::LibString::_These theseStr;
//     utf8LibStr.split_utf8_string(2, theseStr);

//     KERNEL_NS::LibString::_These scatterStr;
//     utf8LibStr.scatter_utf8_string(scatterStr, 10);

//     strcpy(utf82, utf8Piece.c_str());

//     std::cout << str << std::endl;
//     std::cout << "utf8LibStr = "<< utf8LibStr << std::endl;
//     std::cout << "utf82 = "<< utf82 << std::endl;

//     KERNEL_NS::LibString splitSrc ="kslfjalsdjf;lasdjfk dd";
//     KERNEL_NS::LibString::_These dest;
//     KERNEL_NS::StringUtil::SplitString(splitSrc, KERNEL_NS::LibString(), dest);
//     std::cout<< "split count = "<< dest.size() << std::endl;
//     for(auto &piece:dest)
//         std::cout << piece << std::endl;
    
//     // 测试appendformat
//     // 测试append()
//     // 测试findreplace
//     // 测试split
//     // 测试utf8接口
// }