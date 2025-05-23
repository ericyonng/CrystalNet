// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: com.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_com_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_com_2eproto

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
#include <google/protobuf/generated_message_bases.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_com_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_com_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_com_2eproto;
namespace CRYSTAL_NET {
namespace service {
class CreatureAttrKey;
struct CreatureAttrKeyDefaultTypeInternal;
extern CreatureAttrKeyDefaultTypeInternal _CreatureAttrKey_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> ::CRYSTAL_NET::service::CreatureAttrKey* Arena::CreateMaybeMessage<::CRYSTAL_NET::service::CreatureAttrKey>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace CRYSTAL_NET {
namespace service {

enum CreatureAttrKey_ENUM : int {
  CreatureAttrKey_ENUM_BEGIN = 0,
  CreatureAttrKey_ENUM_Atk = 1,
  CreatureAttrKey_ENUM_Def = 2,
  CreatureAttrKey_ENUM_HpLmt = 3,
  CreatureAttrKey_ENUM_HpRcv = 4,
  CreatureAttrKey_ENUM_Cure = 5,
  CreatureAttrKey_ENUM_AngerLmt = 6,
  CreatureAttrKey_ENUM_AngerRcv = 7,
  CreatureAttrKey_ENUM_Anger = 8,
  CreatureAttrKey_ENUM_Hp = 9,
  CreatureAttrKey_ENUM_MoveSpd = 10,
  CreatureAttrKey_ENUM_ActSpd = 11,
  CreatureAttrKey_ENUM_MMoveSpd = 12,
  CreatureAttrKey_ENUM_MActSpd = 13,
  CreatureAttrKey_ENUM_ExtraHp = 14,
  CreatureAttrKey_ENUM_VMoveSpd = 15,
  CreatureAttrKey_ENUM_BuffHp = 17,
  CreatureAttrKey_ENUM_BuffHpLmt = 18,
  CreatureAttrKey_ENUM_BuffHpBloodSpeed = 19,
  CreatureAttrKey_ENUM_Crit = 101,
  CreatureAttrKey_ENUM_Res = 102,
  CreatureAttrKey_ENUM_Hit = 103,
  CreatureAttrKey_ENUM_Prr = 104,
  CreatureAttrKey_ENUM_Ddg = 105,
  CreatureAttrKey_ENUM_Crid = 106,
  CreatureAttrKey_ENUM_Crrd = 107,
  CreatureAttrKey_ENUM_Critv = 108,
  CreatureAttrKey_ENUM_Resv = 109,
  CreatureAttrKey_ENUM_Hitv = 110,
  CreatureAttrKey_ENUM_Prrv = 111,
  CreatureAttrKey_ENUM_Ddgv = 112,
  CreatureAttrKey_ENUM_AtkAdd = 151,
  CreatureAttrKey_ENUM_DefAdd = 152,
  CreatureAttrKey_ENUM_HpLmtAdd = 153,
  CreatureAttrKey_ENUM_Pedm = 201,
  CreatureAttrKey_ENUM_Prdm = 202,
  CreatureAttrKey_ENUM_Medm = 203,
  CreatureAttrKey_ENUM_Mrdm = 204,
  CreatureAttrKey_ENUM_Sedm = 205,
  CreatureAttrKey_ENUM_Srdm = 206,
  CreatureAttrKey_ENUM_Ardm = 210,
  CreatureAttrKey_ENUM_Lv = 251,
  CreatureAttrKey_ENUM_AwakeLv = 252,
  CreatureAttrKey_ENUM_ExpLmt = 253,
  CreatureAttrKey_ENUM_Exp = 254,
  CreatureAttrKey_ENUM_ExpMul = 255,
  CreatureAttrKey_ENUM_ExpAdd = 256,
  CreatureAttrKey_ENUM_GoldMul = 257,
  CreatureAttrKey_ENUM_DoubleExp = 258,
  CreatureAttrKey_ENUM_VipLv = 270,
  CreatureAttrKey_ENUM_Forces = 299,
  CreatureAttrKey_ENUM_CreatureAttrKey_ENUM_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::min(),
  CreatureAttrKey_ENUM_CreatureAttrKey_ENUM_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::max()
};
bool CreatureAttrKey_ENUM_IsValid(int value);
constexpr CreatureAttrKey_ENUM CreatureAttrKey_ENUM_ENUM_MIN = CreatureAttrKey_ENUM_BEGIN;
constexpr CreatureAttrKey_ENUM CreatureAttrKey_ENUM_ENUM_MAX = CreatureAttrKey_ENUM_Forces;
constexpr int CreatureAttrKey_ENUM_ENUM_ARRAYSIZE = CreatureAttrKey_ENUM_ENUM_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* CreatureAttrKey_ENUM_descriptor();
template<typename T>
inline const std::string& CreatureAttrKey_ENUM_Name(T enum_t_value) {
  static_assert(::std::is_same<T, CreatureAttrKey_ENUM>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function CreatureAttrKey_ENUM_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    CreatureAttrKey_ENUM_descriptor(), enum_t_value);
}
inline bool CreatureAttrKey_ENUM_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, CreatureAttrKey_ENUM* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<CreatureAttrKey_ENUM>(
    CreatureAttrKey_ENUM_descriptor(), name, value);
}
// ===================================================================

// AnnotaionInfo[opcode(0), nolog(false), XorEncrypt(false), KeyBase64(false), EnableStorage:(false)]
class CreatureAttrKey final :
    public ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase /* @@protoc_insertion_point(class_definition:CRYSTAL_NET.service.CreatureAttrKey) */ , public KERNEL_NS::ICoder {
public:
virtual void Release() override {
    delete this;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::MT> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message CreatureAttrKey failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message CreatureAttrKey failed, error: %s"), InitializationErrorString().c_str());
        return false;
    }

    stream.ShiftWritePos(payloadSize);
    return true;
}

virtual bool Encode(KERNEL_NS::LibStream<KERNEL_NS::_Build::TL> &stream) const override {
    if (UNLIKELY(!IsInitialized()))
    {
      g_Log->Error(LOGFMT_OBJ_TAG("Encode message CreatureAttrKey failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Encode message CreatureAttrKey failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message CreatureAttrKey failed, error: %s"), InitializationErrorString().c_str());
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
        g_Log->Error(LOGFMT_OBJ_TAG("Decode message CreatureAttrKey failed, error: %s"), InitializationErrorString().c_str());
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
  inline CreatureAttrKey() : CreatureAttrKey(nullptr) {}
  explicit PROTOBUF_CONSTEXPR CreatureAttrKey(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  CreatureAttrKey(const CreatureAttrKey& from);
  CreatureAttrKey(CreatureAttrKey&& from) noexcept
    : CreatureAttrKey() {
    *this = ::std::move(from);
  }

  inline CreatureAttrKey& operator=(const CreatureAttrKey& from) {
    CopyFrom(from);
    return *this;
  }
  inline CreatureAttrKey& operator=(CreatureAttrKey&& from) noexcept {
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
  static const CreatureAttrKey& default_instance() {
    return *internal_default_instance();
  }
  static inline const CreatureAttrKey* internal_default_instance() {
    return reinterpret_cast<const CreatureAttrKey*>(
               &_CreatureAttrKey_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(CreatureAttrKey& a, CreatureAttrKey& b) {
    a.Swap(&b);
  }
  inline void Swap(CreatureAttrKey* other) {
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
  void UnsafeArenaSwap(CreatureAttrKey* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  CreatureAttrKey* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<CreatureAttrKey>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::CopyFrom;
  inline void CopyFrom(const CreatureAttrKey& from) {
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::CopyImpl(*this, from);
  }
  using ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::MergeFrom;
  void MergeFrom(const CreatureAttrKey& from) {
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::MergeImpl(*this, from);
  }
  public:

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "CRYSTAL_NET.service.CreatureAttrKey";
  }
  protected:
  explicit CreatureAttrKey(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  typedef CreatureAttrKey_ENUM ENUM;
  static constexpr ENUM BEGIN =
    CreatureAttrKey_ENUM_BEGIN;
  static constexpr ENUM Atk =
    CreatureAttrKey_ENUM_Atk;
  static constexpr ENUM Def =
    CreatureAttrKey_ENUM_Def;
  static constexpr ENUM HpLmt =
    CreatureAttrKey_ENUM_HpLmt;
  static constexpr ENUM HpRcv =
    CreatureAttrKey_ENUM_HpRcv;
  static constexpr ENUM Cure =
    CreatureAttrKey_ENUM_Cure;
  static constexpr ENUM AngerLmt =
    CreatureAttrKey_ENUM_AngerLmt;
  static constexpr ENUM AngerRcv =
    CreatureAttrKey_ENUM_AngerRcv;
  static constexpr ENUM Anger =
    CreatureAttrKey_ENUM_Anger;
  static constexpr ENUM Hp =
    CreatureAttrKey_ENUM_Hp;
  static constexpr ENUM MoveSpd =
    CreatureAttrKey_ENUM_MoveSpd;
  static constexpr ENUM ActSpd =
    CreatureAttrKey_ENUM_ActSpd;
  static constexpr ENUM MMoveSpd =
    CreatureAttrKey_ENUM_MMoveSpd;
  static constexpr ENUM MActSpd =
    CreatureAttrKey_ENUM_MActSpd;
  static constexpr ENUM ExtraHp =
    CreatureAttrKey_ENUM_ExtraHp;
  static constexpr ENUM VMoveSpd =
    CreatureAttrKey_ENUM_VMoveSpd;
  static constexpr ENUM BuffHp =
    CreatureAttrKey_ENUM_BuffHp;
  static constexpr ENUM BuffHpLmt =
    CreatureAttrKey_ENUM_BuffHpLmt;
  static constexpr ENUM BuffHpBloodSpeed =
    CreatureAttrKey_ENUM_BuffHpBloodSpeed;
  static constexpr ENUM Crit =
    CreatureAttrKey_ENUM_Crit;
  static constexpr ENUM Res =
    CreatureAttrKey_ENUM_Res;
  static constexpr ENUM Hit =
    CreatureAttrKey_ENUM_Hit;
  static constexpr ENUM Prr =
    CreatureAttrKey_ENUM_Prr;
  static constexpr ENUM Ddg =
    CreatureAttrKey_ENUM_Ddg;
  static constexpr ENUM Crid =
    CreatureAttrKey_ENUM_Crid;
  static constexpr ENUM Crrd =
    CreatureAttrKey_ENUM_Crrd;
  static constexpr ENUM Critv =
    CreatureAttrKey_ENUM_Critv;
  static constexpr ENUM Resv =
    CreatureAttrKey_ENUM_Resv;
  static constexpr ENUM Hitv =
    CreatureAttrKey_ENUM_Hitv;
  static constexpr ENUM Prrv =
    CreatureAttrKey_ENUM_Prrv;
  static constexpr ENUM Ddgv =
    CreatureAttrKey_ENUM_Ddgv;
  static constexpr ENUM AtkAdd =
    CreatureAttrKey_ENUM_AtkAdd;
  static constexpr ENUM DefAdd =
    CreatureAttrKey_ENUM_DefAdd;
  static constexpr ENUM HpLmtAdd =
    CreatureAttrKey_ENUM_HpLmtAdd;
  static constexpr ENUM Pedm =
    CreatureAttrKey_ENUM_Pedm;
  static constexpr ENUM Prdm =
    CreatureAttrKey_ENUM_Prdm;
  static constexpr ENUM Medm =
    CreatureAttrKey_ENUM_Medm;
  static constexpr ENUM Mrdm =
    CreatureAttrKey_ENUM_Mrdm;
  static constexpr ENUM Sedm =
    CreatureAttrKey_ENUM_Sedm;
  static constexpr ENUM Srdm =
    CreatureAttrKey_ENUM_Srdm;
  static constexpr ENUM Ardm =
    CreatureAttrKey_ENUM_Ardm;
  static constexpr ENUM Lv =
    CreatureAttrKey_ENUM_Lv;
  static constexpr ENUM AwakeLv =
    CreatureAttrKey_ENUM_AwakeLv;
  static constexpr ENUM ExpLmt =
    CreatureAttrKey_ENUM_ExpLmt;
  static constexpr ENUM Exp =
    CreatureAttrKey_ENUM_Exp;
  static constexpr ENUM ExpMul =
    CreatureAttrKey_ENUM_ExpMul;
  static constexpr ENUM ExpAdd =
    CreatureAttrKey_ENUM_ExpAdd;
  static constexpr ENUM GoldMul =
    CreatureAttrKey_ENUM_GoldMul;
  static constexpr ENUM DoubleExp =
    CreatureAttrKey_ENUM_DoubleExp;
  static constexpr ENUM VipLv =
    CreatureAttrKey_ENUM_VipLv;
  static constexpr ENUM Forces =
    CreatureAttrKey_ENUM_Forces;
  static inline bool ENUM_IsValid(int value) {
    return CreatureAttrKey_ENUM_IsValid(value);
  }
  static constexpr ENUM ENUM_MIN =
    CreatureAttrKey_ENUM_ENUM_MIN;
  static constexpr ENUM ENUM_MAX =
    CreatureAttrKey_ENUM_ENUM_MAX;
  static constexpr int ENUM_ARRAYSIZE =
    CreatureAttrKey_ENUM_ENUM_ARRAYSIZE;
  static inline const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor*
  ENUM_descriptor() {
    return CreatureAttrKey_ENUM_descriptor();
  }
  template<typename T>
  static inline const std::string& ENUM_Name(T enum_t_value) {
    static_assert(::std::is_same<T, ENUM>::value ||
      ::std::is_integral<T>::value,
      "Incorrect type passed to function ENUM_Name.");
    return CreatureAttrKey_ENUM_Name(enum_t_value);
  }
  static inline bool ENUM_Parse(::PROTOBUF_NAMESPACE_ID::ConstStringParam name,
      ENUM* value) {
    return CreatureAttrKey_ENUM_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:CRYSTAL_NET.service.CreatureAttrKey)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
  };
  friend struct ::TableStruct_com_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// CreatureAttrKey

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace service
}  // namespace CRYSTAL_NET

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::CRYSTAL_NET::service::CreatureAttrKey_ENUM> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::CRYSTAL_NET::service::CreatureAttrKey_ENUM>() {
  return ::CRYSTAL_NET::service::CreatureAttrKey_ENUM_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>

class CreatureAttrKeyFactory : public KERNEL_NS::ICoderFactory {
    POOL_CREATE_OBJ_DEFAULT_P1(ICoderFactory, CreatureAttrKeyFactory);
public:

    virtual void Release() override {
        CreatureAttrKeyFactory::Delete_CreatureAttrKeyFactory(this);
    }

    static CreatureAttrKeyFactory *CreateFactory() {
        return CreatureAttrKeyFactory::New_CreatureAttrKeyFactory();
    }

    virtual KERNEL_NS::ICoder *Create() const override {
        return new ::CRYSTAL_NET::service::CreatureAttrKey();
    }

    virtual KERNEL_NS::ICoder *Create(const KERNEL_NS::ICoder *coder) const override {
        return new ::CRYSTAL_NET::service::CreatureAttrKey(*dynamic_cast<const ::CRYSTAL_NET::service::CreatureAttrKey *>(coder));
    }

};

#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_com_2eproto
