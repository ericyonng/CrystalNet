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
 * Date: 2021-03-21 17:07:59
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_EVENT_LIB_EVENT_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_EVENT_LIB_EVENT_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/memory/memory.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Variant/variant_inc.h>

KERNEL_BEGIN

// 释放LibEvent 请使用对象池释放 LibEvent只适用于单线程,所以请使用TL版本
class KERNEL_EXPORT LibEvent
{
    POOL_CREATE_OBJ_DEFAULT(LibEvent);

public:
    LibEvent(int id = 0, bool dontDelAfterFire = false);
    virtual ~LibEvent();
    virtual void Release();

    template<typename Method>
    void SetRelease(Method &&func);

public:
    /**
     * Get the event Id.
     * @return int - the event Id.
     */
    Int32 GetId() const;

    /**
     * Check dont delete after fire option.
     * @return bool - the option.
     */
    bool IsDontDelAfterFire() const;

    /**
     * Set don't delete after fire option.
     * @param[in] dontDelAfterFire - the don't delete after fire option.
     */
    void SetDontDelAfterFire(bool dontDelAfterFire);

public:
    /**
     * Get integer key indexed event param.
     * @param[in] key - the integer key.
     * @return const VariantTL & - the event param.
     */
    const Variant &GetParam(int key) const;

    /**
    * Get constant string key indexed event param.
    * @param[in] key - the constant string key.
    * @return const VariantTL & - the event param.
    */
    const Variant &GetParam(const char* key) const;

    /**
     * Get string key indexed event param.
     * @param[in] key - the string key.
     * @return const VariantTL & - the event param.
     */
    const Variant &GetParam(const LibString &key) const;

    /**
     * Set integer key indexed event param.
     * @param[in] key   - the param key.
     * @param[in] param - the param.
     * @return LibEvent & - this reference.
     */
    LibEvent &SetParam(int key, const Variant &param);
    /**
     * Set integer key indexed event param(template version).
     * @param[in] key   - the param key.
     * @param[in] param - the param.
     * @return LibEvent & - this reference.
     */
    template <typename ParamType>
    LibEvent &SetParam(int key, const ParamType &param);

    /**
    * Set constant string key indexed event param.
    * @param[in] key   - the param key.
    * @param[in] param - the param.
    * @return LibEvent & - this reference.
    */
    LibEvent &SetParam(const char* key, const Variant &param);
    /**
    * Set constant string key indexed event param(template version).
    * @param[in] key   - the param key.
    * @param[in] param - the param.
    * @return LibEvent & - this reference.
    */
    template <typename ParamType>
    LibEvent &SetParam(const char* key, const ParamType &param);

    /**
     * Set string key indexed event param.
     * @param[in] key   - the param key.
     * @param[in] param - the param.
     * @return LibEvent & - this reference.
     */
    LibEvent &SetParam(const LibString &key, const Variant &param);
    /**
     * Set string key indexed event param(template version).
     * @param[in] key   - the param key.
     * @param[in] param - the param.
     * @return LibEvent & - this reference.
     */
    template <typename ParamType>
    LibEvent &SetParam(const LibString &key, const ParamType &param);

public:
    /**
     * Get all int key indexed params.
     * @return const std::map<int, Variant> & - the int key indexed params const reference.
     */
    const std::map<int, Variant> &GetIntKeyParams() const;

    /**
     * Get all string key indexed params count.
     * @return size_t - the integer key indexed params count.
     */
    size_t GetIntKeyParamsCount() const;

    /**
    * Get all constant string key indexed params.
    * @return const std::map<const char*, Variant> & - the constant string key indexed params const reference.
    */
    // const std::map<const char*, Variant> &GetConstantStrKeyParams() const;

    /**
    * Get all constant string key indexed params count.
    * @return size_t - the constant string key indexed params count.
    */
    // size_t GetConstantStrKeyParamsCount() const;

    /**
     * Get all string key indexed params.
     * @return const std::map<LibString, VariantTL> & - the string key indexed params const reference.
     */
    const std::map<LibString, Variant> &GetStrKeyParams() const;

    /**
     * Get all string key indexed params count.
     * @return size_t - the string key indexed params count.
     */
    size_t GetStrKeyParamsCount() const;

public:
    /**
     * Subscript supports.
     */
	Variant &operator [](int key);
	Variant &operator [](const char* key);
	Variant &operator [](const LibString &key);
    const Variant &operator [](int key) const;
    const Variant &operator [](const char* key) const;
    const Variant &operator [](const LibString &key) const;

    /**
     * Disable assignment.
     */
    NO_COPY(LibEvent)

private:
    int _id = 0;
    bool _dontDelAfterFire = true;
    IDelegate<void, LibEvent *> *_releaseFunc = NULL;

    typedef std::map<Int32, Variant> _IntKeyParams;
    _IntKeyParams *_intKeyParams = NULL;

    typedef std::map<LibString, Variant> _StrKeyParams;
    _StrKeyParams *_strKeyParams = NULL;
};

template<typename Method>
ALWAYS_INLINE void LibEvent::SetRelease(Method &&func)
{
    if(_releaseFunc)
        _releaseFunc->Release();

    _releaseFunc = KERNEL_CREATE_CLOSURE_DELEGATE(func, void, LibEvent *);
}

inline int LibEvent::GetId() const
{
    return _id;
}

inline bool LibEvent::IsDontDelAfterFire() const
{
    return _dontDelAfterFire;
}

inline void LibEvent::SetDontDelAfterFire(bool dontDelAfterFire)
{
    _dontDelAfterFire = dontDelAfterFire;
}

template <typename ParamType>
inline LibEvent &LibEvent::SetParam(int key, const ParamType &param)
{
    const Variant varParam(param);
    return SetParam(key, varParam);
}

template <typename ParamType>
inline LibEvent &LibEvent::SetParam(const char *key, const ParamType &param)
{
    const Variant varParam(param);
    return SetParam(key, varParam);
}

template <typename ParamType>
inline LibEvent &LibEvent::SetParam(const LibString &key, const ParamType &param)
{
    const Variant varParam(param);
    return SetParam(key, varParam);
}

inline size_t LibEvent::GetIntKeyParamsCount() const
{
    return _intKeyParams != NULL ? _intKeyParams->size() : 0;
}

inline size_t LibEvent::GetStrKeyParamsCount() const
{
    return _strKeyParams != NULL ? _strKeyParams->size() : 0;
}


KERNEL_END

#endif
