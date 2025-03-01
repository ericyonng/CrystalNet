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
* Author: Eric Yonng
* Description: Generated By protogentool, Dont Modify This File!!!
*/


#include "pch.h"
#include <protocols/cplusplus/library/com_book.pb.h>
#include <protocols/orm_out/BookInfoOrmData.h>
#include <protocols/orm_out/BookVariantInfoOrmData.h>
#include <protocols/cplusplus/library/com_book.pb.h>
#include <protocols/orm_out/SnapshotClientInfoOrmData.h>
#include <protocols/cplusplus/library/com_book.pb.h>

SERVICE_COMMON_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(BookInfoOrmData);

BookInfoOrmData::BookInfoOrmData()
:_ormRawPbData(new ::CRYSTAL_NET::service::BookInfo)
{
}

BookInfoOrmData::BookInfoOrmData(::CRYSTAL_NET::service::BookInfo *pb)
:_ormRawPbData(NULL)
{
    AttachPb(pb);
}

BookInfoOrmData::BookInfoOrmData(const BookInfoOrmData &other)
:IOrmData(reinterpret_cast<const IOrmData &>(other))
,_ormRawPbData(other._ormRawPbData ? new ::CRYSTAL_NET::service::BookInfo(*other._ormRawPbData) : NULL)
{
    SetAttachPbFlag(false);
    if(_variantinfo)
        _variantinfo.Release();

    if(_ormRawPbData->has_variantinfo())
    {
        _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());

        _variantinfo.SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
        }) ;

        _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

    }

    {
        const auto count = _ormRawPbData->snapshotpreivewinfolist_size();

        _snapshotpreivewinfolist.resize(count);

        for(Int32 idx = 0; idx < count; ++idx)
        {
        _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));

        _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
        }) ;

        _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

        }
    }


}

BookInfoOrmData::BookInfoOrmData(BookInfoOrmData &&other)
:IOrmData(std::forward<IOrmData>(other))
,_ormRawPbData(other._ormRawPbData)
{
    other._ormRawPbData = NULL;
    _variantinfo = std::move(other._variantinfo);

    _snapshotpreivewinfolist = std::move(other._snapshotpreivewinfolist);

}

BookInfoOrmData::BookInfoOrmData(const ::CRYSTAL_NET::service::BookInfo &pb)
:_ormRawPbData(new ::CRYSTAL_NET::service::BookInfo(pb))
{
    if(_variantinfo)
        _variantinfo.Release();

    if(_ormRawPbData->has_variantinfo())
    {
        _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());

        _variantinfo.SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
        }) ;

        _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

    }

    {
        const auto count = _ormRawPbData->snapshotpreivewinfolist_size();

        _snapshotpreivewinfolist.resize(count);

        for(Int32 idx = 0; idx < count; ++idx)
        {
        _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));

        _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
        }) ;

        _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

        }
    }



}

BookInfoOrmData::~BookInfoOrmData()
{
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);
}

void BookInfoOrmData::Release()
{
    BookInfoOrmData::DeleteThreadLocal_BookInfoOrmData(this);
}

BookInfoOrmData &BookInfoOrmData::operator =(const ::CRYSTAL_NET::service::BookInfo &pb)
{
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);

    SetAttachPbFlag(false);
    _ormRawPbData = new ::CRYSTAL_NET::service::BookInfo(pb);
    if(_variantinfo)
        _variantinfo.Release();

    if(_ormRawPbData->has_variantinfo())
    {
        _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());

        _variantinfo.SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
        }) ;

        _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

    }

    {
        const auto count = _ormRawPbData->snapshotpreivewinfolist_size();

        _snapshotpreivewinfolist.resize(count);

        for(Int32 idx = 0; idx < count; ++idx)
        {
        _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));

        _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
        }) ;

        _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

        }
    }


    _MaskDirty(true);
    return *this;
}

BookInfoOrmData &BookInfoOrmData::operator =(const BookInfoOrmData &other)
{
    if(this == &other)
        return *this;

    IOrmData::operator =(reinterpret_cast<const IOrmData &>(other));
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);

    _ormRawPbData = NULL;
    SetAttachPbFlag(false);
    if(other._ormRawPbData)
        _ormRawPbData = new ::CRYSTAL_NET::service::BookInfo(*other._ormRawPbData);
    if(_ormRawPbData)
    {
            if(_variantinfo)
                _variantinfo.Release();
        
            if(_ormRawPbData->has_variantinfo())
            {
                _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());
        
                _variantinfo.SetClosureDelegate([](void *ptr){
                    SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
                }) ;
        
                _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
                    _MaskDirty(true);
                }) ;
        
            }
        
            {
                const auto count = _ormRawPbData->snapshotpreivewinfolist_size();
        
                _snapshotpreivewinfolist.resize(count);
        
                for(Int32 idx = 0; idx < count; ++idx)
                {
                _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));
        
                _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
                    SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
                }) ;
        
                _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                    _MaskDirty(true);
                }) ;
        
                }
            }
        
        
    }
    _MaskDirty(true);

    return *this;
}

