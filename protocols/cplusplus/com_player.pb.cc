#include <pch.h>
// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: com_player.proto

#include <protocols/cplusplus/com_player.pb.h>
POOL_CREATE_OBJ_DEFAULT_IMPL(PlayerDataFactory);

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
PROTOBUF_CONSTEXPR PlayerData::PlayerData(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_._has_bits_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}
  , /*decltype(_impl_.account_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.name_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.playerid_)*/int64_t{0}
  , /*decltype(_impl_.sex_)*/0} {}
struct PlayerDataDefaultTypeInternal {
  PROTOBUF_CONSTEXPR PlayerDataDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~PlayerDataDefaultTypeInternal() {}
  union {
    PlayerData _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 PlayerDataDefaultTypeInternal _PlayerData_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
static ::_pb::Metadata file_level_metadata_com_5fplayer_2eproto[1];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_com_5fplayer_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_com_5fplayer_2eproto = nullptr;

const uint32_t TableStruct_com_5fplayer_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::PlayerData, _impl_._has_bits_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::PlayerData, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::PlayerData, _impl_.account_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::PlayerData, _impl_.playerid_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::PlayerData, _impl_.sex_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::PlayerData, _impl_.name_),
  0,
  2,
  3,
  1,
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 10, -1, sizeof(::CRYSTAL_NET::service::PlayerData)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::CRYSTAL_NET::service::_PlayerData_default_instance_._instance,
};

const char descriptor_table_protodef_com_5fplayer_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\020com_player.proto\022\023CRYSTAL_NET.service\""
  "\210\001\n\nPlayerData\022\024\n\007account\030\001 \001(\tH\000\210\001\001\022\025\n\010"
  "playerId\030\002 \001(\022H\001\210\001\001\022\020\n\003sex\030\003 \001(\021H\002\210\001\001\022\021\n"
  "\004name\030\004 \001(\tH\003\210\001\001B\n\n\010_accountB\013\n\t_playerI"
  "dB\006\n\004_sexB\007\n\005_nameb\006proto3"
  ;
static ::_pbi::once_flag descriptor_table_com_5fplayer_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_com_5fplayer_2eproto = {
    false, false, 186, descriptor_table_protodef_com_5fplayer_2eproto,
    "com_player.proto",
    &descriptor_table_com_5fplayer_2eproto_once, nullptr, 0, 1,
    schemas, file_default_instances, TableStruct_com_5fplayer_2eproto::offsets,
    file_level_metadata_com_5fplayer_2eproto, file_level_enum_descriptors_com_5fplayer_2eproto,
    file_level_service_descriptors_com_5fplayer_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_com_5fplayer_2eproto_getter() {
  return &descriptor_table_com_5fplayer_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_com_5fplayer_2eproto(&descriptor_table_com_5fplayer_2eproto);
namespace CRYSTAL_NET {
namespace service {

// ===================================================================

class PlayerData::_Internal {
 public:
  using HasBits = decltype(std::declval<PlayerData>()._impl_._has_bits_);
  static void set_has_account(HasBits* has_bits) {
    (*has_bits)[0] |= 1u;
  }
  static void set_has_playerid(HasBits* has_bits) {
    (*has_bits)[0] |= 4u;
  }
  static void set_has_sex(HasBits* has_bits) {
    (*has_bits)[0] |= 8u;
  }
  static void set_has_name(HasBits* has_bits) {
    (*has_bits)[0] |= 2u;
  }
};

PlayerData::PlayerData(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:CRYSTAL_NET.service.PlayerData)
}
PlayerData::PlayerData(const PlayerData& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  PlayerData* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){from._impl_._has_bits_}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.account_){}
    , decltype(_impl_.name_){}
    , decltype(_impl_.playerid_){}
    , decltype(_impl_.sex_){}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.account_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.account_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (from._internal_has_account()) {
    _this->_impl_.account_.Set(from._internal_account(), 
      _this->GetArenaForAllocation());
  }
  _impl_.name_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.name_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (from._internal_has_name()) {
    _this->_impl_.name_.Set(from._internal_name(), 
      _this->GetArenaForAllocation());
  }
  ::memcpy(&_impl_.playerid_, &from._impl_.playerid_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.sex_) -
    reinterpret_cast<char*>(&_impl_.playerid_)) + sizeof(_impl_.sex_));
  // @@protoc_insertion_point(copy_constructor:CRYSTAL_NET.service.PlayerData)
}

inline void PlayerData::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_._has_bits_){}
    , /*decltype(_impl_._cached_size_)*/{}
    , decltype(_impl_.account_){}
    , decltype(_impl_.name_){}
    , decltype(_impl_.playerid_){int64_t{0}}
    , decltype(_impl_.sex_){0}
  };
  _impl_.account_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.account_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  _impl_.name_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.name_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

PlayerData::~PlayerData() {
  // @@protoc_insertion_point(destructor:CRYSTAL_NET.service.PlayerData)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void PlayerData::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.account_.Destroy();
  _impl_.name_.Destroy();
}

