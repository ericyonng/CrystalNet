#  windows安装

* 安装MySQL Installer 8.0.33 Server端（完整版）

* 初始化root密码等

* ```
  // 会生成一个临时密码
  mysqld --initialize --console
  
  // 安装mysql服务
  // cd 到安装目录
  mysqld install mysql8.0
  
  // 启动mysql
  net start mysql8.0
  
  // 登陆
  mysql -u root -p
  
  // 会提示重设密码
  alter user "root"@"localhost" IDENTIFIED BY "123456"
  
  show databases;
  
  // 允许远程访问 可以将user表添加允许访问的
  update user set host="%" where user="root";// %表示任意都可以访问
  flush privileges;
  
  ```



#  linux安装myql

* 使用lnmp只安装mysql 8.0

  * ```shell
    install.sh db
    # 选择mysql 8.0
    ```

    

* 下载8.0rpm包

  * wget https://dev.mysql.com/get/mysql80-community-release-el8-5.noarch.rpm

* yum安装

  * ```
    yum localinstall mysql80-community-release-el8-5.noarch.rpm
    yum update
    yum install mysql-server
    rpm -qa|grep mysql
    mysqladmin --version
    ```

* 启动mysql

  * ```
    systemctl start mysqld
    systemctl enable mysqld
    systemctl status mysqld
    ```

* 无密登陆修改密码

  * ```
    mysql -uroot
    use mysql;
    alter user 'root'@'localhost' identified by '123456';
    flush privileges;
    
    // 远程权限
    update user set host='%' where user='root';
    flush privileges;
    ```

    