#include <pch.h>
// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: com_variant.proto

#include <protocols/cplusplus/library/com_variant.pb.h>
POOL_CREATE_OBJ_DEFAULT_IMPL(VariantParamTypeFactory);
POOL_CREATE_OBJ_DEFAULT_IMPL(VariantParamFactory);

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace CRYSTAL_NET {
namespace service {
PROTOBUF_CONSTEXPR VariantParamType::VariantParamType(
    ::_pbi::ConstantInitialized) {}
struct VariantParamTypeDefaultTypeInternal {
  PROTOBUF_CONSTEXPR VariantParamTypeDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~VariantParamTypeDefaultTypeInternal() {}
  union {
    VariantParamType _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 VariantParamTypeDefaultTypeInternal _VariantParamType_default_instance_;
PROTOBUF_CONSTEXPR VariantParam::VariantParam(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.varianttype_)*/0
  , /*decltype(_impl_.VariantValue_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}
  , /*decltype(_impl_._oneof_case_)*/{}} {}
struct VariantParamDefaultTypeInternal {
  PROTOBUF_CONSTEXPR VariantParamDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~VariantParamDefaultTypeInternal() {}
  union {
    VariantParam _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 VariantParamDefaultTypeInternal _VariantParam_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
static ::_pb::Metadata file_level_metadata_com_5fvariant_2eproto[2];
static const ::_pb::EnumDescriptor* file_level_enum_descriptors_com_5fvariant_2eproto[1];
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_com_5fvariant_2eproto = nullptr;

const uint32_t TableStruct_com_5fvariant_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::VariantParamType, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::VariantParam, _internal_metadata_),
  ~0u,  // no _extensions_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::VariantParam, _impl_._oneof_case_[0]),
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::VariantParam, _impl_.varianttype_),
  ::_pbi::kInvalidFieldOffsetTag,
  ::_pbi::kInvalidFieldOffsetTag,
  ::_pbi::kInvalidFieldOffsetTag,
  ::_pbi::kInvalidFieldOffsetTag,
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::VariantParam, _impl_.VariantValue_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::CRYSTAL_NET::service::VariantParamType)},
  { 6, -1, -1, sizeof(::CRYSTAL_NET::service::VariantParam)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::CRYSTAL_NET::service::_VariantParamType_default_instance_._instance,
  &::CRYSTAL_NET::service::_VariantParam_default_instance_._instance,
};

const char descriptor_table_protodef_com_5fvariant_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\021com_variant.proto\022\023CRYSTAL_NET.service"
  "\"e\n\020VariantParamType\"Q\n\005ENUMS\022\013\n\007UNKNOWN"
  "\020\000\022\n\n\006STRING\020\001\022\t\n\005VALUE\020\002\022\022\n\016UNSIGNED_VA"
  "LUE\020\003\022\020\n\014DOUBLE_VALUE\020\004\"\213\001\n\014VariantParam"
  "\022\023\n\013VariantType\030\001 \001(\021\022\022\n\010StrValue\030\002 \001(\014H"
  "\000\022\022\n\010IntValue\030\003 \001(\022H\000\022\027\n\rUnSignedValue\030\004"
  " \001(\004H\000\022\025\n\013DoubleValue\030\005 \001(\001H\000B\016\n\014Variant"
  "Valueb\006proto3"
  ;
static ::_pbi::once_flag descriptor_table_com_5fvariant_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_com_5fvariant_2eproto = {
    false, false, 293, descriptor_table_protodef_com_5fvariant_2eproto,
    "com_variant.proto",
    &descriptor_table_com_5fvariant_2eproto_once, nullptr, 0, 2,
    schemas, file_default_instances, TableStruct_com_5fvariant_2eproto::offsets,
    file_level_metadata_com_5fvariant_2eproto, file_level_enum_descriptors_com_5fvariant_2eproto,
    file_level_service_descriptors_com_5fvariant_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_com_5fvariant_2eproto_getter() {
  return &descriptor_table_com_5fvariant_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_com_5fvariant_2eproto(&descriptor_table_com_5fvariant_2eproto);
namespace CRYSTAL_NET {
namespace service {
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* VariantParamType_ENUMS_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_com_5fvariant_2eproto);
  return file_level_enum_descriptors_com_5fvariant_2eproto[0];
}
bool VariantParamType_ENUMS_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      return true;
    default:
      return false;
  }
}

#if (__cplusplus < 201703) && (!defined(_MSC_VER) || (_MSC_VER >= 1900 && _MSC_VER < 1912))
constexpr VariantParamType_ENUMS VariantParamType::UNKNOWN;
constexpr VariantParamType_ENUMS VariantParamType::STRING;
constexpr VariantParamType_ENUMS VariantParamType::VALUE;
constexpr VariantParamType_ENUMS VariantParamType::UNSIGNED_VALUE;
constexpr VariantParamType_ENUMS VariantParamType::DOUBLE_VALUE;
constexpr VariantParamType_ENUMS VariantParamType::ENUMS_MIN;
constexpr VariantParamType_ENUMS VariantParamType::ENUMS_MAX;
constexpr int VariantParamType::ENUMS_ARRAYSIZE;
#endif  // (__cplusplus < 201703) && (!defined(_MSC_VER) || (_MSC_VER >= 1900 && _MSC_VER < 1912))

// ===================================================================

class VariantParamType::_Internal {
 public:
};

VariantParamType::VariantParamType(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase(arena, is_message_owned) {
  // @@protoc_insertion_point(arena_constructor:CRYSTAL_NET.service.VariantParamType)
}
VariantParamType::VariantParamType(const VariantParamType& from)
  : ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase() {
  VariantParamType* const _this = this; (void)_this;
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:CRYSTAL_NET.service.VariantParamType)
}





const ::PROTOBUF_NAMESPACE_ID::Message::ClassData VariantParamType::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::CopyImpl,
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::MergeImpl,
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*VariantParamType::GetClassData() const { return &_class_data_; }







::PROTOBUF_NAMESPACE_ID::Metadata VariantParamType::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_com_5fvariant_2eproto_getter, &descriptor_table_com_5fvariant_2eproto_once,
      file_level_metadata_com_5fvariant_2eproto[0]);
}

