// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: com_system_table.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_com_5fsystem_5ftable_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_com_5fsystem_5ftable_2eproto

// KERNEL_INCLUDED
#include <kernel/kernel.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/text_format.h>

#ifdef GetMessage
 #undef GetMessage
#endif


#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021009 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_com_5fsystem_5ftable_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_com_5fsystem_5ftable_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_com_5fsystem_5ftable_2eproto;
namespace CRYSTAL_NET {
namespace service {
class SimpleInfo;
struct SimpleInfoDefaultTypeInternal;
extern SimpleInfoDefaultTypeInternal _SimpleInfo_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> ::CRYSTAL_NET::service::SimpleInfo* Arena::CreateMaybeMessage<::CRYSTAL_NET::service::SimpleInfo>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace CRYSTAL_NET {
namespace service {

// ===================================================================

// AnnotaionInfo[opcode(0), nolog(false), XorEncrypt(false), KeyBase64(false), EnableStorage:(true)]
class SimpleInfo final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:CRYSTAL_NET.service.SimpleInfo) */ , public KERNEL_NS::ICoder {
public:
virtual void Release() override {
    delete this;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message SimpleInfo failed, error: %s"), InitializationErrorString().c_str());
      return false;
    }

    size_t payloadSize = ByteSizeLong();
    if (payloadSize == 0)
      return true;

    if(UNLIKELY(stream.GetBuffer() == NULL))
        stream.Init(payloadSize);

    auto writableSize = stream.GetWritableSize();
    if (writableSize < static_cast<Int64>(payloadSize))
    {
        if(UNLIKELY(!stream.AppendCapacity(static_cast<Int64>(payloadSize) - writableSize)))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("stream append capacity fail IsAttach:%d"), stream.IsAttach());
            return false;
        }
    }

