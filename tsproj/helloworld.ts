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

var str = new String("hello world");
var number_arr:number[] = new Array(4)
number_arr[0] = 1
number_arr[1] = 3

var normal_arr:number[] = [1, 2]
// 数组解构
var [xx,yy] = normal_arr
var [xx,yy] = number_arr
console.log("xx:" + xx)
console.log("yy:" + yy)

// 字典类型
let dict = new Map()
dict.set("nima", 1)
dict.set("op", 2)
console.log(dict.get("nima"))

for(var k of dict.keys()){
    console.log(k)
}

for(var [k, v] of dict){
    console.log(k, "=>", v)
}

// 元组
var tup_arr = ["hello", 12, "begin"]
tup_arr.push(12)
tup_arr.push("be")
tup_arr[0] = 12
for(v of tup_arr)
{
    console.log(v)
}

// 联合类型 把值的类型限定了
var union_val :number|string
union_val = 12
console.log("数字为:", union_val)
union_val = "beginsdfa"
console.log("字符串：", union_val)

// 如果赋值其它类型就会报错：
// union_val = true

// 联合类型数组(要么是数值数组, 要么是字符串数组，元组就报错)
var union_arr: number[]|string[]
// union_arr = ["dafasf", 123]
union_arr = ["dafasf"]
union_arr = [1233]

// 接口类型
interface IApi {
    nameOfApi:string,
    // 方法是一个lambda
    handle: ()=>string
}

// 实现一个接口就是创建一个对象出来
var customVar:IApi = {
    nameOfApi:"custom",
    handle: ():string => "hello custom api"
}

console.log(customVar.nameOfApi, customVar.handle())

// 联合类型下的接口
interface IUnionApi{
    nameOfApi:string,

    // 意思是handle既可以是字符串数组, 也可以是字符串, 还可以是由lambda返回的字符串
    handle:string[]|string|(()=>string)
}

var unonCustomVar:IUnionApi = {
    nameOfApi:"union custom",
    handle:()=>"from lambda"
}

var unonCustomVar2:IUnionApi = {
    nameOfApi:"union custom",
    handle:["from string ", "array"]
}

var unonCustomVar3:IUnionApi = {
    nameOfApi:"union custom",
    handle:"from string "
}

var fn:any = unonCustomVar.handle
console.log(unonCustomVar.nameOfApi, fn())
console.log(unonCustomVar2.nameOfApi, unonCustomVar2.handle)
console.log(unonCustomVar3.nameOfApi, unonCustomVar3.handle)

// 接口和数组
interface interfaceAndArr {
    [index:number]:string
}

// 正确下标索引是number，value是string
var list2:interfaceAndArr = ["hello", "diff"]
// 错误:元素2不是string
// var list2:interfaceAndArr = ["hello", 2]

// 接口继承
interface testForJicheng{
    age:number
}

// extentds可以支持多继承 extends a,b,c...
interface SunOfTest extends testForJicheng{
    sunName:string
}

// 派生接口的实例 sun 接口需要使用<interfacename>
var sun = <SunOfTest>{}
// 此时sun中的成员都是undefined
console.log(sun)
sun.age = 15
sun.sunName = "beslfkja"
console.log(sun)

// 类
class Person {
    // 字段
    _personName:string;

    constructor(personName:string){
        this._personName = personName
    }

    Disp():void{
        console.log("name:", this._personName)
    }
}

var my_person = new Person("first person")
my_person.Disp()

// 类的继承
class Person2{
    // 字段
    _sex:number
    _name:string

    constructor(){
        this._sex = 1
        this._name = "dasfdlkas"
    }
}

class PersonSun extends Person2{
    _my_style:number

    constructor()
    {
        this._my_style = 1
    }
}