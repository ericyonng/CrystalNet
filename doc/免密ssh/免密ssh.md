1. 使用puttygen生成ssh2-rsa的公私钥

   ![image-20230421142241792](.\img\1.png)

   ![image-20230421142446898](.\img\2.png)

2. 将pub文件拷贝到要免密的目标机上，并去除前两行和最后一行，并使用vim打开将每一行合并成一行（shift + J） 并在前头添加ssh-rsa + 空格

   ![image-20230421142708840](.\img\3.png)

3. 将pub的内容追加到当前账户的.ssh下的authorized_keys文件的最后一行

   ![image-20230421142848008](C:\workplace\mine\笔记\免密ssh\img\4.png)

4. 此时免密成功，在windows下使用plink（安装putty后有此工具）即可成功远程执行程序

5. ```
   plink -ssh -i %CUR_PATH%\remote.ppk root@ip "shell script"
    
   ```

## plink使用

1. 执行ssh

2. ```
   plink.exe -ssh username@host -pw gbG32s4D/ "shell"
   ```