void PlayerData::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void PlayerData::Clear() {
// @@protoc_insertion_point(message_clear_start:CRYSTAL_NET.service.PlayerData)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x00000003u) {
    if (cached_has_bits & 0x00000001u) {
      _impl_.account_.ClearNonDefaultToEmpty();
    }
    if (cached_has_bits & 0x00000002u) {
      _impl_.name_.ClearNonDefaultToEmpty();
    }
  }
  if (cached_has_bits & 0x0000000cu) {
    ::memset(&_impl_.playerid_, 0, static_cast<size_t>(
        reinterpret_cast<char*>(&_impl_.sex_) -
        reinterpret_cast<char*>(&_impl_.playerid_)) + sizeof(_impl_.sex_));
  }
  _impl_._has_bits_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* PlayerData::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  _Internal::HasBits has_bits{};
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // optional string account = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          auto str = _internal_mutable_account();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "CRYSTAL_NET.service.PlayerData.account"));
        } else
          goto handle_unusual;
        continue;
      // optional sint64 playerId = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 16)) {
          _Internal::set_has_playerid(&has_bits);
          _impl_.playerid_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // optional sint32 sex = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 24)) {
          _Internal::set_has_sex(&has_bits);
          _impl_.sex_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // optional string name = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 34)) {
          auto str = _internal_mutable_name();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "CRYSTAL_NET.service.PlayerData.name"));
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
  _impl_._has_bits_.Or(has_bits);
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* PlayerData::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:CRYSTAL_NET.service.PlayerData)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // optional string account = 1;
  if (_internal_has_account()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_account().data(), static_cast<int>(this->_internal_account().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "CRYSTAL_NET.service.PlayerData.account");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_account(), target);
  }

  // optional sint64 playerId = 2;
  if (_internal_has_playerid()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt64ToArray(2, this->_internal_playerid(), target);
  }

  // optional sint32 sex = 3;
  if (_internal_has_sex()) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt32ToArray(3, this->_internal_sex(), target);
  }

  // optional string name = 4;
  if (_internal_has_name()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_name().data(), static_cast<int>(this->_internal_name().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "CRYSTAL_NET.service.PlayerData.name");
    target = stream->WriteStringMaybeAliased(
        4, this->_internal_name(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:CRYSTAL_NET.service.PlayerData)
  return target;
}

size_t PlayerData::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:CRYSTAL_NET.service.PlayerData)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  cached_has_bits = _impl_._has_bits_[0];
  if (cached_has_bits & 0x0000000fu) {
    // optional string account = 1;
    if (cached_has_bits & 0x00000001u) {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
          this->_internal_account());
    }

    // optional string name = 4;
    if (cached_has_bits & 0x00000002u) {
      total_size += 1 +
        ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
          this->_internal_name());
    }

    // optional sint64 playerId = 2;
    if (cached_has_bits & 0x00000004u) {
      total_size += ::_pbi::WireFormatLite::SInt64SizePlusOne(this->_internal_playerid());
    }

    // optional sint32 sex = 3;
    if (cached_has_bits & 0x00000008u) {
      total_size += ::_pbi::WireFormatLite::SInt32SizePlusOne(this->_internal_sex());
    }

  }
  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData PlayerData::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    PlayerData::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*PlayerData::GetClassData() const { return &_class_data_; }


void PlayerData::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<PlayerData*>(&to_msg);
  auto& from = static_cast<const PlayerData&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:CRYSTAL_NET.service.PlayerData)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._impl_._has_bits_[0];
  if (cached_has_bits & 0x0000000fu) {
    if (cached_has_bits & 0x00000001u) {
      _this->_internal_set_account(from._internal_account());
    }
    if (cached_has_bits & 0x00000002u) {
      _this->_internal_set_name(from._internal_name());
    }
    if (cached_has_bits & 0x00000004u) {
      _this->_impl_.playerid_ = from._impl_.playerid_;
    }
    if (cached_has_bits & 0x00000008u) {
      _this->_impl_.sex_ = from._impl_.sex_;
    }
    _this->_impl_._has_bits_[0] |= cached_has_bits;
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void PlayerData::CopyFrom(const PlayerData& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:CRYSTAL_NET.service.PlayerData)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool PlayerData::IsInitialized() const {
  return true;
}

void PlayerData::InternalSwap(PlayerData* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_._has_bits_[0], other->_impl_._has_bits_[0]);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.account_, lhs_arena,
      &other->_impl_.account_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.name_, lhs_arena,
      &other->_impl_.name_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(PlayerData, _impl_.sex_)
      + sizeof(PlayerData::_impl_.sex_)
      - PROTOBUF_FIELD_OFFSET(PlayerData, _impl_.playerid_)>(
          reinterpret_cast<char*>(&_impl_.playerid_),
          reinterpret_cast<char*>(&other->_impl_.playerid_));
}

::PROTOBUF_NAMESPACE_ID::Metadata PlayerData::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_com_5fplayer_2eproto_getter, &descriptor_table_com_5fplayer_2eproto_once,
      file_level_metadata_com_5fplayer_2eproto[0]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::CRYSTAL_NET::service::PlayerData*
Arena::CreateMaybeMessage< ::CRYSTAL_NET::service::PlayerData >(Arena* arena) {
  return Arena::CreateMessageInternal< ::CRYSTAL_NET::service::PlayerData >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
