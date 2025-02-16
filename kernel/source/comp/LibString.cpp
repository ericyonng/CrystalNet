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
		U8 ctrl = _raw[loop];

		auto bytesNum = StringUtil::CalcUtf8CharBytes(ctrl);
		if(bytesNum == 0)
			break;

		++loop;
        --count;
        --bytesNum;

		// 校验除控制位以外的数据位开头必须是10
		for(UInt64 idx = 0; idx < bytesNum; ++idx)
		{
			U8 data = _raw[loop + idx];
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
	if (_raw.empty() || right == 1)
		return *this;
		
	if (right == 0)
	{
		_raw.clear();
		return *this;
	}

	_ThisType unitStr(*this);
	auto &unitRaw = unitStr._raw;

	const _Elem *unitStrBuf = unitRaw.data();
	typename LibString::size_type unitStrSize = unitRaw.size();

	_raw.resize(unitStrSize * right);
	_Elem *buf = const_cast<_Elem *>(_raw.data());
	for (size_type i = 1; i < right; ++i)
		::memcpy(buf + i * unitStrSize, unitStrBuf, unitStrSize * sizeof(_Elem));

	return *this;
}

void LibString::ToHexString(LibString &target) const
{
	const Int64 bufferSize = static_cast<Int64>(_raw.size());
	if(bufferSize == 0)
		return;

	static const Byte8 ChToHexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	std::string info;
	char cache[4] = {0};
	info.reserve(bufferSize * 2);
	for(Int64 i = 0; i < bufferSize; ++i)
	{
		auto &ch = _raw[i];
		cache[0] = ChToHexChars[U8(ch) >> 4];
		cache[1] = ChToHexChars[ch & 0X0F];
		cache[2] = 0;

		info.append(cache, 2);
	}

	target += info;
}  

void LibString::ToHexView(LibString &target) const
{
	const Int64 bufferSize = static_cast<Int64>(_raw.size());
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
			, static_cast<U8>(_raw[i]), ((i + 1) % 16 == 0) ? "\n" : " ");

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

	_raw.reserve(_raw.size() + hexLen / 2);
	for(UInt64 idx = 0; idx < hexLen; idx += 2)
	{
		const Int32 hiIdx = static_cast<Int32>(hexString[idx]);
		const Int32 loIdx = static_cast<Int32>(hexString[idx + 1]);
		auto &hi = ChHexToDecimalValues[hiIdx];
		auto &lo = ChHexToDecimalValues[loIdx];
		U8 decimalNumber = (hi << 4) | lo;
		_raw.append(reinterpret_cast<Byte8 *>(&decimalNumber), 1);
	}

	return true;
}

void LibString::CompressString()
{
	const auto strSize = _raw.size();
	if(strSize == 0)
		return;

	auto len = strlen(_raw.c_str());
	if(strSize > len)
	{
		_raw.erase(len + 1, strSize - len - 1);
	}
}

const LibString &LibString::RemoveZeroTail()
{
	const Int64 bufferSize = static_cast<Int64>(_raw.size());
	if(bufferSize == 0)
		return *this;

	auto pos = _raw.find_first_of((const char)(0), 0);
	if(pos == std::string::npos)
		return *this;

	_raw.erase(pos, bufferSize - pos);
	return *this;
}     

const LibString &LibString::RemoveHeadZero()
{
	const Int64 bufferSize = static_cast<Int64>(_raw.size());
	if(bufferSize == 0)
		return *this;

	auto pos = _raw.find_first_not_of((const char)(0), 0);
	if(pos == std::string::npos)
		return *this;

	_raw.erase(0, pos);
	return *this;
}

