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
 * Date: 2020-10-18 15:26:21
 * Author: Eric Yonng
 * Description: 
*/

#include <pch.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Utils/StringUtil.h>
#include <kernel/comp/LibStream.h>

KERNEL_BEGIN

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    const Byte8 * LibString::endl = "\n";
#else
    const Byte8 * LibString::endl = "\r\n";
#endif

// 默认需要剔除的符号
const std::string LibString::_defStripChars = DEF_STRIP_CHARS;

bool LibString::IsUtf8() const
{
	UInt64 count = static_cast<UInt64>(length());
	UInt64 loop = 0;
	while(count > 0)
	{
		U8 ctrl = (*this)[loop];

		auto bytesNum = StringUtil::CalcUtf8CharBytes(ctrl);
		if(bytesNum == 0)
			break;

		++loop;
        --count;
        --bytesNum;

		// 校验除控制位以外的数据位开头必须是10
		for(UInt64 idx = 0; idx < bytesNum; ++idx)
		{
			U8 data = (*this)[loop + idx];
			if(((data >> 6) ^ 0x02) != 0)
				return false;
		}
		
		loop += (bytesNum);
		count -= bytesNum;
	}

	return count == 0;
}

LibString &LibString::operator *= (size_t right)
{
	if (empty() || right == 1)
		return *this;
		
	if (right == 0)
	{
		clear();
		return *this;
	}

	_ThisType unitStr(*this);
	const _Elem *unitStrBuf = unitStr.data();
	typename LibString::size_type unitStrSize = unitStr.size();

	resize(unitStrSize * right);
	_Elem *buf = const_cast<_Elem *>(data());
	for (size_type i = 1; i < right; ++i)
		::memcpy(buf + i * unitStrSize, unitStrBuf, unitStrSize * sizeof(_Elem));

	return *this;
}

void LibString::ToHexString(LibString &target) const
{
	const Int64 bufferSize = static_cast<Int64>(size());
	if(bufferSize == 0)
		return;

	static const Byte8 ChToHexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	std::string info;
	char cache[4] = {0};
	info.reserve(bufferSize * 2);
	for(Int64 i = 0; i < bufferSize; ++i)
	{
		auto &ch = (*this)[i];
		cache[0] = ChToHexChars[U8(ch) >> 4];
		cache[1] = ChToHexChars[ch & 0X0F];
		cache[2] = 0;

		info.append(cache, 2);
	}

	target += info;
}  

void LibString::ToHexView(LibString &target) const
{
	const Int64 bufferSize = static_cast<Int64>(size());
	if(bufferSize == 0)
		return;

	auto &targetRaw = target.GetRaw();
	targetRaw.reserve(target.size() + (bufferSize * 3));

	// 每16字节一行
	Byte8 cache[8] = { 0 };
	Int32 cacheLen = 0;
	for (Int64 i = 0; i < bufferSize; ++i)
	{
		cacheLen = ::sprintf(cache, "%02x%s"
			, static_cast<U8>((*this)[i]), ((i + 1) % 16 == 0) ? "\n" : " ");

		cache[cacheLen] = 0;
		targetRaw.append(cache, cacheLen);
	}
}

bool LibString::FromHexString(const LibString &hexString)
{
	const UInt64 hexLen = hexString.size();
	if(UNLIKELY(hexLen == 0 || ((hexLen % 2) != 0) ))
		return false;

	static const Byte8 *ChHexToDecimalValues = InitHexToDecimalValues();

	reserve(size() + hexLen / 2);
	for(UInt64 idx = 0; idx < hexLen; idx += 2)
	{
		const Int32 hiIdx = static_cast<Int32>(hexString[idx]);
		const Int32 loIdx = static_cast<Int32>(hexString[idx + 1]);
		auto &hi = ChHexToDecimalValues[hiIdx];
		auto &lo = ChHexToDecimalValues[loIdx];
		U8 decimalNumber = (hi << 4) | lo;
		append(reinterpret_cast<Byte8 *>(&decimalNumber), 1);
	}

	return true;
}

void LibString::CompressString()
{
	const auto strSize = size();
	if(strSize == 0)
		return;

	auto len = strlen(c_str());
	if(strSize > len)
	{
		erase(len + 1, strSize - len - 1);
	}
}

