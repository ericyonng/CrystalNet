/**
 * @file    GamePacket.h
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2018/03/06
 * @brief   游戏协议包封装
 */

#pragma once

#include "Common/ThirdParties.h"

/**
 * 游戏协议包
 */
class GamePacket : public LLBC_Packet
{
public:
    GamePacket();
    virtual ~GamePacket();

public:
    int GetClientSessionId() const;
    void SetClientSessionId(int clientSessionId);

public:
    int _clientSessionId;
};