    if (UNLIKELY(!SerializeToArray(stream.GetWriteBegin(), static_cast<Int32>(stream.GetWritableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message SimpleInfo failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftWritePos(payloadSize);
    return true;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message SimpleInfo failed, error: %s"), InitializationErrorString().c_str());
      return false;
    }

    size_t payloadSize = ByteSizeLong();
    if (payloadSize == 0)
      return true;

    if(UNLIKELY(stream.GetBuffer() == NULL))
        stream.Init(payloadSize);

    auto writableSize = stream.GetWritableSize();
    if (writableSize < static_cast<Int64>(payloadSize))
    {
        if(UNLIKELY(!stream.AppendCapacity(static_cast<Int64>(payloadSize) - writableSize)))
        {
            g_Log->Error(LOGFMT_OBJ_TAG("stream append capacity fail IsAttach:%d"), stream.IsAttach());
            return false;
        }
    }

    if (UNLIKELY(!SerializeToArray(stream.GetWriteBegin(), static_cast<Int32>(stream.GetWritableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message SimpleInfo failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftWritePos(payloadSize);
    return true;
}

virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override {
    if (stream.GetReadableSize() == 0)
    {
        Clear();
        return true;
    }

    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message SimpleInfo failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftReadPos(ByteSizeLong());
    return true;
}

virtual bool Decode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override {
    if (stream.GetReadableSize() == 0)
    {
        Clear();
        return true;
    }

    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message SimpleInfo failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftReadPos(ByteSizeLong());
    return true;
}

virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override {
    auto attachStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    attachStream->Attach(stream);
    auto ret = Decode(*attachStream);
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(attachStream);
    return ret;
}

virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override {
    auto attachStream = KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::NewThreadLocal_LibStream();
    attachStream->Attach(stream);
    auto ret = Decode(*attachStream);
    KERNEL_NS::LibStream<KERNEL_NS::_Build::TL>::DeleteThreadLocal_LibStream(attachStream);
    return ret;
}

virtual KERNEL_NS::LibString ToJsonString() const override {
    KERNEL_NS::LibString data;
    if(!::google::protobuf::util::MessageToJsonString(*this, &data.GetRaw()).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Turn JsonString fail:%s"), KERNEL_NS::RttiUtil::GetByObj(this).c_str());
        return "";
    }

    return data;
}

virtual bool ToJsonString(std::string *data) const override {
    if(!::google::protobuf::util::MessageToJsonString(*this, data).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Turn JsonString fail:%s"), KERNEL_NS::RttiUtil::GetByObj(this).c_str());
        return false;
    }

    return true;
}

virtual bool FromJsonString(const Byte8 *data, size_t len) override {
    auto &&jsonString = ::google::protobuf::StringPiece(data, len);
    if(!::google::protobuf::util::JsonStringToMessage(jsonString, this).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("SimpleInfo field JsonStringToMessage fail jsonString:%s, message name:%s"), jsonString.as_string().c_str(), KERNEL_NS::RttiUtil::GetByObj(this).c_str());
        return false;
    }

    return true;
}


 public:
  inline SimpleInfo() : SimpleInfo(nullptr) {}
  ~SimpleInfo() override;
  explicit PROTOBUF_CONSTEXPR SimpleInfo(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  SimpleInfo(const SimpleInfo& from);
  SimpleInfo(SimpleInfo&& from) noexcept
    : SimpleInfo() {
    *this = ::std::move(from);
  }

  inline SimpleInfo& operator=(const SimpleInfo& from) {
    CopyFrom(from);
    return *this;
  }
  inline SimpleInfo& operator=(SimpleInfo&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const SimpleInfo& default_instance() {
    return *internal_default_instance();
  }
  static inline const SimpleInfo* internal_default_instance() {
    return reinterpret_cast<const SimpleInfo*>(
               &_SimpleInfo_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(SimpleInfo& a, SimpleInfo& b) {
    a.Swap(&b);
  }
  inline void Swap(SimpleInfo* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(SimpleInfo* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  SimpleInfo* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<SimpleInfo>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const SimpleInfo& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const SimpleInfo& from) {
    SimpleInfo::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(SimpleInfo* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "CRYSTAL_NET.service.SimpleInfo";
  }
  protected:
  explicit SimpleInfo(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kMaxIncIdFieldNumber = 1,
    kDirtyCountFieldNumber = 2,
    kVersionNoFieldNumber = 3,
  };
  // sint64 MaxIncId = 1;
  void clear_maxincid();
  int64_t maxincid() const;
  void set_maxincid(int64_t value);
  private:
  int64_t _internal_maxincid() const;
  void _internal_set_maxincid(int64_t value);
  public:

  // sint64 DirtyCount = 2;
  void clear_dirtycount();
  int64_t dirtycount() const;
  void set_dirtycount(int64_t value);
  private:
  int64_t _internal_dirtycount() const;
  void _internal_set_dirtycount(int64_t value);
  public:

  // sint64 VersionNo = 3;
  void clear_versionno();
  int64_t versionno() const;
  void set_versionno(int64_t value);
  private:
  int64_t _internal_versionno() const;
  void _internal_set_versionno(int64_t value);
  public:

  // @@protoc_insertion_point(class_scope:CRYSTAL_NET.service.SimpleInfo)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    int64_t maxincid_;
    int64_t dirtycount_;
    int64_t versionno_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_com_5fsystem_5ftable_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// SimpleInfo

// sint64 MaxIncId = 1;
inline void SimpleInfo::clear_maxincid() {
  _impl_.maxincid_ = int64_t{0};
}
inline int64_t SimpleInfo::_internal_maxincid() const {
  return _impl_.maxincid_;
}
inline int64_t SimpleInfo::maxincid() const {
  // @@protoc_insertion_point(field_get:CRYSTAL_NET.service.SimpleInfo.MaxIncId)
  return _internal_maxincid();
}
inline void SimpleInfo::_internal_set_maxincid(int64_t value) {
  
  _impl_.maxincid_ = value;
}
inline void SimpleInfo::set_maxincid(int64_t value) {
  _internal_set_maxincid(value);
  // @@protoc_insertion_point(field_set:CRYSTAL_NET.service.SimpleInfo.MaxIncId)
}

// sint64 DirtyCount = 2;
inline void SimpleInfo::clear_dirtycount() {
  _impl_.dirtycount_ = int64_t{0};
}
inline int64_t SimpleInfo::_internal_dirtycount() const {
  return _impl_.dirtycount_;
}
inline int64_t SimpleInfo::dirtycount() const {
  // @@protoc_insertion_point(field_get:CRYSTAL_NET.service.SimpleInfo.DirtyCount)
  return _internal_dirtycount();
}
inline void SimpleInfo::_internal_set_dirtycount(int64_t value) {
  
  _impl_.dirtycount_ = value;
}
inline void SimpleInfo::set_dirtycount(int64_t value) {
  _internal_set_dirtycount(value);
  // @@protoc_insertion_point(field_set:CRYSTAL_NET.service.SimpleInfo.DirtyCount)
}

// sint64 VersionNo = 3;
inline void SimpleInfo::clear_versionno() {
  _impl_.versionno_ = int64_t{0};
}
inline int64_t SimpleInfo::_internal_versionno() const {
  return _impl_.versionno_;
}
inline int64_t SimpleInfo::versionno() const {
  // @@protoc_insertion_point(field_get:CRYSTAL_NET.service.SimpleInfo.VersionNo)
  return _internal_versionno();
}
inline void SimpleInfo::_internal_set_versionno(int64_t value) {
  
  _impl_.versionno_ = value;
}
inline void SimpleInfo::set_versionno(int64_t value) {
  _internal_set_versionno(value);
  // @@protoc_insertion_point(field_set:CRYSTAL_NET.service.SimpleInfo.VersionNo)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace service
}  // namespace CRYSTAL_NET

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>

class SimpleInfoFactory : public KERNEL_NS::ICoderFactory {
    POOL_CREATE_OBJ_DEFAULT_P1(ICoderFactory, SimpleInfoFactory);
public:

    virtual void Release() override {
        SimpleInfoFactory::Delete_SimpleInfoFactory(this);
    }

    static SimpleInfoFactory *CreateFactory() {
        return SimpleInfoFactory::New_SimpleInfoFactory();
    }

    virtual KERNEL_NS::ICoder *Create() const override {
        return new ::CRYSTAL_NET::service::SimpleInfo();
    }

    virtual KERNEL_NS::ICoder *Create(const KERNEL_NS::ICoder *coder) const override {
        return new ::CRYSTAL_NET::service::SimpleInfo(*dynamic_cast<const ::CRYSTAL_NET::service::SimpleInfo *>(coder));
    }

};

#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_com_5fsystem_5ftable_2eproto