const LibString &LibString::RemoveZeroTail()
{
	const Int64 bufferSize = static_cast<Int64>(size());
	if(bufferSize == 0)
		return *this;

	auto pos = find_first_of((const char)(0), 0);
	if(pos == _Base::npos)
		return *this;

	erase(pos, bufferSize - pos);
	return *this;
}     

const LibString &LibString::RemoveHeadZero()
{
	const Int64 bufferSize = static_cast<Int64>(size());
	if(bufferSize == 0)
		return *this;

	auto pos = find_first_not_of((const char)(0), 0);
	if(pos == _Base::npos)
		return *this;

	erase(0, pos);
	return *this;
}

LibString &LibString::AppendFormat(const Byte8 *fmt, ...)
{
	// if fmt args is null, return.
	if (UNLIKELY(!fmt))
		return *this;

	// try detach detach format require buffers and resize it.
	va_list va;
	const size_type oldSize = size();
	va_start(va, fmt);
	Int32 len =::vsnprintf(nullptr, 0, fmt, va);
	va_end(va);
	if (len <= 0)
		return *this;

	// exec format.
	resize(oldSize + len);
	va_start(va, fmt);
	len = ::vsnprintf(const_cast<Byte8 *>(data() + oldSize),
						len + 1,
						fmt,
						va);
	va_end(va);

	// len < 0 then _raw.size() - len > oldSize back to old string TODO:release情况有可能报错
//         if (UNLIKELY(oldSize != (_raw.size() - len)))
//         {
//             CRYSTAL_TRACE("wrong apend format and back to old string, oldSize:%llu, len:%d, new size:%llu", static_cast<UInt64>(oldSize), len, static_cast<UInt64>(_raw.size()));
//             throw std::logic_error("rong apend format and back to old string");
//             _raw.resize(oldSize);
//         }

	return *this;
}

LibString::_ThisType &LibString::findreplace(const _Elem &dest, const _Elem &with, Int32 count)
{
	if (dest == with)
		return *this;

	Int32 founded = 0;
	for (size_type i = 0; i < this->size(); ++i)
	{
		if ((*this)[i] == dest)
		{
			++founded;
			replace(i, 1, 1, with);

			if((count > 0) && (founded >= count))
				break;
		}
	}

	return *this;
}

LibString::_ThisType &LibString::findreplace(const _ThisType &dest, const _ThisType &with, Int32 count)
{
	if (dest == with)
		return *this;

	size_type found = 0;
	Int32 foundCount = 0;
	while ((found = find(dest, found)) != _Base::npos)
	{
		replace(found, dest.size(), with);
		found += with.size();
		++foundCount;
		if((count > 0) && (foundCount >= count))
			break; 
	}

	return *this;
}

LibString::_ThisType &LibString::findFirstAppendFormat(const _ThisType &dest, const Byte8 *fmt, ...)
{
	auto pos = find(dest);
	if(pos == _Base::npos)
		return *this;

	// try detach detach format require buffers and resize it.
	va_list va;
	va_start(va, fmt);
	Int32 len =::vsnprintf(nullptr, 0, fmt, va);
	va_end(va);
	if (len <= 0)
		return *this;

	std::string cache;
	cache.resize(len);

	// exec format.
	va_start(va, fmt);
	len = ::vsnprintf(const_cast<Byte8 *>(cache.data()),
						len + 1,
						fmt,
						va);
	va_end(va);

	// len < 0 then _raw.size() - len > oldSize back to old string
	if (UNLIKELY(0 != (cache.size() - len)))
	{
		CRYSTAL_TRACE("wrong apend format len:%d, new size:%llu", len, static_cast<UInt64>(cache.size()));
		return *this;
	}

	if(pos + 1 == size())
	{
		append(cache);
	}
	else
	{
		insert(pos + 1, cache);
	}

	return *this;
}

LibString::_ThisType &LibString::EraseAnyOf(const _ThisType &dest)
{
	const UInt64 len = dest.length();
	UInt64 curPos = 0;
	for (UInt64 idx = 0; idx < len; ++idx)
	{
		do 
		{
			curPos = find(dest[idx]);
			if (curPos != _Base::npos)
				erase(curPos, 1);
		} while (curPos != _Base::npos);
	}

	return *this;
}

