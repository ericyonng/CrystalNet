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


export namespace crystal_net_service
{

    // com_book.proto
    export enum BookType_ENUMS {
        UNKNOWN = 0,

    }
    // com_client_user.proto
    export enum ClientUserStatus_ENUMS {
        // 未登录
        UNLOGIN = 0,
        // 登录中
        LOGINING = 1,
        // 登录完成
        LOGINED = 2,
        // 登录结束
        CLIENT_LOGIN_ENDING = 3,
        // 登陆结束的结束
        CLIENT_LOGIN_ENDING_FINISH = 4,
        // 登出中(定时15秒切换状态)
        LOGOUTING = 5,
        // 登出完成
        LOGOUTED = 6,

    }
    // com.proto
    export enum CreatureAttrKey_ENUM {
        BEGIN = 0,
        Atk = 1,
        Def = 2,
        HpLmt = 3,
        HpRcv = 4,
        Cure = 5,
        AngerLmt = 6,
        AngerRcv = 7,
        Anger = 8,
        Hp = 9,
        MoveSpd = 10,
        ActSpd = 11,
        MMoveSpd = 12,
        MActSpd = 13,
        ExtraHp = 14,
        VMoveSpd = 15,
        //VActSpd = 16; // 载具动画速率
        BuffHp = 17,
        BuffHpLmt = 18,
        BuffHpBloodSpeed = 19,
        Crit = 101,
        Res = 102,
        Hit = 103,
        Prr = 104,
        Ddg = 105,
        Crid = 106,
        Crrd = 107,
        Critv = 108,
        Resv = 109,
        Hitv = 110,
        Prrv = 111,
        Ddgv = 112,
        AtkAdd = 151,
        DefAdd = 152,
        HpLmtAdd = 153,
        Pedm = 201,
        Prdm = 202,
        Medm = 203,
        Mrdm = 204,
        Sedm = 205,
        Srdm = 206,
        Ardm = 210,
        Lv = 251,
        AwakeLv = 252,
        ExpLmt = 253,
        Exp = 254,
        ExpMul = 255,
        ExpAdd = 256,
        GoldMul = 257,
        DoubleExp = 258,
        VipLv = 270,
        Forces = 299,

    }
    // com_user.proto
    export enum LoginMode_ENUMS {
        // 使用密码登录
        PASSWORD = 0,
        // 使用登录令牌登录
        USE_LOGIN_TOKEN = 1,
        // 注册登录
        REGISTER = 2,

    }
    // com_user.proto
    export enum LogoutReason_ENUMS {
        // 其他地方登录
        LOGIN_OTHER_PLACE = 0,
        // 其他原因
        OTHER_REASON = 1,
        // 玩家主动退登
        USER_LOGOUT = 2,
        // 空闲
        USER_IDLE = 3,
        // 超时
        TIMEOUT = 4,
        // 关服
        CLOSE_SERVER = 5,
        // 登录其他账号
        LOG_IN_OTHER_ACCOUNT = 6,

    }
    // com_heartbeat.proto
    export enum NODE_IPTYPE_TYPE_ENUMS {
        IPV4 = 0,
        IPV6 = 1,

    }
    // com_library.proto
    export enum RoleType_ENUMS {
        // 无权限
        NoAuth = 0,
        // 普通成员
        NormalMember = 1,
        // 管理员
        Manager = 2,
        // 馆长
        Librarian = 3,

    }
    // 书籍信息, 数据存储:id, isbn(unique), bookInfo json
    // com_book.proto
    export class BookInfo {
        // 唯一id, 同一个isbn, 同一个id
        Id:number = 0;

        // 图书类型 BookType
        BookType:number = 0;

        // 书名
        BookName:string = "";

        // 书isbn码
        IsbnCode:string = "";

        // 封面
        BookCoverImage:string = "";

        // 封面图片类型
        BookCoverImageType:string = "";

        // 价格(单位分)
        Price:number = 0;

        // 是否上架
        IsOnShelves:number = 0;

        // 库存
        Count:number = 0;

        // 被借数量
        BorrowedCount:number = 0;


    }
    // 图书类型
    // com_book.proto
    export class BookType {

    }
    // com_library.proto
    export class BorrowBookInfo {
        // book id
        BookId:number = 0;

        // isbn码
        IsbnCode:string = "";

        // 借阅数量
        BorrowCount:number = 0;

        // 借阅时间
        BorrowTime:number = 0;

        // 预计归还时间
        PlanGiveBackTime:number = 0;

        // 实际归还时间
        RealGiveBackTime:number = 0;


    }
    // 借还订单信息
    // com_library.proto
    export class BorrowOrderInfo {
        // 订单号
        OrderId:number = 0;

