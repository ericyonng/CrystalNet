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
 * Date: 2021-10-17 22:02:34
 * Author: Eric Yonng
 * Description: 
*/

#ifndef __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IFI_INFO_DEFS_UNP_H__
#define __CRYSTAL_NET_KERNEL_INCLUDE_KERNEL_COMP_IFI_INFO_DEFS_UNP_H__

#pragma once

/* define if socket address structures have length fields */
#undef HAVE_SOCKADDR_SA_LEN
#define HAVE_SOCKADDR_SA_LEN 1

/* Define if <net/if_dl.h> defines struct sockaddr_dl */
#undef HAVE_SOCKADDR_DL_STRUCT
#define HAVE_SOCKADDR_DL_STRUCT 1

/* Define if `ifr_mtu' is member of `struct ifreq'. */
#undef HAVE_STRUCT_IFREQ_IFR_MTU
#define HAVE_STRUCT_IFREQ_IFR_MTU 1

#ifndef min
#define	min(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define	max(a,b)	((a) > (b) ? (a) : (b))
#endif

#undef SA
#define	SA	struct sockaddr

/*
 * Round up 'a' to next multiple of 'size', which must be a power of 2
 */
#undef ROUNDUP
#define ROUNDUP(a, size) (((a) & ((size)-1)) ? (1 + ((a) | ((size)-1))) : (a))

/*
 * Step to next socket address structure;
 * if sa_len is 0, assume it is sizeof(u_long).
 */
#define NEXT_SA(ap)	ap = (SA *) \
	((caddr_t) ap + (ap->sa_len ? ROUNDUP(ap->sa_len, sizeof (u_long)) : \
									sizeof(u_long)))


#endif
