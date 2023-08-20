
#!/usr/bin/env bash

SCRIPT_PATH="$(cd $(dirname $0); pwd)"

sudo chmod a+x $SCRIPT_PATH/tools/Rsa/RsaGen
sudo $SCRIPT_PATH/tools/Rsa/RsaGen
