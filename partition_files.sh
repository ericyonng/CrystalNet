# @file partition_files.sh
# @author EricYonng<120453674@qq.com>
# @brief partition_files
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"

sudo chmod a+x $SCRIPT_PATH/tools/file_tool/filetool
sudo $SCRIPT_PATH/tools/file_tool/filetool --root_path=../../ --partition_targets=../../partition_file_list.file_list --function=partition --partition_size=104857600