        // 借的书
        BorrowBookList:BorrowBookInfo[] = [];

        // 订单创建时间
        CreateOrderTime:number = 0;


    }
    // 客户端的心跳
    /// Opcode:, NoLog:true, XorEncrypt:true, KeyBase64:true
    // heartbeat.proto
    export class ClientHeartbeatReq {

    }
    /// Opcode:, NoLog:true
    // heartbeat.proto
    export class ClientHeartbeatRes {
        // 服务器时间
        ServerTimeMs:number = 0;


    }
    // com_client_user.proto
    export class ClientUserInfo {
        // uid
        UserId:number = 0;

        // 账号
        AccountName:string = "";

        // 姓名
        Name:string = "";

        // 昵称
        Nickname:string = "";

        // 手机设备imei
        PhoneImei:string = "";

        // 登录状态 ClientUserStatus
        ClientStatus:number = 0;

        // token
        LastToken:string = "";

        // token过期时间
        TokenExpireTime:number = 0;

        // 绑定手机
        BindPhone:number = 0;


    }
    // com_client_user.proto
    export class ClientUserStatus {

    }
    /// Opcode:
    // library.proto
    export class CreateLibraryReq {
        // 邀请码
        InviteCode:string = "";

        // 管名
        Name:string = "";

        // 书馆地址
        Address:string = "";

        // 书馆经营时间
        OpenTime:string = "";

        // 书馆联系电话
        TelphoneNumber:string = "";

        // 必须绑定手机号
        BindPhone:number = 0;


    }
    /// Opcode:
    // library.proto
    export class CreateLibraryRes {
        ErrCode:number = 0;


    }
    // 属性类型枚举
    // com.proto
    export class CreatureAttrKey {

    }
    /// Opcode:
    // library.proto
    export class GetLibraryInfoReq {

    }
    /// Opcode:
    // library.proto
    export class GetLibraryInfoRes {
        ErrCode:number = 0;


    }
    // 获取图书馆列表
    /// Opcode:
    // library.proto
    export class GetLibraryListReq {

    }
    // 获取图书馆列表
    /// Opcode:
    // library.proto
    export class GetLibraryListRes {
        LibraryPreviewInfoList:LibraryPreviewInfo[] = [];


    }
    /// Opcode:
    // heartbeat.proto
    export class GetNodeListReq {
        // 集群名
        ClusterName:string = "";

        // 关心的节点服务名列表
        CareNodeServiceNameList:string[] = [];


    }
    /// Opcode:
    // heartbeat.proto
    export class GetNodeListRes {
        NodeInfoList:NodeHeartbeatInfo[] = [];


    }
    // 加入图书馆: 不在图书馆内,
    /// Opcode:
    // library.proto
    export class JoinLibraryReq {
        LibraryId:number = 0;


    }
    // 加入图书馆:刚开始是无权限的, 只有让管理员添加成为会员才有权限借阅
    /// Opcode:
    // library.proto
    export class JoinLibraryRes {
        ErrCode:number = 0;


    }
    // 管理员信息(包含馆长)
    // com_library.proto
    export class LibararyManagerInfo {
        // 管理员id
        UserId:number = 0;


    }
    // com_library.proto
    export class LibraryInfo {
        // 唯一id
        Id:number = 0;

        // 图书馆名
        Name:string = "";

        // 图书馆地址
        Address:string = "";

        // 图书馆经营时间
        OpenTime:string = "";

        // 图书馆联系电话
        TelphoneNumber:string = "";

        // 馆长id
        LibrarianUserId:number = 0;

        // 馆长名
        LibrarianUserNickname:string = "";

        // 管理员列表
        ManagerInfoList:LibararyManagerInfo[] = [];

        // 会员列表
        MemberList:MemberInfo[] = [];


    }
    // 图书馆信息, 登录成功后主动推送
    /// Opcode:
    // library.proto
    export class LibraryInfoNty {
        LibraryInfo:LibraryInfo = new LibraryInfo();


    }
    // 图书馆预览数据
    // com_library.proto
    export class LibraryPreviewInfo {
        // 图书馆id
        Id:number = 0;

        // 图书馆名
        Name:string = "";

        // 馆长id
        LibrarianUserId:number = 0;

        // 馆长昵称
        LibrarianNickname:string = "";


    }
    /// Opcode:
    // login.proto
    export class LoginFinishReq {

    }
    /// Opcode:
    // login.proto
    export class LoginFinishRes {
        ErrCode:number = 0;


    }
    // 登录信息
    // com_user.proto
    export class LoginInfo {
        // 登录模式
        LoginMode:number = 0;

        // 账号
        AccountName:string = "";

        // 密码
        Pwd:string = "";

