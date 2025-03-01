* 安装java环境，要求Java11以上

  ```shell
  sudo yum install java-11-openjdk
  java -version
  # 若java版本是低于11的请执行
  mv /usr/bin/java /usr/bin/java.back
  ln -sv /usr/lib/jvm/java-11-openjdk-11.0.18.0.10-2.tl3.x86_64/bin/java /usr/bin/java
  ```

* jenkins安装

  ```shell
  sudo yum upgrade
  sudo wget -O /etc/yum.repos.d/jenkins.repo https://pkg.jenkins.io/redhat/jenkins.repo
  sudo rpm --import https://pkg.jenkins.io/redhat/jenkins.io.key
  
  sudo yum install jenkins
  ```

* 配置jenkins的java sdk路径

  ```
  vim /etc/init.d/jenkins
  
  
  
  #找到java sdk目录
  find / -name "java"
  # 找到了：/usr/lib/jvm/java-11-openjdk-11.0.16.0.8-1.el7_9.x86_64/bin/java
  
  #找到 candidates= 这一行 向最近的添加一行java sdk路径
  candidates="
  /usr/lib/jvm/java-11-openjdk-11.0.16.0.8-1.el7_9.x86_64/bin/java
  
  # 添加java相关路径：
  /usr/lib/jvm/jre-11/bin/java
  /usr/lib/jvm/jre-11-openjdk/bin/java
  /usr/bin/java
  /usr/lib/jvm/java-11-openjdk-11.0.18.0.10-1.el7_9.x86_64/bin/java
  
  
  # 保存并重启jenkins
  service jenkins restart
  
  ```

  

* jenkins开机启动

  ```
  sudo systemctl enable jenkins
  ```

* 启动jenkins

  ```
  sudo systemctl start jenkins
  ```

* 重启jenkins

  ```
  service jenkins restart
  ```

* 查看jenkins状态以及错误

  ```
  sudo systemctl status jenkins
  ```

  若：Active: active (running) since Tue 2018-11-13 16:19:01
  +03; 

  表示启动成功

* jenkins的配置脚本

  1. /etc/sysconfig/jenkins 这个可以配置jenkins启动时的用户，以及用户组 以及配置jenkins对外的端口号：JENKINS_PORT="8080"
  2. 自启动脚本位置:/etc/init.d/jenkins

* 腾讯防火墙放行，linux防火墙放行

  linux下：

  ```
  YOURPORT=8080
  PERM="--permanent"
  SERV="$PERM --service=jenkins"
  
  firewall-cmd $PERM --new-service=jenkins
  firewall-cmd $SERV --set-short="Jenkins ports"
  firewall-cmd $SERV --set-description="Jenkins port exceptions"
  firewall-cmd $SERV --add-port=$YOURPORT/tcp
  firewall-cmd $PERM --add-service=jenkins
  firewall-cmd --zone=public --add-service=http --permanent
  firewall-cmd --reload
  
  ```

  腾讯需要在云服务器上配置端口规则

* 为jenkins添加sudo权限

  ```
  visudo
  #找到 root ALL=(ALL) ALL
  # 向下一行追加：
  jenkins ALL=(ALL) NOPASSWD: ALL
  # jenkins 组内所有用户免密执行sudo所有权限
  %jenkins ALL=(ALL) NOPASSWD: ALL
  ```

* linux需要安装git，以便jenkins使用git插件

* linux安装svn，若jenkins有使用svn插件的需求

* jenkins需要拉取gitlab的仓库可以看相关jenkins拉gitlab教程