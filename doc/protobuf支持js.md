* 环境

  * ```
    npm i -g protoc-gen-js
    // 对库文件的引用库
    npm install -g require
    
    # 这个是用来打包成前端使用的js文件
    npm install -g browserify
    
    // 会在当前目录下生成一个文件夹，里面装的就是protobuf的库文件。
    npm install google-protobuf
    
    5.都装还以后，只需要再写一个导出文件用browserify执行打包即可
    
    var myProto = require('./myproto_pb');  
      
        module.exports = {  
            DataProto: myProto  
        }  
        
        保存为exports.js。
        
        6.对文件进行编译打包
    执行命令 
    browserify exports.js > bundle.js
    然后会生成一个bundle.js文件。
    ```

* 使用

  * 最近google的protobuf3出来了，抽空看了下，对JavaScript的支持也还是蛮不错的。pb在各行业都还是挺有用的，在数据存储和消息通讯上都是很不错的选择，不论是从效率和占用内存带宽大小上，都有很大的优势，至于什么优势那就不在这里介绍了，感兴趣的同学可以去自己查阅下资料，pb和jason还有各种数据的优缺点，此处不做评价。

    那么回到标题，在google推出了3.40的protobuf以后对javascript有了很好的支持，可以不用在用别的库了，但是仔细阅读github上的说明，确实太草率了，很多地方都不是很清晰，对于刚入门或者跨行的同学确实有点摸不着头。

    那么接下来我把如何使用的方法给大家简单介绍下：

            1.要知道google提供的protobuf主要是针对Node.js来使用的。

    2.我们要做的就是把Node使用的库转换成前端可使用的库文件来使用。

    听到这里有一部分老手可能已经知道怎么做了，好了不卖关子了，接下来一步一步的教大家去处理。

    1.首先下载google的protobuf的compiler，通过编译器可以将.proto文件转换为想要的语言文件。
    下载地址：https://repo1.maven.org/maven2/com/google/protobuf/protoc/
    如不能下载可以试下vpn（大家都懂的），如果还不行那么在网上也有很多人分享的，不过要下3.40版本的
    2.按照示例写一个proto文件
    [plain]  view plain  copy
    syntax = "proto3";  
        package mypackage;  
        message myMessage {  
            int32 my_value =  1;  
        }  

    

     文件名保存为myproto.proto， 然后用编译器对其进行编译
    protoc.exe --js_out=import_style=commonjs,binary:. myproto.proto完成后会生成一个myproto_pb.js的文件，这里面就是protobuf的API和一些函数。

    3.如果是Node的话就可以引入使用了，不过前端用的话还需要做一些处理。
    下载Node.js进行安装，因为要使用到Node安装包里的npm，使用npm安装会方便很多。
    下载地址：https://nodejs.org/en/download/

    4.安装好npm以后，安装需要的库：
    npm install -g require(对库文件的引用库)
    npm install -g browserify(这个是用来打包成前端使用的js文件)
    最后我们执行
    npm install google-protobuf
    会在当前目录下生成一个文件夹，里面装的就是protobuf的库文件。

    5.都装还以后，只需要再写一个导出文件用browserify执行打包即可
    [javascript]  view plain  copy
    var myProto = require('./myproto_pb');  

        module.exports = {  
            DataProto: myProto  
        }  
    保存为exports.js。

    6.对文件进行编译打包
    执行命令 
    browserify exports.js > bundle.js
    然后会生成一个bundle.js文件。

    7.这时候大家就可以畅快的使用了

    [html]  view plain  copy
    <html>  
        <head>  
            <script type="text/javascript" src="./bundle.js"></script>  
        </head>  

        <body>  
          
            <button onclick="">1111</button>  
            <script>  
           <span style="white-space:pre">   </span> <span style="white-space:pre">  </span>var message = new proto.mypackage.myMessage();  
       <span style="white-space:pre">       </span>message.setMyValue(20);  
    <span style="white-space:pre">      </span>var bytes = message.serializeBinary();  
    <span style="white-space:pre">      </span>console.log(bytes);  

    <span style="white-space:pre">      </span>var data = proto.mypackage.myMessage.deserializeBinary(bytes);  
    <span style="white-space:pre">      </span>console.log(data);  
    <span style="white-space:pre">      </span>console.log(data.getMyValue());  
            </script>  
        </body>  
    </html>  

    这样大家就可以和平时的js调用一样使用了，如果再加上websocket，那就更爽了！！！
    就先介绍到这里，大家有兴趣可以留言讨论，如要转载请注明出处！谢谢配合！
    ————————————————
    版权声明：本文为CSDN博主「蜗牛凯」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
    原文链接：https://blog.csdn.net/arvin_kai/article/details/77532595