        // 登录令牌
        LoginToken:string = "";

        // 登录设备
        LoginPhoneImei:string = "";

        // 登录目标ip
        TargetIp:string = "";

        // 登录目标端口
        Port:number = 0;

        UserRegisterInfo?:RegisterUserInfo;

        // 鉴权:ip是黑白校验, 设备黑白校验, 账号黑白校验
        // 产品id
        AppId:string = "";

        // 密文
        cypherText:string = "";

        // 原文
        originText:string = "";

        // 版本号
        versionId:number = 0;


    }
    /// Opcode:
    // login.proto
    export class LoginInfoNty {
        // 更新登录token
        Token:string = "";

        // 过期时间(秒级时间戳)
        KeyExpireTime:number = 0;


    }
    // 登录模式
    // com_user.proto
    export class LoginMode {

    }
    /// Opcode:, XorEncrypt:true, KeyBase64:true
    // login.proto
    export class LoginReq {
        LoginUserInfo:LoginInfo = new LoginInfo();


    }
    /// Opcode:
    // login.proto
    export class LoginRes {
        errCode:number = 0;

        UserId:number = 0;

        // 毫秒级
        ServerTime:number = 0;


    }
    /// Opcode:
    // login.proto
    export class LogoutNty {
        // LogoutReason
        LogoutReason:number = 0;

        // 告知最后登陆的ip
        ip:string = "";


    }
    // com_user.proto
    export class LogoutReason {

    }
    /// Opcode:
    // login.proto
    export class LogoutReq {

    }
    // 会员信息(包括管理员等)
    // com_library.proto
    export class MemberInfo {
        // user id
        UserId:number = 0;

        // 角色 RoleType
        Role:number = 0;

        // 会员昵称
        Nickname:string = "";

        // 当前借阅图书列表 只有有权限的人才会看到订单详情, 其他人只能看自己的
        BorrowList:BorrowOrderInfo[] = [];

        // 锁定操作超时时间
        LockTimestampMs:number = 0;


    }
    // 修改权限 馆长不可修改自己的权限, 只能转让, 管理员不可以将自己升级, 只能管理员修改权限 TODO:
    /// Opcode:
    // library.proto
    export class ModifyMemberInfoReq {
        memberUserId:number = 0;

        newRole?:number;

        newMemberPhone?:number;


    }
    /// Opcode:
    // library.proto
    export class ModifyMemberInfoRes {
        ErrCode:number = 0;


    }
    /// Opcode:
    // player.proto
    export class ModifyPlayerNameReq {
        newName:string = "";


    }
    /// Opcode:
    // player.proto
    export class ModifyPlayerNameRes {
        errCode:number = 0;


    }
    // 修改密码
    // user.proto
    export class ModifyPwdInfo {
        OldPwd:string = "";

        NewPwd:string = "";


    }
    // 修改用户信息 TODO:
    /// Opcode:
    // user.proto
    export class ModifyUserInfoReq {
        // 修改密码
        PwdInfo?:ModifyPwdInfo;

        // 修改昵称
        Nickname?:string;


    }
    // 修改用户信息
    /// Opcode:
    // user.proto
    export class ModifyUserInfoRes {
        ErrCode:number = 0;


    }
    // com_heartbeat.proto
    export class NODE_IPTYPE {

    }
    // 服务器节点间的心跳信息
    // com_heartbeat.proto
    export class NodeHeartbeatInfo {
        // 服务名
        ServiceName:string = "";

        // ip
        address:string = "";

        // ip类型
        IpType:number = 0;

        // 开放的互联端口
        InnerLinkPort:number = 0;

        // 监听的协议列表
        SubscribeOpcodes:number[] = [];

        // api列表
        ApiList:string[] = [];


    }
    /// Opcode:, NoLog:true
    // heartbeat.proto
    export class NodeHeartbeatReq {

    }
    /// Opcode:, NoLog:true
    // heartbeat.proto
    export class NodeHeartbeatRes {
        // 当前服务器对时
        NowTimeNanoseconds:number = 0;


    }
    // com_passtime.proto
    export class PassTimeData {
        // 上次跨天时间
        LastPassDayTime:number = 0;


    }
    // com_player.proto
    export class PlayerData {
        account:string = "";

        playerId:number = 0;

        sex:number = 0;

        name:string = "";


    }
    /// Opcode:
    // player.proto
    export class PlayerDataNty {
        playerData:PlayerData = new PlayerData();


    }
    /// Opcode:
    // player.proto
    export class PlayerDataReq {
        account:string = "";

        pwd:string = "";


    }
    /// Opcode:
    // player.proto
    export class PlayerDataRes {
        errCode:number = 0;