LibString::_These LibString::Split(const LibString &sep, _Base::size_type max_split, bool onlyLikely, bool enableEmptyPart) const
{
	_These substrs;
	if(sep.empty() || max_split == 0 || this->empty())
	{
		if(!this->empty() || enableEmptyPart)
		substrs.push_back(*this);

		return substrs;
	}

	size_type idx = 0;
	UInt32 splitTimes = 0;
	const UInt64 stepSize = onlyLikely ? 1 : sep.size();
	for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
	{
		size_type findIdx = _Base::npos;
		if(onlyLikely)
		{
			for(size_t i = 0; i < sep.size(); i++)
			{
				findIdx = find(sep[i], idx);
				if(findIdx != _Base::npos)
					break;
			}
		}
		else
		{
			findIdx = find(sep, idx);
		}

		if(findIdx == _Base::npos)
			break;
		
		if (findIdx == idx)
		{
			if(enableEmptyPart)
				substrs.push_back(_ThisType());

			if ((idx = findIdx + stepSize) == this->size())
			{
				if (enableEmptyPart)
					substrs.push_back(_ThisType());
				break;
			}

			continue;
		}

		substrs.push_back(substr(idx, findIdx - idx));

		if((idx = findIdx + stepSize) == this->size())
		{
			if(enableEmptyPart)
				substrs.push_back(_ThisType());

			break;
		}
	}

	// 还有剩余
	if(idx != size())
	{
		const auto &subStr = substr(idx);
		if(!subStr.empty() || enableEmptyPart)
			substrs.push_back(subStr);
	}

	return substrs;
}
    
LibString::_These LibString::Split(const _These &seps, size_type max_split, bool enableEmptyPart) const
{
	_These substrs;
	if(seps.empty() || max_split == 0 || this->empty())
	{
		if(!this->empty() || enableEmptyPart)
		substrs.push_back(*this);
		return substrs;
	}

	size_type idx = 0;
	UInt32 splitTimes = 0;
	std::set<size_type> minIdx;
	for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
	{
		size_type findIdx = _Base::npos;
		minIdx.clear();
		for(size_t i = 0; i < seps.size(); i++)
		{
			findIdx = find(seps[i], idx);
			if(findIdx != _Base::npos)
				minIdx.insert(findIdx);
		}

		if(!minIdx.empty())
			findIdx = *minIdx.begin();

		if(findIdx == _Base::npos)
			break;

		substrs.push_back(substr(idx, findIdx - idx));
		if((idx = findIdx + 1) == this->size())
		{
			if(enableEmptyPart)
			substrs.push_back(_ThisType());
			break;
		}
	}

	if(idx != size())
	{
		const auto &subStr = substr(idx);
		if(!subStr.empty() || enableEmptyPart)
			substrs.push_back(subStr);
	}

	return substrs;
}

LibString &LibString::lstrip(const LibString &chars)
{
	_ThisType willStripChars = chars;
	if (chars.empty())
	{
		willStripChars.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));
	}

	size_type stripTo = 0;
	const size_type thisSize = size();
	for (size_type i = 0; i < thisSize; ++i)
	{
		bool found = false;
		const _Elem &now = (*this)[i];
		for (size_type j = 0; j < willStripChars.size(); ++j)
		{
			if (now == willStripChars[j])
			{
				found = true;
				break;
			}
		}

		if (found)
			stripTo = i + 1;
		else
			break;
	}

	if (stripTo != 0)
		erase(0, stripTo);

	return *this;
}

LibString &LibString::lstripString(const LibString &str)
{
	if (str.empty())
	{
		return *this;
	}

	size_type stripTo = 0;
	const size_type thisSize = size();
	const size_type sliceSize = str.size();
	for (size_type i = 0; i < thisSize; i += 1)
	{
		if(sliceSize > (thisSize - i))
			break;

		auto pos = find(str, i);
		if(pos == i)
		{
			stripTo = pos + sliceSize;
			i = stripTo - 1;
		}
		else
		{
			break;
		}
	}

	if (stripTo != 0)
		erase(0, stripTo);

	return *this;
}

