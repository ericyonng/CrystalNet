
# @brief update gcc to 8.3.1 ...
#!/usr/bin/env bash

# 判断没有gcc 8.3则更新
if [ -z $(rpm -qa |grep gcc-8\.3) ] 
then
    echo "have no gcc 8.3 and will install."
    # 安装源
    sudo yum install -y centos-release-scl

    # 查询是否有8版本
    sudo yum list |grep ".*devtoolset-8-gcc.*"

    # 安装gcc 默认8.3.1
    sudo yum install -y devtoolset-8-gcc*

    # 新建软链接替换旧的gcc
    mv /usr/bin/gcc /usr/bin/gcc-4.8.5
    ln -sv /opt/rh/devtoolset-8/root/bin/gcc /usr/bin/gcc
    mv /usr/bin/g++ /usr/bin/g++-4.8.5
    ln -sv /opt/rh/devtoolset-8/root/bin/g++ /usr/bin/g++
else
    echo "have install gcc 8.3 before."
fi

# gcc 版本号
sudo gcc -v

# 切换gcc版本
sudo source /opt/rh/devtoolset-8/enable

# 开机启动时自动切换到gcc 8.3.1
sudo echo "source /opt/rh/devtoolset-8/enable" >>/etc/profile

/usr/bin/gcc --version
/usr/bin/g++ --version

#Cmake指定编译器为新安装的gcc版本
export CC=/opt/rh/devtoolset-8/root/bin/gcc
export CXX=/opt/rh/devtoolset-8/root/bin/g++
echo "CC=/opt/rh/devtoolset-8/root/bin/gcc" >>~/.bashrc
echo "CXX=/opt/rh/devtoolset-8/root/bin/g++" >>~/.bashrc

# 8.3的库
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/gcc-8.3.0/lib64
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/gcc-8.3.0/lib64" >>~/.bashrc

echo "update gcc to 8.3 finish."

# 切换到gcc 8.3.1
sudo scl enable devtoolset-8 bash
