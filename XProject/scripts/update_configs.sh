# EricYonng<120453674@qq.com>
# @brief update_configs.sh ...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"
ROOT_PATH=${SCRIPT_PATH}/../../
XPROJ_PATH=${SCRIPT_PATH}/../

chmod a+x ${ROOT_PATH}/tools/ConfigExporter/ConfigExporter

sudo ${ROOT_PATH}/tools/ConfigExporter/ConfigExporter --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=${XPROJ_PATH}Config/xlsx --target_dir=${XPROJ_PATH}Config/code --data=${XPROJ_PATH}Config/data --meta=${XPROJ_PATH}Config/meta
if [ $? = 0 ]
then
    echo "gen TestService configs success."
else
    echo "gen TestService configs fail."
fi