LibString &LibString::rstrip(const LibString &chars)
{
	_ThisType willStripChars = chars;
	if (chars.empty())
		willStripChars.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));

	const Int64 willStripRawSize = static_cast<Int64>(willStripChars.size());
	const Int64 thisSize = static_cast<Int64>(size());

	Int64 stripFrom = thisSize;
	for (Int64 i = thisSize - 1; i >= 0; --i)
	{
		bool found = false;
		const _Elem &now = (*this)[i];
		for (Int64 j = 0; j < willStripRawSize; ++j)
		{
			if (now == willStripChars[j])
			{
				found = true;
				break;
			}
		}

		if (found)
			stripFrom = i;
		else
			break;
	}

	if (stripFrom != thisSize)
		erase(stripFrom);

	return *this;
}

LibString &LibString::rstripString(const LibString &str)
{
	if (str.empty())
	{
		return *this;
	}
	
	const Int64 thisSize = static_cast<Int64>(size());
	Int64 stripTo = thisSize;
	const Int64 sliceSize = static_cast<Int64>(str.size());
	for (Int64 i = thisSize - 1; i >= 0; i -= 1)
	{
		if(sliceSize > i + 1)
			break;

		auto pos = rfind(str, i);
		if(pos == static_cast<size_type>((i + 1) - sliceSize))
		{
			stripTo = static_cast<Int64>(pos);
			i = stripTo;
		}
		else
		{
			break;
		}
	}

	if (stripTo != thisSize)
		erase(stripTo);

	return *this;
}

LibString LibString::DragAfter(const LibString &start) const
{
	auto pos =  find(start);
	if(pos == _Base::npos)
		return LibString();
	
	return substr(pos + start.size());
}

LibString LibString::DragAfter(const LibString &start, size_t &startPos, size_t &endPos) const
{
	auto pos =  find(start);
	if(pos == _Base::npos)
		return LibString();

	endPos = pos + start.size() - 1;
	startPos = pos;
	const auto &subStr = substr(pos + start.size());
	endPos += subStr.size();
	return subStr;
}

LibString LibString::DragBefore(const LibString &start) const
{
	auto pos =  find(start);
	if(pos == _Base::npos)
		return LibString();

	return substr(0, pos);
}

LibString LibString::lsub(const LibString &flagStr) const
{
	auto pos = find(flagStr, 0);
	if(pos == _Base::npos)
		return LibString();

	return substr(pos + flagStr.size());
}

LibString LibString::rsub(const LibString &flagStr) const
{
	auto pos = rfind(flagStr);
	if(pos == _Base::npos)
		return LibString();

	return substr(0, pos);
}

bool LibString::isalpha(const LibString &s)
{
	if(s.empty())
		return false;

	const size_type sSize = s.size();
	for(size_t i = 0; i < sSize; i++)
	{
		if(!isalpha(s[i]))
			return false;
	}

	return true;
}

bool LibString::islower(const LibString &s)
{
	if(s.empty())
		return false;

	bool foundLower = false;
	const size_type sSize = s.size();
	for(size_type i = 0; i < sSize; i++)
	{
		if(isupper(s[i]))
			return false;
		else if(islower(s[i]))
			foundLower = true;
	}

	return foundLower;
}

bool LibString::isupper(const LibString &s)
{
	if(s.empty())
		return false;

	bool foundUpper = false;
	const size_type sSize = s.size();
	for(size_type i = 0; i < sSize; i++)
	{
		if(islower(s[i]))
			return false;
		else if(isupper(s[i]))
			foundUpper = true;
	}

	return foundUpper;
}

bool LibString::isdigit(const LibString &s)
{
	if(s.empty())
		return false;

	const size_type sSize = s.size();
	for(size_type i = 0; i < sSize; ++i)
	{
		if(!isdigit(s[i]))
			return false;
	}

	return true;
}

bool LibString::isspace(const LibString &s)
{
	if(s.empty())
		return false;

	const size_type sSize = s.size();
	for(size_type i = 0; i < sSize; i++)
	{
		if(!isspace(s[i]))
			return false;
	}

	return false;
}

