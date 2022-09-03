/*!
 * MIT License
 *  
 * Copyright (c) 2020 Eric Yonng<120453674@qq.com>
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *  
 * 
 * Date: 2020-10-08 14:58:57
 * Author: Eric Yonng
 * Description: 
 *              提供二分插入,二分查找，二分移除(移除会导致pos之后的元素迭代器失效)，
 *              但是1000元素以上会有性能影响Elem建议指针避免产生拷贝
 *              TODO:性能优化了insert/find 需要测试
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BINARY_ARRAY_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_BINARY_ARRAY_H__

#pragma once

#include <kernel/kernel_export.h>
#include <kernel/common/common.h>

KERNEL_BEGIN

template<typename Elem>
class BinaryArrayDefaultComp
{
public:
    bool operator() (const Elem &l, const Elem &r) const
    {
        return l < r;    
    }
};

template<typename Elem, typename Comp = BinaryArrayDefaultComp<Elem>>
class BinaryArray
{
public:
    typedef typename std::vector<Elem>::iterator ArrayIterator;
    typedef typename std::vector<Elem>::const_iterator ConstArrayIterator;
    static const Int32 End;

public:
    BinaryArray(){}

    // 接口
    /*!
     * Description: 插入
     * param {e}:元素 
     * return {pos}:返回索引 -1 表示插入失败,元素已存在 
     * [1, 3, 8,10, 11, 19, 22] 7
     * 找的是不小于e的第一个数所在的位置low,第一个数不小于e,这个数在有序的数列中要么等于e要么,大于e
     * low 5
     * middle 4
     * high 4
    */
    Int32 insert(Elem e)
    {
        Int32 low = 0, high = static_cast<Int32>(_elems.size()) - 1, middle;
        while (low <= high)
        {
            middle = (low + high) >> 1;
            if (_comp(_elems[middle], e))
                low = middle + 1;
            else
                high = middle - 1;
        }

        // 性能优化 _elems[low]出来的必定是>=e的第一个数
        if(LIKELY(_elems.size() > low))
        {
            // 排除相等情况
            if(UNLIKELY( (!_comp(_elems[low], e)) && (!_comp(e, _elems[low])) ))
            {
                CRYSTAL_TRACE("binary array insert a exists elem at pos = %d", middle);
                return End;
            }
        }
        // 把越界囊括在内
        _elems.insert(_elems.begin() + low, e);

        return low;
    }

    /*!
     * Description: 移除
     * param {e}:元素 
     * return {void} 
    */
    void erase(Elem e)
    {
        Int32 pos = find(e);
        if(LIKELY(pos != End))
            _elems.erase(_elems.begin() + pos);
    }

    /*!
     * Description: 通过索引移除
     * param {pos}:索引 
     * return {void} 
    */
    void eraseByPos(Int32 pos)
    {
        _elems.erase(_elems.begin() + pos);
    }

    /*!
     * Description: 清空
     * param {void} 
     * return {void} 
    */
    void clear()
    {
        _elems.clear();
    }

    /*!
     * Description: 查找元素
     * param {e}:元素 
     * return {pos}:返回索引 
    */
    Int32 find(Elem e) const
    {
        Int32 low = 0, high = static_cast<Int32>(_elems.size()) - 1, middle;
        while (low <= high)
        {
            middle = (low + high) >> 1;
            if (_comp(_elems[middle], e))
                low = middle + 1;
            else
                high = middle - 1;
        }

        // 相等判断
        if(LIKELY( (_elems.size() > low) && (!_comp(_elems[low], e)) && (!_comp(e, _elems[low])) ))
            return low;

        return End;
    }

    /*!
     * Description: 获取 startPos - endPos 元素
     * param {startPos}:起始索引 
     * param {endPos}:  结束索引 
     * param {posRefElem}:  获取的pos为key的元素列表 
     * return {type} 
    */
    // void get(Int32 startPos, Int32 endPos, std::map<Int32, const Elem> &posRefElem) const
    // {
    //     for(Int32 i = startPos; i <= endPos; ++i)
    //         posRefElem.insert(std::make_pair<Int32, const Elem>(i, _elems.at(i)));
    // }

    /*!
     * Description: 获取 startPos - endPos 元素
     * param {startPos}:起始索引 
     * param {endPos}:  结束索引 
     * param {posRefElem}:  获取的pos为key的元素列表 
     * return {bool}:成功与失败 
    */
    bool get(Int32 startPos, Int32 endPos, std::map<Int32, Elem> &posRefElem) const
    {
        if(!_FixPos(startPos, endPos))
            return false;

        for(Int32 i = startPos; i <= endPos; ++i)
            posRefElem.insert(std::make_pair(i, _elems.at(i)));

        return true;
    }

    template<typename RetType, typename GetFunc>
    bool get(Int32 beginPos, Int32 endPos, RetType &ret, GetFunc getFunc) const
    {
        if(!_FixPos(beginPos, endPos))
            return false;
            
        for(Int32 idx = beginPos; idx <= endPos; ++idx)
            getFunc(ret, _elems[idx]);

        return true;
    }

    /*!
     * Description: 重新排序,有性能消耗,不建议使用
     * param {void} 
     * return {void} 
    */
    void sort()
    {
        std::sort(_elems.begin(), _elems.end(), _comp);
    }

    size_t size() const
    {
        return _elems.size();
    }

    const Elem *data() const
    {
        return _elems.data();
    }

    Elem *data()
    {
        return _elems.data();
    }
    

    // 
    typename BinaryArray<Elem, Comp>::ArrayIterator begin()
    {
        return _elems.begin();
    }

    typename BinaryArray<Elem, Comp>::ConstArrayIterator begin() const
    {
        return _elems.begin();
    }

    typename BinaryArray<Elem, Comp>::ArrayIterator end()
    {
        return _elems.end();
    }

    typename BinaryArray<Elem, Comp>::ConstArrayIterator end() const
    {
        return _elems.end();
    }
    

    // 运算符重载
    const Elem &operator [] (Int32 pos) const
    {
        return _elems[pos];
    }

    Elem &operator [] (Int32 pos)
    {
        return _elems[pos];
    }

    // 最后一名比other的第一名大 TODO:当最后一名没有比other的第一名大呢？结果一定是比other小么
    bool operator < (const BinaryArray<Elem, Comp> &other) const
    {
        if(UNLIKELY(_elems.empty() && other._elems.empty()))
            return false;

        if(UNLIKELY(_elems.empty()))
            return true;

        if(UNLIKELY(other._elems.empty()))
            return false;

        bool lrComp = _comp(_elems.back(), other._elems.front());
        bool rlComp = _comp(other._elems.front(), _elems.back());
        return lrComp && !rlComp;
    }
    

private:
    bool _FixPos(Int32 &beginPos, Int32 &endPos) const
    {
        beginPos = beginPos > endPos ? endPos : beginPos;
        if(UNLIKELY(beginPos > endPos))
            beginPos = endPos;

        const Int32 elemAmount = static_cast<Int32>(_elems.size());
        if(beginPos < 0 || endPos < 0 || beginPos >= elemAmount || endPos >= elemAmount)
            return false;

        return true;
    }

private:
    std::vector<Elem> _elems;   // 元素
    Comp _comp;                 // 排序方法
};

// 实现
template<typename Elem, typename Comp>
const Int32 BinaryArray<Elem, Comp>::End = -1;


KERNEL_END

#endif