        playerData:PlayerData = new PlayerData();

        loginToken:string = "";


    }
    // 退出: 必须把书还完
    /// Opcode:
    // library.proto
    export class QuitLibraryReq {

    }
    // 退出
    /// Opcode:
    // library.proto
    export class QuitLibraryRes {
        // 错误码
        ErrCode:number = 0;


    }
    // 注册节点信息
    /// Opcode:
    // heartbeat.proto
    export class RegisterNodeReq {
        NodeInfo:NodeHeartbeatInfo = new NodeHeartbeatInfo();


    }
    // 注册节点信息
    /// Opcode:
    // heartbeat.proto
    export class RegisterNodeRes {
        ErrCode:number = 0;


    }
    // com_user.proto
    export class RegisterUserInfo {
        // 账号
        AccountName:string = "";

        // 昵称
        Nickname:string = "";

        // 密码
        Pwd:string = "";

        // 创建的收集imei
        CreatePhoneImei:string = "";


    }
    // 按照权限大小排
    // com_library.proto
    export class RoleType {

    }
    // 表结构信息
    // com_system_table.proto
    export class SimpleInfo {
        // 当前自增id最大值（配合id池）
        MaxIncId:number = 0;

        // 标脏次数
        DirtyCount:number = 0;

        // 用于清档, 版本号和当前配置版本号不同则清档
        VersionNo:number = 0;


    }
    // comp_test.proto
    export class TestMgrData {
        Account:string = "";

        TestId:number = 0;


    }
    /// Opcode:
    // test_opcode.proto
    export class TestOpcode2Req {
        id:number = 0;

        content:string = "";


    }
    /// Opcode:
    // test_opcode.proto
    export class TestOpcode2Res {
        id_info:number = 0;

        testInfo:TestOpcodeInfo = new TestOpcodeInfo();


    }
    // test_opcode.proto
    export class TestOpcodeInfo {
        errCode:number = 0;

        errMsg:string = "";


    }
    // bytes 数据结构转json的时候会转成base64编码
    /// Opcode:
    // test_opcode.proto
    export class TestOpcodeReq {
        content:string = "";

        TestId:number = 0;


    }
    /// Opcode:
    // test_opcode.proto
    export class TestOpcodeRes {
        content:string = "";

        TestId:number = 0;


    }
    // com_title.proto
    export class TitleInfo {
        titleCfgId:number = 0;

        expiredTs:number = 0;


    }
    /// Opcode:
    // title.proto
    export class TitleInfoReq {

    }
    /// Opcode:
    // title.proto
    export class TitleInfoRes {
        errCode:number = 0;

        titleList:TitleInfo[] = [];


    }
    // 转让馆长:只能转让给管理员, 转让后降级成管理员 TODO:
    /// Opcode:
    // library.proto
    export class TransferLibraianReq {
        TargetUserId:number = 0;


    }
    // 转让馆长
    /// Opcode:
    // library.proto
    export class TransferLibraianRes {
        ErrCode:number = 0;


    }
    // com_user.proto
    export class UserBaseInfo {
        // uid
        UserId:number = 0;

        // 账号
        AccountName:string = "";

        // 姓名
        Name:string = "";

        // 昵称
        Nickname:string = "";

        // 密码
        Pwd:string = "";

        // 盐
        PwdSalt:string = "";

        // 绑定手机
        BindPhone:number = 0;

        // 最后登录时间
        LastLoginTime:number = 0;

        // 最后登录ip
        LastLoginIp:string = "";

        // 最后登录收集imei
        LastLoginPhoneImei:string = "";

        // 创建号的ip
        CreateIp:string = "";

        // 创建时间
        CreateTime:number = 0;

        // 创建的收集imei
        CreatePhoneImei:string = "";

        // 绑定的邮箱地址
        BindMailAddr:string = "";

        // 上次跨天时间
        LastPassDayTime:number = 0;


    }
    /// Opcode:
    // user.proto
    export class UserClientInfoNty {
        ClientInfo:ClientUserInfo = new ClientUserInfo();


    }
    // 玩家的图书馆数据
    // com_library.proto
    export class UserLibraryInfo {
        // 图书馆id
        LibraryId:number = 0;


    }
    // 用户的图书馆简要信息
    /// Opcode:
    // library.proto
    export class UserLibraryInfoNty {
        UserLibraryInfo:UserLibraryInfo = new UserLibraryInfo();


    }
    // com_login.proto
    export class UserLoginInfo {
        // 生成的token = sha1(imei + ip + userid + key)
        Token:string = "";

        // 生成的随机密钥(8位随机数或字符)
        Key:string = "";

        // 密钥过期时间 精确到秒
        KeyExpireTime:number = 0;


    }

}