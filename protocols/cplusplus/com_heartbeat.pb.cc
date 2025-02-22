#include <pch.h>
// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: com_heartbeat.proto

#include <protocols/cplusplus/com_heartbeat.pb.h>
POOL_CREATE_OBJ_DEFAULT_IMPL(NODE_IPTYPEFactory);
POOL_CREATE_OBJ_DEFAULT_IMPL(NodeHeartbeatInfoFactory);

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
PROTOBUF_CONSTEXPR NODE_IPTYPE::NODE_IPTYPE(
    ::_pbi::ConstantInitialized) {}
struct NODE_IPTYPEDefaultTypeInternal {
  PROTOBUF_CONSTEXPR NODE_IPTYPEDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~NODE_IPTYPEDefaultTypeInternal() {}
  union {
    NODE_IPTYPE _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 NODE_IPTYPEDefaultTypeInternal _NODE_IPTYPE_default_instance_;
PROTOBUF_CONSTEXPR NodeHeartbeatInfo::NodeHeartbeatInfo(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.subscribeopcodes_)*/{}
  , /*decltype(_impl_._subscribeopcodes_cached_byte_size_)*/{0}
  , /*decltype(_impl_.apilist_)*/{}
  , /*decltype(_impl_.servicename_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.address_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.iptype_)*/0
  , /*decltype(_impl_.innerlinkport_)*/0u
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct NodeHeartbeatInfoDefaultTypeInternal {
  PROTOBUF_CONSTEXPR NodeHeartbeatInfoDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~NodeHeartbeatInfoDefaultTypeInternal() {}
  union {
    NodeHeartbeatInfo _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 NodeHeartbeatInfoDefaultTypeInternal _NodeHeartbeatInfo_default_instance_;
}  // namespace service
}  // namespace CRYSTAL_NET
static ::_pb::Metadata file_level_metadata_com_5fheartbeat_2eproto[2];
static const ::_pb::EnumDescriptor* file_level_enum_descriptors_com_5fheartbeat_2eproto[1];
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_com_5fheartbeat_2eproto = nullptr;

const uint32_t TableStruct_com_5fheartbeat_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NODE_IPTYPE, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NodeHeartbeatInfo, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NodeHeartbeatInfo, _impl_.servicename_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NodeHeartbeatInfo, _impl_.address_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NodeHeartbeatInfo, _impl_.iptype_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NodeHeartbeatInfo, _impl_.innerlinkport_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NodeHeartbeatInfo, _impl_.subscribeopcodes_),
  PROTOBUF_FIELD_OFFSET(::CRYSTAL_NET::service::NodeHeartbeatInfo, _impl_.apilist_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::CRYSTAL_NET::service::NODE_IPTYPE)},
  { 6, -1, -1, sizeof(::CRYSTAL_NET::service::NodeHeartbeatInfo)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::CRYSTAL_NET::service::_NODE_IPTYPE_default_instance_._instance,
  &::CRYSTAL_NET::service::_NodeHeartbeatInfo_default_instance_._instance,
};

const char descriptor_table_protodef_com_5fheartbeat_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\023com_heartbeat.proto\022\023CRYSTAL_NET.servi"
  "ce\"/\n\013NODE_IPTYPE\" \n\nTYPE_ENUMS\022\010\n\004IPV4\020"
  "\000\022\010\n\004IPV6\020\001\"\213\001\n\021NodeHeartbeatInfo\022\023\n\013Ser"
  "viceName\030\001 \001(\t\022\017\n\007address\030\002 \001(\t\022\016\n\006IpTyp"
  "e\030\003 \001(\021\022\025\n\rInnerLinkPort\030\004 \001(\r\022\030\n\020Subscr"
  "ibeOpcodes\030\005 \003(\021\022\017\n\007ApiList\030\006 \003(\tb\006proto"
  "3"
  ;
static ::_pbi::once_flag descriptor_table_com_5fheartbeat_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_com_5fheartbeat_2eproto = {
    false, false, 241, descriptor_table_protodef_com_5fheartbeat_2eproto,
    "com_heartbeat.proto",
    &descriptor_table_com_5fheartbeat_2eproto_once, nullptr, 0, 2,
    schemas, file_default_instances, TableStruct_com_5fheartbeat_2eproto::offsets,
    file_level_metadata_com_5fheartbeat_2eproto, file_level_enum_descriptors_com_5fheartbeat_2eproto,
    file_level_service_descriptors_com_5fheartbeat_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_com_5fheartbeat_2eproto_getter() {
  return &descriptor_table_com_5fheartbeat_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_com_5fheartbeat_2eproto(&descriptor_table_com_5fheartbeat_2eproto);
namespace CRYSTAL_NET {
namespace service {
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* NODE_IPTYPE_TYPE_ENUMS_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_com_5fheartbeat_2eproto);
  return file_level_enum_descriptors_com_5fheartbeat_2eproto[0];
}
bool NODE_IPTYPE_TYPE_ENUMS_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
      return true;
    default:
      return false;
  }
}

