* 环境

  * 安装npm工具（和nodejs一起安装的https://nodejs.org/zh-cn/download）

  * 使用npm安装typescript

    * ```
      npm config set registry https://registry.npmmirror.com
      ```

    * ```
      npm install -g typescript
      ```

    * 安装完成后我们可以使用 **tsc** 命令来执行 TypeScript 的相关代码，以下是查看版本号：

    * ```
      tsc -v
      ```

    * ```
      tsc app.ts
      
      # 有些需要指定版本下才能使用，比如Map需要es6
      tsc --target es6 .\helloworld.ts
      ```



​				通常我们使用 **.ts** 作为 TypeScript 代码文件的扩展名。

​			然后执行以下命令将 TypeScript 转换为 JavaScript 代码：

```
	tsc app.ts
```

![img](https://www.runoob.com/wp-content/uploads/2019/01/typescript_compiler.png)

这时候在当前目录下（与 app.ts 同一目录）就会生成一个 app.js 文件，代码如下：

var message = "Hello World"; console.log(message);

使用 node 命令来执行 app.js 文件：

```
$ node app.js 
Hello World
```

TypeScript 转换为 JavaScript 过程如下图：

![img](https://www.runoob.com/wp-content/uploads/2019/01/ts-2020-12-01-1.png)



* vscode解决运行脚本问题:

  您可以通过将此配置添加到此[issue](https://github.com/microsoft/vscode-python/issues/2559#issuecomment-478381840)建议的settings.json文件中来解决此问题

  通过转到您的settings.json (在windows上)：

  ```
  %APPDATA%\Code\User\settings.json
  ```

  或

  ```
  C:\Users\<your user>\AppData\Roaming\Code\User\settings.json
  ```

  并在配置中添加此行(如果有更多配置行，请在末尾使用逗号)：

  ```javascript
  {
  "terminal.integrated.shellArgs.windows": ["-ExecutionPolicy", "Bypass"]
  }
  ```

  复制

  一旦你打开另一个终端，这个问题就应该解决了。

* ts number表示的整数部分只有4个字节，超过的是浮点数了，这时候整数的位运算就会出问题，此时需要把number拆分成两个number，通过对32位整形求余得到part1(低32位数据)，对32位整形最大值取整得到高32位number，然后分别对这两部分进行操作

* ts number一旦超过2 的53次方就无法用准确的整数表示，到时候会不准确

* 对于位运算，如果希望进行的是无符号数右移，需要使用>>>位运算符，这时候高位会补0而不是1

* ts 若要支持protobuf的bytes数据类型需要将string转base64才可以

* ts字符编码转成字符串使用String.fromCharCode, 字符串转字符编码使用:String.charCodeAt

* ts模块导入

  * ```
    
    /// <reference path="./message.ts" />
    import {IMessage} from './message'
    ```

* ts模块导出

  * ```
    
    export namespace pb{
    
      export class LoginReq implements IMessage
      {
        
      }
    
    }
    
    ```

    

* 记忆点

  * 接口类的声明和实现

    * ```
      // 接口类型
      interface IApi {
          nameOfApi:string,
          // 方法是一个lambda
          handle: ()=>string
      }
      
      // 实现一个接口就是创建一个对象出来
      var customVar:IApi = {
          nameOfApi:"custom",
          handle: ():string => "hello custom api"
      }
      
      ```

  * 模块导出

    * 需要声明命名空间，并使用export

      * ```
        
        export namespace network{
        
          
        export interface TcpEndian{
          // 初始化
          init(addr:string, port:number):boolean,
          // 启动
          start():boolean,
          // 关闭
          close():void,
        
          // 连接成功
          SetOnConnected(param:()=>void):void,
          // 消息到来
          SetOnMessage(param:(param1:ArrayBuffer, param2:WechatMiniprogram.RemoteInfo, param3:WechatMiniprogram.LocalInfo)=>void):void,
          // 错误
          SetOnError(cb:(errMsg:string)=>void):void,
          // 关闭
          SetOnClose(param:(errMsg:string)=>void):void,
          // 发送数据
          SendData(opcode:number, data:any):void,
          // 接收数据的缓存
          GetRecvStream():LibStream,
        
          // 获取ip
          GetRemoteIp():string,
          // 获取端口
          GetRemotePort():number,
        }
        
        export function createTcpEndian():TcpEndian{
          return new CrystalTcpEndian()
        }
        
        // tcp endian
        export class CrystalTcpEndian implements TcpEndian{
        
            // 地址
            _addr:string = "";
            // 端口
            _port:number = 0;
            // 是否连接
            _isConnected:boolean = false;
            // tcp
            _tcp?:WechatMiniprogram.TCPSocket;
            // 接收缓存
            _recvStream:LibStream = new LibStream(256)
        
            // 回调
            _onConnectCb = ()=>{};
            _onMessageCb = (_:ArrayBuffer, __:WechatMiniprogram.RemoteInfo, ___:WechatMiniprogram.LocalInfo) =>{};
            _onError = (errMsg: string) =>{};
            _onClose = (errMsg: string) =>{};
        
            init(addr:string, port:number):boolean
            {
              this._addr = addr;
              this._port = port;
        
              return true;
            }
        
            start():boolean{
              if(this._isConnected)
                return true;
        
                this._tcp = wx.createTCPSocket();
                this._tcp.connect(
                  {
                    address:this._addr,
                    port:this._port
                  }
                )
        
                this._tcp.onConnect(this.onConnect)
                this._tcp.onMessage(this.onMessage)
                this._tcp.onError(this.onError)
                this._tcp.onClose(this.onClose)
        
                return true;
            }
        
          close():void{
            if(!this._isConnected)
              return
        
              this._tcp?.close()
              this._isConnected = false
          }
        
            // 连接成功
            public SetOnConnected(param:()=>void):void{
              this._onConnectCb = param;
            }
            // 收到消息
            public SetOnMessage(param:(param1:ArrayBuffer, param2:WechatMiniprogram.RemoteInfo, param3:WechatMiniprogram.LocalInfo)=>void):void
            {
              this._onMessageCb = param
            }
            // 错误
            public SetOnError(cb:(errMsg:string)=>void):void
            {
              this._onError = cb;
            }
        
            // 连接断开
            public SetOnClose(param:(errMsg:string)=>void):void
            {
              this._onClose = param;
            }
        
          // 发送数据
          SendData(opcode:number, data:any, packetId:number = -1):void {
            var newStream = ProtocolStack.ToBinary(opcode, data, packetId)
            this._tcp?.write(newStream.getBuffer())
          }
        
          // 接收缓冲区
          GetRecvStream():LibStream{
            return this._recvStream
          }
        
            // 获取ip
            GetRemoteIp():string{return this._addr}
        
            // 获取端口
            GetRemotePort():number{return this._port}
        
            // 连接成功
            private onConnect(_: any):void{
              this._isConnected = true
              this._onConnectCb()
              console.log(this._addr, ":", this._port, " connected.");
            }
        
            // 消息到来
            private onMessage(result: WechatMiniprogram.TCPSocketOnMessageListenerResult):void
            {
              this._onMessageCb(result.message, result.remoteInfo, result.localInfo)
            }
        
            // 错误
            private onError(result: WechatMiniprogram.GeneralCallbackResult):void
            {
              this._onError(result.errMsg)
            }
        
            // 连接断开
            private onClose(param: WechatMiniprogram.GeneralCallbackResult):void
            {
              this._onClose(param.errMsg)
            }
        }
        
        // 消息投结构
        export class MsgHeaderStructure
        {
          // 长度
          public static LEN_START_POS:number = 0;
          public static LEN_SIZE:number = 3;
        
          // 协议版本号
          public static PROTOCOL_VERSION_NUMBER_START_POS:number = MsgHeaderStructure.LEN_START_POS + MsgHeaderStructure.LEN_SIZE;
          public static PROTOCOL_VERSION_SIZE:number = 8;
        
          // flags起始与大小
          public static FLAGS_START_POS:number = MsgHeaderStructure.PROTOCOL_VERSION_NUMBER_START_POS + MsgHeaderStructure.PROTOCOL_VERSION_SIZE;
          public static FLAGS_SIZE:number = 2;
        
          // opcode起始与大小
          public static OPCODE_START_POS:number = MsgHeaderStructure.FLAGS_START_POS + MsgHeaderStructure.FLAGS_SIZE;
          public static OPCODE_SIZE:number = 3;
        
            // packetId的起始与大小
          public static PACKET_ID_START_POS:number = MsgHeaderStructure.OPCODE_START_POS + MsgHeaderStructure.OPCODE_SIZE;
          public static PACKET_ID_SIZE:number = 8;
        
          // 消息头大小 = 长度大小 + 版本号大小 + flags大小 + opcode大小 + packetid大小
          public static MSG_HEADER_SIZE:number = MsgHeaderStructure.LEN_SIZE + MsgHeaderStructure.PROTOCOL_VERSION_SIZE + MsgHeaderStructure.FLAGS_SIZE + MsgHeaderStructure.OPCODE_SIZE + MsgHeaderStructure.PACKET_ID_SIZE;
        
          // 消息体开始与最大大小 可以进行_len的校验
          public static MSG_BODY_START_POS:number = MsgHeaderStructure.MSG_HEADER_SIZE
          public static MSG_BODY_MAX_SIZE_LIMIT:number = 16777215;
        }
        
        // 消息头
        export class MsgHeader
        {
          _len:number = 0;
          _protocolVersion:number = 0;
          _flags:number = 0;
          _opcodeId:number = 0;
          _packetId:number = 0;
        
          constructor(opcode:number)
          {
            this._opcodeId = opcode;
          }
        
        }
        
        // 字节流
        export class LibStream
        {
          // buffer
          _buffer:ArrayBuffer;
          _byteBuffer:Uint8Array;
          _capacity:number = 0;
          // 可写的位置
          _writePos:number = 0;
          // 可读的位置
          _readPos:number = 0;
        
          constructor(len:number = 8)
          {
              this._buffer = new ArrayBuffer(len)
              this._byteBuffer = new Uint8Array(this._buffer)
              this._capacity = len
              this._writePos = 0
              this._readPos = 0
          }
        
          // 获取buffer
          getBuffer():ArrayBuffer { return this._buffer; }
        
          // 获取容量
          getCapacity():number { return this._capacity; }
        
          // 获取写入大小
          getWriteSize():number { return this._writePos; }
        
          // 获取已读大小
          getReadSize():number { return this._readPos; }
        
          // 获取可读大小
          getReadableSize():number{
            return this._writePos - this._readPos;
          }
        
          // 获取可写大小
          getWritableSize():number{
            return this._capacity - this._writePos;
          }
        
          // 移动已写pos
          shiftWritePos(shiftBytes:number):void{
            if(this._writePos + shiftBytes < 0)
            {
              this._writePos = 0
              return;
            }
            if(shiftBytes > this.getWritableSize())
            {
              this._writePos = this._capacity - 1
              return 
            }
        
            this._writePos += shiftBytes;
          }
        
          // 移动已读pos
          shiftReadPos(shiftBytes:number){
            if(this._readPos + shiftBytes < 0)
            {
              this._readPos = 0
              return;
            }
        
            if(shiftBytes > this.getReadableSize())
            {
              this._readPos = this._writePos
              return
            }
        
            this._readPos += shiftBytes
          }
        
          // 重新调整大小
          resize(len:number):void{
            var newBuffer = new ArrayBuffer(len)
            var newByteArray = new Uint8Array(newBuffer)
        
            // 要拷贝数据的长度
            var newLen = len > this._capacity ? this._capacity : len;
            for(var i = 0; i < newLen; ++i)
              newByteArray.fill(this._byteBuffer[i], i, i + 1)
        
            this._capacity = len;
            this._buffer = newBuffer;
            this._byteBuffer = newByteArray;
          }
        
          // 处理buffer
          attach(buffer:ArrayBuffer, readPos:number = 0, writePos:number = 0)
          {
            this._buffer = buffer
            this._byteBuffer = new Uint8Array(this._buffer)
            this._capacity = this._buffer.byteLength
            this._writePos = writePos;
            this._readPos = readPos
          }
        
          attachSream(otherStream:LibStream)
          {
            this._buffer = otherStream._buffer
            this._byteBuffer = otherStream._byteBuffer
            this._capacity = otherStream._capacity
            this._writePos = otherStream._writePos
            this._readPos = otherStream._readPos
          }
        
          // 空间不够的会resize
          forceWrite(data:any, dataSize:number):void
          {
            if(dataSize > this.getWritableSize())
              this.resize(this._capacity + dataSize - this.getWritableSize())
        
              this._byteBuffer.fill(data, this._writePos, this._writePos + dataSize)
              this._writePos += dataSize;
          }
        
          // 写入
          write(data:any, dataSize:number):boolean
          {
            if(dataSize > this.getWritableSize())
            {
              console.log("data size over writable size dataSize:", dataSize)
              return false
            }
        
            this._byteBuffer.fill(data, this._writePos, this._writePos + dataSize)
            this._writePos += dataSize;
            return true;
          }
        
          forceWriteString(data:string):void
          {
            if(data.length > this.getWritableSize())
              this.resize(this._capacity + data.length - this.getWritableSize())
        
              for(var i = 0; i < data.length; ++i)
              {
                this._byteBuffer.fill(data.charCodeAt(i), this._writePos + i, this._writePos + i + 1)
              }
          
              this._writePos += data.length;
          }
        
          writeString(data:string):boolean
          {
            if(data.length > this.getWritableSize())
            {
              console.log("write fail len over writable size data size:", data.length, ", writable size:", this.getWritableSize())
              return false;
            }
        
            for(var i = 0; i < data.length; ++i)
            {
              this._byteBuffer.fill(data.charCodeAt(i), this._writePos + i, this._writePos + i + 1)
            }
        
            this._writePos += data.length;
            return true;
          }
        
          forceWriteBuffer(buffer:ArrayBuffer)
          {
            var byteBuffer = new Uint8Array(buffer)
            if(byteBuffer.length > this.getWritableSize())
              this.resize(this._capacity + byteBuffer.length - this.getWritableSize())
        
            for(var i = 0; i < byteBuffer.length; ++i)
            {
              this._byteBuffer.fill(byteBuffer[i], this._writePos + i, this._writePos + i + 1)
            }
        
            this._writePos += byteBuffer.length
          }
        
          // 读数据
          readString(data:{data:string, readSize:number}):boolean
          {
            if(data.readSize > this.getReadableSize())
            {
              console.log("data size over readable size dataSize:", data.readSize)
              return false
            }
        
            for(var i = 0; i < data.readSize; ++i)
              data.data += this._byteBuffer[this._readPos + i]
        
             this._readPos += data.readSize;
        
              console.log("read result:", data)
              return true;
          }
        
            // 读数值
            readNum(data:{data:number, readSize:number}):boolean
            {
              if(data.readSize > this.getReadableSize())
              {
                console.log("data size over readable size dataSize:", data.readSize)
                return false;
              }
        
              for(var i = 0; i < data.readSize;++i)
              {
                var v = this._byteBuffer[this._readPos + i];
                data.data |= v << (8 * i);
              }
              
              this._writePos += data.readSize;
              return true;
            }
        
            // 读取数组
            readArray(data:{data:ArrayBuffer, readSize:number}):boolean
            {
              if(data.readSize > this.getReadableSize())
              {
                console.log("data size over readable size dataSize:", data.readSize)
                return false;
              }
        
              var arr = new Uint8Array(data.data)
              for(var i = 0; i < data.readSize; ++i)
                arr.fill(this._byteBuffer[this._readPos + i], i, i + 1)
        
              this._readPos += data.readSize;
        
              return true;
            }
        
            // 压缩数据, 已读的数据会被丢弃
            compress()
            {
              var newBuffer = new ArrayBuffer(this.getReadableSize())
              var newByteBuffer = new Uint8Array(newBuffer)
        
              var newLen = this.getReadableSize();
              for(var i = 0; i < newLen; ++i)
                newByteBuffer.fill(this._byteBuffer[this._readPos + i], i, i + 1)
        
              this._writePos -= this._readPos;
              this._readPos = 0;
              this._capacity = newLen;
              this._buffer = newBuffer;
              this._byteBuffer = newByteBuffer;
            }
        }
        
        // 网络包
        export class LibPacket
        {
          _packetId:number = 0
          _opcode:number = 0
          _jsonData:string = ""
          _objData:any
        
          setPacketId(packetId:number):void
          {
            this._packetId = packetId;
          }
        
          setOpcode(opcode:number){
            this._opcode = opcode;
          }
        
          setJsonData(data:string){
            this._jsonData = data
          }
        
          setObjData(data:any)
          {
            this._objData = data
          }
        
          // 获取包id
          getPacketId():number { return this._packetId }
        
          // 协议id
          getOpcode():number { return this._opcode}
        
          // jsonstring
          getJsonData():string {return this._jsonData}
        
          // obj
          getObjData():any{return this._objData}
        }
        
        // 协议栈
        export class ProtocolStack
        {
          public static ToBinary(opcode:number, data:any, packetId:number = -1):LibStream
          {
              var jsonString = JSON.stringify(data)
              var header = new MsgHeader(opcode)
              header._len = MsgHeaderStructure.MSG_HEADER_SIZE + jsonString.length
              header._packetId = packetId
              
              var newStream = new LibStream(header._len)
              newStream.write(header._len, MsgHeaderStructure.LEN_SIZE)
              newStream.write(header._protocolVersion, MsgHeaderStructure.PROTOCOL_VERSION_SIZE)
              newStream.write(header._flags, MsgHeaderStructure.FLAGS_SIZE)
              newStream.write(header._opcodeId, MsgHeaderStructure.OPCODE_SIZE)
              newStream.write(header._packetId, MsgHeaderStructure.PACKET_ID_SIZE)
              newStream.writeString(jsonString)
          
              return newStream
          }
        
          public static BinaryToPacket(buffer:ArrayBuffer, leftStream:LibStream):any
          {
            // 已经读超过一半容量则压缩
            if(leftStream.getReadSize() >= leftStream.getCapacity() / 2)
                leftStream.compress()
        
            leftStream.forceWriteBuffer(buffer)
            if(leftStream.getReadableSize() < MsgHeaderStructure.MSG_HEADER_SIZE)
              return
        
            var newStream = new LibStream(0)
            newStream.attachSream(leftStream)
        
            var header = new MsgHeader(0)
        
            // 读长度
            var data = {data:0, readSize:MsgHeaderStructure.LEN_SIZE}
            newStream.readNum(data)
            header._len = data.data
        
            // 读版本号
            data.readSize = MsgHeaderStructure.PROTOCOL_VERSION_SIZE
            newStream.readNum(data)
            header._protocolVersion = data.data
        
            // 读flag
            data.readSize = MsgHeaderStructure.FLAGS_SIZE
            newStream.readNum(data)
            header._flags = data.data
        
            // 读opcode
            data.readSize = MsgHeaderStructure.OPCODE_SIZE
            newStream.readNum(data)
            header._opcodeId = data.data
        
            // 读包id
            data.readSize = MsgHeaderStructure.PACKET_ID_SIZE
            newStream.readNum(data)
            header._packetId = data.data
            
            // 数据不到一个包大小
            if(newStream.getReadableSize() < (header._len - MsgHeaderStructure.MSG_HEADER_SIZE))
            {
              return
            }
        
            var packet = new LibPacket()
            packet.setOpcode(header._opcodeId)
            packet.setPacketId(header._packetId)
        
            var stringData = {data:"", readSize:header._len - MsgHeaderStructure.MSG_HEADER_SIZE}
            newStream.readString(stringData)
            packet.setJsonData(stringData.data)
            var objData = JSON.parse(stringData.data)
            packet.setObjData(objData)
            leftStream.shiftReadPos(header._len)
        
            return packet
          }
        }
        
        }
        
        ```

        

    * 在导入该模块时候：

      * ```
        /// <reference path="./utils/network.ts" />
        import {network} from './utils/network'
        ```

        