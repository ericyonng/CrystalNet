// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: syslog.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_syslog_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_syslog_2eproto

// KERNEL_INCLUDED
#include <kernel/kernel.h>
#include <service_common/ServiceCommon.h>
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
#include "com_syslog.pb.h"
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_syslog_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_syslog_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_syslog_2eproto;
namespace CRYSTAL_NET {
namespace service {
class SystemLogDataListReq;
struct SystemLogDataListReqDefaultTypeInternal;
extern SystemLogDataListReqDefaultTypeInternal _SystemLogDataListReq_default_instance_;
class SystemLogDataListRes;
struct SystemLogDataListResDefaultTypeInternal;
extern SystemLogDataListResDefaultTypeInternal _SystemLogDataListRes_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> ::CRYSTAL_NET::service::SystemLogDataListReq* Arena::CreateMaybeMessage<::CRYSTAL_NET::service::SystemLogDataListReq>(Arena*);
template<> ::CRYSTAL_NET::service::SystemLogDataListRes* Arena::CreateMaybeMessage<::CRYSTAL_NET::service::SystemLogDataListRes>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace CRYSTAL_NET {
namespace service {

// ===================================================================

// AnnotaionInfo[opcode(114), nolog(false), XorEncrypt(false), KeyBase64(false)]
class SystemLogDataListReq final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:CRYSTAL_NET.service.SystemLogDataListReq) */ , public KERNEL_NS::ICoder {
public:
virtual void Release() override {
    delete this;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListReq failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListReq failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftWritePos(payloadSize);
    return true;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListReq failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListReq failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftWritePos(payloadSize);
    return true;
}

virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override {
    if (stream.GetReadableSize() == 0)
    {
        Clear();
        return true;
    }

    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message SystemLogDataListReq failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    return true;
}

virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override {
    if (stream.GetReadableSize() == 0)
    {
        Clear();
        return true;
    }

    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message SystemLogDataListReq failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    return true;
}

virtual KERNEL_NS::LibString ToJsonString() const override {
    KERNEL_NS::LibString data;
    if(!::google::protobuf::util::MessageToJsonString(*this, &data.GetRaw()).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Turn JsonString fail:%s"), KERNEL_NS::RttiUtil::GetByObj(this));
        return "";
    }

    return data;
}

virtual bool ToJsonString(std::string *data) const override {
    if(!::google::protobuf::util::MessageToJsonString(*this, data).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Turn JsonString fail:%s"), KERNEL_NS::RttiUtil::GetByObj(this));
        return false;
    }

    return true;
}

virtual bool FromJsonString(const Byte8 *data, size_t len) override {
    auto &&jsonString = ::google::protobuf::StringPiece(data, len);
    if(!::google::protobuf::util::JsonStringToMessage(jsonString, this).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("SimpleInfo field JsonStringToMessage fail jsonString:%s, message name:%s"), jsonString.as_string().c_str(), KERNEL_NS::RttiUtil::GetByObj(this));
        return false;
    }

    return true;
}


 public:
  inline SystemLogDataListReq() : SystemLogDataListReq(nullptr) {}
  ~SystemLogDataListReq() override;
  explicit PROTOBUF_CONSTEXPR SystemLogDataListReq(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  SystemLogDataListReq(const SystemLogDataListReq& from);
  SystemLogDataListReq(SystemLogDataListReq&& from) noexcept
    : SystemLogDataListReq() {
    *this = ::std::move(from);
  }

  inline SystemLogDataListReq& operator=(const SystemLogDataListReq& from) {
    CopyFrom(from);
    return *this;
  }
  inline SystemLogDataListReq& operator=(SystemLogDataListReq&& from) noexcept {
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
  static const SystemLogDataListReq& default_instance() {
    return *internal_default_instance();
  }
  static inline const SystemLogDataListReq* internal_default_instance() {
    return reinterpret_cast<const SystemLogDataListReq*>(
               &_SystemLogDataListReq_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(SystemLogDataListReq& a, SystemLogDataListReq& b) {
    a.Swap(&b);
  }
  inline void Swap(SystemLogDataListReq* other) {
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
  void UnsafeArenaSwap(SystemLogDataListReq* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  SystemLogDataListReq* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<SystemLogDataListReq>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const SystemLogDataListReq& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const SystemLogDataListReq& from) {
    SystemLogDataListReq::MergeImpl(*this, from);
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
  void InternalSwap(SystemLogDataListReq* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "CRYSTAL_NET.service.SystemLogDataListReq";
  }
  protected:
  explicit SystemLogDataListReq(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kBaseLogIdFieldNumber = 1,
    kCountFieldNumber = 2,
  };
  // uint64 BaseLogId = 1;
  void clear_baselogid();
  uint64_t baselogid() const;
  void set_baselogid(uint64_t value);
  private:
  uint64_t _internal_baselogid() const;
  void _internal_set_baselogid(uint64_t value);
  public:

  // sint32 Count = 2;
  void clear_count();
  int32_t count() const;
  void set_count(int32_t value);
  private:
  int32_t _internal_count() const;
  void _internal_set_count(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:CRYSTAL_NET.service.SystemLogDataListReq)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    uint64_t baselogid_;
    int32_t count_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_syslog_2eproto;
};
// -------------------------------------------------------------------

// AnnotaionInfo[opcode(115), nolog(false), XorEncrypt(false), KeyBase64(false)]
class SystemLogDataListRes final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:CRYSTAL_NET.service.SystemLogDataListRes) */ , public KERNEL_NS::ICoder {
public:
virtual void Release() override {
    delete this;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListRes failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListRes failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftWritePos(payloadSize);
    return true;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListRes failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message SystemLogDataListRes failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftWritePos(payloadSize);
    return true;
}

virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) override {
    if (stream.GetReadableSize() == 0)
    {
        Clear();
        return true;
    }

    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message SystemLogDataListRes failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    return true;
}

virtual bool Decode(const KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) override {
    if (stream.GetReadableSize() == 0)
    {
        Clear();
        return true;
    }

    if (UNLIKELY(!ParseFromArray(stream.GetReadBegin(), static_cast<Int32>(stream.GetReadableSize()))))
    {
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message SystemLogDataListRes failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    return true;
}

virtual KERNEL_NS::LibString ToJsonString() const override {
    KERNEL_NS::LibString data;
    if(!::google::protobuf::util::MessageToJsonString(*this, &data.GetRaw()).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Turn JsonString fail:%s"), KERNEL_NS::RttiUtil::GetByObj(this));
        return "";
    }

    return data;
}

virtual bool ToJsonString(std::string *data) const override {
    if(!::google::protobuf::util::MessageToJsonString(*this, data).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("Turn JsonString fail:%s"), KERNEL_NS::RttiUtil::GetByObj(this));
        return false;
    }

    return true;
}

virtual bool FromJsonString(const Byte8 *data, size_t len) override {
    auto &&jsonString = ::google::protobuf::StringPiece(data, len);
    if(!::google::protobuf::util::JsonStringToMessage(jsonString, this).ok())
    {
        g_Log->Warn(LOGFMT_OBJ_TAG("SimpleInfo field JsonStringToMessage fail jsonString:%s, message name:%s"), jsonString.as_string().c_str(), KERNEL_NS::RttiUtil::GetByObj(this));
        return false;
    }

    return true;
}


 public:
  inline SystemLogDataListRes() : SystemLogDataListRes(nullptr) {}
  ~SystemLogDataListRes() override;
  explicit PROTOBUF_CONSTEXPR SystemLogDataListRes(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  SystemLogDataListRes(const SystemLogDataListRes& from);
  SystemLogDataListRes(SystemLogDataListRes&& from) noexcept
    : SystemLogDataListRes() {
    *this = ::std::move(from);
  }

  inline SystemLogDataListRes& operator=(const SystemLogDataListRes& from) {
    CopyFrom(from);
    return *this;
  }
  inline SystemLogDataListRes& operator=(SystemLogDataListRes&& from) noexcept {
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
  static const SystemLogDataListRes& default_instance() {
    return *internal_default_instance();
  }
  static inline const SystemLogDataListRes* internal_default_instance() {
    return reinterpret_cast<const SystemLogDataListRes*>(
               &_SystemLogDataListRes_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(SystemLogDataListRes& a, SystemLogDataListRes& b) {
    a.Swap(&b);
  }
  inline void Swap(SystemLogDataListRes* other) {
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
  void UnsafeArenaSwap(SystemLogDataListRes* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  SystemLogDataListRes* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<SystemLogDataListRes>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const SystemLogDataListRes& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const SystemLogDataListRes& from) {
    SystemLogDataListRes::MergeImpl(*this, from);
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
  void InternalSwap(SystemLogDataListRes* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "CRYSTAL_NET.service.SystemLogDataListRes";
  }
  protected:
  explicit SystemLogDataListRes(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kLogListFieldNumber = 1,
    kCountFieldNumber = 2,
  };
  // repeated .CRYSTAL_NET.service.SystemLogData LogList = 1;
  int loglist_size() const;
  private:
  int _internal_loglist_size() const;
  public:
  void clear_loglist();
  ::CRYSTAL_NET::service::SystemLogData* mutable_loglist(int index);
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::CRYSTAL_NET::service::SystemLogData >*
      mutable_loglist();
  private:
  const ::CRYSTAL_NET::service::SystemLogData& _internal_loglist(int index) const;
  ::CRYSTAL_NET::service::SystemLogData* _internal_add_loglist();
  public:
  const ::CRYSTAL_NET::service::SystemLogData& loglist(int index) const;
  ::CRYSTAL_NET::service::SystemLogData* add_loglist();
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::CRYSTAL_NET::service::SystemLogData >&
      loglist() const;

  // sint32 Count = 2;
  void clear_count();
  int32_t count() const;
  void set_count(int32_t value);
  private:
  int32_t _internal_count() const;
  void _internal_set_count(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:CRYSTAL_NET.service.SystemLogDataListRes)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::CRYSTAL_NET::service::SystemLogData > loglist_;
    int32_t count_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_syslog_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// SystemLogDataListReq

// uint64 BaseLogId = 1;
inline void SystemLogDataListReq::clear_baselogid() {
  _impl_.baselogid_ = uint64_t{0u};
}
inline uint64_t SystemLogDataListReq::_internal_baselogid() const {
  return _impl_.baselogid_;
}
inline uint64_t SystemLogDataListReq::baselogid() const {
  // @@protoc_insertion_point(field_get:CRYSTAL_NET.service.SystemLogDataListReq.BaseLogId)
  return _internal_baselogid();
}
inline void SystemLogDataListReq::_internal_set_baselogid(uint64_t value) {
  
  _impl_.baselogid_ = value;
}
inline void SystemLogDataListReq::set_baselogid(uint64_t value) {
  _internal_set_baselogid(value);
  // @@protoc_insertion_point(field_set:CRYSTAL_NET.service.SystemLogDataListReq.BaseLogId)
}

// sint32 Count = 2;
inline void SystemLogDataListReq::clear_count() {
  _impl_.count_ = 0;
}
inline int32_t SystemLogDataListReq::_internal_count() const {
  return _impl_.count_;
}
inline int32_t SystemLogDataListReq::count() const {
  // @@protoc_insertion_point(field_get:CRYSTAL_NET.service.SystemLogDataListReq.Count)
  return _internal_count();
}
inline void SystemLogDataListReq::_internal_set_count(int32_t value) {
  
  _impl_.count_ = value;
}
inline void SystemLogDataListReq::set_count(int32_t value) {
  _internal_set_count(value);
  // @@protoc_insertion_point(field_set:CRYSTAL_NET.service.SystemLogDataListReq.Count)
}

// -------------------------------------------------------------------

// SystemLogDataListRes

// repeated .CRYSTAL_NET.service.SystemLogData LogList = 1;
inline int SystemLogDataListRes::_internal_loglist_size() const {
  return _impl_.loglist_.size();
}
inline int SystemLogDataListRes::loglist_size() const {
  return _internal_loglist_size();
}
inline ::CRYSTAL_NET::service::SystemLogData* SystemLogDataListRes::mutable_loglist(int index) {
  // @@protoc_insertion_point(field_mutable:CRYSTAL_NET.service.SystemLogDataListRes.LogList)
  return _impl_.loglist_.Mutable(index);
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::CRYSTAL_NET::service::SystemLogData >*
SystemLogDataListRes::mutable_loglist() {
  // @@protoc_insertion_point(field_mutable_list:CRYSTAL_NET.service.SystemLogDataListRes.LogList)
  return &_impl_.loglist_;
}
inline const ::CRYSTAL_NET::service::SystemLogData& SystemLogDataListRes::_internal_loglist(int index) const {
  return _impl_.loglist_.Get(index);
}
inline const ::CRYSTAL_NET::service::SystemLogData& SystemLogDataListRes::loglist(int index) const {
  // @@protoc_insertion_point(field_get:CRYSTAL_NET.service.SystemLogDataListRes.LogList)
  return _internal_loglist(index);
}
inline ::CRYSTAL_NET::service::SystemLogData* SystemLogDataListRes::_internal_add_loglist() {
  return _impl_.loglist_.Add();
}
inline ::CRYSTAL_NET::service::SystemLogData* SystemLogDataListRes::add_loglist() {
  ::CRYSTAL_NET::service::SystemLogData* _add = _internal_add_loglist();
  // @@protoc_insertion_point(field_add:CRYSTAL_NET.service.SystemLogDataListRes.LogList)
  return _add;
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::CRYSTAL_NET::service::SystemLogData >&
SystemLogDataListRes::loglist() const {
  // @@protoc_insertion_point(field_list:CRYSTAL_NET.service.SystemLogDataListRes.LogList)
  return _impl_.loglist_;
}

// sint32 Count = 2;
inline void SystemLogDataListRes::clear_count() {
  _impl_.count_ = 0;
}
inline int32_t SystemLogDataListRes::_internal_count() const {
  return _impl_.count_;
}
inline int32_t SystemLogDataListRes::count() const {
  // @@protoc_insertion_point(field_get:CRYSTAL_NET.service.SystemLogDataListRes.Count)
  return _internal_count();
}
inline void SystemLogDataListRes::_internal_set_count(int32_t value) {
  
  _impl_.count_ = value;
}
inline void SystemLogDataListRes::set_count(int32_t value) {
  _internal_set_count(value);
  // @@protoc_insertion_point(field_set:CRYSTAL_NET.service.SystemLogDataListRes.Count)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace service
}  // namespace CRYSTAL_NET

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>

class SystemLogDataListReqFactory : public KERNEL_NS::ICoderFactory {
    POOL_CREATE_OBJ_DEFAULT_P1(ICoderFactory, SystemLogDataListReqFactory);
public:

    virtual void Release() override {
        SystemLogDataListReqFactory::Delete_SystemLogDataListReqFactory(this);
    }

    static SystemLogDataListReqFactory *CreateFactory() {
        return SystemLogDataListReqFactory::New_SystemLogDataListReqFactory();
    }

    virtual KERNEL_NS::ICoder *Create() const override {
        return new ::CRYSTAL_NET::service::SystemLogDataListReq();
    }

    virtual KERNEL_NS::ICoder *Create(const KERNEL_NS::ICoder *coder) const override {
        return new ::CRYSTAL_NET::service::SystemLogDataListReq(*dynamic_cast<const ::CRYSTAL_NET::service::SystemLogDataListReq *>(coder));
    }

};


class SystemLogDataListResFactory : public KERNEL_NS::ICoderFactory {
    POOL_CREATE_OBJ_DEFAULT_P1(ICoderFactory, SystemLogDataListResFactory);
public:

    virtual void Release() override {
        SystemLogDataListResFactory::Delete_SystemLogDataListResFactory(this);
    }

    static SystemLogDataListResFactory *CreateFactory() {
        return SystemLogDataListResFactory::New_SystemLogDataListResFactory();
    }

    virtual KERNEL_NS::ICoder *Create() const override {
        return new ::CRYSTAL_NET::service::SystemLogDataListRes();
    }

    virtual KERNEL_NS::ICoder *Create(const KERNEL_NS::ICoder *coder) const override {
        return new ::CRYSTAL_NET::service::SystemLogDataListRes(*dynamic_cast<const ::CRYSTAL_NET::service::SystemLogDataListRes *>(coder));
    }

};

#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_syslog_2eproto