#if (__cplusplus < 201703) && (!defined(_MSC_VER) || (_MSC_VER >= 1900 && _MSC_VER < 1912))
constexpr NODE_IPTYPE_TYPE_ENUMS NODE_IPTYPE::IPV4;
constexpr NODE_IPTYPE_TYPE_ENUMS NODE_IPTYPE::IPV6;
constexpr NODE_IPTYPE_TYPE_ENUMS NODE_IPTYPE::TYPE_ENUMS_MIN;
constexpr NODE_IPTYPE_TYPE_ENUMS NODE_IPTYPE::TYPE_ENUMS_MAX;
constexpr int NODE_IPTYPE::TYPE_ENUMS_ARRAYSIZE;
#endif  // (__cplusplus < 201703) && (!defined(_MSC_VER) || (_MSC_VER >= 1900 && _MSC_VER < 1912))

// ===================================================================

class NODE_IPTYPE::_Internal {
 public:
};

NODE_IPTYPE::NODE_IPTYPE(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase(arena, is_message_owned) {
  // @@protoc_insertion_point(arena_constructor:CRYSTAL_NET.service.NODE_IPTYPE)
}
NODE_IPTYPE::NODE_IPTYPE(const NODE_IPTYPE& from)
  : ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase() {
  NODE_IPTYPE* const _this = this; (void)_this;
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:CRYSTAL_NET.service.NODE_IPTYPE)
}





const ::PROTOBUF_NAMESPACE_ID::Message::ClassData NODE_IPTYPE::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::CopyImpl,
    ::PROTOBUF_NAMESPACE_ID::internal::ZeroFieldsBase::MergeImpl,
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*NODE_IPTYPE::GetClassData() const { return &_class_data_; }







::PROTOBUF_NAMESPACE_ID::Metadata NODE_IPTYPE::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_com_5fheartbeat_2eproto_getter, &descriptor_table_com_5fheartbeat_2eproto_once,
      file_level_metadata_com_5fheartbeat_2eproto[0]);
}

// ===================================================================

class NodeHeartbeatInfo::_Internal {
 public:
};

NodeHeartbeatInfo::NodeHeartbeatInfo(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:CRYSTAL_NET.service.NodeHeartbeatInfo)
}
NodeHeartbeatInfo::NodeHeartbeatInfo(const NodeHeartbeatInfo& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  NodeHeartbeatInfo* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.subscribeopcodes_){from._impl_.subscribeopcodes_}
    , /*decltype(_impl_._subscribeopcodes_cached_byte_size_)*/{0}
    , decltype(_impl_.apilist_){from._impl_.apilist_}
    , decltype(_impl_.servicename_){}
    , decltype(_impl_.address_){}
    , decltype(_impl_.iptype_){}
    , decltype(_impl_.innerlinkport_){}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.servicename_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.servicename_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_servicename().empty()) {
    _this->_impl_.servicename_.Set(from._internal_servicename(), 
      _this->GetArenaForAllocation());
  }
  _impl_.address_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.address_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_address().empty()) {
    _this->_impl_.address_.Set(from._internal_address(), 
      _this->GetArenaForAllocation());
  }
  ::memcpy(&_impl_.iptype_, &from._impl_.iptype_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.innerlinkport_) -
    reinterpret_cast<char*>(&_impl_.iptype_)) + sizeof(_impl_.innerlinkport_));
  // @@protoc_insertion_point(copy_constructor:CRYSTAL_NET.service.NodeHeartbeatInfo)
}

inline void NodeHeartbeatInfo::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.subscribeopcodes_){arena}
    , /*decltype(_impl_._subscribeopcodes_cached_byte_size_)*/{0}
    , decltype(_impl_.apilist_){arena}
    , decltype(_impl_.servicename_){}
    , decltype(_impl_.address_){}
    , decltype(_impl_.iptype_){0}
    , decltype(_impl_.innerlinkport_){0u}
    , /*decltype(_impl_._cached_size_)*/{}
  };
  _impl_.servicename_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.servicename_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  _impl_.address_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.address_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

