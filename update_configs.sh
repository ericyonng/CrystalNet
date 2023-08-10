# EricYonng<120453674@qq.com>
# @brief update_configs.sh ...
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"

chmod a+x ${SCRIPT_PATH}/tools/ConfigExporter/ConfigExporter

sudo ${SCRIPT_PATH}/tools/ConfigExporter/ConfigExporter --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/TestService/config/xlsx --target_dir=../../service/TestService/config/code --data=../../service/TestService/config/data --meta=../../service/TestService/config/meta

sudo ${SCRIPT_PATH}/tools/ConfigExporter/ConfigExporter --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/GateService/config/xlsx --target_dir=../../service/GateService/config/code --data=../../service/GateService/config/data --meta=../../service/GateService/config/meta

sudo ${SCRIPT_PATH}/tools/ConfigExporter/ConfigExporter --config=xlsx --lang=S:cpp@C:csharp,lua  --source_dir=../../service/CenterService/config/xlsx --target_dir=../../service/CenterService/config/code --data=../../service/CenterService/config/data --meta=../../service/CenterService/config/meta
