#include <pch.h>
// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: com_syslog.proto

#include <protocols/cplusplus/library/com_syslog.pb.h>
POOL_CREATE_OBJ_DEFAULT_IMPL(SystemLogDataFactory);

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
PROTOBUF_CONSTEXPR SystemLogData::SystemLogData(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.titleparams_)*/{}
  , /*decltype(_impl_.contentparams_)*/{}
  , /*decltype(_impl_.titlewordid_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.contentwordid_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.id_)*/uint64_t{0u}
  , /*decltype(_impl_.libraryid_)*/uint64_t{0u}
  , /*decltype(_impl_.createtime_)*/int64_t{0}
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct SystemLogDataDefaultTypeInternal {
  PROTOBUF_CONSTEXPR SystemLogDataDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~SystemLogDataDefaultTypeInternal() {}
  union {
    SystemLogData _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 SystemLogDataDefaultTypeInternal _SystemLogData_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
static ::_pb::Metadata file_level_metadata_com_5fsyslog_2eproto[1];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_com_5fsyslog_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_com_5fsyslog_2eproto = nullptr;

const uint32_t TableStruct_com_5fsyslog_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _impl_.id_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _impl_.libraryid_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _impl_.titlewordid_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _impl_.titleparams_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _impl_.contentwordid_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _impl_.contentparams_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::SystemLogData, _impl_.createtime_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::CRYSTAL_NET::service::SystemLogData)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::CRYSTAL_NET::service::_SystemLogData_default_instance_._instance,
};

const char descriptor_table_protodef_com_5fsyslog_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\020com_syslog.proto\022\023CRYSTAL_NET.service\032"
  "\021com_variant.proto\"\340\001\n\rSystemLogData\022\n\n\002"
  "Id\030\001 \001(\004\022\021\n\tlibraryId\030\002 \001(\004\022\023\n\013TitleWord"
  "Id\030\003 \001(\t\0226\n\013TitleParams\030\004 \003(\0132!.CRYSTAL_"
  "NET.service.VariantParam\022\025\n\rContentWordI"
  "d\030\005 \001(\t\0228\n\rContentParams\030\006 \003(\0132!.CRYSTAL"
  "_NET.service.VariantParam\022\022\n\nCreateTime\030"
  "\007 \001(\022b\006proto3"
  ;
static const ::_pbi::DescriptorTable* const descriptor_table_com_5fsyslog_2eproto_deps[1] = {
  &::descriptor_table_com_5fvariant_2eproto,
};
static ::_pbi::once_flag descriptor_table_com_5fsyslog_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_com_5fsyslog_2eproto = {
    false, false, 293, descriptor_table_protodef_com_5fsyslog_2eproto,
    "com_syslog.proto",
    &descriptor_table_com_5fsyslog_2eproto_once, descriptor_table_com_5fsyslog_2eproto_deps, 1, 1,
    schemas, file_default_instances, TableStruct_com_5fsyslog_2eproto::offsets,
    file_level_metadata_com_5fsyslog_2eproto, file_level_enum_descriptors_com_5fsyslog_2eproto,
    file_level_service_descriptors_com_5fsyslog_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_com_5fsyslog_2eproto_getter() {
  return &descriptor_table_com_5fsyslog_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_com_5fsyslog_2eproto(&descriptor_table_com_5fsyslog_2eproto);
namespace CRYSTAL_NET {
namespace service {

// ===================================================================

class SystemLogData::_Internal {
 public:
};

void SystemLogData::clear_titleparams() {
  _impl_.titleparams_.Clear();
}
void SystemLogData::clear_contentparams() {
  _impl_.contentparams_.Clear();
}
SystemLogData::SystemLogData(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:CRYSTAL_NET.service.SystemLogData)
}
SystemLogData::SystemLogData(const SystemLogData& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  SystemLogData* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.titleparams_){from._impl_.titleparams_}
    , decltype(_impl_.contentparams_){from._impl_.contentparams_}
    , decltype(_impl_.titlewordid_){}
    , decltype(_impl_.contentwordid_){}
    , decltype(_impl_.id_){}
    , decltype(_impl_.libraryid_){}
    , decltype(_impl_.createtime_){}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.titlewordid_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.titlewordid_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_titlewordid().empty()) {
    _this->_impl_.titlewordid_.Set(from._internal_titlewordid(), 
      _this->GetArenaForAllocation());
  }
  _impl_.contentwordid_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.contentwordid_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_contentwordid().empty()) {
    _this->_impl_.contentwordid_.Set(from._internal_contentwordid(), 
      _this->GetArenaForAllocation());
  }
  ::memcpy(&_impl_.id_, &from._impl_.id_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.createtime_) -
    reinterpret_cast<char*>(&_impl_.id_)) + sizeof(_impl_.createtime_));
  // @@protoc_insertion_point(copy_constructor:CRYSTAL_NET.service.SystemLogData)
}

