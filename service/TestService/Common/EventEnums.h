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
 * Date: 2023-01-02 16:18:00
 * Author: Eric Yonng
 * Description: TestService 事件集开始
*/

#pragma once

#include <service/common/common.h>

namespace EventEnums
{
    enum 
    {
        TEST_SERVICE_EVENT_BEGIN = EventEnums::EVENT_COMMON_END + 1,


        USER_CREATED,                   // user创建事件
                                        /* user创建事件
                                        * Attention: 
                                        * @param(USER_OBJ):IUser * 用户对象
                                        */

        USER_LOGIN,                     // 用户登录事件
                                        /* 用户登录事件
                                        * Attention: 
                                        * @param(USER_OBJ):IUser * 用户对象
                                        */

        USER_WILL_LOGOUT,              // 用户登出事件
                                        /* 用户登出事件
                                        * Attention: 
                                        * @param(USER_OBJ):IUser * 用户对象
                                        */


        USER_WILL_REMOVE,               // 用户移除事件
                                        /* 用户登录事件
                                        * Attention: 
                                        * @param(USER_ID): userId 
                                        */

        DB_LOADED_FINISH_ON_STARTUP,         // db加载完毕事件

        REMOVE_LIBRARY_MEMBER,          // 移除成员事件
                                        /* 用户登录事件
                                        * Attention: 
                                        * @param(USER_ID): userid
                                        */

        JOIN_LIBRARY_MEMBER,            // 加入图书馆事件
                                        /* 加入图书馆事件
                                        * Attention: 
                                        * @param(USER_ID): userid
                                        * @param(LIBRARY_ID): 图书馆id
                                        */
        USER_OBJ_CREATED,               // 用户对象创建事件
                                        /* 用户对象创建事件
                                        * Attention: 
                                        * @param(USER_OBJ): 用户对象
                                        */
        LOGIN_TOKEN_CHANGED,            // 用户登录token变更
                                        /* 用户对象创建事件
                                        * Attention: 
                                        * @param(USER_OBJ): 用户对象
                                        */

        CLIENT_USER_LOGIN_FINISH,       // 客户端登陆结束
                                        /* 客户端登陆结束
                                        * Attention: 
                                        * @param(USER_OBJ): 用户对象
                                        */

        SERVICE_FRAME_TICK,             // 服务帧结束事件
    };
}