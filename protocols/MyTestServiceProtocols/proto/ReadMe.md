## Proto规则

1. 工具的生成规则在GenProto.lua中，方便动态改下规则并生效

2. 注释解析需要使用/// 来用于某条协议的解析参数, /// 后是要解析的内容 /

   // 必须前面没有任何的有效字符 ，特殊字符会被自动trip掉：“\t\v\r\n\f”，包括空格

   要解析的参数格式：@param1=xxx, 每个参数逗号隔开（参数也可以换行写，如果觉得一行过长，但前缀必须是///）， 例如：

   ```protobuf
   /// @enable = true, @opcode = 1022, @from = cli, @to = gs, @cdms = 500, @countincd = 1
   message TestProtoReq
   {
   	string content = 0;	// content
   }
   ```

   

3. 格式参数列表

   ```protobuf
   @enable = true // 启用: 可缺省，默认值是true，除非要禁用，请显示的填写false
   @opcode = xxx // 协议id: 可缺省，默认是工具自动生成，且不会乱序，原先是什么的还是什么
   @from = xxx // 协议发起方: 可缺省, 默认值是cli(client的缩写), 客户端可以依据这个来收集协议并注册回调，服务器除非内部节点需要填写，
   @to = xxx // 协议到达方: 缺省，默认值是gs, 可以根据需要，填写相应的到达方，
   
   // 客户端方缩写:cli
   // 服务器内部节点为(需要维护)：gs(gameserver 玩家服), gw(GateWay, 网关), ls(LoginServer, 登陆验证), ws(WorldServer，世界服)，cs(CenterServer, 中心服)，ss(SceneServer, 场景服)，ds(DbServer, 数据库代理服)，ps(ProxyServer, 服务器组代理服)
   
   @cdms: // 协议请求的cd时间（单位毫秒）:，默认cli是500ms, 其他不限制
   @countincd: // 协议cd时间内允许的请求次数：默认cli是1次，比如cd是500ms，countincd：1，也就是说每500ms内允许客户端请求的最多次数是1次，超过次数是不处理，且丢弃该请求，
   
   ```

4. 生成顺序

   * 调用GenProto.check(lua中)来校验所有proto的格式以及是否冲突等
   * 调用proto的生成程序执行生成 c#, cpp相应的协议结构
   * 生成完成则调用GenProto.GenOpcodes 来生成 Opcodes.h, Opcodes.cpp, AllPbs.h 用于cpp，生成出来的结果会生成一份OpcodeDict.gen用于保存已经生成的opcode信息，用于下次生成Opcodes.h等，若要进行全新生成，请删除OpcodeDict.gen

