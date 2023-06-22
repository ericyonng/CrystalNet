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
 * Date: 2020-12-21 00:49:19
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_STRING_UTIL_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_UTILS_STRING_UTIL_H__

#pragma once

#include <kernel/kernel_inc.h>
#include <kernel/comp/LibString.h>
#include <kernel/comp/Cpu/CpuDefs.h>

KERNEL_BEGIN

class LibTime;

class KERNEL_EXPORT StringUtil
{
public:
    static size_t snprintf(Byte8 *str, size_t size, const char *format, va_list ap);
    static size_t snprintf(std::string &str, const char *format, va_list ap);
    static uintmax_t malloc_strtoumax(const char *nptr, char **endptr, int base);
    static char *d2s(intmax_t x, char sign, char *s, size_t *slen_p); 
    static char *u2s(uintmax_t x, unsigned base, bool uppercase, char *s, size_t *slen_p);
    static char *o2s(uintmax_t x, bool alt_form, char *s, size_t *slen_p);
    static char *x2s(uintmax_t x, bool alt_form, bool uppercase, char *s, size_t *slen_p);

	static Int32 StringToInt32(const char *str);
    static UInt32 StringToUInt32(const char *str);
    static Int16 StringToInt16(const char *str);
    static UInt16 StringToUInt16(const char *str);
    static Long StringToLong(const char *str);
    static ULong StringToULong(const char *str);
    static Int64 StringToInt64(const char *str);
    static UInt64 StringToUInt64(const char *str);
    static Double StringToDouble(const char *str);
    static LibString ItoA(Int32 value, Int32 radix);
    static LibString UItoA(UInt32 value, Int32 radix);
    static LibString I64toA(Int64 value, Int32 radix);
    static LibString UI64toA(UInt64 value, Int32 radix);
	static bool IsHex(U8 ch);

    template <typename ObjType>
    static LibString Num2Str(ObjType val, Int32 radix = 10);
    static bool ToHexString(const LibString &src, LibString &outHexString);
    static bool FromHexString(const LibString &hexString, LibString &outBin);
	// 加了换行 以便显示
    static bool ToHexStringView(const Byte8 *buff, Int64 len, LibString &outHexString);
    static void PreInstertTime(const LibTime &time, LibString &src);

	/**
	 * Split string using specific separator.
	 * @param[in]  str            - the source string.
	 * @param[in]  separator      - separator string.
	 * @param[out] destStrList    - sestination string list.
	 * @param[in]  justSplitFirst - split first flag, if true, when split one time, will stop.
	 * @param[in]  escapeChar     - escape character, default is '\0' 成功匹配 separator后剔除 separator之前的escapeChar字符.
	 * @param[in]  enableEmptyPart - 切割后是否允许存在空字符串
	 */
    static void SplitString(const LibString &str,
                     const LibString &separator,
                     std::vector<LibString> &destStrList,
                     bool justSplitFirst = false,
                     char escapeChar = '\0', bool enableEmptyPart = true);
    // src原文,start:第一个字符串,end第一个结尾
    static LibString CutString(const LibString &src, const LibString &start, const LibString &end);
    static LibString FilterOutString(const LibString &str, const LibString &filterStr);
	static bool CheckDoubleString(const LibString &str);

	static UInt64 CalcUtf8CharBytes(U8 ctrlChar);
	static bool IsUtf8String(const LibString &str);

	static void MergerMultiLine(const std::vector<LibString> &lines, LibString &target);
	static void SepMultiLine(const LibString &multiLine, std::vector<LibString> &lines);

	static LibString ToString(const std::vector<LibString> &contents, const LibString &sep);
	static void ToString(const std::vector<LibString> &contents, const LibString &sep, LibString &target);
	template<typename T>
	static LibString ToString(const std::vector<T> &contents, const LibString &sep);
	template<typename T>
	static LibString ToString(const std::vector<T *> &contents, const LibString &sep);


	// 校验标准名字:英文, 数字, 下划线, 且首字母非数字, name 长度为0也是非法
	static bool CheckGeneralName(const LibString &name);

	// 移除命名空间
	static LibString RemoveNameSpace(const LibString &name);
	// 转成接口类
	static LibString InterfaceObjName(const LibString &name);
};

