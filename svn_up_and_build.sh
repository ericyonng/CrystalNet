# EricYonng<120453674@qq.com>
# @brief gitupdate ...
#!/usr/bin/env bash

svn up
echo "svn up finish"

sh ./onekeybuild_linux.sh $* > err.log 2>&1

echo "build finish."
