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
 * Date: 2021-03-21 17:39:05
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/Event/LibEvent.h>

KERNEL_BEGIN


static const Variant __nilSmartVar;
static const std::map<int, Variant> __emptyIntKeyParams;
static const std::map<LibString, Variant> __emptyStrKeyParams;

KERNEL_END

KERNEL_BEGIN

POOL_CREATE_OBJ_DEFAULT_IMPL(LibEvent);

LibEvent::LibEvent(int id /*= 0*/, bool dontDelAfterFire /*= false*/)
{
    _id = id;
    _dontDelAfterFire = dontDelAfterFire;
    _intKeyParams = NULL;
    _strKeyParams = NULL;
    _releaseFunc = NULL;
}

LibEvent::~LibEvent()
{
    if(_intKeyParams)
        CRYSTAL_DELETE_SAFE(_intKeyParams);

//     if(_constantStrKeyParams)
//         Fs_SafeFree(_constantStrKeyParams);

    if(_strKeyParams)
        CRYSTAL_DELETE_SAFE(_strKeyParams);

    if(_releaseFunc)
        _releaseFunc->Release();
    _releaseFunc = NULL;
}

void LibEvent::Release()
{
    if(_releaseFunc)
    {
        _releaseFunc->Invoke(this);
    }
    else
    {
        LibEvent::DeleteThreadLocal_LibEvent(this);
    }
}

const Variant &LibEvent::GetParam(int key) const
{
    if(_intKeyParams == NULL)
        return __nilSmartVar;

    _IntKeyParams::const_iterator it = _intKeyParams->find(key);
    return it != _intKeyParams->end() ? it->second : __nilSmartVar;
}

const Variant &LibEvent::GetParam(const char *key) const
{
    return GetParam(LibString(key));
//     if(_constantStrKeyParams == NULL)
//         return __nilSmartVar;
// 
//     _ConstantStrKeyParams::const_iterator it = _constantStrKeyParams->find(key);
//     return it != _constantStrKeyParams->end() ? it->second : __nilSmartVar;
}

const Variant &LibEvent::GetParam(const LibString &key) const
{
    if(_strKeyParams == NULL)
        return __nilSmartVar;

    _StrKeyParams::const_iterator it = _strKeyParams->find(key);
    return it != _strKeyParams->end() ? it->second : __nilSmartVar;
}

LibEvent &LibEvent::SetParam(int key, const Variant &param)
{
    if(_intKeyParams == NULL)
        _intKeyParams = new _IntKeyParams;

    _IntKeyParams::iterator it = _intKeyParams->find(key);
    if(it == _intKeyParams->end())
        _intKeyParams->insert(std::make_pair(key, param));
    else
        it->second = param;

    return *this;
}

LibEvent &LibEvent::SetParam(const char* key, const Variant &param)
{
    return SetParam(LibString(key), param);
//     if(_constantStrKeyParams == NULL)
//         _constantStrKeyParams = new _ConstantStrKeyParams;
// 
//     _ConstantStrKeyParams::iterator it = _constantStrKeyParams->find(key);
//     if(it == _constantStrKeyParams->end())
//         _constantStrKeyParams->insert(std::make_pair(key, param));
//     else
//         it->second = param;
// 
//     return *this;
}

LibEvent &LibEvent::SetParam(const LibString &key, const Variant &param)
{
    if(_strKeyParams == NULL)
        _strKeyParams = new _StrKeyParams;

    _StrKeyParams::iterator it = _strKeyParams->find(key);
    if(it == _strKeyParams->end())
        _strKeyParams->insert(std::make_pair(key, param));
    else
        it->second = param;

    return *this;
}

const std::map<int, Variant> &LibEvent::GetIntKeyParams() const
{
    return _intKeyParams != NULL ? *_intKeyParams : __emptyIntKeyParams;
}

// const std::map<const char*, SmartVar> &FS_Event::GetConstantStrKeyParams() const
// {
//     return _constantStrKeyParams != NULL ? *_constantStrKeyParams : __emptyConstantStrKeyParams;
// }

const std::map<LibString, Variant> &LibEvent::GetStrKeyParams() const
{
    return _strKeyParams != NULL ? *_strKeyParams : __emptyStrKeyParams;
}

Variant &LibEvent::operator [](int key)
{
    if(!_intKeyParams)
        _intKeyParams = new _IntKeyParams;

    _IntKeyParams::iterator it = _intKeyParams->find(key);
    if(it == _intKeyParams->end())
        return _intKeyParams->insert(std::make_pair(key, Variant())).first->second;
    else
        return it->second;
}

Variant &LibEvent::operator [](const char *key)
{
    return this->operator[](LibString(key));
//     if(!_constantStrKeyParams)
//         _constantStrKeyParams = new _ConstantStrKeyParams;
// 
//     _ConstantStrKeyParams::iterator it = _constantStrKeyParams->find(key);
//     if(it == _constantStrKeyParams->end())
//         return _constantStrKeyParams->insert(std::make_pair(key, SmartVar())).first->second;
//     else
//         return it->second;
}

Variant &LibEvent::operator [](const LibString &key)
{
    if(!_strKeyParams)
        _strKeyParams = new _StrKeyParams;

    _StrKeyParams::iterator it = _strKeyParams->find(key);
    if(it == _strKeyParams->end())
        return _strKeyParams->insert(std::make_pair(key, Variant())).first->second;
    else
        return it->second;
}

const Variant &LibEvent::operator [](int key) const
{
    if(!_intKeyParams)
        return __nilSmartVar;

    _IntKeyParams::const_iterator it = _intKeyParams->find(key);
    return it != _intKeyParams->end() ? it->second : __nilSmartVar;
}

const Variant &LibEvent::operator [](const char* key) const
{
    return this->operator[](LibString(key));
//     if(!_constantStrKeyParams)
//         return __nilSmartVar;
// 
//     _ConstantStrKeyParams::const_iterator it = _constantStrKeyParams->find(key);
//     return it != _constantStrKeyParams->end() ? it->second : __nilSmartVar;
}

const Variant &LibEvent::operator [](const LibString &key) const
{
    if(!_strKeyParams)
        return __nilSmartVar;

    _StrKeyParams::const_iterator it = _strKeyParams->find(key);
    return it != _strKeyParams->end() ? it->second : __nilSmartVar;
}

KERNEL_END