BookInfoOrmData &BookInfoOrmData::operator =(BookInfoOrmData &&other)
{
    if(this == &other)
        return *this;

    IOrmData::operator =(std::forward<IOrmData>(other));
    _ormRawPbData = other._ormRawPbData;
    other._ormRawPbData = NULL;

    _variantinfo = std::move(other._variantinfo);

    _snapshotpreivewinfolist = std::move(other._snapshotpreivewinfolist);


    return *this;
}

void BookInfoOrmData::Clear()
{
    _variantinfo.Release();

    _snapshotpreivewinfolist.clear();


    if(_ormRawPbData)
        _ormRawPbData->Clear();

    _MaskDirty(true);
}

void BookInfoOrmData::_AttachPb(void *pb)
{
    if(LIKELY(!IsAttachPb()))
        CRYSTAL_RELEASE_SAFE(_ormRawPbData);

    _ormRawPbData = reinterpret_cast<::CRYSTAL_NET::service::BookInfo *>(pb);

    if(_variantinfo)
        _variantinfo.Release();

    if(_ormRawPbData->has_variantinfo())
    {
        _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());

        _variantinfo.SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
        }) ;

        _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

    }

    {
        const auto count = _ormRawPbData->snapshotpreivewinfolist_size();

        _snapshotpreivewinfolist.resize(count);

        for(Int32 idx = 0; idx < count; ++idx)
        {
        _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));

        _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
        }) ;

        _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

        }
    }



}

KERNEL_NS::LibString BookInfoOrmData::ToJsonString() const
{
    return _ormRawPbData->ToJsonString();
}

bool BookInfoOrmData::ToJsonString(std::string *data) const
{
    return _ormRawPbData->ToJsonString(data);
}

bool BookInfoOrmData::FromJsonString(const Byte8 *data, size_t len)
{
    const auto ret = _ormRawPbData->FromJsonString(data, len);
    if(ret)
    {
        if(_variantinfo)
            _variantinfo.Release();
    
        if(_ormRawPbData->has_variantinfo())
        {
            _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());
    
            _variantinfo.SetClosureDelegate([](void *ptr){
                SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
            }) ;
    
            _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
                _MaskDirty(true);
            }) ;
    
        }
    
        {
            const auto count = _ormRawPbData->snapshotpreivewinfolist_size();
    
            _snapshotpreivewinfolist.resize(count);
    
            for(Int32 idx = 0; idx < count; ++idx)
            {
            _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));
    
            _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
                SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
            }) ;
    
            _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                _MaskDirty(true);
            }) ;
    
            }
        }
    
    
    }

    return ret;
}

const ::CRYSTAL_NET::service::BookInfo *BookInfoOrmData::GetPbRawData() const
{
    return _ormRawPbData;
}

void BookInfoOrmData::clear_id()
{
    _ormRawPbData->clear_id();
    _MaskDirty(true);
}

uint64_t BookInfoOrmData::id() const
{
    return _ormRawPbData->id();
}

void BookInfoOrmData::set_id(uint64_t value)
{
    _ormRawPbData->set_id(value);
    _MaskDirty(true);
}

void BookInfoOrmData::clear_booktype()
{
    _ormRawPbData->clear_booktype();
    _MaskDirty(true);
}

int32_t BookInfoOrmData::booktype() const
{
    return _ormRawPbData->booktype();
}

void BookInfoOrmData::set_booktype(int32_t value)
{
    _ormRawPbData->set_booktype(value);
    _MaskDirty(true);
}