// startswith/endswith
bool LibString::IsStartsWith(const LibString &s) const
{
	if (s.empty())
		return true;

	return (size() >= s.size() && memcmp(s.data(), data(), s.size() * sizeof(_Elem)) == 0);
}

bool LibString::IsEndsWith(const LibString &s) const
{
	if (s.empty())
		return true;

	return (size() >= s.size() && 
		memcmp(s.data(), data() + (size() - s.size()) * sizeof(_Elem), s.size() * sizeof(_Elem)) == 0);
}

LibString LibString::StartCut(const LibString &startStr) const
{
    auto pos = find(startStr, 0);
    if(pos == _Base::npos)
        return "";

    return substr(pos + startStr.size());
}

LibString LibString::EndCut(const LibString &endStr) const
{
    auto pos = find(endStr, 0);
    if(pos == _Base::npos)
        return "";

    return substr(0, pos);
}

LibString LibString::tolower() const
{
	const _Elem *buf = data();
	const size_type size = this->size();

	_ThisType lower;
	lower.resize(size);
	for (size_type i = 0; i < size; ++i)
	{
		if (buf[i] >= 0x41 && buf[i] <= 0x5A)
			lower[i] = buf[i] + 0x20;
		else
			lower[i] = buf[i];
	}

	return lower;
}

LibString LibString::toupper() const
{
	const _Elem *buf = this->data();
	const size_type size = this->size();

	_ThisType upper;
	upper.resize(size);
	for (size_type i = 0; i < size; ++i)
		if (buf[i] >= 0x61 && buf[i] <= 0x7a)
			upper[i] = buf[i] - 0x20;
		else
			upper[i] = buf[i];

	return upper;
}

LibString LibString::FirstCharToUpper() const
{
	const _Elem *buf = this->data();
	const size_type size = this->size();
	if(size == 0)
		return *this;

	_ThisType upper = *this;

	if (buf[0] >= 0x61 && buf[0] <= 0x7a)
		upper[0] = buf[0] - 0x20;

	return upper;
}

LibString LibString::FirstCharToLower() const
{
	const _Elem *buf = this->data();
	const size_type size = this->size();
	if(size == 0)
		return *this;

	_ThisType lower = *this;

	if (buf[0] >= 0x41 && buf[0] <= 0x5A)
		lower[0] = buf[0] + 0x20;

	return lower; 
}

LibString::_ThisType &LibString::escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar)
{
	if (this->empty())
		return *this;

	const long len = static_cast<long>(this->size());
	for (long i = len - 1; i >= 0; --i)
	{
		const _Elem &ch = (*this)[i];
		if (ch == escapeChar ||
			willbeEscapeChars.find(ch) != _Base::npos)
			insert(i, 1, escapeChar);
	}

	return *this;
}

LibString::_ThisType &LibString::unescape(const _Elem &escapeChar)
{
	if (empty())
		return *this;

	const Int64 len = static_cast<Int64>(size());
	for (Int64 i = len - 1; i >= 0; --i)
	{
		const _Elem &ch = (*this)[i];
		if (ch == escapeChar)
		{
			if (i > 0 && (*this)[i - 1] == escapeChar)
				erase(i--, 1);
			else
				erase(i, 1);
		}
	}

	return *this;
}

LibString::_ThisType LibString::substr_with_utf8(_Base::size_type pos, _Base::size_type n) const
{
	size_type utf8Len = this->length_with_utf8();
	if (pos >= utf8Len || n == 0)
		return _ThisType();

	_These substrs;
	this->split_utf8_string(pos, substrs);
	if (substrs.empty())
		return _ThisType();

	_ThisType str1 = *substrs.rbegin();
	utf8Len = str1.length_with_utf8();
	pos = (n == LibString::_Base::npos || n > utf8Len) ? utf8Len : n;

	substrs.clear();
	str1.split_utf8_string(pos, substrs);
	if (substrs.empty())
		return _ThisType();

	return substrs[0];
}