inline size_t StringUtil::snprintf(std::string &str, const char *format, va_list ap)
{
	UInt64 fmtLen = ::strlen(format);
    str.resize(fmtLen + str.size() + 1);
    UInt64 size = str.size();

    size_t i;
    const char *f;

#define CHECK_STR_SIZE(idx, str, size)                                          \
    while (UNLIKELY(idx >= size))                                               \
    {                                                                           \
        size <<= 1;                                                             \
        str.resize(size);                                                       \
    }                                                                           

#define APPEND_C(c) do {                                                        \
    CHECK_STR_SIZE(i, str, size);                                               \
    str[i] = (c);                                                               \
	++i;                                                                        \
} while (0)
#define APPEND_S(s, slen) do {                                                  \
    size_t tmp = i + slen + 1;                                                  \
    CHECK_STR_SIZE(tmp , str, size);                                            \
    ::memcpy(&str[i], s, slen);                                                 \
	i += slen;                                                                  \
} while (0)
#define APPEND_PADDED_S(s, slen, width, left_justify) do {		                \
	/* Left padding. */						                                    \
	size_t pad_len = (width == -1) ? 0 : ((slen < (size_t)width) ?	            \
	    (size_t)width - slen : 0);					                            \
	if (!left_justify && pad_len != 0) {				                        \
		size_t j;						                                        \
		for (j = 0; j < pad_len; ++j) {				                            \
			if (pad_zero) {					                                    \
				APPEND_C('0');				                                    \
			} else {					                                        \
				APPEND_C(' ');                                                  \
			}                                                                   \
		}                                                                       \
	}                                                                           \
	/* Value. */							                                    \
	APPEND_S(s, slen);						                                    \
	/* Right padding. */						                                \
	if (left_justify && pad_len != 0) {				                            \
		size_t j;						                                        \
		for (j = 0; j < pad_len; ++j) {				                            \
			APPEND_C(' ');					                                    \
		}							                                            \
	}								                                            \
} while (0)
#define GET_ARG_NUMERIC(val, len) do {                                          \
	switch ((unsigned char)len) {                                               \
	case '?':                                                                   \
		val = va_arg(ap, int);                                                  \
		break;                                                                  \
	case '?' | 0x80:                                                            \
		val = va_arg(ap, unsigned int);                                         \
		break;                                                                  \
	case 'l':                                                                   \
		val = va_arg(ap, long);                                                 \
		break;                                                                  \
	case 'l' | 0x80:                                                            \
		val = va_arg(ap, unsigned long);                                        \
		break;                                                                  \
	case 'q':                                                                   \
		val = va_arg(ap, long long);                                            \
		break;                                                                  \
	case 'q' | 0x80:                                                            \
		val = va_arg(ap, unsigned long long);                                   \
		break;                                                                  \
	case 'j':                                                                   \
		val = va_arg(ap, intmax_t);                                             \
		break;                                                                  \
	case 'j' | 0x80:                                                            \
		val = va_arg(ap, uintmax_t);                                            \
		break;                                                                  \
	case 't':                                                                   \
		val = va_arg(ap, ptrdiff_t);                                            \
		break;                                                                  \
	case 'z':                                                                   \
		val = va_arg(ap, ssize_t);                                              \
		break;                                                                  \
	case 'z' | 0x80:                                                            \
		val = va_arg(ap, size_t);                                               \
		break;                                                                  \
	case 'p': /* Synthetic; used for %p. */                                     \
		val = va_arg(ap, uintptr_t);                                            \
		break;                                                                  \
	default:                                                                    \
		not_reached();                                                          \
		val = 0;                                                                \
	}                                                                           \
} while (0)

	i = 0;
	f = format;
	const Byte8 *noFmtStart = NULL;
	while (true) {
		switch (*f) {
            case '\0': 
			{
				if(noFmtStart)
				{
					noFmtStart = NULL;
					str.append(noFmtStart, f - noFmtStart);
				}
			}
			goto label_out;
            case '%': {
                bool alt_form = false;
                bool left_justify = false;
                bool plus_space = false;
                bool plus_plus = false;
                int prec = -1;
                int width = -1;
                unsigned char len = '?';
                char *s;
                size_t slen;
                bool first_width_digit = true;
                bool pad_zero = false;

				if(noFmtStart)
				{
					noFmtStart = NULL;
					str.append(noFmtStart, f - noFmtStart);
				}

                ++f;
                /* Flags. */
                while (true) {
                    switch (*f) {
                    case '#':
                        ASSERT(!alt_form);
                        alt_form = true;
                        break;
                    case '-':
                        ASSERT(!left_justify);
                        left_justify = true;
                        break;
                    case ' ':
                        ASSERT(!plus_space);
                        plus_space = true;
                        break;
                    case '+':
                        ASSERT(!plus_plus);
                        plus_plus = true;
                        break;

                    default: goto label_width;
                    }
                    ++f;
                }

                /* Width. */
                label_width:
                switch (*f) {
                case '*':
                    width = va_arg(ap, int);
                    ++f;
                    if (width < 0) {
                        left_justify = true;
                        width = -width;
                    }
                    break;
                case '0':
                    if (first_width_digit) {
                        pad_zero = true;
                    }
                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': {
                    uintmax_t uwidth;
                    set_errno(0);
                    uwidth = malloc_strtoumax(f, (char **)&f, 10);
                    ASSERT(uwidth != UINTMAX_MAX || get_errno() !=
                        ERANGE);
                    width = (int)uwidth;
                    first_width_digit = false;
                    break;
                    } 
                default:
                    break;
                }

                /* Width/precision separator. */
                if (*f == '.') {
                    ++f;
                } else {
                    goto label_length;
                }

                /* Precision. */
                switch (*f) {
                case '*':
                    prec = va_arg(ap, int);
                    ++f;
                    break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': {
                    uintmax_t uprec;
                    set_errno(0);
                    uprec = malloc_strtoumax(f, (char **)&f, 10);
                    ASSERT(uprec != UINTMAX_MAX || get_errno() !=
                        ERANGE);
                    prec = (int)uprec;
                    break;
                }

                default: 
                    break;
                }

                /* Length. */
                label_length:
                switch (*f) {
                case 'l':
                    ++f;
                    if (*f == 'l') {
                        len = 'q';
                        ++f;
                    } else {
                        len = 'l';
                    }
                    break;
                case 'q': case 'j': case 't': case 'z':
                    len = *f;
                    ++f;
                    break;
                default: 
                    break;
                }

                /* Conversion specifier. */
                switch (*f) {
                case '%':
                    /* %% */
                    APPEND_C(*f);
                    ++f;
                    break;
                case 'd': case 'i': {
                    intmax_t val = 0;
                    char buf[D2S_BUFSIZE];

                    /*
                    * Outputting negative, zero-padded numbers
                    * would require a nontrivial rework of the
                    * interaction between the width and padding
                    * (since 0 padding goes between the '-' and the
                    * number, while ' ' padding goes either before
                    * the - or after the number.  Since we
                    * currently don't ever need 0-padded negative
                    * numbers, just don't bother supporting it.
                    */
                    ASSERT(!pad_zero);

                    GET_ARG_NUMERIC(val, len);
                    s = d2s(val, (plus_plus ? '+' : (plus_space ?
                        ' ' : '-')), buf, &slen);
                    APPEND_PADDED_S(s, slen, width, left_justify);
                    ++f;
                    break;
                } case 'o': {
                    uintmax_t val = 0;
                    char buf[O2S_BUFSIZE];

                    GET_ARG_NUMERIC(val, len | 0x80);
                    s = o2s(val, alt_form, buf, &slen);
                    APPEND_PADDED_S(s, slen, width, left_justify);
                    ++f;
                    break;
                } case 'u': {
                    uintmax_t val = 0;
                    char buf[U2S_BUFSIZE];

                    GET_ARG_NUMERIC(val, len | 0x80);
                    s = u2s(val, 10, false, buf, &slen);
                    APPEND_PADDED_S(s, slen, width, left_justify);
                    ++f;
                    break;
                } case 'x': case 'X': {
                    uintmax_t val = 0;
                    char buf[X2S_BUFSIZE];

                    GET_ARG_NUMERIC(val, len | 0x80);
                    s = x2s(val, alt_form, *f == 'X', buf, &slen);
                    APPEND_PADDED_S(s, slen, width, left_justify);
                    ++f;
                    break;
                } case 'c': {
                    unsigned char val;
                    char buf[2];

                    ASSERT(len == '?' || len == 'l');
                    ASSERT(len != 'l');
                    val = va_arg(ap, int);
                    buf[0] = val;
                    buf[1] = '\0';
                    APPEND_PADDED_S(buf, 1, width, left_justify);
                    ++f;
                    break;
                } case 's':
                    ASSERT(len == '?' || len == 'l');
                    ASSERT(len != 'l');
                    s = va_arg(ap, char *);
                    slen = (prec < 0) ? strlen(s) : (size_t)prec;
                    APPEND_PADDED_S(s, slen, width, left_justify);
                    ++f;
                    break;
                case 'p': {
                    uintmax_t val;
                    char buf[X2S_BUFSIZE];

                    GET_ARG_NUMERIC(val, 'p');
                    s = x2s(val, true, false, buf, &slen);
                    APPEND_PADDED_S(s, slen, width, left_justify);
                    ++f;
                    break;
                } 
                
                default: 
                    not_reached();
                }
                break;
            } 
            
            default: {
                //APPEND_C(*f);
				if(!noFmtStart)
					noFmtStart = f;

                ++i;
                ++f;
                break;
            }
        }
	}

	label_out:
    CHECK_STR_SIZE(i, str, size);
    str[i] = '\0';


