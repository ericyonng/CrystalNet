<!--
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
 * @Date: 2023-09-29 14:58:47
 * @Author: Eric Yonng
 * @Description: 
 -->

# 代码分析器

#### 目的

* 实现对代码的通用分析

#### 原理介绍

引入一个概念就是代码单元，它的起始和结束使用特殊的字符比如{}, 当分析代码碰到{时创建一个CodeUnit, 并入栈，那么当前的代码单元就是CodeUnit, 当碰到}时候CodeUnit结束并弹栈，所有创建的CodeUnit会被放到字典中

实现办法

* 数据结构：
  * 语法单元堆栈: CodeUnitQueue, std::vector<KERNEL_NS::SmartPtr<CodeUnit, KERNEL_NS::AutoMethods::Release>>，
    * 作用: 通过堆栈结构, 入栈表示一个新的语法单元的分析, 弹栈表示一个语法单元的结束，通过入栈和弹栈操作，可以解决语法单元的嵌套
  * 语法单元：CodeUnit, 每个语法的最小单位，用于存储语法单元的信息，如语法关键字(class/message/sint32/int/void等常见关键字或者不同语言的关键字), 语法内容等
    * 关键字
    * 单元起始标志，例如:{
    * 单元结束标志, 例如:}
    * 语法单元是否开始
    * 语法单元是否结束
    * 语法单元名: 例如class xxx, xxx就是语法单元名
    * 语法修饰, 例如 export/ final/public 等语法修饰符
    * 文件名
    * 语法单元所在起始行
    * 域信息, 俗称命名空间等类型所在的空间是一个std::vector<KERNEL_NS::LibString>结构
  * 语法分析行为树
    * 顺序节点
    * 选择节点
    * 循环节点
    * 并行节点
    * 根节点
    * 业务节点
    * 
* 算法
  * 语法分析以文件为单位
  * 语法分析依赖语法分析行为树
  * 给语法状态机注入语法关键字, 用于识别语法关键字，域连接符等
  * 给每个语法关键字设置语法处理委托
  * 设置语法单元开启的条件, 例如识别到 class/ function/message等就是单元的开始, 单元的开始会创建一个新的单元
  * 流程
    * 识别语法单元 => 创建语法单元 => 当前新创建的语法单元会被嵌套到栈顶语法单元作为其子单元 => 新创建的语法单元入栈 => 之后扫描的内容属于当前栈顶的语法单元 => 语法单元结束 => 弹栈 => 识别语法单元 => ...
  * 所有创建的语法单元信息都会被保存到一个字典中，用于后续生成所有的类型信息或者不同语言生成对应的语言代码
  

