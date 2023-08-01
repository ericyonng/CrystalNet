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
 * Date: 2023-08-01 23:54:00
 * Author: Eric Yonng
 * Description: 
*/

#include <service/common/macro.h>
#include <service_common/ServiceCommon.h>

SERVICE_BEGIN

class VarErrInfo
{
    POOL_CREATE_OBJ_DEFAULT(VarErrInfo);

public:
    VarErrInfo();
    ~VarErrInfo();

public:
    static VarErrInfo *Create();
    void Release();

    KERNEL_NS::LibString ToString() const;

    Int32 _errCode;
    UInt64 _seqId;
};

ALWAYS_INLINE VarErrInfo::VarErrInfo()
:_errCode(Status::Success)
,_seqId(0)
{

}

ALWAYS_INLINE VarErrInfo::~VarErrInfo()
{

}

ALWAYS_INLINE VarErrInfo *VarErrInfo::Create()
{
    return VarErrInfo::NewThreadLocal_VarErrInfo();
}

ALWAYS_INLINE void VarErrInfo::Release()
{
    VarErrInfo::DeleteThreadLocal_VarErrInfo(this);
}

ALWAYS_INLINE KERNEL_NS::LibString VarErrInfo::ToString() const
{
    return KERNEL_NS::LibString().AppendFormat("seq id:%llu, errCode:%d"
            , _seqId, _errCode);
}


struct VarErrInfoList
{
    POOL_CREATE_OBJ_DEFAULT(VarErrInfoList);

public:
    VarErrInfoList();
    ~VarErrInfoList();

public:
    static VarErrInfoList *Create();
    void Release();

    KERNEL_NS::LibString ToString() const;

    bool IsEmpty() const;

    std::vector<VarErrInfo *> _errList;
};

ALWAYS_INLINE VarErrInfoList::VarErrInfoList()
{

}

ALWAYS_INLINE VarErrInfoList::~VarErrInfoList()
{
    KERNEL_NS::ContainerUtil::DelContainer2(_errList);
}

ALWAYS_INLINE VarErrInfoList *VarErrInfoList::Create()
{
    return VarErrInfoList::NewThreadLocal_VarErrInfoList();
}

ALWAYS_INLINE void VarErrInfoList::Release()
{
    VarErrInfoList::DeleteThreadLocal_VarErrInfoList(this);
}

ALWAYS_INLINE KERNEL_NS::LibString VarErrInfoList::ToString() const
{
    std::vector<KERNEL_NS::LibString> errs;
    for(auto errInfo : _errList)
        errs.push_back(errInfo->ToString());

    return KERNEL_NS::StringUtil::ToString(errs, "\n");
}

ALWAYS_INLINE bool VarErrInfoList::IsEmpty() const
{
    return _errList.empty();
}

SERVICE_END