#undef APPEND_C
#undef APPEND_S
#undef APPEND_PADDED_S
#undef GET_ARG_NUMERIC

	return i;    
}

inline size_t StringUtil::snprintf(Byte8 *str, size_t size, const char *format, va_list ap)
{
	size_t i;
	const char *f;

#define APPEND_C(c) do {                                                        \
	if (i < size) {                                                             \
		str[i] = (c);                                                           \
	}                                                                           \
	++i;                                                                        \
} while (0)
#define APPEND_S(s, slen) do {						                            \
	if (i < size) {                                                             \
		size_t cpylen = (slen <= size - i) ? slen : size - i;	                \
		::memcpy(&str[i], s, cpylen);				                            \
	}								                                            \
	i += slen;							                                        \
} while (0)
#define APPEND_PADDED_S(s, slen, width, left_justify) do {		                \
	/* Left padding. */						                                    \
	size_t pad_len = (width == -1) ? 0 : ((slen < (size_t)width) ?	            \
	    (size_t)width - slen : 0);					                            \
	if (!left_justify && pad_len != 0) {				                        \
		size_t j;						                                        \
		for (j = 0; j < pad_len; ++j) {				                            \
			if (pad_zero) {					                                    \
				APPEND_C('0');				                                    \
			} else {					                                        \
				APPEND_C(' ');                                                  \
			}                                                                   \
		}                                                                       \
	}                                                                           \
	/* Value. */							                                    \
	APPEND_S(s, slen);						                                    \
	/* Right padding. */						                                \
	if (left_justify && pad_len != 0) {				                            \
		size_t j;						                                        \
		for (j = 0; j < pad_len; ++j) {				                            \
			APPEND_C(' ');					                                    \
		}							                                            \
	}								                                            \
} while (0)
#define GET_ARG_NUMERIC(val, len) do {                                          \
	switch ((unsigned char)len) {                                               \
	case '?':                                                                   \
		val = va_arg(ap, int);                                                  \
		break;                                                                  \
	case '?' | 0x80:                                                            \
		val = va_arg(ap, unsigned int);                                         \
		break;                                                                  \
	case 'l':                                                                   \
		val = va_arg(ap, long);                                                 \
		break;                                                                  \
	case 'l' | 0x80:                                                            \
		val = va_arg(ap, unsigned long);                                        \
		break;                                                                  \
	case 'q':                                                                   \
		val = va_arg(ap, long long);                                            \
		break;                                                                  \
	case 'q' | 0x80:                                                            \
		val = va_arg(ap, unsigned long long);                                   \
		break;                                                                  \
	case 'j':                                                                   \
		val = va_arg(ap, intmax_t);                                             \
		break;                                                                  \
	case 'j' | 0x80:                                                            \
		val = va_arg(ap, uintmax_t);                                            \
		break;                                                                  \
	case 't':                                                                   \
		val = va_arg(ap, ptrdiff_t);                                            \
		break;                                                                  \
	case 'z':                                                                   \
		val = va_arg(ap, ssize_t);                                              \
		break;                                                                  \
	case 'z' | 0x80:                                                            \
		val = va_arg(ap, size_t);                                               \
		break;                                                                  \
	case 'p': /* Synthetic; used for %p. */                                     \
		val = va_arg(ap, uintptr_t);                                            \
		break;                                                                  \
	default:                                                                    \
		not_reached();                                                          \
		val = 0;                                                                \
	}                                                                           \
} while (0)

	i = 0;
	f = format;
	while (true) {
		switch (*f) {
		case '\0': goto label_out;
		case '%': {
			bool alt_form = false;
			bool left_justify = false;
			bool plus_space = false;
			bool plus_plus = false;
			int prec = -1;
			int width = -1;
			unsigned char len = '?';
			char *s;
			size_t slen;
			bool first_width_digit = true;
			bool pad_zero = false;

			++f;
			/* Flags. */
			while (true) {
				switch (*f) {
				case '#':
					ASSERT(!alt_form);
					alt_form = true;
					break;
				case '-':
					ASSERT(!left_justify);
					left_justify = true;
					break;
				case ' ':
					ASSERT(!plus_space);
					plus_space = true;
					break;
				case '+':
					ASSERT(!plus_plus);
					plus_plus = true;
					break;
				default: goto label_width;
				}
				++f;
			}
			/* Width. */
			label_width:
			switch (*f) {
			case '*':
				width = va_arg(ap, int);
				++f;
				if (width < 0) {
					left_justify = true;
					width = -width;
				}
				break;
			case '0':
				if (first_width_digit) {
					pad_zero = true;
				}
			case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': {
				uintmax_t uwidth;
				set_errno(0);
				uwidth = malloc_strtoumax(f, (char **)&f, 10);
				ASSERT(uwidth != UINTMAX_MAX || get_errno() !=
				    ERANGE);
				width = (int)uwidth;
				first_width_digit = false;
				break;
			} default:
				break;
			}
			/* Width/precision separator. */
			if (*f == '.') {
				++f;
			} else {
				goto label_length;
			}
			/* Precision. */
			switch (*f) {
			case '*':
				prec = va_arg(ap, int);
				++f;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': {
				uintmax_t uprec;
				set_errno(0);
				uprec = malloc_strtoumax(f, (char **)&f, 10);
				ASSERT(uprec != UINTMAX_MAX || get_errno() !=
				    ERANGE);
				prec = (int)uprec;
				break;
			}
			default: break;
			}
			/* Length. */
			label_length:
			switch (*f) {
			case 'l':
				++f;
				if (*f == 'l') {
					len = 'q';
					++f;
				} else {
					len = 'l';
				}
				break;
			case 'q': case 'j': case 't': case 'z':
				len = *f;
				++f;
				break;
			default: break;
			}
			/* Conversion specifier. */
			switch (*f) {
			case '%':
				/* %% */
				APPEND_C(*f);
				++f;
				break;
			case 'd': case 'i': {
				intmax_t val = 0;
				char buf[D2S_BUFSIZE];

				/*
				 * Outputting negative, zero-padded numbers
				 * would require a nontrivial rework of the
				 * interaction between the width and padding
				 * (since 0 padding goes between the '-' and the
				 * number, while ' ' padding goes either before
				 * the - or after the number.  Since we
				 * currently don't ever need 0-padded negative
				 * numbers, just don't bother supporting it.
				 */
				ASSERT(!pad_zero);

				GET_ARG_NUMERIC(val, len);
				s = d2s(val, (plus_plus ? '+' : (plus_space ?
				    ' ' : '-')), buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				++f;
				break;
			} case 'o': {
				uintmax_t val = 0;
				char buf[O2S_BUFSIZE];

				GET_ARG_NUMERIC(val, len | 0x80);
				s = o2s(val, alt_form, buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				++f;
				break;
			} case 'u': {
				uintmax_t val = 0;
				char buf[U2S_BUFSIZE];

				GET_ARG_NUMERIC(val, len | 0x80);
				s = u2s(val, 10, false, buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				++f;
				break;
			} case 'x': case 'X': {
				uintmax_t val = 0;
				char buf[X2S_BUFSIZE];

				GET_ARG_NUMERIC(val, len | 0x80);
				s = x2s(val, alt_form, *f == 'X', buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				++f;
				break;
			} case 'c': {
				unsigned char val;
				char buf[2];

				ASSERT(len == '?' || len == 'l');
                ASSERT(len != 'l');
				val = va_arg(ap, int);
				buf[0] = val;
				buf[1] = '\0';
				APPEND_PADDED_S(buf, 1, width, left_justify);
				++f;
				break;
			} case 's':
				ASSERT(len == '?' || len == 'l');
				ASSERT(len != 'l');
				s = va_arg(ap, char *);
				slen = (prec < 0) ? strlen(s) : (size_t)prec;
				APPEND_PADDED_S(s, slen, width, left_justify);
				++f;
				break;
			case 'p': {
				uintmax_t val;
				char buf[X2S_BUFSIZE];

				GET_ARG_NUMERIC(val, 'p');
				s = x2s(val, true, false, buf, &slen);
				APPEND_PADDED_S(s, slen, width, left_justify);
				++f;
				break;
			} default: not_reached();
			}
			break;
		} default: {
			APPEND_C(*f);
			++f;
			break;
		}}
	}
	label_out:
	if (i < size) {
		str[i] = '\0';
	} else {
		str[size - 1] = '\0';
	}

#undef APPEND_C
#undef APPEND_S
#undef APPEND_PADDED_S
#undef GET_ARG_NUMERIC
	return i;    
}

inline char *StringUtil::d2s(intmax_t x, char sign, char *s, size_t *slen_p)
{
    bool neg;

	if ((neg = (x < 0))) {
		x = -x;
	}
	s = u2s(x, 10, false, s, slen_p);
	if (neg) {
		sign = '-';
	}
	switch (sign) {
	case '-':
		if (!neg) {
			break;
		}
	case ' ':
	case '+':
		--s;
		(*slen_p)++;
		*s = sign;
		break;
	default: not_reached();
	}

	return s;
}

inline char *StringUtil::u2s(uintmax_t x, unsigned base, bool uppercase, char *s, size_t *slen_p)
{
    unsigned i;
	i = U2S_BUFSIZE - 1;
	s[i] = '\0';
	switch (base) {
	case 10:
		do {
			i--;
			s[i] = "0123456789"[x % (uint64_t)10];
			x /= (uint64_t)10;
		} while (x > 0);
		break;
	case 16: {
		const char *digits = (uppercase)
		    ? "0123456789ABCDEF"
		    : "0123456789abcdef";

		do {
			--i;
			s[i] = digits[x & 0xf];
			x >>= 4;
		} while (x > 0);
		break;
	} default: {
		const char *digits = (uppercase)
		    ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		    : "0123456789abcdefghijklmnopqrstuvwxyz";

		ASSERT(base >= 2 && base <= 36);
		do {
			--i;
			s[i] = digits[x % (uint64_t)base];
			x /= (uint64_t)base;
		} while (x > 0);
	}}

	*slen_p = U2S_BUFSIZE - 1 - i;
	return &s[i];
}

inline char *StringUtil::o2s(uintmax_t x, bool alt_form, char *s, size_t *slen_p)
{
	s = u2s(x, 8, false, s, slen_p);
	if (alt_form && *s != '0') {
		--s;
		++(*slen_p);
		*s = '0';
	}

	return s;
}

inline char *StringUtil::x2s(uintmax_t x, bool alt_form, bool uppercase, char *s, size_t *slen_p)
{
	s = u2s(x, 16, uppercase, s, slen_p);
	if (alt_form) {
		s -= 2;
		(*slen_p) += 2;
		::memcpy(s, uppercase ? "0X" : "0x", 2);
	}

	return s;
}

inline Int32 StringUtil::StringToInt32(const char *str)
{
    return ::atoi(str);
}

inline UInt32 StringUtil::StringToUInt32(const char *str)
{
    return static_cast<UInt32>(StringToInt32(str));
}

inline Int16 StringUtil::StringToInt16(const char *str)
{
    return static_cast<Int16>(StringToInt32(str));
}

inline UInt16 StringUtil::StringToUInt16(const char *str)
{
    return static_cast<UInt16>(StringToInt32(str));
}

inline Long StringUtil::StringToLong(const char *str)
{
    return ::atol(str);
}

inline ULong StringUtil::StringToULong(const char *str)
{
    return static_cast<ULong>(StringToLong(str));
}

inline Int64 StringUtil::StringToInt64(const char *str)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    return ::atoll(str);
#else
    return ::_atoi64(str);
#endif
}