// ===================================================================

class VariantParam::_Internal {
 public:
};

VariantParam::VariantParam(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:CRYSTAL_NET.service.VariantParam)
}
VariantParam::VariantParam(const VariantParam& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  VariantParam* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.varianttype_){}
    , decltype(_impl_.VariantValue_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , /*decltype(_impl_._oneof_case_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _this->_impl_.varianttype_ = from._impl_.varianttype_;
  clear_has_VariantValue();
  switch (from.VariantValue_case()) {
    case kStrValue: {
      _this->_internal_set_strvalue(from._internal_strvalue());
      break;
    }
    case kIntValue: {
      _this->_internal_set_intvalue(from._internal_intvalue());
      break;
    }
    case kUnSignedValue: {
      _this->_internal_set_unsignedvalue(from._internal_unsignedvalue());
      break;
    }
    case kDoubleValue: {
      _this->_internal_set_doublevalue(from._internal_doublevalue());
      break;
    }
    case VARIANTVALUE_NOT_SET: {
      break;
    }
  }
  // @@protoc_insertion_point(copy_constructor:CRYSTAL_NET.service.VariantParam)
}

inline void VariantParam::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.varianttype_){0}
    , decltype(_impl_.VariantValue_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , /*decltype(_impl_._oneof_case_)*/{}
  };
  clear_has_VariantValue();
}

VariantParam::~VariantParam() {
  // @@protoc_insertion_point(destructor:CRYSTAL_NET.service.VariantParam)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void VariantParam::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  if (has_VariantValue()) {
    clear_VariantValue();
  }
}

void VariantParam::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void VariantParam::clear_VariantValue() {
// @@protoc_insertion_point(one_of_clear_start:CRYSTAL_NET.service.VariantParam)
  switch (VariantValue_case()) {
    case kStrValue: {
      _impl_.VariantValue_.strvalue_.Destroy();
      break;
    }
    case kIntValue: {
      // No need to clear
      break;
    }
    case kUnSignedValue: {
      // No need to clear
      break;
    }
    case kDoubleValue: {
      // No need to clear
      break;
    }
    case VARIANTVALUE_NOT_SET: {
      break;
    }
  }
  _impl_._oneof_case_[0] = VARIANTVALUE_NOT_SET;
}


