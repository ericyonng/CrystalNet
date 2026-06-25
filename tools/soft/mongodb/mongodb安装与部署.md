# Windows

* windows: mongodb-windows-x86_64-8.0.4-signed.msi, 并安装mongodb compass



# Linux

* 解压mongodb-linux-x86_64-rhel8-8.0.6.tgz，并添加bin路径到path

  * ```shell
    export PATH=$PATH:/root/mongodb/bin
    
    # 运行指令
    mongod  mongod --dbpath /root/mongodb/db_data --port 27017 --logpath /root/mongodb/db_data/mongod.log --fork
    ```

* 解压 mongosh-2.4.2-linux-x64.tgz

  * ```
    export PATH=$PATH:/root/mongosh/bin
    
    # 执行mongosh 会默认连接mongodb
    ```

* 赋予权限：

  * ```
    use admin
    db.createUser({ user:"xxxx", pwd:"xxxx", roles:[{role:"userAdminAnyDatabase", db:"admin"}, "readWriteAnyDatabase"]})
    
    ```

* 解压database-tools, 并添加bin目录到PATH

  * ```
    export PATH=$PATH:/root/mongodb/mongodb-database-tools-rhel88-x86_64-100.11.0/bin
    
    ```

* database-tools中的mongodbdump 和 mongodbrestore可以备份和还原mongodb数据

* 或者可以编辑.bash_profile 在末尾追加. ~/mongodb/mongodb_export.sh 来导出环境变量,如果这里用sh其实是给sh导出环境变量，所以这里必须要使用.

  * ```shell
    #!/usr/bin/env bash
    # mongodb_export.sh
    
    export PATH=$PATH:/root/mongodb/mongodb-database-tools-rhel88-x86_64-100.11.0/bin
    export PATH=$PATH:/root/mongodb/mongodb-linux-x86_64-rhel88-8.0.6/bin
    export PATH=$PATH:/root/mongodb/mongosh-2.4.2-linux-x64/bin
    
    ```

* 一键安装脚本（解压mongodb, mongosh, mongodb-tools等, 以及导出命令，使得命令可用）

  * ```shell
    source install_mongodb.sh /root/CrystalNet/CrystalNet/tools/soft/mongodb /root/mongodb_soft
    
    ```

    

* 相关运维脚本

  * ```shell
    # 创建复制集
    sh create_replicaset.sh <iplist.txt> <用户名> <密码> <工作路径(数据运行路径)> <安装路径(数据库安装路径)>
    # 启动复制集
    sh start_replicaset.sh <start_iplist.txt> <账号> <密码> <软件安装路径>
    # 关停复制集
    sh stop_replicaset.sh <iplist.txt> <用户名> <密码>
    # 添加复制集节点
    sh replicaset_add_node.sh <iplist.txt> <用户名> <密码> <软件包安装路径> <数据库数据路径> <keyfile绝对路径> <mongod主节点地址> <mongod主节点端口>
    # 备份
    sh mongod_replicaset_dump.sh <主节点地址> <主节点port> <账号> <密码> <生成dump的绝对路径名> <db名> <collectionName>
    # 恢复
    sh mongod_replicaset_restore.sh <主节点地址> <主节点port> <账号> <密码> <dump的绝对路径名>
    
    ```

    

