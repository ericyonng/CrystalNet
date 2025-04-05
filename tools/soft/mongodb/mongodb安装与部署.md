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

* 

