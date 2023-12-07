# @file install_perf.sh
# @author EricYonng<120453674@qq.com>
# @brief install perf...
#!/usr/bin/env bash

yum -y install git
yum -y install perl
yum -y install perf

# flamegraphs
rm -rf ~/FlameGraphsClone
mkdir ~/FlameGraphsClone
cd ~/FlameGraphsClone && git clone https://github.com/brendangregg/FlameGraph

