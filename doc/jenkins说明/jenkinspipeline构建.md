pipeline {

  agent {

​    node {

​      label '压测客户端_10.0.3.74'

​      customWorkspace "/root/auto_presure_test/"

​    }

  }

  

  parameters {

​    string(name: 'Changelist', defaultValue: '', description: '版本号')

​    string(name: 'GameServer_IP', defaultValue: '10.0.3.72', description: '游戏服节点ip使用空格隔开')

​    string(name: 'GateWay_IP', defaultValue: '10.0.3.75', description: '网关服节点IP使用空格隔开')

​    string(name: 'LoginServer_IP', defaultValue: '10.0.3.76', description: '登录服')

​    string(name: 'OtherServers_IP', defaultValue: '10.0.3.76', description: '其他服')

​    string(name: 'ProtocolHandleMsThreshold', defaultValue: '', description: '协议处理耗时（gs收到request到gs返回response的时间）告警阈值')

​    string(name: 'ProtocolMsFromRequestToResponseThreshold', defaultValue: '', description: '从gw收到request到gw返回客户端response直接的时间差告警阈值')

​    string(name: 'RunSeconds', defaultValue: '3600', description: '压测运行时长')

​    string(name: 'Branch', defaultValue: 'development', description: '分支')

​    booleanParam(name: 'CleanDb', defaultValue: false, description: '启动前清档')

​    string(name: 'ShelveNumber', defaultValue: '382265', description: '搁置号')

​    string(name: 'RobotScriptAddress', defaultValue: '10.0.3.74', description: '机器所在ip')

​    string(name: 'PlayerNum', defaultValue: '50', description: '压测客户端玩家人数')

​    string(name: 'LocustClientNum', defaultValue: '1', description: '压测客户端实例数量')

​    booleanParam(name: 'SkipUpdateServer', defaultValue: false, description: '跳过更新服务器')

​    string(name: 'ClusterName', defaultValue: 'AutoPressureTest', description: '集群名')

​    string(name: 'MinimumLogLevel', defaultValue: 'Information', description: '日志压测时最小打印级别')

​    string(name: 'BuildConfiguration', defaultValue: 'Release', description: '编译版本')

​    string(name: 'LoginPort', defaultValue: '5500', description: '登录端口号')

  }

  

  environment { 

​    P4CLIENT="aki_auto_test_client"

​    P4ROOT="/data/server/${Branch}"

​    WORKPLACE="/root/auto_presure_test"

​    CLIENT_IP="10.0.3.74"

​    CLIENT_PORT=9212

  }

  

  options { timestamps () }

  

  stages {

​    stage('初始化') {

​      steps {

​        sh '''

​        \# 判断要构建的分支是否存在

​        if [ -z $(p4 streams | grep "Stream ${AKI_DEPOT_URL}/${Branch}" | awk '{print $3}') ]; then 

​          echo "该分支不存在，请检测分支是否填写正确，或联系perforce管理员。"

​          exit 1

​        fi



​        \# 如果要构建的客户端不存在，则创建一个

​        if [ -z $(p4 clients | grep " ${P4CLIENT} " | awk '{print $2}') ]; then 

​          p4 client -t aki-jenkins -o ${P4CLIENT} | sed "s/Created by.*$/Created by automated build/" | sed "s!^Root:.*$!!Root: ${P4ROOT}!" | p4 client -i

​            

​          p4 client -s -f -S ${AKI_DEPOT_URL}/${Branch} ${P4CLIENT} 

​        fi

​        '''

​      }

​    }

​    



​    stage('更新P4') {

​      steps{

​        sh '''

​        p4 revert -w //...

​        if [ ${Changelist} ]; then

​          p4 sync -f ${P4ROOT}/Source/Script/...@${Changelist}

​        else

​          p4 sync -f ${P4ROOT}/Source/Script/...

​        fi

​        

​        echo "ShelveNumber:${ShelveNumber}"

​        if [ ${ShelveNumber} ]; then

​          p4 unshelve -f -s ${ShelveNumber} ${P4ROOT}/Source/Script/...

​        fi

​        

​        sh ${P4ROOT}/Source/Script/AutoPressureTest/init_workplace.sh

​        '''

​      }

​    }

​    

​    stage('清理'){

​      steps{

​        sh '''

​        sh ${P4ROOT}/Source/Script/AutoPressureTest/clean_workplace.sh

​        sh ${P4ROOT}/Source/Script/AutoPressureTest/stop_pressure_locust_client.sh

​        '''

​      }

​    }

​    

​    stage('关服') {

​      steps{

​        sh '''

​        echo "will close server..."

​        ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript && sh ${P4ROOT}/Source/Script/ServerScript/stop.sh"

​        ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript && sh ${P4ROOT}/Source/Script/ServerScript/stop.sh"

​        ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript && sh ${P4ROOT}/Source/Script/ServerScript/stop.sh"

​        

​        \# 清理nettrace

​        ssh ${GameServer_IP} "rm -rf ${P4ROOT}/Source/Script/ServerScript/*.nettrace"

​        ssh ${GateWay_IP} "rm -rf ${P4ROOT}/Source/Script/ServerScript/*.nettrace"

​        ssh ${OtherServers_IP} "rm -rf ${P4ROOT}/Source/Script/ServerScript/*.nettrace"

​        

​        echo "close server finish."

​        '''

​      }

​    }



​    stage('更新压测服版本') {

​      steps{

​        sh '''

​        if [ $SkipUpdateServer == false ]; then

​          echo "will update server..."

​          ssh ${GameServer_IP} "p4 revert -w //..."

​          ssh ${GateWay_IP} "p4 revert -w //..."

​          ssh ${OtherServers_IP} "p4 revert -w //..."

​          ssh ${GameServer_IP} "p4 clean //..."

​          ssh ${GateWay_IP} "p4 clean //..."

​          ssh ${OtherServers_IP} "p4 clean //..."

​          if [ ${Changelist} ]; then

​            ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/update.sh ${Changelist}"

​            ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/update.sh ${Changelist}"

​            ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/update.sh ${Changelist}"

​          else

​            ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/update.sh"

​            ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/update.sh"

​            ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/update.sh"

​          fi

​          

​          if [ ${ShelveNumber} ]; then

​            ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/unshelves.sh ${ShelveNumber} server"

​            ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/unshelves.sh ${ShelveNumber} server"

​            ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript;sh ${P4ROOT}/Source/Script/ServerScript/unshelves.sh ${ShelveNumber} server"

​          fi



​          echo "update server finish."

​        else

​          \# 脚本必更

​          ssh ${GameServer_IP} "p4 revert -w ${P4ROOT}/Source/Script/..."

​          ssh ${GateWay_IP} "p4 revert -w ${P4ROOT}/Source/Script/..."

​          ssh ${OtherServers_IP} "p4 revert -w ${P4ROOT}/Source/Script/..."

​          ssh ${GameServer_IP} "p4 clean ${P4ROOT}/Source/Script/..."

​          ssh ${GateWay_IP} "p4 clean ${P4ROOT}/Source/Script/..."

​          ssh ${OtherServers_IP} "p4 clean ${P4ROOT}/Source/Script/..."



​          if [ ${Changelist} ]; then

​            ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript;p4 sync -f  ${P4ROOT}/Source/Script/...@${Changelist}"

​            ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript;p4 sync -f  ${P4ROOT}/Source/Script/...@${Changelist}"

​            ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript;p4 sync -f  ${P4ROOT}/Source/Script/...@${Changelist}"

​          else

​            ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript;p4 sync -f  ${P4ROOT}/Source/Script/..."

​            ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript;p4 sync -f  ${P4ROOT}/Source/Script/..."

​            ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript;p4 sync -f  ${P4ROOT}/Source/Script/..."

​          fi

​          

​          if [ ${ShelveNumber} ]; then

​            ssh ${GameServer_IP} "p4 unshelve -f -s ${ShelveNumber} ${P4ROOT}/Source/Script/..."

​            ssh ${GateWay_IP} "p4 unshelve -f -s ${ShelveNumber} ${P4ROOT}/Source/Script/..."

​            ssh ${OtherServers_IP} "p4 unshelve -f -s ${ShelveNumber} ${P4ROOT}/Source/Script/..."

​          fi

​        fi



​        \# 更新集群名

​        sh ${P4ROOT}/Source/Script/AutoPressureTest/update_cluster_name.sh ${GameServer_IP} ${ClusterName} ${P4ROOT}/Source/Server/Config/Common.yml

​        sh ${P4ROOT}/Source/Script/AutoPressureTest/update_cluster_name.sh ${GateWay_IP} ${ClusterName} ${P4ROOT}/Source/Server/Config/Common.yml

​        sh ${P4ROOT}/Source/Script/AutoPressureTest/update_cluster_name.sh ${OtherServers_IP} ${ClusterName} ${P4ROOT}/Source/Server/Config/Common.yml

​        '''

​      }

​    }

​    

​    stage('压测服编译') {

​      steps{

​        sh '''

​        if [ $SkipUpdateServer == false ]; then

​          ssh ${GameServer_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/build_server.sh ${P4ROOT} ${BuildConfiguration}"

​          ssh ${GateWay_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/build_server.sh ${P4ROOT} ${BuildConfiguration}"

​          ssh ${OtherServers_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/build_server.sh ${P4ROOT} ${BuildConfiguration}"

​        fi

​        '''

​      }

​    }

​    

​    stage('压测服起服') {

​      steps{

​        withEnv(['JENKINS_NODE_COOKIE=dontkillme']) {

​          sh '''

​          \# 备份服务器日志

​          ssh ${GameServer_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/backup_server_logs.sh ${P4ROOT}"

​          ssh ${GateWay_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/backup_server_logs.sh ${P4ROOT}"

​          ssh ${OtherServers_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/backup_server_logs.sh ${P4ROOT}"



​          if [ $CleanDb == true ]; then

​            ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript/;sh ${P4ROOT}/Source/Script/ServerScript/clean_mongodb.sh"

​            ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript/;sh ${P4ROOT}/Source/Script/ServerScript/clean_mongodb.sh"

​            ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript/;sh ${P4ROOT}/Source/Script/ServerScript/clean_mongodb.sh"

​          fi

​          

​          ssh ${GameServer_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/run_server.sh ${P4ROOT} gs ${MinimumLogLevel}"

​          ssh ${GateWay_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/run_server.sh ${P4ROOT} gw ${MinimumLogLevel}"

​          ssh ${OtherServers_IP} "sh ${P4ROOT}/Source/Script/AutoPressureTest/run_server.sh ${P4ROOT} other ${MinimumLogLevel}"

​          '''

​        }

​      }

​    }

​    

​    stage('启动压测客户端') {

​      steps{

​        sh '''

​        echo "will run ${RobotScriptAddress} robot."

​        for (( i=1; i <= ${LocustClientNum}; i++ ))

​        do

​          echo "will start locust i:${i}"

​          let "REAL_RUN=$RunSeconds+60"

​          echo "REAL_RUN:${REAL_RUN}"

​          sh ${P4ROOT}/Source/Script/AutoPressureTest/start_pressure_locust_client.sh $i "${LoginServer_IP}:${LoginPort}" ${PlayerNum} ${REAL_RUN}

​        done

​        '''

​      }

​    }

​    

​    stage('等待压测结束') {

​      steps{

​        withEnv(['JENKINS_NODE_COOKIE=dontkillme']) {

​          sh '''

​          sleep ${RunSeconds}

​          '''

​        }

​      }

​    }

​    

​    stage('终止压测') {

​      steps{

​        wrap([$class: 'BuildUser']) {

​          sh '''

​          \# 打包压测数据

​          NOW_YYMMDDHHMMSS=$(date "+%Y_%m_%d_%H_%M_%S")

​          PACKAGE_NAME=auto_pressure_report_package_${NOW_YYMMDDHHMMSS}

​          echo "BUILD_USER:${BUILD_USER}, BUILD_URL:${BUILD_URL}"

​          sh ${P4ROOT}/Source/Script/AutoPressureTest/pack_pressure_info.sh ${CLIENT_IP} ${CLIENT_PORT} ${P4ROOT} ${GameServer_IP} ${GateWay_IP} ${OtherServers_IP} ${BUILD_NUMBER} ${BUILD_USER} ${RunSeconds} ${NOW_YYMMDDHHMMSS} ${PACKAGE_NAME} ${BUILD_URL}



​          \# 关闭压测客户端

​          sh ${P4ROOT}/Source/Script/AutoPressureTest/stop_pressure_locust_client.sh

​          

​          ls ${WORKPLACE}/WorkPlace



​          \# 通知消息

​          sh ${P4ROOT}/Source/Script/ServerScript/send_feishu_robot_text.sh https://open.feishu.cn/open-apis/bot/v2/hook/97812302-2a49-4c26-ade1-7fdea0061583 "自动压测报告原始文件生成：\\n操作人:${BUILD_USER}\\n\\n构建连接:${BUILD_URL}\\n报告原始文件URL:http://${RobotScriptAddress}/download/${PACKAGE_NAME}.tar.gz"

​          '''

​        }

​      }

​    }

​    

​    stage('压测完关闭压测服') {

​      steps{

​        sh '''

​        echo "will close server..."

​        ssh ${GameServer_IP} "cd ${P4ROOT}/Source/Script/ServerScript && sh ${P4ROOT}/Source/Script/ServerScript/stop.sh"

​        ssh ${GateWay_IP} "cd ${P4ROOT}/Source/Script/ServerScript && sh ${P4ROOT}/Source/Script/ServerScript/stop.sh"

​        ssh ${OtherServers_IP} "cd ${P4ROOT}/Source/Script/ServerScript && sh ${P4ROOT}/Source/Script/ServerScript/stop.sh"

​        

​        echo "close server finish."

​        '''

​      }

​    }

  }

}