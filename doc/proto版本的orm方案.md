* 数据库存储以protobuf的message为单位

* message上添加注解: /// [Storage]表示需要生成自动标脏数据

* 生成的数据降protobuf作为自己的一个成员，并生成每个字段的接口

* 简单字段的生成set/get接口

* 数组字段会生成一个数组对象（内部包含一个protobuf的RepeatField），并支持添加元素，mutable等接口，非const的接口会自动标脏

* 字典字段会生成一个字典对象(内部包含protobuf的Map)，并支持增删元素，mutable等接口，非const接口会自动标脏

* 最后一个message的脏 = 所有message字段的脏(在判断脏标记时采用遍历内部每个字段的办法，并新增一个最后清洗脏标记的时间，以及message最后一次调用非const接口时间, 并记录脏标记的字段id号,)

* 伪代码如下：

* ```
  /// EnableStorage:true
  message TestOrm
  {
      int32 TestInt = 1;
  
      string TestString = 2;
  
      repeated int32 TestIntArray = 3;
  
      repeated string TestStringArray = 4;
  
      TestCustomData TestCustom = 5;
  
      repeated TestCustomData TestCustomArray = 6;
  
      oneof TestOneOf
      {
          int32 TestOneOfInt = 7;
  
          string TestOneOfString = 8;
  
          TestCustomData TestOneOfCustom = 9;
      }
  }
  
  
  class OrmIdEnums
  {
  public:
      enum ENUMS
      {
          UNKNOWN = 0,
          TestOrmOrmData = 1,
          MAX_ORM_ID = TestOrmOrmData,
      };
  };
  
  class TestOrmOrmData : public SERVICE_COMMON_NS::IOrmData
  {
      POOL_CREATE_OBJ_DEFAULT_P1(IOrmData, TestOrmOrmData)
  
  public:
      TestOrmOrmData();
      TestOrmOrmData(::CRYSTAL_NET::service::TestOrm *pb);
      TestOrmOrmData(const TestOrmOrmData &other);
      TestOrmOrmData(TestOrmOrmData &&other);
      TestOrmOrmData(const ::CRYSTAL_NET::service::TestOrm &pb);
      ~TestOrmOrmData();
  
      virtual void Release() override;
  
      TestOrmOrmData &operator =(const ::CRYSTAL_NET::service::TestOrm &pb);
  
      TestOrmOrmData &operator =(const TestOrmOrmData &other);
  
      TestOrmOrmData &operator =(TestOrmOrmData &&other);
  
      virtual KERNEL_NS::LibString ToJsonString() const override;
  
      virtual bool ToJsonString(std::string *data) const override;
  
      virtual bool FromJsonString(const Byte8 *data, size_t len) override;
  
      virtual Int64 GetOrmId() const override{ return 4; }
  
      void Clear();
      const ::CRYSTAL_NET::service::TestOrm *GetPbRawData() const;
  
      void clear_testint();
  
      int32_t testint() const;
  
      void set_testint(int32_t value);
  
      void clear_teststring();
  
      const std::string &teststring() const;
  
      void set_teststring(const std::string &value);
  
      std::string *mutable_teststring();
  
      Int32 testintarray_size() const;
  
      void clear_testintarray();
  
      int32_t testintarray(Int32 idx) const;
  
      void set_testintarray(Int32 idx, int32_t value);
  
      void add_testintarray(int32_t value);
  
      void DeleteArray_testintarray(Int32 idx, Int32 count = 1);
  
      const ::google::protobuf::RepeatedField<int32_t> &testintarray() const;
  
      Int32 teststringarray_size() const;
  
      void clear_teststringarray();
  
      const std::string &teststringarray(Int32 idx) const;
  
      std::string *mutable_teststringarray(Int32 idx);
  
      void set_teststringarray(Int32 idx, const std::string &value);
  
      void set_teststringarray(Int32 idx, std::string &&value);
  
      void set_teststringarray(Int32 idx, const Byte8 *value);
  
      void set_teststringarray(Int32 idx, const Byte8 *value, size_t sz);
  
      std::string *add_teststringarray();
  
      void add_teststringarray(const std::string &value);
  
      void add_teststringarray(std::string &&value);
  
      void add_teststringarray(const Byte8 *value);
  
      void add_teststringarray(const Byte8 *value, size_t sz);
  
      void DeleteArray_teststringarray(Int32 idx, Int32 count = 1);
  
      const ::google::protobuf::RepeatedPtrField<std::string> &teststringarray() const;
  
      KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &mutable_testcustom();
  
      const ::CRYSTAL_NET::service::TestCustomData &testcustom() const;
  
      const KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &testcustom_OrmData() const;
  
      bool has_testcustom() const;
  
      void clear_testcustom();
  
      Int32 testcustomarray_size() const;
  
      KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &mutable_testcustomarray(Int32 idx);
  
      void DeleteArray_testcustomarray(Int32 idx, Int32 count = 1);
  
      const std::vector<KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>> &testcustomarray_OrmDataArray() const;
  
      const ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::TestCustomData> &testcustomarray() const;
  
      const KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &testcustomarray_OrmDataArray(Int32 idx) const;
  
      const ::CRYSTAL_NET::service::TestCustomData &testcustomarray(Int32 idx) const;
  
      KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &add_testcustomarray();
  
      void clear_testcustomarray();
  
      bool has_testoneofint() const;
  
      void clear_testoneofint();
  
      int32_t testoneofint() const;
  
      void set_testoneofint(int32_t value);
  
      bool has_testoneofstring() const;
  
      void clear_testoneofstring();
  
      const std::string &testoneofstring() const;
  
      void set_testoneofstring(const std::string &value);
  
      std::string *mutable_testoneofstring();
  
      KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &mutable_testoneofcustom();
  
      const ::CRYSTAL_NET::service::TestCustomData &testoneofcustom() const;
  
      const KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &testoneofcustom_OrmData() const;
  
      bool has_testoneofcustom() const;
  
      void clear_testoneofcustom();
  
  
  protected:
  
      virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override;
      virtual bool _OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override;
  
      virtual bool _OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override;
      virtual bool _OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override;
  
      virtual void _AttachPb(void *pb) override;
  
  private:
  
      ::CRYSTAL_NET::service::TestOrm *_ormRawPbData;
  
      KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> _testcustom;
  
      std::vector<KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>> _testcustomarray;
  
      KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> _testoneofcustom;
  
  };
  
  class TestOrmOrmDataFactory : public IOrmDataFactory
  {
      POOL_CREATE_OBJ_DEFAULT_P1(IOrmDataFactory, TestOrmOrmDataFactory);
  public:
      TestOrmOrmDataFactory(){}
      ~TestOrmOrmDataFactory(){}
  
      virtual void Release() override { TestOrmOrmDataFactory::DeleteThreadLocal_TestOrmOrmDataFactory(this);}
  
      virtual IOrmData *Create() const override;
      virtual Int64 GetOrmId() const override { return 4; }
  };
  
  implement:
  
  
  
  
  ```
  
  Orm Implement:
  
  ```
  
  POOL_CREATE_OBJ_DEFAULT_IMPL(TestOrmOrmData);
  
  TestOrmOrmData::TestOrmOrmData()
  :_ormRawPbData(new ::CRYSTAL_NET::service::TestOrm)
  {
  }
  
  TestOrmOrmData::TestOrmOrmData(::CRYSTAL_NET::service::TestOrm *pb)
  :_ormRawPbData(NULL)
  {
      AttachPb(pb);
  }
  
  TestOrmOrmData::TestOrmOrmData(const TestOrmOrmData &other)
  :IOrmData(reinterpret_cast<const IOrmData &>(other))
  ,_ormRawPbData(other._ormRawPbData ? new ::CRYSTAL_NET::service::TestOrm(*other._ormRawPbData) : NULL)
  {
      SetAttachPbFlag(false);
      if(_testcustom)
          _testcustom.Release();
  
      if(_ormRawPbData->has_testcustom())
      {
          _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
  
          _testcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
      {
          const auto count = _ormRawPbData->testcustomarray_size();
  
          _testcustomarray.resize(count);
  
          for(Int32 idx = 0; idx < count; ++idx)
          {
          _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
  
          _testcustomarray[idx].SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
          }
      }
  
  
      if(_testoneofcustom)
          _testoneofcustom.Release();
  
      if(_ormRawPbData->has_testoneofcustom())
      {
          _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
  
          _testoneofcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
  }
  
  TestOrmOrmData::TestOrmOrmData(TestOrmOrmData &&other)
  :IOrmData(std::forward<IOrmData>(other))
  ,_ormRawPbData(other._ormRawPbData)
  {
      other._ormRawPbData = NULL;
      _testcustom = std::move(other._testcustom);
  
      _testcustomarray = std::move(other._testcustomarray);
  
      _testoneofcustom = std::move(other._testoneofcustom);
  
  }
  
  TestOrmOrmData::TestOrmOrmData(const ::CRYSTAL_NET::service::TestOrm &pb)
  :_ormRawPbData(new ::CRYSTAL_NET::service::TestOrm(pb))
  {
      if(_testcustom)
          _testcustom.Release();
  
      if(_ormRawPbData->has_testcustom())
      {
          _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
  
          _testcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
      {
          const auto count = _ormRawPbData->testcustomarray_size();
  
          _testcustomarray.resize(count);
  
          for(Int32 idx = 0; idx < count; ++idx)
          {
          _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
  
          _testcustomarray[idx].SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
          }
      }
  
  
      if(_testoneofcustom)
          _testoneofcustom.Release();
  
      if(_ormRawPbData->has_testoneofcustom())
      {
          _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
  
          _testoneofcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
  
  }
  
  TestOrmOrmData::~TestOrmOrmData()
  {
      if(LIKELY(!IsAttachPb()))
          CRYSTAL_RELEASE_SAFE(_ormRawPbData);
  }
  
  void TestOrmOrmData::Release()
  {
      TestOrmOrmData::DeleteThreadLocal_TestOrmOrmData(this);
  }
  
  TestOrmOrmData &TestOrmOrmData::operator =(const ::CRYSTAL_NET::service::TestOrm &pb)
  {
      if(LIKELY(!IsAttachPb()))
          CRYSTAL_RELEASE_SAFE(_ormRawPbData);
  
      SetAttachPbFlag(false);
      _ormRawPbData = new ::CRYSTAL_NET::service::TestOrm(pb);
      if(_testcustom)
          _testcustom.Release();
  
      if(_ormRawPbData->has_testcustom())
      {
          _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
  
          _testcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
      {
          const auto count = _ormRawPbData->testcustomarray_size();
  
          _testcustomarray.resize(count);
  
          for(Int32 idx = 0; idx < count; ++idx)
          {
          _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
  
          _testcustomarray[idx].SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
          }
      }
  
  
      if(_testoneofcustom)
          _testoneofcustom.Release();
  
      if(_ormRawPbData->has_testoneofcustom())
      {
          _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
  
          _testoneofcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
      _MaskDirty(true);
      return *this;
  }
  
  TestOrmOrmData &TestOrmOrmData::operator =(const TestOrmOrmData &other)
  {
      if(this == &other)
          return *this;
  
      IOrmData::operator =(reinterpret_cast<const IOrmData &>(other));
      if(LIKELY(!IsAttachPb()))
          CRYSTAL_RELEASE_SAFE(_ormRawPbData);
  
      _ormRawPbData = NULL;
      SetAttachPbFlag(false);
      if(other._ormRawPbData)
          _ormRawPbData = new ::CRYSTAL_NET::service::TestOrm(*other._ormRawPbData);
      if(_ormRawPbData)
      {
              if(_testcustom)
                  _testcustom.Release();
          
              if(_ormRawPbData->has_testcustom())
              {
                  _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
          
                  _testcustom.SetClosureDelegate([](void *ptr){
                      SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
                  }) ;
          
                  _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                      _MaskDirty(true);
                  }) ;
          
              }
          
              {
                  const auto count = _ormRawPbData->testcustomarray_size();
          
                  _testcustomarray.resize(count);
          
                  for(Int32 idx = 0; idx < count; ++idx)
                  {
                  _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
          
                  _testcustomarray[idx].SetClosureDelegate([](void *ptr){
                      SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
                  }) ;
          
                  _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                      _MaskDirty(true);
                  }) ;
          
                  }
              }
          
          
              if(_testoneofcustom)
                  _testoneofcustom.Release();
          
              if(_ormRawPbData->has_testoneofcustom())
              {
                  _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
          
                  _testoneofcustom.SetClosureDelegate([](void *ptr){
                      SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
                  }) ;
          
                  _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                      _MaskDirty(true);
                  }) ;
          
              }
          
      }
      _MaskDirty(true);
  
      return *this;
  }
  
  TestOrmOrmData &TestOrmOrmData::operator =(TestOrmOrmData &&other)
  {
      if(this == &other)
          return *this;
  
      IOrmData::operator =(std::forward<IOrmData>(other));
      _ormRawPbData = other._ormRawPbData;
      other._ormRawPbData = NULL;
  
      _testcustom = std::move(other._testcustom);
  
      _testcustomarray = std::move(other._testcustomarray);
  
      _testoneofcustom = std::move(other._testoneofcustom);
  
  
      return *this;
  }
  
  void TestOrmOrmData::Clear()
  {
      _testcustom.Release();
  
      _testcustomarray.clear();
  
      _testoneofcustom.Release();
  
  
      if(_ormRawPbData)
          _ormRawPbData->Clear();
  
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::_AttachPb(void *pb)
  {
      if(LIKELY(!IsAttachPb()))
          CRYSTAL_RELEASE_SAFE(_ormRawPbData);
  
      _ormRawPbData = reinterpret_cast<::CRYSTAL_NET::service::TestOrm *>(pb);
  
      if(_testcustom)
          _testcustom.Release();
  
      if(_ormRawPbData->has_testcustom())
      {
          _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
  
          _testcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
      {
          const auto count = _ormRawPbData->testcustomarray_size();
  
          _testcustomarray.resize(count);
  
          for(Int32 idx = 0; idx < count; ++idx)
          {
          _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
  
          _testcustomarray[idx].SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
          }
      }
  
  
      if(_testoneofcustom)
          _testoneofcustom.Release();
  
      if(_ormRawPbData->has_testoneofcustom())
      {
          _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
  
          _testoneofcustom.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      }
  
  
  }
  
  KERNEL_NS::LibString TestOrmOrmData::ToJsonString() const
  {
      return _ormRawPbData->ToJsonString();
  }
  
  bool TestOrmOrmData::ToJsonString(std::string *data) const
  {
      return _ormRawPbData->ToJsonString(data);
  }
  
  bool TestOrmOrmData::FromJsonString(const Byte8 *data, size_t len)
  {
      const auto ret = _ormRawPbData->FromJsonString(data, len);
      if(ret)
      {
          if(_testcustom)
              _testcustom.Release();
      
          if(_ormRawPbData->has_testcustom())
          {
              _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
      
              _testcustom.SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
          }
      
          {
              const auto count = _ormRawPbData->testcustomarray_size();
      
              _testcustomarray.resize(count);
      
              for(Int32 idx = 0; idx < count; ++idx)
              {
              _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
      
              _testcustomarray[idx].SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
              }
          }
      
      
          if(_testoneofcustom)
              _testoneofcustom.Release();
      
          if(_ormRawPbData->has_testoneofcustom())
          {
              _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
      
              _testoneofcustom.SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
          }
      
      }
  
      return ret;
  }
  
  const ::CRYSTAL_NET::service::TestOrm *TestOrmOrmData::GetPbRawData() const
  {
      return _ormRawPbData;
  }
  
  void TestOrmOrmData::clear_testint()
  {
      _ormRawPbData->clear_testint();
      _MaskDirty(true);
  }
  
  int32_t TestOrmOrmData::testint() const
  {
      return _ormRawPbData->testint();
  }
  
  void TestOrmOrmData::set_testint(int32_t value)
  {
      _ormRawPbData->set_testint(value);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::clear_teststring()
  {
      _ormRawPbData->clear_teststring();
      _MaskDirty(true);
  }
  
  const std::string &TestOrmOrmData::teststring() const
  {
      return _ormRawPbData->teststring();
  }
  
  void TestOrmOrmData::set_teststring(const std::string &value)
  {
      _ormRawPbData->set_teststring(value);
      _MaskDirty(true);
  }
  
  std::string *TestOrmOrmData::mutable_teststring()
  {
      _MaskDirty(true);
      return _ormRawPbData->mutable_teststring();
  }
  
  Int32 TestOrmOrmData::testintarray_size() const
  {
      return _ormRawPbData->testintarray_size();
  }
  
  void TestOrmOrmData::clear_testintarray()
  {
      _ormRawPbData->clear_testintarray();
      _MaskDirty(true);
  }
  
  int32_t TestOrmOrmData::testintarray(Int32 idx) const
  {
      return _ormRawPbData->testintarray(idx);
  }
  
  void TestOrmOrmData::set_testintarray(Int32 idx, int32_t value)
  {
      _ormRawPbData->set_testintarray(idx, value);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::add_testintarray(int32_t value)
  {
      _ormRawPbData->add_testintarray(value);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::DeleteArray_testintarray(Int32 idx, Int32 count)
  {
      _ormRawPbData->mutable_testintarray()->erase(_ormRawPbData->testintarray().begin() + idx, _ormRawPbData->testintarray().begin() + idx + count);
      _MaskDirty(true);
  }
  
  const ::google::protobuf::RepeatedField<int32_t> &TestOrmOrmData::testintarray() const
  {
      return _ormRawPbData->testintarray();
  }
  
  Int32 TestOrmOrmData::teststringarray_size() const
  {
      return _ormRawPbData->teststringarray_size();
  }
  
  void TestOrmOrmData::clear_teststringarray()
  {
      _ormRawPbData->clear_teststringarray();
      _MaskDirty(true);
  }
  
  const std::string &TestOrmOrmData::teststringarray(Int32 idx) const
  {
      return _ormRawPbData->teststringarray(idx);
  }
  
  std::string *TestOrmOrmData::mutable_teststringarray(Int32 idx)
  {
      _MaskDirty(true);
      return _ormRawPbData->mutable_teststringarray(idx);
  }
  
  void TestOrmOrmData::set_teststringarray(Int32 idx, const std::string &value)
  {
      _ormRawPbData->set_teststringarray(idx, value);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::set_teststringarray(Int32 idx, std::string &&value)
  {
      _ormRawPbData->set_teststringarray(idx, std::forward<std::string>(value));
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::set_teststringarray(Int32 idx, const Byte8 *value)
  {
      _ormRawPbData->set_teststringarray(idx, value);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::set_teststringarray(Int32 idx, const Byte8 *value, size_t sz)
  {
      _ormRawPbData->set_teststringarray(idx, value, sz);
      _MaskDirty(true);
  }
  
  std::string *TestOrmOrmData::add_teststringarray()
  {
      auto newElem = _ormRawPbData->add_teststringarray();
      _MaskDirty(true);
      return newElem;
  }
  
  void TestOrmOrmData::add_teststringarray(const std::string &value)
  {
      _ormRawPbData->add_teststringarray(value);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::add_teststringarray(std::string &&value)
  {
      _ormRawPbData->add_teststringarray(std::forward<std::string>(value));
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::add_teststringarray(const Byte8 *value)
  {
      _ormRawPbData->add_teststringarray(value);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::add_teststringarray(const Byte8 *value, size_t sz)
  {
      _ormRawPbData->add_teststringarray(value, sz);
      _MaskDirty(true);
  }
  
  void TestOrmOrmData::DeleteArray_teststringarray(Int32 idx, Int32 count)
  {
      _ormRawPbData->mutable_teststringarray()->DeleteSubrange(idx, count);
      _MaskDirty(true);
  }
  
  const ::google::protobuf::RepeatedPtrField<std::string> &TestOrmOrmData::teststringarray() const
  {
      return _ormRawPbData->teststringarray();
  }
  
  KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &TestOrmOrmData::mutable_testcustom()
  {
      if(LIKELY(_testcustom))
          return _testcustom;
  
      _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
  
      _testcustom.SetClosureDelegate([](void *ptr){
          SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
      }) ;
  
      _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
         _MaskDirty(true);
      }) ;
  
      return _testcustom;
  }
  
  const ::CRYSTAL_NET::service::TestCustomData &TestOrmOrmData::testcustom() const
  {
      return _ormRawPbData->testcustom();
  }
  
  const KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &TestOrmOrmData::testcustom_OrmData() const
  {
      return _testcustom;
  }
  
  bool TestOrmOrmData::has_testcustom() const
  {
      return _ormRawPbData->has_testcustom();
  }
  
  void TestOrmOrmData::clear_testcustom()
  {
      if(_testcustom)
          _testcustom.Release();
  
      _ormRawPbData->clear_testcustom();
      _MaskDirty(true);
  }
  
  Int32 TestOrmOrmData::testcustomarray_size() const
  {
      return _ormRawPbData->testcustomarray_size();
  }
  
  KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &TestOrmOrmData::mutable_testcustomarray(Int32 idx)
  {
      return _testcustomarray[idx];
  }
  
  void TestOrmOrmData::DeleteArray_testcustomarray(Int32 idx, Int32 count)
  {
      for(Int32 pos = idx + count - 1; pos >= idx; --pos)
      {
          _testcustomarray.erase(_testcustomarray.begin() + pos);
      }
  
      _ormRawPbData->mutable_testcustomarray()->DeleteSubrange(idx, count);
      _MaskDirty(true);
  }
  
  const std::vector<KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>> &TestOrmOrmData::testcustomarray_OrmDataArray() const
  {
      return _testcustomarray;
  }
  
  const ::google::protobuf::RepeatedPtrField<::CRYSTAL_NET::service::TestCustomData> &TestOrmOrmData::testcustomarray() const
  {
      return _ormRawPbData->testcustomarray();
  }
  
  const KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &TestOrmOrmData::testcustomarray_OrmDataArray(Int32 idx) const
  {
      return _testcustomarray[idx];
  }
  
  const ::CRYSTAL_NET::service::TestCustomData &TestOrmOrmData::testcustomarray(Int32 idx) const
  {
      return _ormRawPbData->testcustomarray(idx);
  }
  
  KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &TestOrmOrmData::add_testcustomarray()
  {
      auto newPb = _ormRawPbData->add_testcustomarray();
      _testcustomarray.push_back(KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete>());
      auto &elem = _testcustomarray.back();
          elem = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(newPb);
  
          elem.SetClosureDelegate([](void *ptr){
              SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
          }) ;
  
          elem->SetMaskDirtyCallback([this](IOrmData *ptr){
              _MaskDirty(true);
          }) ;
  
      _MaskDirty(true);
      return elem;
  }
  
  void TestOrmOrmData::clear_testcustomarray()
  {
      _ormRawPbData->clear_testcustomarray();
      _testcustomarray.clear();
      _MaskDirty(true);
  }
  
  bool TestOrmOrmData::has_testoneofint() const
  {
      return _ormRawPbData->has_testoneofint();
  }
  
  void TestOrmOrmData::clear_testoneofint()
  {
      _ormRawPbData->clear_testoneofint();
      _MaskDirty(true);
  }
  
  int32_t TestOrmOrmData::testoneofint() const
  {
      return _ormRawPbData->testoneofint();
  }
  
  void TestOrmOrmData::set_testoneofint(int32_t value)
  {
      _ormRawPbData->set_testoneofint(value);
      _MaskDirty(true);
  }
  
  bool TestOrmOrmData::has_testoneofstring() const
  {
      return _ormRawPbData->has_testoneofstring();
  }
  
  void TestOrmOrmData::clear_testoneofstring()
  {
      _ormRawPbData->clear_testoneofstring();
      _MaskDirty(true);
  }
  
  const std::string &TestOrmOrmData::testoneofstring() const
  {
      return _ormRawPbData->testoneofstring();
  }
  
  void TestOrmOrmData::set_testoneofstring(const std::string &value)
  {
      _ormRawPbData->set_testoneofstring(value);
      _MaskDirty(true);
  }
  
  std::string *TestOrmOrmData::mutable_testoneofstring()
  {
      _MaskDirty(true);
      return _ormRawPbData->mutable_testoneofstring();
  }
  
  KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &TestOrmOrmData::mutable_testoneofcustom()
  {
      if(LIKELY(_testoneofcustom))
          return _testoneofcustom;
  
      _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
  
      _testoneofcustom.SetClosureDelegate([](void *ptr){
          SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
      }) ;
  
      _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
         _MaskDirty(true);
      }) ;
  
      return _testoneofcustom;
  }
  
  const ::CRYSTAL_NET::service::TestCustomData &TestOrmOrmData::testoneofcustom() const
  {
      return _ormRawPbData->testoneofcustom();
  }
  
  const KERNEL_NS::SmartPtr<TestCustomDataOrmData, KERNEL_NS::AutoDelMethods::CustomDelete> &TestOrmOrmData::testoneofcustom_OrmData() const
  {
      return _testoneofcustom;
  }
  
  bool TestOrmOrmData::has_testoneofcustom() const
  {
      return _ormRawPbData->has_testoneofcustom();
  }
  
  void TestOrmOrmData::clear_testoneofcustom()
  {
      if(_testoneofcustom)
          _testoneofcustom.Release();
  
      _ormRawPbData->clear_testoneofcustom();
      _MaskDirty(true);
  }
  
  bool TestOrmOrmData::_OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const
  {
      return _ormRawPbData->Encode(stream);
  }
  
  bool TestOrmOrmData::_OnEncode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const
  {
      return _ormRawPbData->Encode(stream);
  }
  
  bool TestOrmOrmData::_OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream)
  {
      const auto ret = _ormRawPbData->Decode(stream);
      if(ret)
      {
          if(_testcustom)
              _testcustom.Release();
      
          if(_ormRawPbData->has_testcustom())
          {
              _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
      
              _testcustom.SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
          }
      
          {
              const auto count = _ormRawPbData->testcustomarray_size();
      
              _testcustomarray.resize(count);
      
              for(Int32 idx = 0; idx < count; ++idx)
              {
              _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
      
              _testcustomarray[idx].SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
              }
          }
      
      
          if(_testoneofcustom)
              _testoneofcustom.Release();
      
          if(_ormRawPbData->has_testoneofcustom())
          {
              _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
      
              _testoneofcustom.SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
          }
      
      }
  
      return ret;
  }
  
  bool TestOrmOrmData::_OnDecode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream)
  {
      const auto ret = _ormRawPbData->Decode(stream);
      if(ret)
      {
          if(_testcustom)
              _testcustom.Release();
      
          if(_ormRawPbData->has_testcustom())
          {
              _testcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustom());
      
              _testcustom.SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
          }
      
          {
              const auto count = _ormRawPbData->testcustomarray_size();
      
              _testcustomarray.resize(count);
      
              for(Int32 idx = 0; idx < count; ++idx)
              {
              _testcustomarray[idx] = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testcustomarray(idx));
      
              _testcustomarray[idx].SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testcustomarray[idx]->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
              }
          }
      
      
          if(_testoneofcustom)
              _testoneofcustom.Release();
      
          if(_ormRawPbData->has_testoneofcustom())
          {
              _testoneofcustom = SERVICE_COMMON_NS::TestCustomDataOrmData::NewThreadLocal_TestCustomDataOrmData(_ormRawPbData->mutable_testoneofcustom());
      
              _testoneofcustom.SetClosureDelegate([](void *ptr){
                  SERVICE_COMMON_NS::TestCustomDataOrmData::DeleteThreadLocal_TestCustomDataOrmData(KERNEL_NS::KernelCastTo<SERVICE_COMMON_NS::TestCustomDataOrmData>(ptr));
              }) ;
      
              _testoneofcustom->SetMaskDirtyCallback([this](IOrmData *ptr){
                  _MaskDirty(true);
              }) ;
      
          }
      
      }
  
      return ret;
  }
  
  POOL_CREATE_OBJ_DEFAULT_IMPL(TestOrmOrmDataFactory);
  
  IOrmData *TestOrmOrmDataFactory::Create() const
  {
      return TestOrmOrmData::NewThreadLocal_TestOrmOrmData();
  }
  
  ```
  
  