NodeHeartbeatInfo::~NodeHeartbeatInfo() {
  // @@protoc_insertion_point(destructor:CRYSTAL_NET.service.NodeHeartbeatInfo)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void NodeHeartbeatInfo::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.subscribeopcodes_.~RepeatedField();
  _impl_.apilist_.~RepeatedPtrField();
  _impl_.servicename_.Destroy();
  _impl_.address_.Destroy();
}

void NodeHeartbeatInfo::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void NodeHeartbeatInfo::Clear() {
// @@protoc_insertion_point(message_clear_start:CRYSTAL_NET.service.NodeHeartbeatInfo)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.subscribeopcodes_.Clear();
  _impl_.apilist_.Clear();
  _impl_.servicename_.ClearToEmpty();
  _impl_.address_.ClearToEmpty();
  ::memset(&_impl_.iptype_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&_impl_.innerlinkport_) -
      reinterpret_cast<char*>(&_impl_.iptype_)) + sizeof(_impl_.innerlinkport_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* NodeHeartbeatInfo::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string ServiceName = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          auto str = _internal_mutable_servicename();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "CRYSTAL_NET.service.NodeHeartbeatInfo.ServiceName"));
        } else
          goto handle_unusual;
        continue;
      // string address = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 18)) {
          auto str = _internal_mutable_address();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "CRYSTAL_NET.service.NodeHeartbeatInfo.address"));
        } else
          goto handle_unusual;
        continue;
      // sint32 IpType = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 24)) {
          _impl_.iptype_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // uint32 InnerLinkPort = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 32)) {
          _impl_.innerlinkport_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint32(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // repeated sint32 SubscribeOpcodes = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 42)) {
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::PackedSInt32Parser(_internal_mutable_subscribeopcodes(), ptr, ctx);
          CHK_(ptr);
        } else if (static_cast<uint8_t>(tag) == 40) {
          _internal_add_subscribeopcodes(::PROTOBUF_NAMESPACE_ID::internal::ReadVarintZigZag32(&ptr));
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // repeated string ApiList = 6;
      case 6:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 50)) {
          ptr -= 1;
          do {
            ptr += 1;
            auto str = _internal_add_apilist();
            ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
            CHK_(ptr);
            CHK_(::_pbi::VerifyUTF8(str, "CRYSTAL_NET.service.NodeHeartbeatInfo.ApiList"));
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<50>(ptr));
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

uint8_t* NodeHeartbeatInfo::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:CRYSTAL_NET.service.NodeHeartbeatInfo)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // string ServiceName = 1;
  if (!this->_internal_servicename().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_servicename().data(), static_cast<int>(this->_internal_servicename().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "CRYSTAL_NET.service.NodeHeartbeatInfo.ServiceName");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_servicename(), target);
  }

  // string address = 2;
  if (!this->_internal_address().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_address().data(), static_cast<int>(this->_internal_address().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "CRYSTAL_NET.service.NodeHeartbeatInfo.address");
    target = stream->WriteStringMaybeAliased(
        2, this->_internal_address(), target);
  }

  // sint32 IpType = 3;
  if (this->_internal_iptype() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteSInt32ToArray(3, this->_internal_iptype(), target);
  }

  // uint32 InnerLinkPort = 4;
  if (this->_internal_innerlinkport() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt32ToArray(4, this->_internal_innerlinkport(), target);
  }

  // repeated sint32 SubscribeOpcodes = 5;
  {
    int byte_size = _impl_._subscribeopcodes_cached_byte_size_.load(std::memory_order_relaxed);
    if (byte_size > 0) {
      target = stream->WriteSInt32Packed(
          5, _internal_subscribeopcodes(), byte_size, target);
    }
  }

  // repeated string ApiList = 6;
  for (int i = 0, n = this->_internal_apilist_size(); i < n; i++) {
    const auto& s = this->_internal_apilist(i);
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      s.data(), static_cast<int>(s.length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "CRYSTAL_NET.service.NodeHeartbeatInfo.ApiList");
    target = stream->WriteString(6, s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:CRYSTAL_NET.service.NodeHeartbeatInfo)
  return target;
}

