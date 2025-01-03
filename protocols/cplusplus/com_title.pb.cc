#include <pch.h>
// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: com_title.proto

#include <protocols/cplusplus/com_title.pb.h>
POOL_CREATE_OBJ_DEFAULT_IMPL(TitleInfoFactory);

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
PROTOBUF_CONSTEXPR TitleInfo::TitleInfo(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.expiredts_)*/int64_t{0}
  , /*decltype(_impl_.titlecfgid_)*/0
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct TitleInfoDefaultTypeInternal {
  PROTOBUF_CONSTEXPR TitleInfoDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~TitleInfoDefaultTypeInternal() {}
  union {
    TitleInfo _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 TitleInfoDefaultTypeInternal _TitleInfo_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
static ::_pb::Metadata file_level_metadata_com_5ftitle_2eproto[1];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_com_5ftitle_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_com_5ftitle_2eproto = nullptr;

const uint32_t TableStruct_com_5ftitle_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::TitleInfo, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::TitleInfo, _impl_.titlecfgid_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::TitleInfo, _impl_.expiredts_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::CRYSTAL_NET::service::TitleInfo)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::CRYSTAL_NET::service::_TitleInfo_default_instance_._instance,
};

const char descriptor_table_protodef_com_5ftitle_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\017com_title.proto\022\023CRYSTAL_NET.service\"2"
  "\n\tTitleInfo\022\022\n\ntitleCfgId\030\001 \001(\021\022\021\n\texpir"
  "edTs\030\002 \001(\022b\006proto3"
  ;
static ::_pbi::once_flag descriptor_table_com_5ftitle_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_com_5ftitle_2eproto = {
    false, false, 98, descriptor_table_protodef_com_5ftitle_2eproto,
    "com_title.proto",
    &descriptor_table_com_5ftitle_2eproto_once, nullptr, 0, 1,
    schemas, file_default_instances, TableStruct_com_5ftitle_2eproto::offsets,
    file_level_metadata_com_5ftitle_2eproto, file_level_enum_descriptors_com_5ftitle_2eproto,
    file_level_service_descriptors_com_5ftitle_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_com_5ftitle_2eproto_getter() {
  return &descriptor_table_com_5ftitle_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_com_5ftitle_2eproto(&descriptor_table_com_5ftitle_2eproto);
namespace CRYSTAL_NET {
namespace service {

// ===================================================================

class TitleInfo::_Internal {
 public:
};

TitleInfo::TitleInfo(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:CRYSTAL_NET.service.TitleInfo)
}
TitleInfo::TitleInfo(const TitleInfo& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  TitleInfo* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.expiredts_){}
    , decltype(_impl_.titlecfgid_){}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  ::memcpy(&_impl_.expiredts_, &from._impl_.expiredts_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.titlecfgid_) -
    reinterpret_cast<char*>(&_impl_.expiredts_)) + sizeof(_impl_.titlecfgid_));
  // @@protoc_insertion_point(copy_constructor:CRYSTAL_NET.service.TitleInfo)
}

inline void TitleInfo::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.expiredts_){int64_t{0}}
    , decltype(_impl_.titlecfgid_){0}
    , /*decltype(_impl_._cached_size_)*/{}
  };
}

TitleInfo::~TitleInfo() {
  // @@protoc_insertion_point(destructor:CRYSTAL_NET.service.TitleInfo)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void TitleInfo::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
}

void TitleInfo::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void TitleInfo::Clear() {
// @@protoc_insertion_point(message_clear_start:CRYSTAL_NET.service.TitleInfo)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  ::memset(&_impl_.expiredts_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&_impl_.titlecfgid_) -
      reinterpret_cast<char*>(&_impl_.expiredts_)) + sizeof(_impl_.titlecfgid_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* TitleInfo::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // sint32 titleCfgId = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          _impl_.titlecfgid_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // sint64 expiredTs = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 16)) {
          _impl_.expiredts_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag64(&ptr);
          CHK_(ptr);
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

uint8_t* TitleInfo::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:CRYSTAL_NET.service.TitleInfo)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // sint32 titleCfgId = 1;
  if (this->_internal_titlecfgid() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt32ToArray(1, this->_internal_titlecfgid(), target);
  }

  // sint64 expiredTs = 2;
  if (this->_internal_expiredts() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt64ToArray(2, this->_internal_expiredts(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:CRYSTAL_NET.service.TitleInfo)
  return target;
}

size_t TitleInfo::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:CRYSTAL_NET.service.TitleInfo)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // sint64 expiredTs = 2;
  if (this->_internal_expiredts() != 0) {
    total_size += ::_pbi::WireFormatLite::SInt64SizePlusOne(this->_internal_expiredts());
  }

  // sint32 titleCfgId = 1;
  if (this->_internal_titlecfgid() != 0) {
    total_size += ::_pbi::WireFormatLite::SInt32SizePlusOne(this->_internal_titlecfgid());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData TitleInfo::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    TitleInfo::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*TitleInfo::GetClassData() const { return &_class_data_; }


void TitleInfo::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<TitleInfo*>(&to_msg);
  auto& from = static_cast<const TitleInfo&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:CRYSTAL_NET.service.TitleInfo)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_expiredts() != 0) {
    _this->_internal_set_expiredts(from._internal_expiredts());
  }
  if (from._internal_titlecfgid() != 0) {
    _this->_internal_set_titlecfgid(from._internal_titlecfgid());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void TitleInfo::CopyFrom(const TitleInfo& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:CRYSTAL_NET.service.TitleInfo)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool TitleInfo::IsInitialized() const {
  return true;
}

void TitleInfo::InternalSwap(TitleInfo* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(TitleInfo, _impl_.titlecfgid_)
      + sizeof(TitleInfo::_impl_.titlecfgid_)
      - PROTOBUF_FIELD_OFFSET(TitleInfo, _impl_.expiredts_)>(
          reinterpret_cast<char*>(&_impl_.expiredts_),
          reinterpret_cast<char*>(&other->_impl_.expiredts_));
}

::PROTOBUF_NAMESPACE_ID::Metadata TitleInfo::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_com_5ftitle_2eproto_getter, &descriptor_table_com_5ftitle_2eproto_once,
      file_level_metadata_com_5ftitle_2eproto[0]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::CRYSTAL_NET::service::TitleInfo*
Arena::CreateMaybeMessage< ::CRYSTAL_NET::service::TitleInfo >(Arena* arena) {
  return Arena::CreateMessageInternal< ::CRYSTAL_NET::service::TitleInfo >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
