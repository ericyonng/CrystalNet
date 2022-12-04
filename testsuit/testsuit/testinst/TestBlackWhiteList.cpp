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
 * Date: 2022-09-03 20:33:22
 * Author: Eric Yonng
 * Description: 
*/
#include <pch.h>
#include <testsuit/testinst/TestBlackWhiteList.h>

void TestBlackWhiteList::Run()
{
    KERNEL_NS::BlackWhiteList<KERNEL_NS::LibString> blackWhiteList;

    // 1.测试黑名单，白名单，允许未知通过模式:黑名单拦截，白名单放行，其他未知的也放行
    {
        UInt32 flags = KERNEL_NS::BlackWhiteFlag::CheckBlackFlag | KERNEL_NS::BlackWhiteFlag::CheckWhiteFlag | KERNEL_NS::BlackWhiteFlag::AllowUnknownFlag;
        blackWhiteList.SetMode(flags);

        // lili 黑名单, lilei 白名单, liming未知 预期效果: lili fail, lilei suc, liming suc
        blackWhiteList.PushBlack("lili");
        blackWhiteList.PushWhite("lilei");
        if(blackWhiteList.Check("lili"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked fail."));
        }

        if(blackWhiteList.Check("lilei"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked fail."));
        }

        if(blackWhiteList.Check("liming"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked fail."));
        }
    }

    // 2.测试黑名单:黑名单拦截，其他放行
    {
        UInt32 flags = KERNEL_NS::BlackWhiteFlag::CheckBlackFlag;
        blackWhiteList.SetMode(flags);

        // lili 黑名单, lilei 白名单, liming未知 预期效果: lili fail, lilei suc, liming suc
        blackWhiteList.PushBlack("lili");
        blackWhiteList.PushWhite("lilei");
        if(blackWhiteList.Check("lili"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked fail."));
        }

        if(blackWhiteList.Check("lilei"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked fail."));
        }

        if(blackWhiteList.Check("liming"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked fail."));
        }
    }

    // 3.测试白名单:白名单放行，其他拦截
    {
        UInt32 flags = KERNEL_NS::BlackWhiteFlag::CheckWhiteFlag;
        blackWhiteList.SetMode(flags);
        // lili 黑名单, lilei 白名单, liming未知 预期效果: lili fail, lilei suc, liming fail
        blackWhiteList.PushBlack("lili");
        blackWhiteList.PushWhite("lilei");
        if(blackWhiteList.Check("lili"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked fail."));
        }

        if(blackWhiteList.Check("lilei"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked fail."));
        }

        if(blackWhiteList.Check("liming"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked fail."));
        }
    }

    // 4.测试黑名单拦截，白名单放行，其他未知也拦截
    {
        UInt32 flags = KERNEL_NS::BlackWhiteFlag::CheckBlackFlag | KERNEL_NS::BlackWhiteFlag::CheckWhiteFlag;
        blackWhiteList.SetMode(flags);

        // lili 黑名单, lilei 白名单, liming未知 预期效果: lili fail, lilei suc, liming fail
        blackWhiteList.PushBlack("lili");
        blackWhiteList.PushWhite("lilei");
        if(blackWhiteList.Check("lili"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lili is checked fail."));
        }

        if(blackWhiteList.Check("lilei"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "lilei is checked fail."));
        }

        if(blackWhiteList.Check("liming"))
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked suc."));
        }
        else
        {
            g_Log->Info(LOGFMT_NON_OBJ_TAG(TestBlackWhiteList, "liming is checked fail."));
        }
    }
}