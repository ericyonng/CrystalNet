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
 * Date: 2024.08.01 16:34:00
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <testsuit/testinst/TestConcepts/TestConcepts.h>

template<typename T, typename U>
concept Derived = std::is_base_of<U, T>::value;

class BaseClass
{
public:
    Int32 GetValue() const
    {
        return 5;
    }
};

// 定义concept 约束: T必须是BaseClass的子类
template<typename T>
concept DerivedBaseClass = std::is_base_of_v<BaseClass, T>;

// 定义concept 约束 比DerivedBaseClass 更精准的表达式, 如果有多个版本编译期会选择更精确的版本
template<typename T>
concept DDerivedBaseClass = DerivedBaseClass<T> && requires (T h1) 
{
    // T必须满足又GetValue2接口
    h1.GetValue2();
};

template<typename T>
// T必须满足DerivedBaseClass约束
requires DerivedBaseClass<T>
void DoGetValue(const T &v)
{
    std::cout << "value:" << v.GetValue() << std::endl;
}

template<typename T>
requires DDerivedBaseClass<T>
void DoGetValue(const T &v)
{
    std::cout << "value:" << v.GetValue2() << std::endl;
}

class DerivedClass : public BaseClass
{
public:
    Int32 GetValue() const
    {
        return 6;
    }
};

class DDerivedClass : public BaseClass
{
public:
    Int32 GetValue2() const
    {
        return 7;
    }

    Int32 GetValue() const
    {
        return 8;
    }
};

class NonDerivedClass
{
public:
    Int32 GetValue() const
    {
        return 6;
    }
};

void TestConcepts::Run()
{
    DerivedClass a;
    DoGetValue(a);

    BaseClass b;
    DoGetValue(b);

    // NonDerivedClass不是继承于BaseClass编译期报错
    // NonDerivedClass c;
    // DoGetValue(c);

    // DDerivedClass既满足DerivedBaseClass又满足DDerivedBaseClass 编译期会选择更精准的 DDerivedBaseClass 约束版本
    DDerivedClass d;
    DoGetValue(d);

    // 只满足DerivedBaseClass约束, 所以编译期只选择DerivedBaseClass版本
    DerivedClass e;
    DoGetValue(e);
}