inline void SystemLogData::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.titleparams_){arena}
    , decltype(_impl_.contentparams_){arena}
    , decltype(_impl_.titlewordid_){}
    , decltype(_impl_.contentwordid_){}
    , decltype(_impl_.id_){uint64_t{0u}}
    , decltype(_impl_.libraryid_){uint64_t{0u}}
    , decltype(_impl_.createtime_){int64_t{0}}
    , /*decltype(_impl_._cached_size_)*/{}
  };
  _impl_.titlewordid_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.titlewordid_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  _impl_.contentwordid_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.contentwordid_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

SystemLogData::~SystemLogData() {
  // @@protoc_insertion_point(destructor:CRYSTAL_NET.service.SystemLogData)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void SystemLogData::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.titleparams_.~RepeatedPtrField();
  _impl_.contentparams_.~RepeatedPtrField();
  _impl_.titlewordid_.Destroy();
  _impl_.contentwordid_.Destroy();
}

void SystemLogData::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void SystemLogData::Clear() {
// @@protoc_insertion_point(message_clear_start:CRYSTAL_NET.service.SystemLogData)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.titleparams_.Clear();
  _impl_.contentparams_.Clear();
  _impl_.titlewordid_.ClearToEmpty();
  _impl_.contentwordid_.ClearToEmpty();
  ::memset(&_impl_.id_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&_impl_.createtime_) -
      reinterpret_cast<char*>(&_impl_.id_)) + sizeof(_impl_.createtime_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* SystemLogData::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // uint64 Id = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          _impl_.id_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint64 libraryId = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 16)) {
          _impl_.libraryid_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // string TitleWordId = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 26)) {
          auto str = _internal_mutable_titlewordid();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "CRYSTAL_NET.service.SystemLogData.TitleWordId"));
        } else
          goto handle_unusual;
        continue;
      // repeated .CRYSTAL_NET.service.VariantParam TitleParams = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 34)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_titleparams(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<34>(ptr));
        } else
          goto handle_unusual;
        continue;
      // string ContentWordId = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 42)) {
          auto str = _internal_mutable_contentwordid();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "CRYSTAL_NET.service.SystemLogData.ContentWordId"));
        } else
          goto handle_unusual;
        continue;
      // repeated .CRYSTAL_NET.service.VariantParam ContentParams = 6;
      case 6:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 50)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_contentparams(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<50>(ptr));
        } else
          goto handle_unusual;
        continue;
      // sint64 CreateTime = 7;
      case 7:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 56)) {
          _impl_.createtime_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag64(&ptr);
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

uint8_t* SystemLogData::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:CRYSTAL_NET.service.SystemLogData)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // uint64 Id = 1;
  if (this->_internal_id() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt64ToArray(1, this->_internal_id(), target);
  }

  // uint64 libraryId = 2;
  if (this->_internal_libraryid() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt64ToArray(2, this->_internal_libraryid(), target);
  }

  // string TitleWordId = 3;
  if (!this->_internal_titlewordid().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_titlewordid().data(), static_cast<int>(this->_internal_titlewordid().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "CRYSTAL_NET.service.SystemLogData.TitleWordId");
    target = stream->WriteStringMaybeAliased(
        3, this->_internal_titlewordid(), target);
  }

  // repeated .CRYSTAL_NET.service.VariantParam TitleParams = 4;
  for (unsigned i = 0,
      n = static_cast<unsigned>(this->_internal_titleparams_size()); i < n; i++) {
    const auto& repfield = this->_internal_titleparams(i);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
        InternalWriteMessage(4, repfield, repfield.GetCachedSize(), target, stream);
  }

  // string ContentWordId = 5;
  if (!this->_internal_contentwordid().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_contentwordid().data(), static_cast<int>(this->_internal_contentwordid().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "CRYSTAL_NET.service.SystemLogData.ContentWordId");
    target = stream->WriteStringMaybeAliased(
        5, this->_internal_contentwordid(), target);
  }

  // repeated .CRYSTAL_NET.service.VariantParam ContentParams = 6;
  for (unsigned i = 0,
      n = static_cast<unsigned>(this->_internal_contentparams_size()); i < n; i++) {
    const auto& repfield = this->_internal_contentparams(i);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
        InternalWriteMessage(6, repfield, repfield.GetCachedSize(), target, stream);
  }

  // sint64 CreateTime = 7;
  if (this->_internal_createtime() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt64ToArray(7, this->_internal_createtime(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:CRYSTAL_NET.service.SystemLogData)
  return target;
}

size_t SystemLogData::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:CRYSTAL_NET.service.SystemLogData)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .CRYSTAL_NET.service.VariantParam TitleParams = 4;
  total_size += 1UL * this->_internal_titleparams_size();
  for (const auto& msg : this->_impl_.titleparams_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  // repeated .CRYSTAL_NET.service.VariantParam ContentParams = 6;
  total_size += 1UL * this->_internal_contentparams_size();
  for (const auto& msg : this->_impl_.contentparams_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  // string TitleWordId = 3;
  if (!this->_internal_titlewordid().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_titlewordid());
  }

  // string ContentWordId = 5;
  if (!this->_internal_contentwordid().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_contentwordid());
  }

  // uint64 Id = 1;
  if (this->_internal_id() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt64SizePlusOne(this->_internal_id());
  }

  // uint64 libraryId = 2;
  if (this->_internal_libraryid() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt64SizePlusOne(this->_internal_libraryid());
  }

  // sint64 CreateTime = 7;
  if (this->_internal_createtime() != 0) {
    total_size += ::_pbi::WireFormatLite::SInt64SizePlusOne(this->_internal_createtime());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData SystemLogData::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    SystemLogData::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*SystemLogData::GetClassData() const { return &_class_data_; }


void SystemLogData::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<SystemLogData*>(&to_msg);
  auto& from = static_cast<const SystemLogData&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:CRYSTAL_NET.service.SystemLogData)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.titleparams_.MergeFrom(from._impl_.titleparams_);
  _this->_impl_.contentparams_.MergeFrom(from._impl_.contentparams_);
  if (!from._internal_titlewordid().empty()) {
    _this->_internal_set_titlewordid(from._internal_titlewordid());
  }
  if (!from._internal_contentwordid().empty()) {
    _this->_internal_set_contentwordid(from._internal_contentwordid());
  }
  if (from._internal_id() != 0) {
    _this->_internal_set_id(from._internal_id());
  }
  if (from._internal_libraryid() != 0) {
    _this->_internal_set_libraryid(from._internal_libraryid());
  }
  if (from._internal_createtime() != 0) {
    _this->_internal_set_createtime(from._internal_createtime());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void SystemLogData::CopyFrom(const SystemLogData& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:CRYSTAL_NET.service.SystemLogData)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool SystemLogData::IsInitialized() const {
  return true;
}

void SystemLogData::InternalSwap(SystemLogData* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.titleparams_.InternalSwap(&other->_impl_.titleparams_);
  _impl_.contentparams_.InternalSwap(&other->_impl_.contentparams_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.titlewordid_, lhs_arena,
      &other->_impl_.titlewordid_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.contentwordid_, lhs_arena,
      &other->_impl_.contentwordid_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(SystemLogData, _impl_.createtime_)
      + sizeof(SystemLogData::_impl_.createtime_)
      - PROTOBUF_FIELD_OFFSET(SystemLogData, _impl_.id_)>(
          reinterpret_cast<char*>(&_impl_.id_),
          reinterpret_cast<char*>(&other->_impl_.id_));
}

::PROTOBUF_NAMESPACE_ID::Metadata SystemLogData::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_com_5fsyslog_2eproto_getter, &descriptor_table_com_5fsyslog_2eproto_once,
      file_level_metadata_com_5fsyslog_2eproto[0]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::CRYSTAL_NET::service::SystemLogData*
Arena::CreateMaybeMessage< ::CRYSTAL_NET::service::SystemLogData >(Arena* arena) {
  return Arena::CreateMessageInternal< ::CRYSTAL_NET::service::SystemLogData >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>