void BookInfoOrmData::clear_bookname()
{
    _ormRawPbData->clear_bookname();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::bookname() const
{
    return _ormRawPbData->bookname();
}

void BookInfoOrmData::set_bookname(const std::string &value)
{
    _ormRawPbData->set_bookname(value);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::mutable_bookname()
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_bookname();
}

void BookInfoOrmData::clear_isbncode()
{
    _ormRawPbData->clear_isbncode();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::isbncode() const
{
    return _ormRawPbData->isbncode();
}

void BookInfoOrmData::set_isbncode(const std::string &value)
{
    _ormRawPbData->set_isbncode(value);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::mutable_isbncode()
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_isbncode();
}

void BookInfoOrmData::clear_bookcoverimage()
{
    _ormRawPbData->clear_bookcoverimage();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::bookcoverimage() const
{
    return _ormRawPbData->bookcoverimage();
}

void BookInfoOrmData::set_bookcoverimage(const std::string &value)
{
    _ormRawPbData->set_bookcoverimage(value);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::mutable_bookcoverimage()
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_bookcoverimage();
}

void BookInfoOrmData::clear_isonshelves()
{
    _ormRawPbData->clear_isonshelves();
    _MaskDirty(true);
}

int32_t BookInfoOrmData::isonshelves() const
{
    return _ormRawPbData->isonshelves();
}

void BookInfoOrmData::set_isonshelves(int32_t value)
{
    _ormRawPbData->set_isonshelves(value);
    _MaskDirty(true);
}

KERNEL_NS::SmartPtr<BookVariantInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &BookInfoOrmData::mutable_variantinfo()
{
    if(LIKELY(_variantinfo))
        return _variantinfo;

    _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());

    _variantinfo.SetClosureDelegate([](void *ptr){
        SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
    }) ;

    _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
       _MaskDirty(true);
    }) ;

    return _variantinfo;
}

const ::CRYSTAL_NET::service::BookVariantInfo &BookInfoOrmData::variantinfo() const
{
    return _ormRawPbData->variantinfo();
}

const KERNEL_NS::SmartPtr<BookVariantInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &BookInfoOrmData::variantinfo_OrmData() const
{
    return _variantinfo;
}

bool BookInfoOrmData::has_variantinfo() const
{
    return _ormRawPbData->has_variantinfo();
}

void BookInfoOrmData::clear_variantinfo()
{
    if(_variantinfo)
        _variantinfo.Release();

    _ormRawPbData->clear_variantinfo();
    _MaskDirty(true);
}

void BookInfoOrmData::clear_borrowedcount()
{
    _ormRawPbData->clear_borrowedcount();
    _MaskDirty(true);
}

uint64_t BookInfoOrmData::borrowedcount() const
{
    return _ormRawPbData->borrowedcount();
}

void BookInfoOrmData::set_borrowedcount(uint64_t value)
{
    _ormRawPbData->set_borrowedcount(value);
    _MaskDirty(true);
}

Int32 BookInfoOrmData::keywords_size() const
{
    return _ormRawPbData->keywords_size();
}

void BookInfoOrmData::clear_keywords()
{
    _ormRawPbData->clear_keywords();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::keywords(Int32 idx) const
{
    return _ormRawPbData->keywords(idx);
}

std::string *BookInfoOrmData::mutable_keywords(Int32 idx)
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_keywords(idx);
}

void BookInfoOrmData::set_keywords(Int32 idx, const std::string &value)
{
    _ormRawPbData->set_keywords(idx, value);
    _MaskDirty(true);
}

void BookInfoOrmData::set_keywords(Int32 idx, std::string &&value)
{
    _ormRawPbData->set_keywords(idx, std::forward<std::string>(value));
    _MaskDirty(true);
}

void BookInfoOrmData::set_keywords(Int32 idx, const Byte8 *value)
{
    _ormRawPbData->set_keywords(idx, value);
    _MaskDirty(true);
}

