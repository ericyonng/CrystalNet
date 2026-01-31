# 文件监视

----

## 背景

* 配置文件繁杂, 热更配置, 要实现及时更新的复杂度较高
* 希望通过实现监视器的东西，它自动的监视文件的变化及时的更新配置等内存内容, 自己形成一个闭环，类似C# 的OptionMonitor那样



## 规则

* 通过定时的监听文件大小变化，文件修改时间变化，以最小的代价来监听文件变化，及时更新内存内容
* 需要提供一个FileMonitorDeserializer 反序列化器，以便将文件内容反序列化成对象
* 需要提供一个Poller，用于文件的监听
* 更新文件应该保证原子更新，rename是原子的，更新文件的流程建议使用自动化
  * 先把新文件重命名成.new文件
  * 再把旧文件重命名成.back文件
  * 最后把.new文件更名为最终文件
* 避免文件只更新一半的中间状态，文件监视器只是做了比较简单的热更新, 无法做到文件的完整性校验，因为那样消耗太大，文件完整性校验，一般是在启服加载那种可以有时间去读取meta文件的sha256值并校验文件计算出来的sha256来判断完整性, 本功能无法做到，也不合适
* FileChangeManager会定时5秒检查文件变化，有变化就重新加载文件实现热更

```

void FileChangeManager::_InitWorker()
{
    g_LibEventLoopThreadPool->Send([this]()
    {
       KERNEL_NS::PostCaller([this]()->KERNEL_NS::CoTask<>
       {
           auto poller = KERNEL_NS::TlsUtil::GetPoller();

           _workerPoller.exchange(poller);

           _isWorking.exchange(true, std::memory_order_release);

           // 启动后继续
           while (!_isStart.load(std::memory_order_acquire))
           {
               co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(1));
           }

           g_Log->Info(LOGFMT_OBJ_TAG("file change manage working"));

            // 阻塞等待
            while (!poller->IsQuit() && !_isQuit.load(std::memory_order_acquire))
            {
                // 唤醒者在当前poller执行唤醒时, 一定处于挂起状态, 即使挂起点在Waiting之后, 只要params一样, 那么一定可以使用同一个param唤醒, 如果不想要那么
                co_await KERNEL_NS::CoDelay(KERNEL_NS::TimeSlice::FromSeconds(5));

                // 扫描文件看是否文件变化
                for(auto iter : _filePathRefFileObj)
                {
                    auto monitorInfo = iter.second;
                    if(!monitorInfo->_checkChange->Invoke())
                        continue;

                    auto newObj = monitorInfo->_loadNewObj->Invoke();
                    if(!newObj)
                    {
                        if (g_Log)
                            g_Log->Error(LOGFMT_OBJ_TAG("file: %s, load file fail"), monitorInfo->_path.c_str());

                        continue;
                    }

                    if (g_Log)
                        g_Log->Info(LOGFMT_OBJ_TAG("file: %s, changed, and load new one"), monitorInfo->_path.c_str());

                    {
                        if(monitorInfo->_sourceObj)
                            monitorInfo->_releaseObj->Invoke(monitorInfo->_sourceObj);

                        monitorInfo->_sourceObj = newObj;
                    }
                    
                    for(auto iterHandle : monitorInfo->_keyRefFileChangeHandle)
                    {
                        auto handle = iterHandle.second;
                        if(handle->_notListen.load(std::memory_order_acquire))
                            continue;

                        // 反序列化新数据
                        auto newData = handle->_deserialize->Invoke(newObj);
                        if(!newData)
                        {
                            if (g_Log)
                                g_Log->Warn(LOGFMT_OBJ_TAG("file:%s deserialize from file fail dataName:%s, ")
                                , monitorInfo->_path.c_str(), handle->_dataName.c_str());
                            continue;
                        }

                        // 切换成新数据,移除旧的数据
                        if(auto oldData = handle->_data.exchange(newData))
                        {
                            handle->_release->Invoke(oldData);

                            if (g_Log)
                                g_Log->Info(LOGFMT_OBJ_TAG("new data:%s updated"), handle->_dataName.c_str());
                        }
                    }
                }
            }

           _isWorking.exchange(false, std::memory_order_release);
       }); 
    });
}
```

