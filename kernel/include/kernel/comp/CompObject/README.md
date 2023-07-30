# 概述

抽象了ECS的四个重要对象: IObject(所有需要组成ECS系统的基类), CompObject（组件对象）, CompHostObject（宿主组件对象），CompFactory(对外提供的工厂类，用于创建组件对象)

# 详细介绍

* IObject

  * 数据成员:

    * ```
          const UInt64 _id;	// 唯一id
          UInt64 _entityId;	// entityId, 可通过接口设置
          LibString _objName;	// 真实派生类的obj名, 通过运行时类型识别获得
          Int32 _type;		// 可缺省, 如果需要设置类型, 可手动设置
          Int32 _kernelObjType; // KernelObjectType, 表示组件的类型是Comp还是Host
          std::atomic_bool _isCreated;	// 是否已创建完成
          std::atomic_bool _isInited;		// 是否初始化完成
          std::atomic_bool _isStarted;	// 是否start完成
          std::atomic_bool _isWillClose;	// 是否即将关闭
          std::atomic_bool _isClose;		// 是否关闭完成
          std::atomic_bool _isReady;		// 对象是否就绪
      
          IObject *_owner;				// 宿主对象
          Int32 _maxFocusTypeEnumEnd;		// 关注的接口: ObjFocusInterfaceFlag
          std::unordered_map<Int32, UInt64> _focusInterfaceBitmapFlag; // 关注的接口是否注册标志
          std::atomic<Int32> _errCode;	// 错误码, 如果流程过程中有错误会设置错误码
      ```

  * 需要关注的接口

    ```
    // 对象释放的方法
    virtual void Release() = 0;
    
    // 默认在start/WillClose调用,若觉得这个时候不合适则请重写成空函数(一般组件内部有线程的都需要重写该接口,因为此时组件还没准备好只有线程也准备好了才是准备好的状态)
    virtual void DefaultMaskReady(bool isReady);
    ```

* CompObject

  * 所有接口都可以缺省不关注, 需要的时候重写对应接口即可

* CompHostObject

  * 数据成员

    * ```
          // 需要注册的组件信息
          std::vector<_WillRegComp> _willRegComps;
      
      	// 已经注册的所有组件对象
          std::vector<CompObject *> _comps;
          // 按照对象名映射组件对象
          std::unordered_map<LibString, std::vector<CompObject *>> _compNameRefComps;
          // 按照对象的接口类名(例如:若组件对象是CompA, 其接口对象名是:ICompA)映射组件
          std::unordered_map<LibString, std::vector<CompObject *>> _icompNameRefComps;
          // 组件id映射组件
          std::unordered_map<UInt64, CompObject *> _compIdRefComp;
          // 组件类型映射组件
          std::unordered_map<Int32, std::vector<CompObject *>> _compTypeRefComps;
      
          // 关注的接口
          std::vector<std::vector<CompObject *>> _focusTypeRefComps;
      ```

  * 需要关注的接口

    * ```
          // 注册组件 // 注意递归死循环,若HostC中有HostA, HostA中也有HostC那么将导致死循环， 内部已经做了循环依赖的运行时判断错处理
          virtual void OnRegisterComps() = 0; 
          
          // 获取组件对象, 例如: GetComp<ICompA>()
          template<typename ObjType>
          ObjType *GetComp();
          template<typename ObjType>
          const ObjType *GetComp() const;
          
          // 在组件初始化前 必须重写，在创建组件之前调用，是对宿主自己的初始化
          virtual Int32 _OnHostInit() = 0;
          
           // 所有组件创建完成, 这个是在组件创建完成，但还没初始化时候调用, 可以对组件进行设置，可缺省，如果有需要对组件进行设置时候才重写
          virtual Int32 _OnCompsCreated();
          
          // 在组件启动之前 请勿在WillStart及之前的接口启动线程，此时都是线程不安全的状态 可缺省
          virtual Int32 _OnHostWillStart(){ return Status::Success; }
          
          // 组件启动之后 此时可以启动线程 必须重写 所有组件都启动完成, 此时可以启动线程等资源
          virtual Int32 _OnHostStart() = 0;
          
          // 在组件调用WillClose之前调用, 可以在组件之前做一些操作，可以缺省
          virtual void _OnHostBeforeCompsWillClose() {}
          
          // 在组件willclose之后，可以缺省
          virtual void _OnHostWillClose(){}
          
          // 在组件close之前 可缺省
          virtual void _OnHostBeforeCompsClose() {}
          
          // 在组件Close之后 必须重写
          virtual void _OnHostClose() = 0;
      ```