void BookInfoOrmData::set_keywords(Int32 idx, const Byte8 *value, size_t sz)
{
    _ormRawPbData->set_keywords(idx, value, sz);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::add_keywords()
{
    auto newElem = _ormRawPbData->add_keywords();
    _MaskDirty(true);
    return newElem;
}

void BookInfoOrmData::add_keywords(const std::string &value)
{
    _ormRawPbData->add_keywords(value);
    _MaskDirty(true);
}

void BookInfoOrmData::add_keywords(std::string &&value)
{
    _ormRawPbData->add_keywords(std::forward<std::string>(value));
    _MaskDirty(true);
}

void BookInfoOrmData::add_keywords(const Byte8 *value)
{
    _ormRawPbData->add_keywords(value);
    _MaskDirty(true);
}

void BookInfoOrmData::add_keywords(const Byte8 *value, size_t sz)
{
    _ormRawPbData->add_keywords(value, sz);
    _MaskDirty(true);
}

void BookInfoOrmData::DeleteArray_keywords(Int32 idx, Int32 count)
{
    _ormRawPbData->mutable_keywords()->DeleteSubrange(idx, count);
    _MaskDirty(true);
}

const ::google::protobuf::RepeatedPtrField<std::string> &BookInfoOrmData::keywords() const
{
    return _ormRawPbData->keywords();
}

void BookInfoOrmData::clear_content()
{
    _ormRawPbData->clear_content();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::content() const
{
    return _ormRawPbData->content();
}

void BookInfoOrmData::set_content(const std::string &value)
{
    _ormRawPbData->set_content(value);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::mutable_content()
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_content();
}

Int32 BookInfoOrmData::snapshot_size() const
{
    return _ormRawPbData->snapshot_size();
}

void BookInfoOrmData::clear_snapshot()
{
    _ormRawPbData->clear_snapshot();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::snapshot(Int32 idx) const
{
    return _ormRawPbData->snapshot(idx);
}

std::string *BookInfoOrmData::mutable_snapshot(Int32 idx)
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_snapshot(idx);
}

void BookInfoOrmData::set_snapshot(Int32 idx, const std::string &value)
{
    _ormRawPbData->set_snapshot(idx, value);
    _MaskDirty(true);
}

void BookInfoOrmData::set_snapshot(Int32 idx, std::string &&value)
{
    _ormRawPbData->set_snapshot(idx, std::forward<std::string>(value));
    _MaskDirty(true);
}

void BookInfoOrmData::set_snapshot(Int32 idx, const Byte8 *value)
{
    _ormRawPbData->set_snapshot(idx, value);
    _MaskDirty(true);
}

void BookInfoOrmData::set_snapshot(Int32 idx, const Byte8 *value, size_t sz)
{
    _ormRawPbData->set_snapshot(idx, value, sz);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::add_snapshot()
{
    auto newElem = _ormRawPbData->add_snapshot();
    _MaskDirty(true);
    return newElem;
}

void BookInfoOrmData::add_snapshot(const std::string &value)
{
    _ormRawPbData->add_snapshot(value);
    _MaskDirty(true);
}

void BookInfoOrmData::add_snapshot(std::string &&value)
{
    _ormRawPbData->add_snapshot(std::forward<std::string>(value));
    _MaskDirty(true);
}

void BookInfoOrmData::add_snapshot(const Byte8 *value)
{
    _ormRawPbData->add_snapshot(value);
    _MaskDirty(true);
}

void BookInfoOrmData::add_snapshot(const Byte8 *value, size_t sz)
{
    _ormRawPbData->add_snapshot(value, sz);
    _MaskDirty(true);
}

void BookInfoOrmData::DeleteArray_snapshot(Int32 idx, Int32 count)
{
    _ormRawPbData->mutable_snapshot()->DeleteSubrange(idx, count);
    _MaskDirty(true);
}

const ::google::protobuf::RepeatedPtrField<std::string> &BookInfoOrmData::snapshot() const
{
    return _ormRawPbData->snapshot();
}

void BookInfoOrmData::clear_coverimagepath()
{
    _ormRawPbData->clear_coverimagepath();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::coverimagepath() const
{
    return _ormRawPbData->coverimagepath();
}

void BookInfoOrmData::set_coverimagepath(const std::string &value)
{
    _ormRawPbData->set_coverimagepath(value);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::mutable_coverimagepath()
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_coverimagepath();
}

void BookInfoOrmData::clear_keywordsstring()
{
    _ormRawPbData->clear_keywordsstring();
    _MaskDirty(true);
}

const std::string &BookInfoOrmData::keywordsstring() const
{
    return _ormRawPbData->keywordsstring();
}

void BookInfoOrmData::set_keywordsstring(const std::string &value)
{
    _ormRawPbData->set_keywordsstring(value);
    _MaskDirty(true);
}

std::string *BookInfoOrmData::mutable_keywordsstring()
{
    _MaskDirty(true);
    return _ormRawPbData->mutable_keywordsstring();
}

Int32 BookInfoOrmData::snapshotpreivewinfolist_size() const
{
    return _ormRawPbData->snapshotpreivewinfolist_size();
}

KERNEL_NS::SmartPtr<SnapshotClientInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &BookInfoOrmData::mutable_snapshotpreivewinfolist(Int32 idx)
{
    return _snapshotpreivewinfolist[idx];
}

void BookInfoOrmData::DeleteArray_snapshotpreivewinfolist(Int32 idx, Int32 count)
{
    for(Int32 pos = idx + count - 1; pos >= idx; --pos)
    {
        _snapshotpreivewinfolist.erase(_snapshotpreivewinfolist.begin() + pos);
    }

    _ormRawPbData->mutable_snapshotpreivewinfolist()->DeleteSubrange(idx, count);
    _MaskDirty(true);
}

const std::vector<KERNEL_NS::SmartPtr<SnapshotClientInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>> &BookInfoOrmData::snapshotpreivewinfolist_OrmDataArray() const
{
    return _snapshotpreivewinfolist;
}

const ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::SnapshotClientInfo> &BookInfoOrmData::snapshotpreivewinfolist() const
{
    return _ormRawPbData->snapshotpreivewinfolist();
}

const KERNEL_NS::SmartPtr<SnapshotClientInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &BookInfoOrmData::snapshotpreivewinfolist_OrmDataArray(Int32 idx) const
{
    return _snapshotpreivewinfolist[idx];
}

const ::CRYSTAL_NET::service::SnapshotClientInfo &BookInfoOrmData::snapshotpreivewinfolist(Int32 idx) const
{
    return _ormRawPbData->snapshotpreivewinfolist(idx);
}

KERNEL_NS::SmartPtr<SnapshotClientInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &BookInfoOrmData::add_snapshotpreivewinfolist()
{
    auto newPb = _ormRawPbData->add_snapshotpreivewinfolist();
    _snapshotpreivewinfolist.push_back(KERNEL_NS::SmartPtr<SnapshotClientInfoOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>());
    auto &elem = _snapshotpreivewinfolist.back();
        elem = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(newPb);

        elem.SetClosureDelegate([](void *ptr){
            SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
        }) ;

        elem->SetMaskDirtyCallback([this](IOrmData *ptr){
            _MaskDirty(true);
        }) ;

    _MaskDirty(true);
    return elem;
}

void BookInfoOrmData::clear_snapshotpreivewinfolist()
{
    _ormRawPbData->clear_snapshotpreivewinfolist();
    _snapshotpreivewinfolist.clear();
    _MaskDirty(true);
}

bool BookInfoOrmData::_OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const
{
    return _ormRawPbData->Encode(stream);
}

bool BookInfoOrmData::_OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const
{
    return _ormRawPbData->Encode(stream);
}

bool BookInfoOrmData::_OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
{
    const auto ret = _ormRawPbData->Decode(stream);
    if(ret)
    {
        if(_variantinfo)
            _variantinfo.Release();
    
        if(_ormRawPbData->has_variantinfo())
        {
            _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());
    
            _variantinfo.SetClosureDelegate([](void *ptr){
                SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
            }) ;
    
            _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
                _MaskDirty(true);
            }) ;
    
        }
    
        {
            const auto count = _ormRawPbData->snapshotpreivewinfolist_size();
    
            _snapshotpreivewinfolist.resize(count);
    
            for(Int32 idx = 0; idx < count; ++idx)
            {
            _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));
    
            _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
                SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
            }) ;
    
            _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                _MaskDirty(true);
            }) ;
    
            }
        }
    
    
    }

    return ret;
}

