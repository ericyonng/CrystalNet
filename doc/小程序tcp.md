* 创建tcpsocket并连接远端

  ```
  
      // tcp连接
      const tcp = wx.createTCPSocket()
      tcp.connect(
        {
          address:'127.0.0.1',
          port:3900
        }
      )
      tcp.onConnect(()=>{
        console.log('connect success.')
        var buffer = new ArrayBuffer(2)
        var view = new Uint16Array(buffer)
        view.fill(13)
        tcp.write(buffer)
        tcp.write('hello world')
      })
  
      tcp.onMessage( (message, remoteInfo, localInfo) => {
        console.log('message coming.')
        console.log(message)
        console.log(remoteInfo)
        console.log(localInfo)
      })
  
      tcp.onError((res)=>{
        console.log('error:')
        console.log(res)
      })
  
      setTimeout(function(){
        tcp.close()
        console.log('tcp close.')
      }, 3000)
  ```

* 数据传输

  * 传输二进制
    * 使用ArrayBuffer
    * 填入包长度
    * 填入包数据(建议json)
  * 调用tcp.write发送数据