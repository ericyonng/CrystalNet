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
 * Date: 2022-06-27 13:23:18
 * Author: Eric Yonng
 * Description: 协议体定义
*/

#ifndef __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_PROTOCOL_DEFS_H__
#define __CRYSTAL_NET_SERVICE_COMMON_PROTOCOL_CRYSTAL_PROTOCOL_PROTOCOL_DEFS_H__

#pragma once

#include <service_common/common/common.h>

SERVICE_COMMON_BEGIN

// 协议头定义 (区分客户端协议栈/内网协议栈（不加密,统一小端,）)
// 1.包长度 3字节
// 2.flags 开启加解密(aes, xor, rsa, ), 开启,压缩解压 2字节
// 3.opcode 3字节
// 4.PacketId 8字节（本组包代码，一组完整的包可能需要多个Packet组成, 高47位为序列号 低16位：8位包个数 + 8 位序号） 业务层可以依据48位包序号进行组包 = seriaid + packetnum + 包编号
// 5.包体

SERVICE_COMMON_END

#endif
