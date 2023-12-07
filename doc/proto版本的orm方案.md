* 数据库存储以protobuf的message为单位

* message上添加注解: /// [Storage]表示需要生成自动标脏数据

* 生成的数据降protobuf作为自己的一个成员，并生成每个字段的接口

* 简单字段的生成set/get接口

* 数组字段会生成一个数组对象（内部包含一个protobuf的RepeatField），并支持添加元素，mutable等接口，非const的接口会自动标脏

* 字典字段会生成一个字典对象(内部包含protobuf的Map)，并支持增删元素，mutable等接口，非const接口会自动标脏

* 最后一个message的脏 = 所有message字段的脏(在判断脏标记时采用遍历内部每个字段的办法，并新增一个最后清洗脏标记的时间，以及message最后一次调用非const接口时间, 并记录脏标记的字段id号,)

* 伪代码如下：

* ```
  
  /// [Storage]
  message TestORMData
  {
    uint64 Id = 1;
  
    string Data = 2;
  
    repeated int32 ConfigIdList = 3;
  }
  
  // =>
  class TestORMDataORMData
  {
      POOL_CREATE_OBJ_DEFAULT(TestORMDataORMData);
  
  public:
      void set_userid(UInt64 id)
      {
          _testORMData->set_userid(id);
          _MaskDirty(1);
      }
  
      UInt64 get_userid() const
      {
          return _testORMData->userid();
      }
  
      bool IsDirty() const;
      {
          return _lastInvokeNotConstTimeNs > _lastPurgeTimeNs;
      }
  
      void Purge()
      {
        _lastPurgeTimeNs = KERNEL_NS::LibTime::NowNanoTimestamp();
        _dirtyIndexes.clear();
      }
  
      void SetDirtyCallback(KERNEL_NS::IDelegate<void, TestORMDataORMData *> *cb)
      {
          CRYSTAL_RELEASE_SAFE(_dirtyCb);
          _dirtyCb = cb;
      }
  
      const ::google::protobuf::TestORMData &GetRawData() const
      {
          return *_testORMData;
      }
  
      const std::set<Int32> &GetDirtyIndexList() const
      {
          return _dirtyIndexes;
      }
  
      ::google::RepeatedField<int32> *mutable_configidlist()
      {
          _MaskDirty(3);
  
          return _testORMData->mutable_configidlist();
      }
  
  private:
      void _MaskDirty(Int32 idx)
      {
          const auto isDirty = IsDirty();
          _lastInvokeNotConstTimeNs = KERNEL_NS::LibTime::NowNanoTimestamp();
          _dirtyIndexes.insert(idx);
  
          if(!isDirty && _dirtyCb)
              _dirtyCb->Invoke(this);
      }
  
  private:
      ::google::protobuf::TestORMData *_testORMData;
  
      Int64 _lastPurgeTimeNs;
      Int64 _lastInvokeNotConstTimeNs;
      std::set<Int32> _dirtyIndexes;
      KERNEL_NS::IDelegate<void, TestORMDataORMData *> *_dirtyCb;
  };
  
  
  ```

  