bool BookInfoOrmData::_OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
{
    const auto ret = _ormRawPbData->Decode(stream);
    if(ret)
    {
        if(_variantinfo)
            _variantinfo.Release();
    
        if(_ormRawPbData->has_variantinfo())
        {
            _variantinfo = SERVICE_COMMON_NS::BookVariantInfoOrmData::NewThreadLocal_BookVariantInfoOrmData(_ormRawPbData->mutable_variantinfo());
    
            _variantinfo.SetClosureDelegate([](void *ptr){
                SERVICE_COMMON_NS::BookVariantInfoOrmData::DeleteThreadLocal_BookVariantInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::BookVariantInfoOrmData>(ptr));
            }) ;
    
            _variantinfo->SetMaskDirtyCallback([this](IOrmData *ptr){
                _MaskDirty(true);
            }) ;
    
        }
    
        {
            const auto count = _ormRawPbData->snapshotpreivewinfolist_size();
    
            _snapshotpreivewinfolist.resize(count);
    
            for(Int32 idx = 0; idx < count; ++idx)
            {
            _snapshotpreivewinfolist[idx] = SERVICE_COMMON_NS::SnapshotClientInfoOrmData::NewThreadLocal_SnapshotClientInfoOrmData(_ormRawPbData->mutable_snapshotpreivewinfolist(idx));
    
            _snapshotpreivewinfolist[idx].SetClosureDelegate([](void *ptr){
                SERVICE_COMMON_NS::SnapshotClientInfoOrmData::DeleteThreadLocal_SnapshotClientInfoOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::SnapshotClientInfoOrmData>(ptr));
            }) ;
    
            _snapshotpreivewinfolist[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                _MaskDirty(true);
            }) ;
    
            }
        }
    
    
    }

    return ret;
}

POOL_CREATE_OBJ_DEFAULT_IMPL(BookInfoOrmDataFactory);

IOrmData *BookInfoOrmDataFactory::Create() const
{
    return BookInfoOrmData::NewThreadLocal_BookInfoOrmData();
}


SERVICE_COMMON_END