inline UInt64 StringUtil::StringToUInt64(const char *str)
{
    return static_cast<UInt64>(StringToInt64(str));
}

inline Double StringUtil::StringToDouble(const char *str)
{
#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    return ::atof(str);
#else
    return ::atof(str);
#endif
}

inline LibString StringUtil::ItoA(Int32 value, Int32 radix)
{
    return I64toA(value, radix);
}

inline LibString StringUtil::UItoA(UInt32 value, Int32 radix)
{
    return UI64toA(value, radix);
}


template <>
inline LibString StringUtil::Num2Str(Int64 val, Int32 radix)
{
    return I64toA(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(UInt64 val, Int32 radix)
{
    return UI64toA(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(Int32 val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(UInt32 val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(Int16 val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(UInt16 val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(Byte8 val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(U8 val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(Long val, Int32 radix)
{
    return Num2Str<Int64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(ULong val, Int32 radix)
{
    return Num2Str<UInt64>(val, radix);
}

template <>
inline LibString StringUtil::Num2Str(Double val, Int32 radix)
{
    char buf[64] = {0};

#if CRYSTAL_TARGET_PLATFORM_NON_WINDOWS
    sprintf(buf, "%f", val);
#else // LLBC_TARGET_PLATFORM_WIN32
    sprintf_s(buf, sizeof(buf), "%f", val);
#endif // LLBC_TARGET_PLATFORM_NON_WIN32
    return buf;
}

template <>
inline LibString StringUtil::Num2Str(Float val, Int32 radix)
{
    return Num2Str<Double>(val, radix);
}

template <typename ObjType>
inline LibString StringUtil::Num2Str(ObjType val, Int32 radix)
{
    if(radix != 10 && radix != 16)
        radix = 10;

    LibString str;
    if(radix == 16)
        str.GetRaw() += "0x";

    UInt64 ptrVal = 0;
	auto objsz = sizeof(ObjType);
	auto u64sz = sizeof(UInt64);
    ::memcpy(&ptrVal, &val, objsz > u64sz ? u64sz : objsz);
    return (str + Num2Str<UInt64>(ptrVal, radix));
}

inline UInt64 StringUtil::CalcUtf8CharBytes(U8 ctrlChar)
{
	if ((ctrlChar & (U8)0x80) == 0x00)
	{
		return 1;
	}
    // 110x xxxx
    // Encoding len: 2 bytes.
    else if ((ctrlChar & (U8)0xe0) == 0xc0)
	{
        return 2;
	}
    // 1110 xxxx
    // Encoding len: 3 bytes.
    else if ((ctrlChar & (U8)0xf0) == 0xe0)
	{
        return 3;
	}
    // 1111 0xxx
    // Encoding len: 4 bytes.
    else if ((ctrlChar & (U8)0xf8) == 0xf0)
	{
        return 4;
	}
    // 1111 10xx
    // Encoding len: 5 bytes.
    else if ((ctrlChar & (U8)0xfc) == 0xf8)
	{
        return 5;
	}
    // 1111 110x
    // Encoding len: 6 bytes.
    else if ((ctrlChar & (U8)0xfe) == 0xfc)
	{
        return 6;
	}

	return 0;
}

ALWAYS_INLINE bool StringUtil::IsUtf8String(const LibString &str)
{
	return str.IsUtf8();
}

ALWAYS_INLINE void StringUtil::MergerMultiLine(const std::vector<LibString> &lines, LibString &target)
{
	const Int32 maxLine = static_cast<Int32>(lines.size());
	for(Int32 idx = 0; idx < maxLine; ++idx)
	{
		auto &lineData = lines[idx];
		target.AppendFormat("%s", lineData.c_str());
		if(idx != (maxLine - 1))
			target.AppendFormat("\n");
	}
}

ALWAYS_INLINE void StringUtil::SepMultiLine(const LibString &multiLine, std::vector<LibString> &lines)
{
	lines = multiLine.Split("\n");
}

ALWAYS_INLINE LibString StringUtil::ToString(const std::vector<LibString> &contents, const LibString &sep)
{
	LibString target;
	ToString(contents, sep, target);
	return target;
}

ALWAYS_INLINE void StringUtil::ToString(const std::vector<LibString> &contents, const LibString &sep, LibString &target)
{
	const Int32 count = static_cast<Int32>(contents.size());
	for(Int32 idx = 0; idx < count; ++idx)
	{
		auto &content = contents[idx];
		target.AppendData(content);
		if(idx != (count - 1))
			target.AppendData(sep);
	}
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::vector<T> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(elem.ToString());
	
	return StringUtil::ToString(strs, sep);
}

template<typename T>
ALWAYS_INLINE LibString StringUtil::ToString(const std::vector<T *> &contents, const LibString &sep)
{
	std::vector<LibString> strs;
	for(auto &elem : contents)
		strs.push_back(elem->ToString());
	
	return StringUtil::ToString(strs, sep);
}

ALWAYS_INLINE bool StringUtil::CheckGeneralName(const LibString &name)
{
	if(name.empty())
	{
		return false;
	}

	// 英文,数值,下划线
	const auto len = static_cast<Int32>(name.size());
	for(Int32 idx = 0; idx < len; ++idx)
	{
		const auto ch = name[idx];
		if(!KERNEL_NS::LibString::isalpha(ch) && !KERNEL_NS::LibString::isdigit(ch) && ch != '_')
		{
			return false;
		}
	}

	// 首字母不能是数值
	if(KERNEL_NS::LibString::isdigit(name[0]))
		return false;

	return true;
}

ALWAYS_INLINE LibString StringUtil::RemoveNameSpace(const LibString &name)
{
    // 命名空间检测
    auto splitNameSpace = name.Split("::", -1, false, true);
    Int32 splitSize = static_cast<Int32>(splitNameSpace.size());

    // 去末尾空
    for(Int32 idx = splitSize - 1; idx >= 0; --idx)
    {
        if(splitNameSpace[idx].empty())
        {
            splitNameSpace.erase(splitNameSpace.begin() + idx);
        }
        else
            break;
    }

	return splitNameSpace.empty() ? "" : splitNameSpace[splitNameSpace.size() - 1];

    LibString icompName;
    splitSize = static_cast<Int32>(splitNameSpace.size());
    for(Int32 idx = 0; idx < splitSize; ++idx)
    {
        if(idx == splitSize -1)
            icompName.AppendFormat("%s", ConstantGather::interfacePrefix.c_str());

        if(!splitNameSpace[idx].empty())
            icompName.AppendFormat("%s", splitNameSpace[idx].c_str());

        if(idx != splitSize -1)
            icompName.AppendFormat("::");
    }
}

ALWAYS_INLINE LibString StringUtil::InterfaceObjName(const LibString &name)
{
	// 命名空间检测
    auto &&splitNameSpace = name.Split("::", -1, false, true);
    Int32 splitSize = static_cast<Int32>(splitNameSpace.size());

    // 去末尾空
    for(Int32 idx = splitSize - 1; idx >= 0; --idx)
    {
        if(splitNameSpace[idx].empty())
        {
            splitNameSpace.erase(splitNameSpace.begin() + idx);
        }
        else
            break;
    }

    LibString icompName;
    splitSize = static_cast<Int32>(splitNameSpace.size());
    for(Int32 idx = 0; idx < splitSize; ++idx)
    {
        if(idx == splitSize -1)
            icompName.AppendFormat("%s", ConstantGather::interfacePrefix.c_str());

        if(!splitNameSpace[idx].empty())
            icompName.AppendFormat("%s", splitNameSpace[idx].c_str());

        if(idx != splitSize -1)
            icompName.AppendFormat("::");
    }

	return icompName;
}

KERNEL_END

#endif
