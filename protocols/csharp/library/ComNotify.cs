// <auto-generated>
//     Generated by the protocol buffer compiler.  DO NOT EDIT!
//     source: com_notify.proto
// </auto-generated>
#pragma warning disable 1591, 0612, 3021, 8981
#region Designer generated code

using pb = global::Google.Protobuf;
using pbc = global::Google.Protobuf.Collections;
using pbr = global::Google.Protobuf.Reflection;
using scg = global::System.Collections.Generic;
namespace CRYSTALNET.Service {

using ProtoPackage.Attributes;

  /// <summary>Holder for reflection information generated from com_notify.proto</summary>
  public static partial class ComNotifyReflection {

    #region Descriptor
    /// <summary>File descriptor for com_notify.proto</summary>
    public static pbr::FileDescriptor Descriptor {
      get { return descriptor; }
    }
    private static pbr::FileDescriptor descriptor;

    static ComNotifyReflection() {
      byte[] descriptorData = global::System.Convert.FromBase64String(
          string.Concat(
            "ChBjb21fbm90aWZ5LnByb3RvEhNDUllTVEFMX05FVC5zZXJ2aWNlGhFjb21f",
            "dmFyaWFudC5wcm90byL0AQoSVXNlck5vdGlmeURhdGFJdGVtEhAKCE5vdGlm",
            "eUlkGAEgASgEEhkKEU5vdGlmeVRpdGxlV29yZElkGAIgASgJEjYKC1RpdGxl",
            "UGFyYW1zGAMgAygLMiEuQ1JZU1RBTF9ORVQuc2VydmljZS5WYXJpYW50UGFy",
            "YW0SGwoTTm90aWZ5Q29udGVudFdvcmRJZBgEIAEoCRI4Cg1Db250ZW50UGFy",
            "YW1zGAUgAygLMiEuQ1JZU1RBTF9ORVQuc2VydmljZS5WYXJpYW50UGFyYW0S",
            "EgoKQ3JlYXRlVGltZRgGIAEoEhIOCgZJc1JlYWQYByABKBEiSwoOVXNlck5v",
            "dGlmeURhdGESOQoISXRlbUxpc3QYASADKAsyJy5DUllTVEFMX05FVC5zZXJ2",
            "aWNlLlVzZXJOb3RpZnlEYXRhSXRlbSI2Cg9DbGVhck5vdGlmeVR5cGUiIwoF",
            "RU5VTVMSDAoIT25seVJlYWQQABIMCghDbGVhckFsbBABYgZwcm90bzM="));
      descriptor = pbr::FileDescriptor.FromGeneratedCode(descriptorData,
          new pbr::FileDescriptor[] { global::CRYSTALNET.Service.ComVariantReflection.Descriptor, },
          new pbr::GeneratedClrTypeInfo(null, null, new pbr::GeneratedClrTypeInfo[] {
            new pbr::GeneratedClrTypeInfo(typeof(global::CRYSTALNET.Service.UserNotifyDataItem), global::CRYSTALNET.Service.UserNotifyDataItem.Parser, new[]{ "NotifyId", "NotifyTitleWordId", "TitleParams", "NotifyContentWordId", "ContentParams", "CreateTime", "IsRead" }, null, null, null, null),
            new pbr::GeneratedClrTypeInfo(typeof(global::CRYSTALNET.Service.UserNotifyData), global::CRYSTALNET.Service.UserNotifyData.Parser, new[]{ "ItemList" }, null, null, null, null),
            new pbr::GeneratedClrTypeInfo(typeof(global::CRYSTALNET.Service.ClearNotifyType), global::CRYSTALNET.Service.ClearNotifyType.Parser, null, null, new[]{ typeof(global::CRYSTALNET.Service.ClearNotifyType.Types.ENUMS) }, null, null)
          }));
    }
    #endregion

  }
  #region Messages
  /// <summary>
  /// 通知数据
  /// </summary>
  public sealed partial class UserNotifyDataItem : pb::IMessage<UserNotifyDataItem>
  #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      , pb::IBufferMessage
  #endif
  {
    private static readonly pb::MessageParser<UserNotifyDataItem> _parser = new pb::MessageParser<UserNotifyDataItem>(() => new UserNotifyDataItem());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pb::MessageParser<UserNotifyDataItem> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::CRYSTALNET.Service.ComNotifyReflection.Descriptor.MessageTypes[0]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public UserNotifyDataItem() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public UserNotifyDataItem(UserNotifyDataItem other) : this() {
      notifyId_ = other.notifyId_;
      notifyTitleWordId_ = other.notifyTitleWordId_;
      titleParams_ = other.titleParams_.Clone();
      notifyContentWordId_ = other.notifyContentWordId_;
      contentParams_ = other.contentParams_.Clone();
      createTime_ = other.createTime_;
      isRead_ = other.isRead_;
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public UserNotifyDataItem Clone() {
      return new UserNotifyDataItem(this);
    }

    /// <summary>Field number for the "NotifyId" field.</summary>
    public const int NotifyIdFieldNumber = 1;
    private ulong notifyId_;
    /// <summary>
    /// 用于识别的唯一id
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ulong NotifyId {
      get { return notifyId_; }
      set {
        notifyId_ = value;
      }
    }

    /// <summary>Field number for the "NotifyTitleWordId" field.</summary>
    public const int NotifyTitleWordIdFieldNumber = 2;
    private string notifyTitleWordId_ = "";
    /// <summary>
    /// 通知的title文字id
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public string NotifyTitleWordId {
      get { return notifyTitleWordId_; }
      set {
        notifyTitleWordId_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "TitleParams" field.</summary>
    public const int TitleParamsFieldNumber = 3;
    private static readonly pb::FieldCodec<global::CRYSTALNET.Service.VariantParam> _repeated_titleParams_codec
        = pb::FieldCodec.ForMessage(26, global::CRYSTALNET.Service.VariantParam.Parser);
    private readonly pbc::RepeatedField<global::CRYSTALNET.Service.VariantParam> titleParams_ = new pbc::RepeatedField<global::CRYSTALNET.Service.VariantParam>();
    /// <summary>
    /// title的参数
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public pbc::RepeatedField<global::CRYSTALNET.Service.VariantParam> TitleParams {
      get { return titleParams_; }
    }

    /// <summary>Field number for the "NotifyContentWordId" field.</summary>
    public const int NotifyContentWordIdFieldNumber = 4;
    private string notifyContentWordId_ = "";
    /// <summary>
    /// 通知的内容文字id
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public string NotifyContentWordId {
      get { return notifyContentWordId_; }
      set {
        notifyContentWordId_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "ContentParams" field.</summary>
    public const int ContentParamsFieldNumber = 5;
    private static readonly pb::FieldCodec<global::CRYSTALNET.Service.VariantParam> _repeated_contentParams_codec
        = pb::FieldCodec.ForMessage(42, global::CRYSTALNET.Service.VariantParam.Parser);
    private readonly pbc::RepeatedField<global::CRYSTALNET.Service.VariantParam> contentParams_ = new pbc::RepeatedField<global::CRYSTALNET.Service.VariantParam>();
    /// <summary>
    /// 内容的参数
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public pbc::RepeatedField<global::CRYSTALNET.Service.VariantParam> ContentParams {
      get { return contentParams_; }
    }

    /// <summary>Field number for the "CreateTime" field.</summary>
    public const int CreateTimeFieldNumber = 6;
    private long createTime_;
    /// <summary>
    /// 时间
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public long CreateTime {
      get { return createTime_; }
      set {
        createTime_ = value;
      }
    }

    /// <summary>Field number for the "IsRead" field.</summary>
    public const int IsReadFieldNumber = 7;
    private int isRead_;
    /// <summary>
    /// 是否已读
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public int IsRead {
      get { return isRead_; }
      set {
        isRead_ = value;
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override bool Equals(object other) {
      return Equals(other as UserNotifyDataItem);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public bool Equals(UserNotifyDataItem other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if (NotifyId != other.NotifyId) return false;
      if (NotifyTitleWordId != other.NotifyTitleWordId) return false;
      if(!titleParams_.Equals(other.titleParams_)) return false;
      if (NotifyContentWordId != other.NotifyContentWordId) return false;
      if(!contentParams_.Equals(other.contentParams_)) return false;
      if (CreateTime != other.CreateTime) return false;
      if (IsRead != other.IsRead) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override int GetHashCode() {
      int hash = 1;
      if (NotifyId != 0UL) hash ^= NotifyId.GetHashCode();
      if (NotifyTitleWordId.Length != 0) hash ^= NotifyTitleWordId.GetHashCode();
      hash ^= titleParams_.GetHashCode();
      if (NotifyContentWordId.Length != 0) hash ^= NotifyContentWordId.GetHashCode();
      hash ^= contentParams_.GetHashCode();
      if (CreateTime != 0L) hash ^= CreateTime.GetHashCode();
      if (IsRead != 0) hash ^= IsRead.GetHashCode();
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void WriteTo(pb::CodedOutputStream output) {
    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      output.WriteRawMessage(this);
    #else
      if (NotifyId != 0UL) {
        output.WriteRawTag(8);
        output.WriteUInt64(NotifyId);
      }
      if (NotifyTitleWordId.Length != 0) {
        output.WriteRawTag(18);
        output.WriteString(NotifyTitleWordId);
      }
      titleParams_.WriteTo(output, _repeated_titleParams_codec);
      if (NotifyContentWordId.Length != 0) {
        output.WriteRawTag(34);
        output.WriteString(NotifyContentWordId);
      }
      contentParams_.WriteTo(output, _repeated_contentParams_codec);
      if (CreateTime != 0L) {
        output.WriteRawTag(48);
        output.WriteSInt64(CreateTime);
      }
      if (IsRead != 0) {
        output.WriteRawTag(56);
        output.WriteSInt32(IsRead);
      }
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    #endif
    }

    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    void pb::IBufferMessage.InternalWriteTo(ref pb::WriteContext output) {
      if (NotifyId != 0UL) {
        output.WriteRawTag(8);
        output.WriteUInt64(NotifyId);
      }
      if (NotifyTitleWordId.Length != 0) {
        output.WriteRawTag(18);
        output.WriteString(NotifyTitleWordId);
      }
      titleParams_.WriteTo(ref output, _repeated_titleParams_codec);
      if (NotifyContentWordId.Length != 0) {
        output.WriteRawTag(34);
        output.WriteString(NotifyContentWordId);
      }
      contentParams_.WriteTo(ref output, _repeated_contentParams_codec);
      if (CreateTime != 0L) {
        output.WriteRawTag(48);
        output.WriteSInt64(CreateTime);
      }
      if (IsRead != 0) {
        output.WriteRawTag(56);
        output.WriteSInt32(IsRead);
      }
      if (_unknownFields != null) {
        _unknownFields.WriteTo(ref output);
      }
    }
    #endif

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public int CalculateSize() {
      int size = 0;
      if (NotifyId != 0UL) {
        size += 1 + pb::CodedOutputStream.ComputeUInt64Size(NotifyId);
      }
      if (NotifyTitleWordId.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeStringSize(NotifyTitleWordId);
      }
      size += titleParams_.CalculateSize(_repeated_titleParams_codec);
      if (NotifyContentWordId.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeStringSize(NotifyContentWordId);
      }
      size += contentParams_.CalculateSize(_repeated_contentParams_codec);
      if (CreateTime != 0L) {
        size += 1 + pb::CodedOutputStream.ComputeSInt64Size(CreateTime);
      }
      if (IsRead != 0) {
        size += 1 + pb::CodedOutputStream.ComputeSInt32Size(IsRead);
      }
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void MergeFrom(UserNotifyDataItem other) {
      if (other == null) {
        return;
      }
      if (other.NotifyId != 0UL) {
        NotifyId = other.NotifyId;
      }
      if (other.NotifyTitleWordId.Length != 0) {
        NotifyTitleWordId = other.NotifyTitleWordId;
      }
      titleParams_.Add(other.titleParams_);
      if (other.NotifyContentWordId.Length != 0) {
        NotifyContentWordId = other.NotifyContentWordId;
      }
      contentParams_.Add(other.contentParams_);
      if (other.CreateTime != 0L) {
        CreateTime = other.CreateTime;
      }
      if (other.IsRead != 0) {
        IsRead = other.IsRead;
      }
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void MergeFrom(pb::CodedInputStream input) {
    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      input.ReadRawMessage(this);
    #else
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
          case 8: {
            NotifyId = input.ReadUInt64();
            break;
          }
          case 18: {
            NotifyTitleWordId = input.ReadString();
            break;
          }
          case 26: {
            titleParams_.AddEntriesFrom(input, _repeated_titleParams_codec);
            break;
          }
          case 34: {
            NotifyContentWordId = input.ReadString();
            break;
          }
          case 42: {
            contentParams_.AddEntriesFrom(input, _repeated_contentParams_codec);
            break;
          }
          case 48: {
            CreateTime = input.ReadSInt64();
            break;
          }
          case 56: {
            IsRead = input.ReadSInt32();
            break;
          }
        }
      }
    #endif
    }

    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    void pb::IBufferMessage.InternalMergeFrom(ref pb::ParseContext input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, ref input);
            break;
          case 8: {
            NotifyId = input.ReadUInt64();
            break;
          }
          case 18: {
            NotifyTitleWordId = input.ReadString();
            break;
          }
          case 26: {
            titleParams_.AddEntriesFrom(ref input, _repeated_titleParams_codec);
            break;
          }
          case 34: {
            NotifyContentWordId = input.ReadString();
            break;
          }
          case 42: {
            contentParams_.AddEntriesFrom(ref input, _repeated_contentParams_codec);
            break;
          }
          case 48: {
            CreateTime = input.ReadSInt64();
            break;
          }
          case 56: {
            IsRead = input.ReadSInt32();
            break;
          }
        }
      }
    }
    #endif

  }

  public sealed partial class UserNotifyData : pb::IMessage<UserNotifyData>
  #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      , pb::IBufferMessage
  #endif
  {
    private static readonly pb::MessageParser<UserNotifyData> _parser = new pb::MessageParser<UserNotifyData>(() => new UserNotifyData());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pb::MessageParser<UserNotifyData> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::CRYSTALNET.Service.ComNotifyReflection.Descriptor.MessageTypes[1]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public UserNotifyData() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public UserNotifyData(UserNotifyData other) : this() {
      itemList_ = other.itemList_.Clone();
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public UserNotifyData Clone() {
      return new UserNotifyData(this);
    }

    /// <summary>Field number for the "ItemList" field.</summary>
    public const int ItemListFieldNumber = 1;
    private static readonly pb::FieldCodec<global::CRYSTALNET.Service.UserNotifyDataItem> _repeated_itemList_codec
        = pb::FieldCodec.ForMessage(10, global::CRYSTALNET.Service.UserNotifyDataItem.Parser);
    private readonly pbc::RepeatedField<global::CRYSTALNET.Service.UserNotifyDataItem> itemList_ = new pbc::RepeatedField<global::CRYSTALNET.Service.UserNotifyDataItem>();
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public pbc::RepeatedField<global::CRYSTALNET.Service.UserNotifyDataItem> ItemList {
      get { return itemList_; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override bool Equals(object other) {
      return Equals(other as UserNotifyData);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public bool Equals(UserNotifyData other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if(!itemList_.Equals(other.itemList_)) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override int GetHashCode() {
      int hash = 1;
      hash ^= itemList_.GetHashCode();
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void WriteTo(pb::CodedOutputStream output) {
    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      output.WriteRawMessage(this);
    #else
      itemList_.WriteTo(output, _repeated_itemList_codec);
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    #endif
    }

    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    void pb::IBufferMessage.InternalWriteTo(ref pb::WriteContext output) {
      itemList_.WriteTo(ref output, _repeated_itemList_codec);
      if (_unknownFields != null) {
        _unknownFields.WriteTo(ref output);
      }
    }
    #endif

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public int CalculateSize() {
      int size = 0;
      size += itemList_.CalculateSize(_repeated_itemList_codec);
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void MergeFrom(UserNotifyData other) {
      if (other == null) {
        return;
      }
      itemList_.Add(other.itemList_);
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void MergeFrom(pb::CodedInputStream input) {
    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      input.ReadRawMessage(this);
    #else
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
          case 10: {
            itemList_.AddEntriesFrom(input, _repeated_itemList_codec);
            break;
          }
        }
      }
    #endif
    }

    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    void pb::IBufferMessage.InternalMergeFrom(ref pb::ParseContext input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, ref input);
            break;
          case 10: {
            itemList_.AddEntriesFrom(ref input, _repeated_itemList_codec);
            break;
          }
        }
      }
    }
    #endif

  }

  public sealed partial class ClearNotifyType : pb::IMessage<ClearNotifyType>
  #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      , pb::IBufferMessage
  #endif
  {
    private static readonly pb::MessageParser<ClearNotifyType> _parser = new pb::MessageParser<ClearNotifyType>(() => new ClearNotifyType());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pb::MessageParser<ClearNotifyType> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::CRYSTALNET.Service.ComNotifyReflection.Descriptor.MessageTypes[2]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ClearNotifyType() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ClearNotifyType(ClearNotifyType other) : this() {
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ClearNotifyType Clone() {
      return new ClearNotifyType(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override bool Equals(object other) {
      return Equals(other as ClearNotifyType);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public bool Equals(ClearNotifyType other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override int GetHashCode() {
      int hash = 1;
      if (_unknownFields != null) {
        hash ^= _unknownFields.GetHashCode();
      }
      return hash;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override string ToString() {
      return pb::JsonFormatter.ToDiagnosticString(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void WriteTo(pb::CodedOutputStream output) {
    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      output.WriteRawMessage(this);
    #else
      if (_unknownFields != null) {
        _unknownFields.WriteTo(output);
      }
    #endif
    }

    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    void pb::IBufferMessage.InternalWriteTo(ref pb::WriteContext output) {
      if (_unknownFields != null) {
        _unknownFields.WriteTo(ref output);
      }
    }
    #endif

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public int CalculateSize() {
      int size = 0;
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void MergeFrom(ClearNotifyType other) {
      if (other == null) {
        return;
      }
      _unknownFields = pb::UnknownFieldSet.MergeFrom(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void MergeFrom(pb::CodedInputStream input) {
    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      input.ReadRawMessage(this);
    #else
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, input);
            break;
        }
      }
    #endif
    }

    #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    void pb::IBufferMessage.InternalMergeFrom(ref pb::ParseContext input) {
      uint tag;
      while ((tag = input.ReadTag()) != 0) {
        switch(tag) {
          default:
            _unknownFields = pb::UnknownFieldSet.MergeFieldFrom(_unknownFields, ref input);
            break;
        }
      }
    }
    #endif

    #region Nested types
    /// <summary>Container for nested types declared in the ClearNotifyType message type.</summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static partial class Types {
      public enum ENUMS {
        /// <summary>
        /// 只清理已读
        /// </summary>
        [pbr::OriginalName("OnlyRead")] OnlyRead = 0,
        /// <summary>
        /// 清理全部
        /// </summary>
        [pbr::OriginalName("ClearAll")] ClearAll = 1,
      }

    }
    #endregion

  }

  #endregion

}

#endregion Designer generated code