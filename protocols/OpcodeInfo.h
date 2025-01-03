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
* Author: Eric Yonng
* Description: Generated By protogentool, Dont Modify This File!!!
*/


    {// LoginReq
        auto info = OpcodeInfo();
        info._opcode = 1;
        info._noLog = false;
        info._enableStorage = false;
        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::XOR_ENCRYPT_FLAG;
        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::KEY_IN_BASE64_FLAG;
        info._opcodeName = "LoginReq";
        info._protoFile = "login.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LoginReqFactory::CreateFactory()));
    }

    {// LoginRes
        auto info = OpcodeInfo();
        info._opcode = 2;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "LoginRes";
        info._protoFile = "login.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LoginResFactory::CreateFactory()));
    }

    {// TestOpcodeReq
        auto info = OpcodeInfo();
        info._opcode = 3;
        info._noLog = true;
        info._enableStorage = false;
        info._opcodeName = "TestOpcodeReq";
        info._protoFile = "test_opcode.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TestOpcodeReqFactory::CreateFactory()));
    }

    {// TestOpcodeRes
        auto info = OpcodeInfo();
        info._opcode = 4;
        info._noLog = true;
        info._enableStorage = false;
        info._opcodeName = "TestOpcodeRes";
        info._protoFile = "test_opcode.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TestOpcodeResFactory::CreateFactory()));
    }

    {// TestOpcode2Req
        auto info = OpcodeInfo();
        info._opcode = 5;
        info._noLog = true;
        info._enableStorage = false;
        info._opcodeName = "TestOpcode2Req";
        info._protoFile = "test_opcode.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TestOpcode2ReqFactory::CreateFactory()));
    }

    {// TestOpcode2Res
        auto info = OpcodeInfo();
        info._opcode = 6;
        info._noLog = true;
        info._enableStorage = false;
        info._opcodeName = "TestOpcode2Res";
        info._protoFile = "test_opcode.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TestOpcode2ResFactory::CreateFactory()));
    }

    {// PlayerDataReq
        auto info = OpcodeInfo();
        info._opcode = 7;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "PlayerDataReq";
        info._protoFile = "player.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, PlayerDataReqFactory::CreateFactory()));
    }

    {// PlayerDataRes
        auto info = OpcodeInfo();
        info._opcode = 8;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "PlayerDataRes";
        info._protoFile = "player.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, PlayerDataResFactory::CreateFactory()));
    }

    {// ModifyPlayerNameReq
        auto info = OpcodeInfo();
        info._opcode = 9;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ModifyPlayerNameReq";
        info._protoFile = "player.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ModifyPlayerNameReqFactory::CreateFactory()));
    }

    {// ModifyPlayerNameRes
        auto info = OpcodeInfo();
        info._opcode = 10;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ModifyPlayerNameRes";
        info._protoFile = "player.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ModifyPlayerNameResFactory::CreateFactory()));
    }

    {// PlayerDataNty
        auto info = OpcodeInfo();
        info._opcode = 11;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "PlayerDataNty";
        info._protoFile = "player.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, PlayerDataNtyFactory::CreateFactory()));
    }

    {// TitleInfoReq
        auto info = OpcodeInfo();
        info._opcode = 20;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "TitleInfoReq";
        info._protoFile = "title.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TitleInfoReqFactory::CreateFactory()));
    }

    {// TitleInfoRes
        auto info = OpcodeInfo();
        info._opcode = 21;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "TitleInfoRes";
        info._protoFile = "title.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TitleInfoResFactory::CreateFactory()));
    }

    {// LoginInfoNty
        auto info = OpcodeInfo();
        info._opcode = 22;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "LoginInfoNty";
        info._protoFile = "login.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LoginInfoNtyFactory::CreateFactory()));
    }

    {// NodeHeartbeatReq
        auto info = OpcodeInfo();
        info._opcode = 23;
        info._noLog = true;
        info._enableStorage = false;
        info._opcodeName = "NodeHeartbeatReq";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, NodeHeartbeatReqFactory::CreateFactory()));
    }

    {// NodeHeartbeatRes
        auto info = OpcodeInfo();
        info._opcode = 24;
        info._noLog = true;
        info._enableStorage = false;
        info._opcodeName = "NodeHeartbeatRes";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, NodeHeartbeatResFactory::CreateFactory()));
    }

    {// RegisterNodeReq
        auto info = OpcodeInfo();
        info._opcode = 25;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "RegisterNodeReq";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, RegisterNodeReqFactory::CreateFactory()));
    }

    {// RegisterNodeRes
        auto info = OpcodeInfo();
        info._opcode = 26;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "RegisterNodeRes";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, RegisterNodeResFactory::CreateFactory()));
    }

    {// GetNodeListReq
        auto info = OpcodeInfo();
        info._opcode = 27;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetNodeListReq";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetNodeListReqFactory::CreateFactory()));
    }

    {// GetNodeListRes
        auto info = OpcodeInfo();
        info._opcode = 28;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetNodeListRes";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetNodeListResFactory::CreateFactory()));
    }

    {// ClientHeartbeatReq
        auto info = OpcodeInfo();
        info._opcode = 29;
        info._noLog = true;
        info._enableStorage = false;
        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::XOR_ENCRYPT_FLAG;
        info._msgFlags |= SERVICE_COMMON_NS::MsgFlagsType::KEY_IN_BASE64_FLAG;
        info._opcodeName = "ClientHeartbeatReq";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ClientHeartbeatReqFactory::CreateFactory()));
    }

    {// ClientHeartbeatRes
        auto info = OpcodeInfo();
        info._opcode = 30;
        info._noLog = true;
        info._enableStorage = false;
        info._opcodeName = "ClientHeartbeatRes";
        info._protoFile = "heartbeat.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ClientHeartbeatResFactory::CreateFactory()));
    }

    {// LogoutReq
        auto info = OpcodeInfo();
        info._opcode = 31;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "LogoutReq";
        info._protoFile = "login.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LogoutReqFactory::CreateFactory()));
    }

    {// LogoutNty
        auto info = OpcodeInfo();
        info._opcode = 32;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "LogoutNty";
        info._protoFile = "login.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LogoutNtyFactory::CreateFactory()));
    }

    {// UserClientInfoNty
        auto info = OpcodeInfo();
        info._opcode = 37;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "UserClientInfoNty";
        info._protoFile = "user.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, UserClientInfoNtyFactory::CreateFactory()));
    }

    {// GetLibraryInfoReq
        auto info = OpcodeInfo();
        info._opcode = 42;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetLibraryInfoReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetLibraryInfoReqFactory::CreateFactory()));
    }

    {// GetLibraryInfoRes
        auto info = OpcodeInfo();
        info._opcode = 43;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetLibraryInfoRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetLibraryInfoResFactory::CreateFactory()));
    }

    {// CreateLibraryReq
        auto info = OpcodeInfo();
        info._opcode = 44;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "CreateLibraryReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, CreateLibraryReqFactory::CreateFactory()));
    }

    {// CreateLibraryRes
        auto info = OpcodeInfo();
        info._opcode = 45;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "CreateLibraryRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, CreateLibraryResFactory::CreateFactory()));
    }

    {// LibraryInfoNty
        auto info = OpcodeInfo();
        info._opcode = 46;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "LibraryInfoNty";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LibraryInfoNtyFactory::CreateFactory()));
    }

    {// JoinLibraryReq
        auto info = OpcodeInfo();
        info._opcode = 47;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "JoinLibraryReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, JoinLibraryReqFactory::CreateFactory()));
    }

    {// JoinLibraryRes
        auto info = OpcodeInfo();
        info._opcode = 48;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "JoinLibraryRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, JoinLibraryResFactory::CreateFactory()));
    }

    {// GetLibraryListReq
        auto info = OpcodeInfo();
        info._opcode = 49;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetLibraryListReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetLibraryListReqFactory::CreateFactory()));
    }

    {// GetLibraryListRes
        auto info = OpcodeInfo();
        info._opcode = 50;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetLibraryListRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetLibraryListResFactory::CreateFactory()));
    }

    {// QuitLibraryReq
        auto info = OpcodeInfo();
        info._opcode = 51;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "QuitLibraryReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, QuitLibraryReqFactory::CreateFactory()));
    }

    {// QuitLibraryRes
        auto info = OpcodeInfo();
        info._opcode = 52;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "QuitLibraryRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, QuitLibraryResFactory::CreateFactory()));
    }

    {// TransferLibraianReq
        auto info = OpcodeInfo();
        info._opcode = 53;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "TransferLibraianReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TransferLibraianReqFactory::CreateFactory()));
    }

    {// TransferLibraianRes
        auto info = OpcodeInfo();
        info._opcode = 54;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "TransferLibraianRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TransferLibraianResFactory::CreateFactory()));
    }

    {// ModifyMemberInfoReq
        auto info = OpcodeInfo();
        info._opcode = 55;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ModifyMemberInfoReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ModifyMemberInfoReqFactory::CreateFactory()));
    }

    {// ModifyMemberInfoRes
        auto info = OpcodeInfo();
        info._opcode = 56;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ModifyMemberInfoRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ModifyMemberInfoResFactory::CreateFactory()));
    }

    {// UserLibraryInfoNty
        auto info = OpcodeInfo();
        info._opcode = 57;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "UserLibraryInfoNty";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, UserLibraryInfoNtyFactory::CreateFactory()));
    }

    {// ModifyUserInfoReq
        auto info = OpcodeInfo();
        info._opcode = 58;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ModifyUserInfoReq";
        info._protoFile = "user.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ModifyUserInfoReqFactory::CreateFactory()));
    }

    {// ModifyUserInfoRes
        auto info = OpcodeInfo();
        info._opcode = 59;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ModifyUserInfoRes";
        info._protoFile = "user.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ModifyUserInfoResFactory::CreateFactory()));
    }

    {// LoginFinishReq
        auto info = OpcodeInfo();
        info._opcode = 60;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "LoginFinishReq";
        info._protoFile = "login.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LoginFinishReqFactory::CreateFactory()));
    }

    {// LoginFinishRes
        auto info = OpcodeInfo();
        info._opcode = 61;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "LoginFinishRes";
        info._protoFile = "login.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, LoginFinishResFactory::CreateFactory()));
    }

    {// GetLibraryMemberSimpleInfoReq
        auto info = OpcodeInfo();
        info._opcode = 62;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetLibraryMemberSimpleInfoReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetLibraryMemberSimpleInfoReqFactory::CreateFactory()));
    }

    {// GetLibraryMemberSimpleInfoRes
        auto info = OpcodeInfo();
        info._opcode = 63;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetLibraryMemberSimpleInfoRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetLibraryMemberSimpleInfoResFactory::CreateFactory()));
    }

    {// AddLibraryBookReq
        auto info = OpcodeInfo();
        info._opcode = 64;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "AddLibraryBookReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, AddLibraryBookReqFactory::CreateFactory()));
    }

    {// AddLibraryBookRes
        auto info = OpcodeInfo();
        info._opcode = 65;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "AddLibraryBookRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, AddLibraryBookResFactory::CreateFactory()));
    }

    {// AddLibraryBookCountReq
        auto info = OpcodeInfo();
        info._opcode = 66;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "AddLibraryBookCountReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, AddLibraryBookCountReqFactory::CreateFactory()));
    }

    {// AddLibraryBookCountRes
        auto info = OpcodeInfo();
        info._opcode = 67;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "AddLibraryBookCountRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, AddLibraryBookCountResFactory::CreateFactory()));
    }

    {// GetBookListReq
        auto info = OpcodeInfo();
        info._opcode = 68;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookListReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookListReqFactory::CreateFactory()));
    }

    {// BookListNty
        auto info = OpcodeInfo();
        info._opcode = 69;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "BookListNty";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, BookListNtyFactory::CreateFactory()));
    }

    {// BooksChangeNty
        auto info = OpcodeInfo();
        info._opcode = 70;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "BooksChangeNty";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, BooksChangeNtyFactory::CreateFactory()));
    }

    {// GetBookListRes
        auto info = OpcodeInfo();
        info._opcode = 72;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookListRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookListResFactory::CreateFactory()));
    }

    {// BookVariantInfoItemsNty
        auto info = OpcodeInfo();
        info._opcode = 73;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "BookVariantInfoItemsNty";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, BookVariantInfoItemsNtyFactory::CreateFactory()));
    }

    {// GetBookInfoReq
        auto info = OpcodeInfo();
        info._opcode = 74;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookInfoReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookInfoReqFactory::CreateFactory()));
    }

    {// GetBookInfoRes
        auto info = OpcodeInfo();
        info._opcode = 75;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookInfoRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookInfoResFactory::CreateFactory()));
    }

    {// GetBookByBookNameReq
        auto info = OpcodeInfo();
        info._opcode = 76;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookByBookNameReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookByBookNameReqFactory::CreateFactory()));
    }

    {// GetBookByBookNameRes
        auto info = OpcodeInfo();
        info._opcode = 77;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookByBookNameRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookByBookNameResFactory::CreateFactory()));
    }

    {// BookBagInfoReq
        auto info = OpcodeInfo();
        info._opcode = 80;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "BookBagInfoReq";
        info._protoFile = "bookbag.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, BookBagInfoReqFactory::CreateFactory()));
    }

    {// BookBagInfoNty
        auto info = OpcodeInfo();
        info._opcode = 81;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "BookBagInfoNty";
        info._protoFile = "bookbag.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, BookBagInfoNtyFactory::CreateFactory()));
    }

    {// BookBagInfoRes
        auto info = OpcodeInfo();
        info._opcode = 82;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "BookBagInfoRes";
        info._protoFile = "bookbag.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, BookBagInfoResFactory::CreateFactory()));
    }

    {// SetBookBagInfoReq
        auto info = OpcodeInfo();
        info._opcode = 83;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "SetBookBagInfoReq";
        info._protoFile = "bookbag.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, SetBookBagInfoReqFactory::CreateFactory()));
    }

    {// SetBookBagInfoRes
        auto info = OpcodeInfo();
        info._opcode = 84;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "SetBookBagInfoRes";
        info._protoFile = "bookbag.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, SetBookBagInfoResFactory::CreateFactory()));
    }

    {// GetBookInfoListReq
        auto info = OpcodeInfo();
        info._opcode = 85;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookInfoListReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookInfoListReqFactory::CreateFactory()));
    }

    {// GetBookInfoListRes
        auto info = OpcodeInfo();
        info._opcode = 86;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookInfoListRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookInfoListResFactory::CreateFactory()));
    }

    {// SubmitBookBagBorrowInfoReq
        auto info = OpcodeInfo();
        info._opcode = 89;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "SubmitBookBagBorrowInfoReq";
        info._protoFile = "bookbag.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, SubmitBookBagBorrowInfoReqFactory::CreateFactory()));
    }

    {// SubmitBookBagBorrowInfoRes
        auto info = OpcodeInfo();
        info._opcode = 90;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "SubmitBookBagBorrowInfoRes";
        info._protoFile = "bookbag.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, SubmitBookBagBorrowInfoResFactory::CreateFactory()));
    }

    {// UserNotifyDataNty
        auto info = OpcodeInfo();
        info._opcode = 91;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "UserNotifyDataNty";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, UserNotifyDataNtyFactory::CreateFactory()));
    }

    {// AddUserNotifyDataItemNty
        auto info = OpcodeInfo();
        info._opcode = 92;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "AddUserNotifyDataItemNty";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, AddUserNotifyDataItemNtyFactory::CreateFactory()));
    }

    {// RemoveUserNotifyDataItemNty
        auto info = OpcodeInfo();
        info._opcode = 93;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "RemoveUserNotifyDataItemNty";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, RemoveUserNotifyDataItemNtyFactory::CreateFactory()));
    }

    {// UserNotifyChangeNty
        auto info = OpcodeInfo();
        info._opcode = 94;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "UserNotifyChangeNty";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, UserNotifyChangeNtyFactory::CreateFactory()));
    }

    {// ReadNotifyReq
        auto info = OpcodeInfo();
        info._opcode = 95;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ReadNotifyReq";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ReadNotifyReqFactory::CreateFactory()));
    }

    {// ReadNotifyRes
        auto info = OpcodeInfo();
        info._opcode = 96;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ReadNotifyRes";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ReadNotifyResFactory::CreateFactory()));
    }

    {// GetBookOrderDetailInfoReq
        auto info = OpcodeInfo();
        info._opcode = 97;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookOrderDetailInfoReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookOrderDetailInfoReqFactory::CreateFactory()));
    }

    {// GetBookOrderDetailInfoNty
        auto info = OpcodeInfo();
        info._opcode = 98;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookOrderDetailInfoNty";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookOrderDetailInfoNtyFactory::CreateFactory()));
    }

    {// GetBookOrderDetailInfoRes
        auto info = OpcodeInfo();
        info._opcode = 100;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "GetBookOrderDetailInfoRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, GetBookOrderDetailInfoResFactory::CreateFactory()));
    }

    {// OutStoreOrderReq
        auto info = OpcodeInfo();
        info._opcode = 101;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "OutStoreOrderReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, OutStoreOrderReqFactory::CreateFactory()));
    }

    {// OutStoreOrderRes
        auto info = OpcodeInfo();
        info._opcode = 102;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "OutStoreOrderRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, OutStoreOrderResFactory::CreateFactory()));
    }

    {// ManagerScanOrderForUserGettingBooksReq
        auto info = OpcodeInfo();
        info._opcode = 103;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ManagerScanOrderForUserGettingBooksReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ManagerScanOrderForUserGettingBooksReqFactory::CreateFactory()));
    }

    {// UserGetBooksOrderConfirmNty
        auto info = OpcodeInfo();
        info._opcode = 104;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "UserGetBooksOrderConfirmNty";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, UserGetBooksOrderConfirmNtyFactory::CreateFactory()));
    }

    {// ManagerScanOrderForUserGettingBooksRes
        auto info = OpcodeInfo();
        info._opcode = 105;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ManagerScanOrderForUserGettingBooksRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ManagerScanOrderForUserGettingBooksResFactory::CreateFactory()));
    }

    {// UserGetBooksOrderConfirmReq
        auto info = OpcodeInfo();
        info._opcode = 106;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "UserGetBooksOrderConfirmReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, UserGetBooksOrderConfirmReqFactory::CreateFactory()));
    }

    {// UserGetBooksOrderConfirmRes
        auto info = OpcodeInfo();
        info._opcode = 107;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "UserGetBooksOrderConfirmRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, UserGetBooksOrderConfirmResFactory::CreateFactory()));
    }

    {// CancelOrderReq
        auto info = OpcodeInfo();
        info._opcode = 108;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "CancelOrderReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, CancelOrderReqFactory::CreateFactory()));
    }

    {// CancelOrderRes
        auto info = OpcodeInfo();
        info._opcode = 109;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "CancelOrderRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, CancelOrderResFactory::CreateFactory()));
    }

    {// OnekeyClearNotifyReq
        auto info = OpcodeInfo();
        info._opcode = 110;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "OnekeyClearNotifyReq";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, OnekeyClearNotifyReqFactory::CreateFactory()));
    }

    {// OnekeyClearNotifyRes
        auto info = OpcodeInfo();
        info._opcode = 111;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "OnekeyClearNotifyRes";
        info._protoFile = "notify.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, OnekeyClearNotifyResFactory::CreateFactory()));
    }

    {// ReturnBackReq
        auto info = OpcodeInfo();
        info._opcode = 112;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ReturnBackReq";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ReturnBackReqFactory::CreateFactory()));
    }

    {// ReturnBackRes
        auto info = OpcodeInfo();
        info._opcode = 113;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "ReturnBackRes";
        info._protoFile = "library.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, ReturnBackResFactory::CreateFactory()));
    }

    {// SystemLogDataListReq
        auto info = OpcodeInfo();
        info._opcode = 114;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "SystemLogDataListReq";
        info._protoFile = "syslog.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, SystemLogDataListReqFactory::CreateFactory()));
    }

    {// SystemLogDataListRes
        auto info = OpcodeInfo();
        info._opcode = 115;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "SystemLogDataListRes";
        info._protoFile = "syslog.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, SystemLogDataListResFactory::CreateFactory()));
    }

    {// TestRpcReq
        auto info = OpcodeInfo();
        info._opcode = 116;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "TestRpcReq";
        info._protoFile = "test_opcode.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TestRpcReqFactory::CreateFactory()));
    }

    {// TestRpcRes
        auto info = OpcodeInfo();
        info._opcode = 117;
        info._noLog = false;
        info._enableStorage = false;
        info._opcodeName = "TestRpcRes";
        info._protoFile = "test_opcode.proto";
        _allOpcodeInfo.push_back(info);
        _opcodeRefCoderFactory.insert(std::make_pair(info._opcode, TestRpcResFactory::CreateFactory()));
    }