void VariantParam::Clear() {
// @@protoc_insertion_point(message_clear_start:CRYSTAL_NET.service.VariantParam)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.varianttype_ = 0;
  clear_VariantValue();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* VariantParam::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // sint32 VariantType = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          _impl_.varianttype_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // bytes StrValue = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 18)) {
          auto str = _internal_mutable_strvalue();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // sint64 IntValue = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 24)) {
          _internal_set_intvalue(::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag64(&ptr));
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint64 UnSignedValue = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 32)) {
          _internal_set_unsignedvalue(::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr));
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // double DoubleValue = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 41)) {
          _internal_set_doublevalue(::PROTOBUF_NAMESPACE_ID::internal::UnalignedLoad<double>(ptr));
          ptr += sizeof(double);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* VariantParam::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:CRYSTAL_NET.service.VariantParam)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // sint32 VariantType = 1;
  if (this->_internal_varianttype() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt32ToArray(1, this->_internal_varianttype(), target);
  }

  // bytes StrValue = 2;
  if (_internal_has_strvalue()) {
    target = stream->WriteBytesMaybeAliased(
        2, this->_internal_strvalue(), target);
  }

  // sint64 IntValue = 3;
  if (_internal_has_intvalue()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt64ToArray(3, this->_internal_intvalue(), target);
  }

  // uint64 UnSignedValue = 4;
  if (_internal_has_unsignedvalue()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt64ToArray(4, this->_internal_unsignedvalue(), target);
  }

  // double DoubleValue = 5;
  if (_internal_has_doublevalue()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteDoubleToArray(5, this->_internal_doublevalue(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:CRYSTAL_NET.service.VariantParam)
  return target;
}

size_t VariantParam::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:CRYSTAL_NET.service.VariantParam)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // sint32 VariantType = 1;
  if (this->_internal_varianttype() != 0) {
    total_size += ::_pbi::WireFormatLite::SInt32SizePlusOne(this->_internal_varianttype());
  }

  switch (VariantValue_case()) {
    // bytes StrValue = 2;
    case kStrValue: {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::BytesSize(
          this->_internal_strvalue());
      break;
    }
    // sint64 IntValue = 3;
    case kIntValue: {
      total_size += ::_pbi::WireFormatLite::SInt64SizePlusOne(this->_internal_intvalue());
      break;
    }
    // uint64 UnSignedValue = 4;
    case kUnSignedValue: {
      total_size += ::_pbi::WireFormatLite::UInt64SizePlusOne(this->_internal_unsignedvalue());
      break;
    }
    // double DoubleValue = 5;
    case kDoubleValue: {
      total_size += 1 + 8;
      break;
    }
    case VARIANTVALUE_NOT_SET: {
      break;
    }
  }
  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData VariantParam::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    VariantParam::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*VariantParam::GetClassData() const { return &_class_data_; }


void VariantParam::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<VariantParam*>(&to_msg);
  auto& from = static_cast<const VariantParam&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:CRYSTAL_NET.service.VariantParam)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_varianttype() != 0) {
    _this->_internal_set_varianttype(from._internal_varianttype());
  }
  switch (from.VariantValue_case()) {
    case kStrValue: {
      _this->_internal_set_strvalue(from._internal_strvalue());
      break;
    }
    case kIntValue: {
      _this->_internal_set_intvalue(from._internal_intvalue());
      break;
    }
    case kUnSignedValue: {
      _this->_internal_set_unsignedvalue(from._internal_unsignedvalue());
      break;
    }
    case kDoubleValue: {
      _this->_internal_set_doublevalue(from._internal_doublevalue());
      break;
    }
    case VARIANTVALUE_NOT_SET: {
      break;
    }
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void VariantParam::CopyFrom(const VariantParam& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:CRYSTAL_NET.service.VariantParam)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool VariantParam::IsInitialized() const {
  return true;
}

void VariantParam::InternalSwap(VariantParam* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_.varianttype_, other->_impl_.varianttype_);
  swap(_impl_.VariantValue_, other->_impl_.VariantValue_);
  swap(_impl_._oneof_case_[0], other->_impl_._oneof_case_[0]);
}

::PROTOBUF_NAMESPACE_ID::Metadata VariantParam::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_com_5fvariant_2eproto_getter, &descriptor_table_com_5fvariant_2eproto_once,
      file_level_metadata_com_5fvariant_2eproto[1]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::CRYSTAL_NET::service::VariantParamType*
Arena::CreateMaybeMessage< ::CRYSTAL_NET::service::VariantParamType >(Arena* arena) {
  return Arena::CreateMessageInternal< ::CRYSTAL_NET::service::VariantParamType >(arena);
}
template<> PROTOBUF_NOINLINE ::CRYSTAL_NET::service::VariantParam*
Arena::CreateMaybeMessage< ::CRYSTAL_NET::service::VariantParam >(Arena* arena) {
  return Arena::CreateMessageInternal< ::CRYSTAL_NET::service::VariantParam >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>