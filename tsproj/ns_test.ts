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
 * Date: 2023-08-16 13:32:46
 * Author: Eric Yonng
 * Description: 
*/

import { IShape } from "./shape";

// 命名空间举例

// 要外部可用需要在命名空间中导出接口或者类型 export
export namespace TestNs{
    export interface IBasePoint{
        // 接口disp
        Disp:()=>string,
    }
    export class Point3D{}

    // IBasePoint的工厂函数
    export function PointFactory(point_type:number):any{
        if(point_type == 1)
            return new VectorPoint();
    }

    // shape工厂
    export function ShapeFactory():IShape{
        return new MyShape();
    }
}

class VectorPoint implements TestNs.IBasePoint{
    // 实现接口
    Disp: () => "VectorPoint";
}

class MyShape implements IShape{
    draw() {
        console.log('myshape draw')
    }
}

