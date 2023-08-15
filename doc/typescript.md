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