void LibString::split_utf8_string(size_type charIndex, _These &strs) const
{
	strs.clear();
	if (charIndex == 0)
	{
		strs.push_back(*this);
		return;
	}

	size_type utf8Count = _ThisType::length_with_utf8();
	if (UNLIKELY(utf8Count == _Base::npos))
	{
		strs.push_back(*this);
		return;
	}

	charIndex = (charIndex < 0) ? 
		static_cast<size_type>(utf8Count) + charIndex : charIndex;
	if (charIndex <= 0 || charIndex >= static_cast<size_type>(utf8Count))
	{
		strs.push_back(*this);
		return;
	}

	size_type bytePos = 0;
	size_type charPos = 0;
	while (charPos != charIndex)
	{
		bytePos = _ThisType::_next_utf8_char_pos(bytePos);
		++charPos;
	}

	strs.push_back(substr(0, bytePos));
	strs.push_back(substr(bytePos));
}

void LibString::scatter_utf8_string(_These &chars, _Base::size_type scatterCount) const
{
	chars.clear();

	if (scatterCount == 0)
		scatterCount = _Base::npos;
	else if (scatterCount != _Base::npos)
		scatterCount -= 1;

	if (scatterCount == 0)
	{
		chars.push_back(*this);
		return;
	}

	size_type curPos = 0;
	size_type prevPos = 0;
	size_type curScatterCount = 0;
	while ((curPos = this->_next_utf8_char_pos(prevPos)) != _Base::npos)
	{
		chars.push_back(substr(prevPos, curPos - prevPos));

		if (scatterCount != _Base::npos && ++curScatterCount >= scatterCount)
		{
			chars.push_back(substr(curPos));
			break;
		}

		prevPos = curPos;
	}
}

// 下一个utf8字符索引pos
LibString::_Base::size_type LibString::_next_utf8_char_pos(_Base::size_type &beginBytePos) const
{
	if (beginBytePos == 0 && this->has_utf8_bomb())
		beginBytePos += 3;

	if (beginBytePos == _Base::npos || beginBytePos >= size())
		return _Base::npos;

	size_type waitCheckCount = _Base::npos;

	// 0xxx xxxx
	// Encoding len: 1 byte.
	U8 ch = static_cast<U8>(at(beginBytePos));
	if ((ch & 0x80) == 0x00)
		waitCheckCount = 0;
	// 110x xxxx
	// Encoding len: 2 bytes.
	else if ((ch & 0xe0) == 0xc0)
		waitCheckCount = 1;
	// 1110 xxxx
	// Encoding len: 3 bytes.
	else if ((ch & 0xf0) == 0xe0)
		waitCheckCount = 2;
	// 1111 0xxx
	// Encoding len: 4 bytes.
	else if ((ch & 0xf8) == 0xf0)
		waitCheckCount = 3;
	// 1111 10xx
	// Encoding len: 5 bytes.
	else if ((ch & 0xfc) == 0xf8)
		waitCheckCount = 4;
	// 1111 110x
	// Encoding len: 6 bytes.
	else if ((ch & 0xfe) == 0xfc)
		waitCheckCount = 5;

	if (waitCheckCount == _Base::npos)
		return _Base::npos;

	size_type curPos = beginBytePos + 1;
	size_type endPos = curPos + waitCheckCount;
	if (endPos > size())
		return _Base::npos;

	for (; curPos != endPos; ++curPos)
	{
		ch = static_cast<U8>(at(curPos));
		if ((ch & 0xc0) != 0x80)
			return _Base::npos;
	}

	return endPos;
}

// hex=>decimal
U8 LibString::_TurnDecimal(const Byte8 hexChar)
{
	switch (hexChar)
	{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': 
		case 'A': return 10;
		case 'b': 
		case 'B': return 11;
		case 'c': 
		case 'C': return 12;
		case 'd': 
		case 'D': return 13;
		case 'e': 
		case 'E': return 14;
		case 'f': 
		case 'F': return 15;
		default:
			break;
	}

	throw std::logic_error("_TurnDecimal hex char not standard hex number");

	return 0;
}

LibString &KernelAppendFormat(LibString &o, const Byte8 *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    auto fmtSize = o.CheckFormatSize(fmt, va);
    va_end(va);

    va_start(va, fmt);
    o.AppendFormatWithVaList(fmtSize, fmt, va);
    va_end(va);
    
    return o;
}

KERNEL_END