* CompFactory  组件工厂类，用于创建组件

  * 数据成员:_buildType: 创建组件的构建类型, _Build, 目前是多线程版本的构建, 还是线程局部存储的构建

  * 方法

    * ```
          // 释放工程类的方法
          virtual void Release() = 0;
      	// 创建组件的方法
          virtual CompObject *Create() const = 0;
      ```

* 注意

  * 所有组件需要基于对象池创建，只要添加对象池宏即可, 例如：POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IKillMonitorMgr);, 在实现文件加上:POOL_CREATE_OBJ_DEFAULT_IMPL(IKillMonitorMgr);

    

* 案例:

  * 基于CompObject 的 KillMonitorMgr(程序关闭监控组件)

    * ```
      // 对外提供的接口类 .h
      class IKillMonitorMgr : public KERNEL_NS::CompObject
      {
          POOL_CREATE_OBJ_DEFAULT_P1(CompObject, IKillMonitorMgr);
      
      public:
          virtual bool IsReadyToDie() const = 0;
      
          // 存在该文件说明是关闭程序的操作, 这个检测是每隔10秒检测一次， detectionFile需要带绝对路径
          virtual void SetDeadthDetectionFile(const KERNEL_NS::LibString &detectionFile) = 0;
      
          // 设置timerMgr
          virtual void SetTimerMgr(KERNEL_NS::TimerMgr *timerMgr) = 0;
      
          // 设置检测时间间隔, 没有设置默认10秒
          virtual void SetDetectionTimeInterval(Int64 seconds) = 0;
      };
      
      // 对外提供的接口类 .cpp
      POOL_CREATE_OBJ_DEFAULT_IMPL(IKillMonitorMgr);
      
      // 真正的实现类对象 .h
      class KillMonitorMgr : public IKillMonitorMgr
      {
          POOL_CREATE_OBJ_DEFAULT_P1(IKillMonitorMgr, KillMonitorMgr);
      
      public:
          KillMonitorMgr();
          ~KillMonitorMgr();
          virtual void Release() override;
      
          virtual bool IsReadyToDie() const override;
      
          // 存在该文件说明是关闭程序的操作, 这个检测是每隔10秒检测一次， detectionFile需要带绝对路径
          virtual void SetDeadthDetectionFile(const KERNEL_NS::LibString &detectionFile) override;
      
          // 设置timerMgr
          virtual void SetTimerMgr(KERNEL_NS::TimerMgr *timerMgr) override;
      
          // 设置检测时间间隔, 没有设置默认10秒
          virtual void SetDetectionTimeInterval(Int64 seconds) override;
      
      protected:
          virtual Int32 _OnInit() override;
          virtual void _OnClose() override;
          void _Clear();
      
          // 删除检测文件
          void _DelDetectionFile() const;
      
          // 检测定时器
          void _OnDetectionTimerOut(KERNEL_NS::LibTimer *timer);
      
      private:
          bool _isReadyToDie;
          bool _enableDetection;
          KERNEL_NS::TimeSlice _detectionInterval;
          KERNEL_NS::TimerMgr *_timerMgr;
          KERNEL_NS::LibTimer *_timer;
          KERNEL_NS::LibString _deadthDetectionFile;
          KERNEL_NS::LibString _detectionFileNameWithoutDir;
          
      };
      
      // 真正的实现类对象 .cpp
      
      POOL_CREATE_OBJ_DEFAULT_IMPL(KillMonitorMgr);
      
      KillMonitorMgr::KillMonitorMgr()
      :_isReadyToDie(false)
      ,_enableDetection(false)
      ,_detectionInterval(10)
      ,_timerMgr(NULL)
      ,_timer(NULL)
      {
      
      }
      
      KillMonitorMgr::~KillMonitorMgr()
      {
          _Clear();
      }
      
      void KillMonitorMgr::Release()
      {
          KillMonitorMgr::DeleteByAdapter_KillMonitorMgr(KillMonitorMgrFactory::_buildType.V, this);
      }
      
      bool KillMonitorMgr::IsReadyToDie() const
      {
          return _isReadyToDie;
      }
      
      void KillMonitorMgr::SetDeadthDetectionFile(const KERNEL_NS::LibString &detectionFile)
      {
          _deadthDetectionFile = detectionFile;
      }
      
      void KillMonitorMgr::SetTimerMgr(KERNEL_NS::TimerMgr *timerMgr)
      {
          _timerMgr = timerMgr;
      }
      
      void KillMonitorMgr::SetDetectionTimeInterval(Int64 seconds)
      {
          _detectionInterval = KERNEL_NS::TimeSlice::FromSeconds(seconds);
      }
      
      Int32 KillMonitorMgr::_OnInit()
      {
          _isReadyToDie = false;
          _detectionFileNameWithoutDir = KERNEL_NS::DirectoryUtil::GetFileNameInPath(_deadthDetectionFile.c_str());
      
          if(!_detectionFileNameWithoutDir.empty())
          {
              _enableDetection = true;
              _DelDetectionFile();
      
              if(LIKELY(_timerMgr))
              {
                  _timer = KERNEL_NS::LibTimer::NewThreadLocal_LibTimer(_timerMgr);
                  _timer->SetTimeOutHandler(this, &KillMonitorMgr::_OnDetectionTimerOut);
                  _timer->Schedule(_detectionInterval);
              }
          }
      
          g_Log->Info(LOGFMT_OBJ_TAG("enableDetection:%d _detectionInterval:%lld ms will detect deadth file:%s, name without dir:%s")
              , _enableDetection, _detectionInterval.GetTotalMilliSeconds(), _deadthDetectionFile.c_str(), _detectionFileNameWithoutDir.c_str());
      
          return Status::Success;
      }
      
      void KillMonitorMgr::_OnClose()
      {
          _Clear();
      }
      
      void KillMonitorMgr::_Clear()
      {
          if(_enableDetection)
              _DelDetectionFile();
      
          if(_timer)
          {
              KERNEL_NS::LibTimer::DeleteThreadLocal_LibTimer(_timer);
              _timer = NULL;
          }
      
          _enableDetection = false;
      }
      
      void KillMonitorMgr::_DelDetectionFile() const
      {
          if(!_deadthDetectionFile.empty())
              KERNEL_NS::FileUtil::DelFileCStyle(_deadthDetectionFile.c_str());
      }
      
      void KillMonitorMgr::_OnDetectionTimerOut(KERNEL_NS::LibTimer *timer)
      {
          // 文件检测到说明需要关闭
          if(KERNEL_NS::FileUtil::IsFileExist(_deadthDetectionFile.c_str()))
          {
              _isReadyToDie = true;
              _timer->Cancel();
          }
      }
      
      // 工厂类 .h
      class KillMonitorMgrFactory : public KERNEL_NS::CompFactory
      {
          // 创建factory对象时候使用创建的方法类型
      public:
          static constexpr KERNEL_NS::_Build::TL _buildType{};
      
          static KERNEL_NS::CompFactory *FactoryCreate();
      
          virtual void Release() override;
      
      public:
          virtual KERNEL_NS::CompObject *Create() const;
      };
      
      // 工厂类 .cpp
      KERNEL_NS::CompFactory *KillMonitorMgrFactory::FactoryCreate()
      {
          return KERNEL_NS::ObjPoolWrap<KillMonitorMgrFactory>::NewByAdapter(_buildType.V);
      }
      
      void KillMonitorMgrFactory::Release()
      {
          KERNEL_NS::ObjPoolWrap<KillMonitorMgrFactory>::DeleteByAdapter(_buildType.V, this);
      }
      
      KERNEL_NS::CompObject *KillMonitorMgrFactory::Create() const
      {
          return KillMonitorMgr::NewByAdapter_KillMonitorMgr(_buildType.V);
      }
      ```

  * 基于CompHostObject 的 PollerMgr

    * ```
      // 对外提供接口类 .h
      class KERNEL_EXPORT IPollerMgr : public CompHostObject
      {
          POOL_CREATE_OBJ_DEFAULT_P1(CompHostObject, IPollerMgr);
      
      public:
          IPollerMgr(){}
          virtual ~IPollerMgr(){}
      
          virtual const PollerConfig *GetConfig() const = 0;
          virtual void SetConfig(const PollerConfig &cfg) = 0;
      
          virtual UInt64 NewSessionId() = 0;
      
          virtual void AddSessionPending(UInt64 num) = 0;
          virtual bool CheckAddSessionPending(UInt64 num, UInt64 &totalSessionNum) = 0;
          virtual void ReduceSessionPending(UInt64 num) = 0;
          virtual void AddSessionQuantity(UInt64 num) = 0;
          virtual void ReduceSessionQuantity(UInt64 num) = 0;
          virtual UInt64 GetSessionQuantityLimit() const = 0;
      
          virtual void SetServiceProxy(IServiceProxy *serviceProxy) = 0;
          virtual IServiceProxy *GetServiceProxy() = 0;
          virtual const IServiceProxy *GetServiceProxy() const = 0;
          virtual void OnMonitor(LibString &info) = 0;
      
          // 收发统计
          virtual void AddRecvPacketCount(UInt64 num) = 0;
          virtual void AddRecvBytes(UInt64 num) = 0;
          virtual void AddSendPacketCount(UInt64 num) = 0;
          virtual void AddSendBytes(UInt64 num) = 0;
      
          // 会话统计
          virtual void AddAcceptedSessionCount(UInt64 num) = 0;
          virtual void AddConnectedSessionCount(UInt64 num) = 0;
          virtual void AddListenerSessionCount(UInt64 num) = 0;
          virtual void ReduceAcceptedSessionCount(UInt64 num) = 0;
          virtual void ReduceConnectedSessionCount(UInt64 num) = 0;
          virtual void ReduceListenerSessionCount(UInt64 num) = 0;
      
          // poller统计
          virtual void AddLinkerPollerCount(UInt64 num) = 0;
          virtual void AddDataTransferPollerCount(UInt64 num) = 0;
          virtual void AddPollerCount(UInt64 num) = 0;
      
          // 剔除所有session
          virtual void QuitAllSessions(UInt64 serviceId) = 0;
      };
      
      // 真实类 .h
      class KERNEL_EXPORT PollerMgr : public IPollerMgr
      {
          POOL_CREATE_OBJ_DEFAULT_P1(IPollerMgr, PollerMgr);
          
      public:
          PollerMgr();
          virtual ~PollerMgr();
      
      public:
          virtual void Clear() override;
          virtual void Release() override;
          virtual void OnRegisterComps() override;
          LibString ToString() const override;
      
      public:
          virtual const PollerConfig *GetConfig() const override;
          virtual void SetConfig(const PollerConfig &cfg) override;
          virtual UInt64 NewSessionId() override;
      
          virtual void AddSessionPending(UInt64 num) override;
          virtual bool CheckAddSessionPending(UInt64 num, UInt64 &totalSessionNum) override;
          virtual void ReduceSessionPending(UInt64 num) override;
          virtual void AddSessionQuantity(UInt64 num) override;
          virtual void ReduceSessionQuantity(UInt64 num) override;
          virtual UInt64 GetSessionQuantityLimit() const override;
      
          virtual void SetServiceProxy(IServiceProxy *serviceProxy) override;
          virtual IServiceProxy *GetServiceProxy() override;
          virtual const IServiceProxy *GetServiceProxy() const override;
          virtual void OnMonitor(LibString &info) override;
      
          // 统计数据
          virtual void AddRecvPacketCount(UInt64 num) override;
          virtual void AddRecvBytes(UInt64 num) override;
          virtual void AddSendPacketCount(UInt64 num) override;
          virtual void AddSendBytes(UInt64 num) override;
      
          virtual void AddAcceptedSessionCount(UInt64 num) override;
          virtual void AddConnectedSessionCount(UInt64 num) override;
          virtual void AddListenerSessionCount(UInt64 num) override;
          virtual void ReduceAcceptedSessionCount(UInt64 num) override;
          virtual void ReduceConnectedSessionCount(UInt64 num) override;
          virtual void ReduceListenerSessionCount(UInt64 num) override;
      
          virtual void AddLinkerPollerCount(UInt64 num) override;
          virtual void AddDataTransferPollerCount(UInt64 num) override;
          virtual void AddPollerCount(UInt64 num) override;
      
          virtual void QuitAllSessions(UInt64 serviceId) override;
      
      protected:
          // 在组件初始化前
          virtual Int32 _OnHostInit() override;
          // 所有组件创建完成
          virtual Int32 _OnCompsCreated() override;
          // 在组件启动之前
          virtual Int32 _OnHostWillStart() override;
          // 组件启动之后
          virtual Int32 _OnHostStart() override;
          // 在组件willclose之后
          virtual void _OnHostWillClose() override;
          // 在组件Close之后
          virtual void _OnHostClose() override;
      
      private:
          void _Clear();
      
          PollerConfig *_config;
          std::atomic<UInt64> _maxSessionId;
          IServiceProxy *_serviceProxy;
      
          // 会话总数
          std::atomic<UInt64> _sessionQuantity;
          std::atomic<UInt64> _sessionQuantityPending;
      
          // 收到的数据
          std::atomic<UInt64> _recvPacketCount;           // 单帧包数量
          std::atomic<UInt64> _recvBytes;                 // 单帧字节数
          std::atomic<UInt64> _historyRecvPacketCount;    // 历史包总量
          std::atomic<UInt64> _historyRecvBytes;          // 历史总流量
      
          // 发送的数据
          std::atomic<UInt64> _sendPacketCount;           // 单帧包数量
          std::atomic<UInt64> _sendBytes;                 // 单帧字节数
          std::atomic<UInt64> _historySendPacketCount;    // 历史包总量
          std::atomic<UInt64> _historySendBytes;          // 历史总流量
      
          // 所有会话
          std::atomic<UInt64> _sessionCount;          // 单帧数量
          std::atomic<UInt64> _onlineSessionCount;    // 在线会话数量
          std::atomic<UInt64> _historySessionCount;   // 历史会话总数
      
          // 连入
          std::atomic<UInt64> _acceptedSessionCount;          // 单帧数量
          std::atomic<UInt64> _onlineAcceptedSessionCount;    // 在线会话数量
          std::atomic<UInt64> _historyAcceptedSessionCount;   // 历史会话总数
      
          // 连出
          std::atomic<UInt64> _connectedSessionCount;             // 单帧数量
          std::atomic<UInt64> _onlineConnectedSessionCount;       // 在线会话数量
          std::atomic<UInt64> _historyConnectedSessionCount;       // 历史会话总数
      
          // 监听
          std::atomic<UInt64> _onlineListenerSessionCount;        // 在线监听会话数量
      
          // poller
          std::atomic<UInt64> _pollerCounts;      // poller总数量
          std::atomic<UInt64> _linkerCount;       // 连接器数量
          std::atomic<UInt64> _dataTransferCount;  // 数据传输器数量
      };
      
      // 真实类 .cpp
      
      POOL_CREATE_OBJ_DEFAULT_IMPL(IPollerMgr);
      
      POOL_CREATE_OBJ_DEFAULT_IMPL(PollerMgr);
      
      PollerMgr::PollerMgr()
      :_config(NULL)
      ,_maxSessionId{0}
      ,_serviceProxy(NULL)
      ,_sessionQuantity{0}
      ,_sessionQuantityPending{0}
      ,_recvPacketCount{0}
      ,_recvBytes{0}
      ,_historyRecvPacketCount{0}
      ,_historyRecvBytes{0}
      ,_sendPacketCount{0}
      ,_sendBytes{0}
      ,_historySendPacketCount{0}
      ,_historySendBytes{0}
      ,_sessionCount{0}
      ,_onlineSessionCount{0}
      ,_historySessionCount{0}
      ,_acceptedSessionCount{0}
      ,_onlineAcceptedSessionCount{0}
      ,_historyAcceptedSessionCount{0}
      ,_connectedSessionCount{0}
      ,_onlineConnectedSessionCount{0}
      ,_historyConnectedSessionCount{0}
      ,_onlineListenerSessionCount{0}
      ,_pollerCounts{0}
      ,_linkerCount{0}
      ,_dataTransferCount{0}
      {
      
      }
      
      PollerMgr::~PollerMgr()
      {
          _Clear();
      }
      
      void PollerMgr::Clear()
      {
          _Clear();
          IPollerMgr::Clear();
      }
      
      void PollerMgr::Release()
      {
          PollerMgr::DeleteByAdapter_PollerMgr(PollerMgrFactory::_buildType.V, this);
      }
      
      void PollerMgr::OnRegisterComps()
      {
          // ip控制
          // RegisterComp<IpRuleMgrFactory>();
          // 支持tcp
          RegisterComp<TcpPollerMgrFactory>();
      }
      
      LibString PollerMgr::ToString() const
      {
          // TODO:
          LibString info;
          info.AppendFormat("poller mgr comp object info:%s", CompHostObject::ToString().c_str());
          
          info.AppendFormat("poller configs:%s\n", _config->ToString().c_str());
      
          auto &allComps = GetAllComps();
          for(auto comp : allComps)
              info.AppendFormat("comp info:%s\n", comp->ToString().c_str());
      
          return info;
      }
      
      const PollerConfig *PollerMgr::GetConfig() const
      {
          return _config;
      }
      
      void PollerMgr::SetConfig(const PollerConfig &cfg)
      {
          _config = PollerConfig::New_PollerConfig();
          _config->Copy(cfg);
      }
      
      UInt64 PollerMgr::NewSessionId()
      {
          return ++_maxSessionId;
      }
      
      void PollerMgr::AddSessionPending(UInt64 num)
      {
          _sessionQuantityPending += num;
      }
      
      bool PollerMgr::CheckAddSessionPending(UInt64 num, UInt64 &totalSessionNum) 
      {
          _sessionQuantityPending += num;
          if(UNLIKELY(_config->_maxSessionQuantity == 0))
          {
              totalSessionNum = _sessionQuantityPending + _sessionQuantity;
              return true;
          }
      
          if(UNLIKELY(_sessionQuantityPending + _sessionQuantity > _config->_maxSessionQuantity))
          {
              _sessionQuantityPending -= num;
              totalSessionNum = _sessionQuantityPending + _sessionQuantity;
              return false;
          }
      
          totalSessionNum = _sessionQuantityPending + _sessionQuantity;
          return true;
      }
      
      void PollerMgr::ReduceSessionPending(UInt64 num)
      {
          _sessionQuantityPending -= num;
      }
      
      void PollerMgr::AddSessionQuantity(UInt64 num)
      {
          _sessionQuantity += num;
      }
      
      void PollerMgr::ReduceSessionQuantity(UInt64 num)
      {
          _sessionQuantity -= num;
      }
      
      UInt64 PollerMgr::GetSessionQuantityLimit() const
      {
          return _config->_maxSessionQuantity;
      }
      
      void PollerMgr::SetServiceProxy(IServiceProxy *serviceProxy)
      {
          _serviceProxy = serviceProxy;
      }
      
      IServiceProxy *PollerMgr::GetServiceProxy()
      {
          return _serviceProxy;
      }
      
      const IServiceProxy *PollerMgr::GetServiceProxy() const
      {
          return _serviceProxy;
      }
      
      void PollerMgr::OnMonitor(LibString &info)
      {
          info.AppendFormat("\n[- POLLER MGR BEGIN -]\n");
      
          UInt64 recvPacketPerFrame = _recvPacketCount.load();
          UInt64 recvBytesPerFrame = _recvBytes.load();
          UInt64 sendPacketPerFrame = _sendPacketCount.load();
          UInt64 sendBytesPerFrame = _sendBytes.load();
          UInt64 sessionCountPerFrame = _sessionCount.load();
          UInt64 acceptedSessionCountPerFrame = _acceptedSessionCount.load();
          UInt64 connectedSessionCountPerFrame = _connectedSessionCount.load();
      
          // TODO:监控信息输出
          info.AppendFormat("session-[online amount:%llu, speed:%llu, history amount:%llu]\n"
                          , _onlineSessionCount.load(), sessionCountPerFrame, _historySessionCount.load());
          info.AppendFormat("accepted session-[online amount:%llu, speed:%llu, history amount:%llu]\n"
                          , _onlineAcceptedSessionCount.load(), acceptedSessionCountPerFrame, _historyAcceptedSessionCount.load());
          info.AppendFormat("connect to remote session-[online amount:%llu, speed:%llu, history amount:%llu]\n"
                          , _onlineConnectedSessionCount.load(), connectedSessionCountPerFrame, _historyConnectedSessionCount.load());
          info.AppendFormat("listener session-[online amount:%llu]\n"
                          , _onlineListenerSessionCount.load());
      
      
          LibString pollerInfo;
          {
              auto tcpPollerMgr = GetComp<TcpPollerMgr>();
              auto &allPollers = tcpPollerMgr->GetAllPollers();
              for(auto &iter : allPollers)
              {
                  // TODO:loaded
                  auto tcpPoller = iter.second;
                  auto poller = tcpPoller->GetComp<Poller>();
                  pollerInfo.AppendFormat("pollerId:%llu, sessions:%llu, %s\n"
                          , tcpPoller->GetPollerId(), tcpPoller->GetSessionAmount()
                          , poller->OnMonitor().c_str());
              }
          }
      
          info.AppendFormat("poller-[total:%llu, linker:%llu, data transfer:%llu]\n[\n%s\n]"
          , _pollerCounts.load(),  _linkerCount.load(), _dataTransferCount.load(), pollerInfo.c_str());
      
          info.AppendFormat("\nrecv-[packet qps:%llu, speed:%s, history packet:%llu, history bytes:%llu]\n"
                          , recvPacketPerFrame, SocketUtil::ToFmtSpeedPerSec(static_cast<Int64>(recvBytesPerFrame)).c_str()
                          , _historyRecvPacketCount.load(), _historyRecvBytes.load());
          info.AppendFormat("send-[packet qps:%llu, speed:%s, history packet:%llu, history bytes:%llu]\n"
                          , sendPacketPerFrame, SocketUtil::ToFmtSpeedPerSec(static_cast<Int64>(sendBytesPerFrame)).c_str()
                          , _historySendPacketCount.load(), _historySendBytes.load());
      
          info.AppendFormat("[- POLLER MGR END -]\n");
      
          // 帧清零
          _recvPacketCount = 0;
          _recvBytes = 0;
          _sendPacketCount = 0;
          _sendBytes = 0;
          _sessionCount = 0;
          _acceptedSessionCount = 0;
          _connectedSessionCount = 0;
          
          // _recvPacketCount -= recvPacketPerFrame;
          // _recvBytes -= recvBytesPerFrame;
          // _sendPacketCount -= sendPacketPerFrame;
          // _sendBytes -= sendBytesPerFrame;
          // _sessionCount -= sessionCountPerFrame;
          // _acceptedSessionCount -= acceptedSessionCountPerFrame;
          // _connectedSessionCount -= connectedSessionCountPerFrame;
      }
      
      void PollerMgr::AddRecvPacketCount(UInt64 num)
      {
          _recvPacketCount += num;
          _historyRecvPacketCount += num;
      }
      
      void PollerMgr::AddRecvBytes(UInt64 num)
      {
          _recvBytes += num;
          _historyRecvBytes += num;
      }
      
      void PollerMgr::AddSendPacketCount(UInt64 num)
      {
          _sendPacketCount += num;
          _historySendPacketCount += num;
      }
      
      void PollerMgr::AddSendBytes(UInt64 num)
      {
          _sendBytes += num;
          _historySendBytes += num;
      }
      
      void PollerMgr::AddAcceptedSessionCount(UInt64 num)
      {
          _sessionCount += num;
          _onlineSessionCount += num;
          _historySessionCount += num;
      
          _acceptedSessionCount += num;
          _onlineAcceptedSessionCount += num;
          _historyAcceptedSessionCount += num;
      }
      
      void PollerMgr::AddConnectedSessionCount(UInt64 num)
      {
          _sessionCount += num;
          _onlineSessionCount += num;
          _historySessionCount += num;
      
          _connectedSessionCount += num;
          _onlineConnectedSessionCount += num;
          _historyConnectedSessionCount += num;
      }
      
      void PollerMgr::AddListenerSessionCount(UInt64 num)
      {
          _sessionCount += num;
          _onlineSessionCount += num;
          _historySessionCount += num;
      
          _onlineListenerSessionCount += num;
      }
      
      void PollerMgr::ReduceAcceptedSessionCount(UInt64 num)
      {
          _sessionCount -= num;
          _onlineSessionCount -= num;
      
          _acceptedSessionCount -= num;
          _onlineAcceptedSessionCount -= num;
      }
      
      void PollerMgr::ReduceConnectedSessionCount(UInt64 num)
      {
          _sessionCount -= num;
          _onlineSessionCount -= num;
      
          _connectedSessionCount -= num;
          _onlineConnectedSessionCount -= num;
      }
      
      void PollerMgr::ReduceListenerSessionCount(UInt64 num)
      {
          _sessionCount -= num;
          _onlineSessionCount -= num;
      
          _onlineListenerSessionCount -= num;
      }
      
      void PollerMgr::AddLinkerPollerCount(UInt64 num)
      {
          _linkerCount += num;
      }
      
      void PollerMgr::AddDataTransferPollerCount(UInt64 num)
      {
          _dataTransferCount += num;
      }
      
      void PollerMgr::AddPollerCount(UInt64 num)
      {
          _pollerCounts += num;
      }
      
      void PollerMgr::QuitAllSessions(UInt64 serviceId)
      {
          // 通知所有poller 关闭session
          auto tcpPollerMgr = GetComp<TcpPollerMgr>();
          tcpPollerMgr->QuitAllSessions(serviceId);
      
          // 等待所有session退出
          while(true)
          {
              if(_onlineSessionCount == 0)
                  break;
      
              SystemUtil::ThreadSleep(1);
          }
      }
      
      Int32 PollerMgr::_OnHostInit()
      {
          g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr init."));
          return Status::Success;
      }
      
      Int32 PollerMgr::_OnCompsCreated()
      {
          auto owner = GetOwner();
          auto ret = CompHostObject::_OnCompsCreated();
          if(ret != Status::Success)
          {
              g_Log->NetError(LOGFMT_OBJ_TAG("_OnCompsCreated fail ret:%d"), ret);
              if(owner)
                  owner->SetErrCode(this, ret);
              return ret;
          }
      
          // ip rule mgr 设置
          // auto ipRuleMgr = GetComp<IpRuleMgr>();
          // if(!ipRuleMgr->SetBlackWhiteListFlag(_config->_blackWhiteListFlag))
          // {
          //     g_Log->NetError(LOGFMT_OBJ_TAG("SetBlackWhiteListFlag fail black white list flag:%u"), _config->_blackWhiteListFlag);
          //     if(owner)
          //         owner->SetErrCode(this, Status::Failed);
          //     return Status::Failed;
          // }
      
          // tcp poller mgr 设置    
          auto tcpPollerMgr = GetComp<TcpPollerMgr>();
          tcpPollerMgr->SetConfig(&_config->_tcpPollerConfig);
          tcpPollerMgr->SetServiceProxy(_serviceProxy);
      
          g_Log->NetInfo(LOGFMT_OBJ_TAG("_OnCompsCreated suc."));
          return Status::Success;
      }
      
      Int32 PollerMgr::_OnHostWillStart()
      {
          g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnWillStart."));
      
          return Status::Success;
      }
      
      Int32 PollerMgr::_OnHostStart()
      {
          g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnStart."));
      
          // 等待所有组件成功
          CompObject *notReady = NULL;
          for(;!IsAllCompsReady(notReady);)
          {
              g_Log->Warn(LOGFMT_OBJ_TAG("poller mgr not ready comp:%s"), notReady->ToString().c_str());
              if(GetErrCode() != Status::Success)
              {
                  g_Log->Error(LOGFMT_OBJ_TAG("error happen errCode:%d"), GetErrCode());
                  break;
              }
          }
      
          return GetErrCode();
      }
      
      void PollerMgr::_OnHostWillClose()
      {
          g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnWillClose."));
      }
      
      void PollerMgr::_OnHostClose()
      {
          CompObject *notDown = NULL;
          for(;!IsAllCompsDown(notDown);)
              g_Log->Warn(LOGFMT_OBJ_TAG("poller mgr not down comp:%s"), notDown->ToString().c_str());
      
          _Clear();
          g_Log->NetInfo(LOGFMT_OBJ_TAG("poller mgr _OnWillClose."));
      }
      
      void PollerMgr::_Clear()
      {
          if(LIKELY(_config))
          {
              PollerConfig::Delete_PollerConfig(_config);
              _config = NULL;
          }
      }
      
      // 工厂类
      class KERNEL_EXPORT PollerMgrFactory : public CompFactory
      {
      public:
          static constexpr _Build::MT _buildType{};
          
          static CompFactory *FactoryCreate();
          virtual void Release() override;
      public:
          virtual CompObject *Create() const;
      };
      
      // 工厂类实现
      CompFactory *PollerMgrFactory::FactoryCreate()
      {
          return KERNEL_NS::ObjPoolWrap<PollerMgrFactory>::NewByAdapter(_buildType.V);
      }
      
      void PollerMgrFactory::Release()
      {
          KERNEL_NS::ObjPoolWrap<PollerMgrFactory>::DeleteByAdapter(_buildType.V, this);
      }
      
      CompObject *PollerMgrFactory::Create() const
      {
          return PollerMgr::NewByAdapter_PollerMgr(PollerMgrFactory::_buildType.V);
      }
      
      ```

      

