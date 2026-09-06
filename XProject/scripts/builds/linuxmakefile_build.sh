# @file linuxmakefile_build.sh
# @author EricYonng<120453674@qq.com>
# @brief generate makefile
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
ROOT_PATH=${SCRIPT_PATH}/../../../
PREMAKE_SCRIPT_PATH=$SCRIPT_PATH/../../tools
PREMAKE_TOOL=${ROOT_PATH}/tools/premake/linux_premake5

echo "=======================Generate gmake Makefiles==================="
sudo chmod +x ${PREMAKE_TOOL}
${PREMAKE_TOOL} --file=$PREMAKE_SCRIPT_PATH/premake5.lua gmake clang use_kernel_so use_storage