LibString &LibString::AppendFormat(const Byte8 *fmt, ...)
{
	// if fmt args is null, return.
	if (UNLIKELY(!fmt))
		return *this;

	// try detach detach format require buffers and resize it.
	va_list va;
	const size_type oldSize = _raw.size();
	va_start(va, fmt);
	Int32 len =::vsnprintf(nullptr, 0, fmt, va);
	va_end(va);
	if (len <= 0)
		return *this;

	// exec format.
	_raw.resize(oldSize + len);
	va_start(va, fmt);
	len = ::vsnprintf(const_cast<Byte8 *>(_raw.data() + oldSize),
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
		if (_raw[i] == dest)
		{
			++founded;
			_raw.replace(i, 1, 1, with);

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
	const std::string &destRaw = dest._raw;
	const std::string &withRaw = with._raw;
	Int32 foundCount = 0;
	while ((found = _raw.find(destRaw, found)) != std::string::npos)
	{
		_raw.replace(found, destRaw.size(), withRaw);
		found += withRaw.size();
		++foundCount;
		if((count > 0) && (foundCount >= count))
			break; 
	}

	return *this;
}

LibString::_ThisType &LibString::findFirstAppendFormat(const _ThisType &dest, const Byte8 *fmt, ...)
{
	auto &destRaw = dest._raw;
	auto pos = _raw.find(destRaw);
	if(pos == std::string::npos)
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

	if(pos + 1 == _raw.size())
	{
		_raw.append(cache);
	}
	else
	{
		_raw.insert(pos + 1, cache);
	}

	return *this;
}

LibString::_ThisType &LibString::EraseAnyOf(const _ThisType &dest)
{
	auto &destRaw = dest.GetRaw();
	const UInt64 len = destRaw.length();
	UInt64 curPos = 0;
	for (UInt64 idx = 0; idx < len; ++idx)
	{
		do 
		{
			curPos = _raw.find(destRaw[idx]);
			if (curPos != std::string::npos)
				_raw.erase(curPos, 1);
		} while (curPos != std::string::npos);
	}

	return *this;
}

LibString::_These LibString::Split(const LibString &sep, std::string::size_type max_split, bool onlyLikely, bool enableEmptyPart) const
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
	const std::string &sepRaw = sep._raw;
	const UInt64 stepSize = onlyLikely ? 1 : sepRaw.size();
	for(; splitTimes < static_cast<UInt32>(max_split); ++splitTimes)
	{
		size_type findIdx = std::string::npos;
		if(onlyLikely)
		{
			for(size_t i = 0; i < sepRaw.size(); i++)
			{
				findIdx = _raw.find(sepRaw[i], idx);
				if(findIdx != std::string::npos)
					break;
			}
		}
		else
		{
			findIdx = _raw.find(sepRaw, idx);
		}

		if(findIdx == std::string::npos)
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

		substrs.push_back(_raw.substr(idx, findIdx - idx));

		if((idx = findIdx + stepSize) == this->size())
		{
			if(enableEmptyPart)
			substrs.push_back(_ThisType());

			break;
		}
	}

	// 还有剩余
	if(idx != _raw.size())
	{
		const auto &subStr = _raw.substr(idx);
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
		size_type findIdx = std::string::npos;
		minIdx.clear();
		for(size_t i = 0; i < seps.size(); i++)
		{
			findIdx = _raw.find(seps[i]._raw, idx);
			if(findIdx != std::string::npos)
				minIdx.insert(findIdx);
		}

		if(!minIdx.empty())
			findIdx = *minIdx.begin();

		if(findIdx == std::string::npos)
			break;

		substrs.push_back(_raw.substr(idx, findIdx - idx));
		if((idx = findIdx + 1) == this->size())
		{
			if(enableEmptyPart)
			substrs.push_back(_ThisType());
			break;
		}
	}

	if(idx != this->size())
	{
		const auto &subStr = _raw.substr(idx);
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
		willStripChars._raw.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));
	}

	std::string &thisRaw = _raw;
	size_type stripTo = 0;
	std::string &willStripRaw = willStripChars._raw;
	const size_type thisSize = static_cast<size_type>(thisRaw.size());
	for (size_type i = 0; i < thisSize; ++i)
	{
		bool found = false;
		const _Elem &now = thisRaw[i];
		for (size_type j = 0; j < willStripRaw.size(); ++j)
		{
			if (now == willStripRaw[j])
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
		thisRaw.erase(0, stripTo);

	return *this;
}

LibString &LibString::lstripString(const LibString &str)
{
	if (str.empty())
	{
		return *this;
	}

	std::string &thisRaw = _raw;
	size_type stripTo = 0;
	const size_type thisSize = static_cast<size_type>(thisRaw.size());
	const size_type sliceSize = static_cast<size_type>(str.size());
	for (size_type i = 0; i < thisSize; i += 1)
	{
		if(sliceSize > (thisSize - i))
			break;

		auto pos = thisRaw.find(str._raw, i);
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
		thisRaw.erase(0, stripTo);

	return *this;
}

LibString &LibString::rstrip(const LibString &chars)
{
	_ThisType willStripChars = chars;
	if (chars.empty())
		willStripChars._raw.append(reinterpret_cast<const _Elem *>(" \t\v\r\n\f"));

	std::string &thisRaw = _raw;
	std::string &willStripRaw = willStripChars._raw;
	const Int64 willStripRawSize = static_cast<Int64>(willStripRaw.size());
	const Int64 thisSize = static_cast<Int64>(thisRaw.size());

	Int64 stripFrom = thisSize;
	for (Int64 i = thisSize - 1; i >= 0; --i)
	{
		bool found = false;
		const _Elem &now = thisRaw[i];
		for (Int64 j = 0; j < willStripRawSize; ++j)
		{
			if (now == willStripRaw[j])
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
		thisRaw.erase(stripFrom);

	return *this;
}

LibString &LibString::rstripString(const LibString &str)
{
	if (str.empty())
	{
		return *this;
	}

	std::string &thisRaw = _raw;
	const Int64 thisSize = static_cast<Int64>(thisRaw.size());
	Int64 stripTo = thisSize;
	const Int64 sliceSize = static_cast<Int64>(str.size());
	for (Int64 i = thisSize - 1; i >= 0; i -= 1)
	{
		if(sliceSize > i + 1)
			break;

		auto pos = thisRaw.rfind(str._raw, i);
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
		thisRaw.erase(stripTo);

	return *this;
}

LibString LibString::DragAfter(const LibString &start) const
{
	auto pos =  _raw.find(start._raw);
	if(pos == std::string::npos)
		return LibString();
	
	return _raw.substr(pos + start._raw.size());
}

LibString LibString::DragAfter(const LibString &start, size_t &startPos, size_t &endPos) const
{
	auto pos =  _raw.find(start._raw);
	if(pos == std::string::npos)
		return LibString();

	endPos = pos + start._raw.size() - 1;
	startPos = pos;
	const auto &subStr = _raw.substr(pos + start._raw.size());
	endPos += subStr.size();
	return subStr;
}

LibString LibString::DragBefore(const LibString &start) const
{
	auto pos =  _raw.find(start._raw);
	if(pos == std::string::npos)
		return LibString();

	return _raw.substr(0, pos);
}

LibString LibString::lsub(const LibString &flagStr) const
{
	auto pos = _raw.find(flagStr.GetRaw(), 0);
	if(pos == std::string::npos)
		return LibString();

	return _raw.substr(pos + flagStr.size());
}

LibString LibString::rsub(const LibString &flagStr) const
{
	auto pos = _raw.rfind(flagStr.GetRaw());
	if(pos == std::string::npos)
		return LibString();

	return _raw.substr(0, pos);
}

bool LibString::isalpha(const LibString &s)
{
	if(s.empty())
		return false;

	const std::string &sRaw = s._raw;
	const size_type sSize = sRaw.size();
	for(size_t i = 0; i < sSize; i++)
	{
		if(!isalpha(sRaw[i]))
			return false;
	}

	return true;
}

bool LibString::islower(const LibString &s)
{
	if(s.empty())
		return false;

	bool foundLower = false;
	const std::string &sRaw = s._raw;
	const size_type sSize = sRaw.size();
	for(size_type i = 0; i < sSize; i++)
	{
		if(isupper(sRaw[i]))
			return false;
		else if(islower(sRaw[i]))
			foundLower = true;
	}

	return foundLower;
}

bool LibString::isupper(const LibString &s)
{
	if(s.empty())
		return false;

	bool foundUpper = false;
	const std::string &sRaw = s._raw;
	const size_type sSize = sRaw.size();
	for(size_type i = 0; i < sSize; i++)
	{
		if(islower(sRaw[i]))
			return false;
		else if(isupper(sRaw[i]))
			foundUpper = true;
	}

	return foundUpper;
}

bool LibString::isdigit(const LibString &s)
{
	if(s.empty())
		return false;

	const std::string &sRaw = s._raw;
	const size_type sSize = sRaw.size();
	for(size_type i = 0; i < sSize; ++i)
	{
		if(!isdigit(sRaw[i]))
			return false;
	}

	return true;
}

bool LibString::isspace(const LibString &s)
{
	if(s.empty())
		return false;

	const std::string &sRaw = s._raw;
	const size_type sSize = sRaw.size();
	for(size_type i = 0; i < sSize; i++)
	{
		if(!isspace(sRaw[i]))
			return false;
	}

	return false;
}

// startswith/endswith
bool LibString::IsStartsWith(const LibString &s) const
{
	if (s.empty())
		return true;

	const std::string &sRaw = s._raw;
	return (_raw.size() >= sRaw.size() && memcmp(sRaw.data(), _raw.data(), sRaw.size() * sizeof(_Elem)) == 0);
}

bool LibString::IsEndsWith(const LibString &s) const
{
	if (s.empty())
		return true;

	const std::string &sRaw = s._raw;
	return (_raw.size() >= sRaw.size() && 
		memcmp(sRaw.data(), _raw.data() + (_raw.size() - sRaw.size()) * sizeof(_Elem), sRaw.size() * sizeof(_Elem)) == 0);
}

LibString LibString::StartCut(const LibString &startStr) const
{
    auto pos = _raw.find(startStr._raw, 0);
    if(pos == std::string::npos)
        return "";

    return _raw.substr(pos + startStr._raw.size());
}

LibString LibString::EndCut(const LibString &endStr) const
{
    auto pos = _raw.find(endStr._raw, 0);
    if(pos == std::string::npos)
        return "";

    return _raw.substr(0, pos);
}

LibString LibString::tolower() const
{
	const _Elem *buf = _raw.data();
	const size_type size = this->size();

	_ThisType lower;
	std::string &lowerRaw = lower._raw;
	lowerRaw.resize(size);
	for (size_type i = 0; i < size; ++i)
	{
		if (buf[i] >= 0x41 && buf[i] <= 0x5A)
			lowerRaw[i] = buf[i] + 0x20;
		else
			lowerRaw[i] = buf[i];
	}

	return lower;
}

LibString LibString::toupper() const
{
	const _Elem *buf = this->data();
	const size_type size = this->size();

	_ThisType upper;
	std::string &upperRaw = upper._raw;
	upperRaw.resize(size);
	for (size_type i = 0; i < size; ++i)
		if (buf[i] >= 0x61 && buf[i] <= 0x7a)
			upperRaw[i] = buf[i] - 0x20;
		else
			upperRaw[i] = buf[i];

	return upper;
}

LibString LibString::FirstCharToUpper() const
{
	const _Elem *buf = this->data();
	const size_type size = this->size();
	if(size == 0)
		return *this;

	_ThisType upper;
	std::string &upperRaw = upper._raw;
	upper = *this;

	if (buf[0] >= 0x61 && buf[0] <= 0x7a)
		upperRaw[0] = buf[0] - 0x20;

	return upper;
}

LibString LibString::FirstCharToLower() const
{
	const _Elem *buf = this->data();
	const size_type size = this->size();
	if(size == 0)
		return *this;

	_ThisType lower;
	std::string &lowerRaw = lower._raw;
	lower = *this;

	if (buf[0] >= 0x41 && buf[0] <= 0x5A)
		lowerRaw[0] = buf[0] + 0x20;

	return lower; 
}

LibString::_ThisType &LibString::escape(const _ThisType &willbeEscapeChars, const _Elem &escapeChar)
{
	if (this->empty())
		return *this;

	const long len = static_cast<long>(this->size());
	std::string &thisRaw = _raw;
	const std::string &willbeEscapeRaw = willbeEscapeChars._raw;
	for (long i = len - 1; i >= 0; --i)
	{
		const _Elem &ch = thisRaw[i];
		if (ch == escapeChar ||
			willbeEscapeRaw.find(ch) != std::string::npos)
			thisRaw.insert(i, 1, escapeChar);
	}

	return *this;
}

LibString::_ThisType &LibString::unescape(const _Elem &escapeChar)
{
	if (_raw.empty())
		return *this;

	std::string &thisRaw = _raw;
	const Int64 len = static_cast<Int64>(thisRaw.size());
	for (Int64 i = len - 1; i >= 0; --i)
	{
		const _Elem &ch = thisRaw[i];
		if (ch == escapeChar)
		{
			if (i > 0 && thisRaw[i - 1] == escapeChar)
				thisRaw.erase(i--, 1);
			else
				thisRaw.erase(i, 1);
		}
	}

	return *this;
}

LibString::_ThisType LibString::substr_with_utf8(std::string::size_type pos, std::string::size_type n) const
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
	pos = (n == std::string::npos || n > utf8Len) ? utf8Len : n;

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
	if (UNLIKELY(utf8Count == std::string::npos))
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

	strs.push_back(_raw.substr(0, bytePos));
	strs.push_back(_raw.substr(bytePos));
}

void LibString::scatter_utf8_string(_These &chars, std::string::size_type scatterCount) const
{
	chars.clear();

	if (scatterCount == 0)
		scatterCount = std::string::npos;
	else if (scatterCount != std::string::npos)
		scatterCount -= 1;

	if (scatterCount == 0)
	{
		chars.push_back(*this);
		return;
	}

	size_type curPos = 0;
	size_type prevPos = 0;
	size_type curScatterCount = 0;
	while ((curPos = this->_next_utf8_char_pos(prevPos)) != std::string::npos)
	{
		chars.push_back(_raw.substr(prevPos, curPos - prevPos));

		if (scatterCount != std::string::npos && ++curScatterCount >= scatterCount)
		{
			chars.push_back(_raw.substr(curPos));
			break;
		}

		prevPos = curPos;
	}
}

// 下一个utf8字符索引pos
std::string::size_type LibString::_next_utf8_char_pos(std::string::size_type &beginBytePos) const
{
	if (beginBytePos == 0 && this->has_utf8_bomb())
		beginBytePos += 3;

	if (beginBytePos == std::string::npos || beginBytePos >= _raw.size())
		return std::string::npos;

	size_type waitCheckCount = std::string::npos;

	// 0xxx xxxx
	// Encoding len: 1 byte.
	U8 ch = static_cast<U8>(_raw.at(beginBytePos));
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

	if (waitCheckCount == std::string::npos)
		return std::string::npos;

	size_type curPos = beginBytePos + 1;
	size_type endPos = curPos + waitCheckCount;
	if (endPos > _raw.size())
		return std::string::npos;

	for (; curPos != endPos; ++curPos)
	{
		ch = static_cast<U8>(_raw.at(curPos));
		if ((ch & 0xc0) != 0x80)
			return std::string::npos;
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

std::string &operator <<(std::string &o, const KERNEL_NS::LibString &str)
{
    o += str.GetRaw();
    return o;
}
