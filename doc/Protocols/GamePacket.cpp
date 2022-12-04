/**
 * @file    GamePacket.cpp
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2018/03/06
 * @brief   
 */

#include "Common/pch.h"
#include "Common/Protocols/GamePacket.h"

GamePacket::GamePacket()
: _clientSessionId(0)
{
}

GamePacket::~GamePacket()
{
}

int GamePacket::GetClientSessionId() const
{
    return _clientSessionId;
}

void GamePacket::SetClientSessionId(int clientSessionId)
{
    _clientSessionId = clientSessionId;
}
