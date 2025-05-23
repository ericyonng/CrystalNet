# @brief one key build
#!/usr/bin/env bash

BUILD_PATH="build_tools/gmake/"
SCRIPT_PATH="$(cd $(dirname $0); pwd)"
ROOT_PATH=${SCRIPT_PATH}/../..

# ver:default debug
VER="debug"
if [ -n "$1" ]
then
    if [ "$1" = "release" ]
	then
	    VER="release"
		# PATH=$PATH:${SCRIPT_PATH}/3rd/openssl/linux/lib/release/
		# PATH=$PATH:${SCRIPT_PATH}/3rd/openssl/linux/lib/debug/
	fi
fi

echo $PATH

# check param 1,2 if need clean
NEED_CLEAN=0
if [ -n "$1" ]
then
    if [ "$1" = "clean" ]
	then
	    NEED_CLEAN=1
	fi
fi
if [ -n "$2" ]
then
    if [ "$2" = "clean" ]
	then
	    NEED_CLEAN=1
	fi
fi

# give scripts right
# sudo chmod 777 $SCRIPT_PATH/*.sh

# 开始时间戳
START_TIMESTAMP=$(date +%s)
echo "Start build time:$START_TIMESTAMP"

# run firstly
echo "run first scripts"
sh $SCRIPT_PATH/runfirstly_scripts.sh

# generate make files
echo "run linuxmakefile_build"
sh $SCRIPT_PATH/linuxmakefile_build.sh

# build
cd ${ROOT_PATH}/$BUILD_PATH
if [ $NEED_CLEAN = 1 ]
then
    echo -e "clean $VER..."
    sudo make clean config=$VER;
fi
echo -e "making $VER..."

sudo make -j4 config=$VER

if [ $? -ne 0 ]; then
    echo "make fail..."
	
	# quit build dir
	cd ${SCRIPT_PATH}
	
	exit 1
else
    echo "make suc!"
	
	# quit build dir
	cd ${SCRIPT_PATH}
fi

# install default:opencoredump
echo "installing..."
sudo sh $SCRIPT_PATH/install.sh $VER

# 结束时间戳
END_TIMESTAMP=$(date +%s)
COST_TIME=$((END_TIMESTAMP - START_TIMESTAMP))

echo "Done COST_TIME:$COST_TIME!"

