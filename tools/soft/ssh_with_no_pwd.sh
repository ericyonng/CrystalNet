#!/usr/bin/env bash
# @author EricYonng<120453674@qq.com>
# 免密执行指令
# sh ssh_with_no_pwd.sh remote_ip

# 检查有没有id_rsa.pub
if [ -f ~/.ssh/id_rsa.pub ]; then
    echo "公钥文件存在，路径：~/.ssh/id_rsa.pub"
else
    echo "公钥文件不存在 即将创建..."
    ssh-keygen -t rsa -b 4096 -C "create id_rsa.pub" || {
        echo "错误：ssh-keygen 失败" >&2
        exit 1
    }
fi

# 安装 ssh-copy-id
if ! command -v ssh-copy-id &> /dev/null; then
    echo "当前环境未安装 ssh-copy-id 正在安装 ssh-copy-id..."
    if sudo yum install openssh-clients -y &> /dev/null; then
        echo "ssh-copy-id 安装成功！"
    else
        echo "ssh-copy-id 安装失败！"
        exit 1
    fi
fi

# 上传公钥到指定机器
ssh-copy-id -i ~/.ssh/id_rsa.pub root@$1 || {
    echo "错误： ssh-copy-id 失败" >&2
    exit 1
}

echo "免密设置成功 remote ip:$1, enjoy!!!"