size_t NodeHeartbeatInfo::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:CRYSTAL_NET.service.NodeHeartbeatInfo)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated sint32 SubscribeOpcodes = 5;
  {
    size_t data_size = ::_pbi::WireFormatLite::
      SInt32Size(this->_impl_.subscribeopcodes_);
    if (data_size > 0) {
      total_size += 1 +
        ::_pbi::WireFormatLite::Int32Size(static_cast<int32_t>(data_size));
    }
    int cached_size = ::_pbi::ToCachedSize(data_size);
    _impl_._subscribeopcodes_cached_byte_size_.store(cached_size,
                                    std::memory_order_relaxed);
    total_size += data_size;
  }

  // repeated string ApiList = 6;
  total_size += 1 *
      ::PROTOBUF_NAMESPACE_ID::internal::FromIntSize(_impl_.apilist_.size());
  for (int i = 0, n = _impl_.apilist_.size(); i < n; i++) {
    total_size += ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
      _impl_.apilist_.Get(i));
  }

  // string ServiceName = 1;
  if (!this->_internal_servicename().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_servicename());
  }

  // string address = 2;
  if (!this->_internal_address().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_address());
  }

  // sint32 IpType = 3;
  if (this->_internal_iptype() != 0) {
    total_size += ::_pbi::WireFormatLite::SInt32SizePlusOne(this->_internal_iptype());
  }

  // uint32 InnerLinkPort = 4;
  if (this->_internal_innerlinkport() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt32SizePlusOne(this->_internal_innerlinkport());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData NodeHeartbeatInfo::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    NodeHeartbeatInfo::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*NodeHeartbeatInfo::GetClassData() const { return &_class_data_; }


void NodeHeartbeatInfo::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<NodeHeartbeatInfo*>(&to_msg);
  auto& from = static_cast<const NodeHeartbeatInfo&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:CRYSTAL_NET.service.NodeHeartbeatInfo)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.subscribeopcodes_.MergeFrom(from._impl_.subscribeopcodes_);
  _this->_impl_.apilist_.MergeFrom(from._impl_.apilist_);
  if (!from._internal_servicename().empty()) {
    _this->_internal_set_servicename(from._internal_servicename());
  }
  if (!from._internal_address().empty()) {
    _this->_internal_set_address(from._internal_address());
  }
  if (from._internal_iptype() != 0) {
    _this->_internal_set_iptype(from._internal_iptype());
  }
  if (from._internal_innerlinkport() != 0) {
    _this->_internal_set_innerlinkport(from._internal_innerlinkport());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void NodeHeartbeatInfo::CopyFrom(const NodeHeartbeatInfo& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:CRYSTAL_NET.service.NodeHeartbeatInfo)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool NodeHeartbeatInfo::IsInitialized() const {
  return true;
}

void NodeHeartbeatInfo::InternalSwap(NodeHeartbeatInfo* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.subscribeopcodes_.InternalSwap(&other->_impl_.subscribeopcodes_);
  _impl_.apilist_.InternalSwap(&other->_impl_.apilist_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.servicename_, lhs_arena,
      &other->_impl_.servicename_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.address_, lhs_arena,
      &other->_impl_.address_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(NodeHeartbeatInfo, _impl_.innerlinkport_)
      + sizeof(NodeHeartbeatInfo::_impl_.innerlinkport_)
      - PROTOBUF_FIELD_OFFSET(NodeHeartbeatInfo, _impl_.iptype_)>(
          reinterpret_cast<char*>(&_impl_.iptype_),
          reinterpret_cast<char*>(&other->_impl_.iptype_));
}

::PROTOBUF_NAMESPACE_ID::Metadata NodeHeartbeatInfo::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_com_5fheartbeat_2eproto_getter, &descriptor_table_com_5fheartbeat_2eproto_once,
      file_level_metadata_com_5fheartbeat_2eproto[1]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace service
}  // namespace CRYSTAL_NET
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::CRYSTAL_NET::service::NODE_IPTYPE*
Arena::CreateMaybeMessage< ::CRYSTAL_NET::service::NODE_IPTYPE >(Arena* arena) {
  return Arena::CreateMessageInternal< ::CRYSTAL_NET::service::NODE_IPTYPE >(arena);
}
template<> PROTOBUF_NOINLINE ::CRYSTAL_NET::service::NodeHeartbeatInfo*
Arena::CreateMaybeMessage< ::CRYSTAL_NET::service::NodeHeartbeatInfo >(Arena* arena) {
  return Arena::CreateMessageInternal< ::CRYSTAL_NET::service::NodeHeartbeatInfo >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
