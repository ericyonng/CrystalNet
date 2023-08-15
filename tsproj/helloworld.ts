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
 * Date: 2023-08-15 15:00:46
 * Author: Eric Yonng
 * Description: 
*/

var message:string = "Hello World" 
console.log(message)

class MyRobot{
    name():string{
        return "Robot"
    }
}

var robot = new MyRobot();
console.log(robot.name())

// var 变量名: 数据类型 = 值
// 未初始化的变量值是:undefined
let x: any = 1;
x = 'hello world'

// 使用类型推断
var num = 2
console.log(num)

// 数据类型:any, number, string, boolean, number[], Array<number>
var numArr: number[] = [1, 2];
console.log(numArr)

// 元组，数组各个元素不必相同
let metas: [string, number] = ['hello', 2]
console.log(metas)

// 枚举
enum Color {Red, Green, Blue};
let c: Color = Color.Blue;
console.log(c)
if (num == 2){
    console.log('num is: ' + num)
}

switch(num)
{
    case 2:{
        console.log("num is 2")
        break
    }
    default:{
        console.log('default:' + num)
    }
}

for(var i = 0; i < 10; ++i)
{
    console.log('i:' + i)
}

// idx是下标索引
var arr: number[] = [1, 2, 3]
for(var idx in arr)
{
    console.log('loop i:' + arr[idx])
}

for(var elem of arr)
{
    console.log('loop elem:' + elem)
}

arr.forEach((val, idx, array)=>{
    console.log(val)
})

arr.every((val, idx, array)=>{
    console.log('every:' + val)
    // continue loop
    return true
})

arr.every((val, idx, array)=>{
    console.log('every 2:' + val)
    // quit loop
    return false
})

// never:是其它类型（包括 null 和 undefined）的子类型，代表从不会出现的值

var iter: number = 1;
while(true)
{
    if(arr[iter] == 3)
    {
        console.log('break')
        break
    }

    ++iter;
}

function AnyFunc(){
    console.log('do anything.')
}

AnyFunc()

// function 函数名(参数列表): 返回值类型 { code; }
function RetStr(defname:string, defname2:string):string {
    return defname + defname2
}

// 可选参数必须在最后几个, 如果param2没有传值, 那么param2就是undefined
function OptFunc(param1:string, param2?:string):string{
    if(param2 == undefined)
    {
        return param1 + 'undefined param2'
    }

    return param1 + param2;
}

// 默认参数
function DefParamFunc(param1:string, param2:number = 1):string{
    return param1 + ' ' + param2
}

// 剩余参数, 传入的不知道多少个参数
function LeftParamFunc(param1:string, ...restOfString: string[]) : string{
    return param1 + restOfString.join(' ')
}
console.log(RetStr('hello', 'def func'))
console.log(OptFunc('test opt param'))
console.log(OptFunc('test opt param', '2'))
console.log(DefParamFunc('test default param'))
console.log(LeftParamFunc('test left param:'), "hello", 2, 'baby')

// 匿名函数:没有名字，其他和函数一样
var nimingFunc = function():string{return 'test niming func'}
console.log(nimingFunc())

// lambda函数 又称箭头函数
var lamb = (a:number) =>{
    return a;
}
console.log('test lambda:' + lamb(5))

// 重载函数
function DefOverride1(a:number):string;
function DefOverride1(a:string, b:number):string;


function DefOverride1(a:any, b?:any):string{
    return '' + a + (b== undefined) ? "None" : b
}

console.log('test override:' + DefOverride1(1))
console.log('test override:' + DefOverride1("test override", 100))

