// <auto-generated>
//     Generated by the protocol buffer compiler.  DO NOT EDIT!
//     source: com_book.proto
// </auto-generated>
#pragma warning disable 1591, 0612, 3021, 8981
#region Designer generated code

using pb = global::Google.Protobuf;
using pbc = global::Google.Protobuf.Collections;
using pbr = global::Google.Protobuf.Reflection;
using scg = global::System.Collections.Generic;
namespace CRYSTALNET.Service {

using ProtoPackage.Attributes;

  /// <summary>Holder for reflection information generated from com_book.proto</summary>
  public static partial class ComBookReflection {

    #region Descriptor
    /// <summary>File descriptor for com_book.proto</summary>
    public static pbr::FileDescriptor Descriptor {
      get { return descriptor; }
    }
    private static pbr::FileDescriptor descriptor;

    static ComBookReflection() {
      byte[] descriptorData = global::System.Convert.FromBase64String(
          string.Concat(
            "Cg5jb21fYm9vay5wcm90bxITQ1JZU1RBTF9ORVQuc2VydmljZSIgCghCb29r",
            "VHlwZSIUCgVFTlVNUxILCgdVTktOT1dOEAAiygEKCEJvb2tJbmZvEgoKAklk",
            "GAEgASgEEhAKCEJvb2tUeXBlGAIgASgREhAKCEJvb2tOYW1lGAMgASgJEhAK",
            "CElzYm5Db2RlGAQgASgJEhYKDkJvb2tDb3ZlckltYWdlGAUgASgMEhoKEkJv",
            "b2tDb3ZlckltYWdlVHlwZRgGIAEoCRINCgVQcmljZRgHIAEoBBITCgtJc09u",
            "U2hlbHZlcxgIIAEoERINCgVDb3VudBgJIAEoBBIVCg1Cb3Jyb3dlZENvdW50",
            "GAogASgEYgZwcm90bzM="));
      descriptor = pbr::FileDescriptor.FromGeneratedCode(descriptorData,
          new pbr::FileDescriptor[] { },
          new pbr::GeneratedClrTypeInfo(null, null, new pbr::GeneratedClrTypeInfo[] {
            new pbr::GeneratedClrTypeInfo(typeof(global::CRYSTALNET.Service.BookType), global::CRYSTALNET.Service.BookType.Parser, null, null, new[]{ typeof(global::CRYSTALNET.Service.BookType.Types.ENUMS) }, null, null),
            new pbr::GeneratedClrTypeInfo(typeof(global::CRYSTALNET.Service.BookInfo), global::CRYSTALNET.Service.BookInfo.Parser, new[]{ "Id", "BookType", "BookName", "IsbnCode", "BookCoverImage", "BookCoverImageType", "Price", "IsOnShelves", "Count", "BorrowedCount" }, null, null, null, null)
          }));
    }
    #endregion

  }
  #region Messages
  /// <summary>
  /// 图书类型
  /// </summary>
  public sealed partial class BookType : pb::IMessage<BookType>
  #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      , pb::IBufferMessage
  #endif
  {
    private static readonly pb::MessageParser<BookType> _parser = new pb::MessageParser<BookType>(() => new BookType());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pb::MessageParser<BookType> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::CRYSTALNET.Service.ComBookReflection.Descriptor.MessageTypes[0]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public BookType() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public BookType(BookType other) : this() {
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public BookType Clone() {
      return new BookType(this);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override bool Equals(object other) {
      return Equals(other as BookType);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public bool Equals(BookType other) {
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
    public void MergeFrom(BookType other) {
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
    /// <summary>Container for nested types declared in the BookType message type.</summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static partial class Types {
      public enum ENUMS {
        [pbr::OriginalName("UNKNOWN")] Unknown = 0,
      }

    }
    #endregion

  }

  /// <summary>
  /// 书籍信息, 数据存储:id, isbn(unique), bookInfo json
  /// </summary>
  public sealed partial class BookInfo : pb::IMessage<BookInfo>
  #if !GOOGLE_PROTOBUF_REFSTRUCT_COMPATIBILITY_MODE
      , pb::IBufferMessage
  #endif
  {
    private static readonly pb::MessageParser<BookInfo> _parser = new pb::MessageParser<BookInfo>(() => new BookInfo());
    private pb::UnknownFieldSet _unknownFields;
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pb::MessageParser<BookInfo> Parser { get { return _parser; } }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public static pbr::MessageDescriptor Descriptor {
      get { return global::CRYSTALNET.Service.ComBookReflection.Descriptor.MessageTypes[1]; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    pbr::MessageDescriptor pb::IMessage.Descriptor {
      get { return Descriptor; }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public BookInfo() {
      OnConstruction();
    }

    partial void OnConstruction();

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public BookInfo(BookInfo other) : this() {
      id_ = other.id_;
      bookType_ = other.bookType_;
      bookName_ = other.bookName_;
      isbnCode_ = other.isbnCode_;
      bookCoverImage_ = other.bookCoverImage_;
      bookCoverImageType_ = other.bookCoverImageType_;
      price_ = other.price_;
      isOnShelves_ = other.isOnShelves_;
      count_ = other.count_;
      borrowedCount_ = other.borrowedCount_;
      _unknownFields = pb::UnknownFieldSet.Clone(other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public BookInfo Clone() {
      return new BookInfo(this);
    }

    /// <summary>Field number for the "Id" field.</summary>
    public const int IdFieldNumber = 1;
    private ulong id_;
    /// <summary>
    /// 唯一id, 同一个isbn, 同一个id
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ulong Id {
      get { return id_; }
      set {
        id_ = value;
      }
    }

    /// <summary>Field number for the "BookType" field.</summary>
    public const int BookTypeFieldNumber = 2;
    private int bookType_;
    /// <summary>
    /// 图书类型 BookType
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public int BookType {
      get { return bookType_; }
      set {
        bookType_ = value;
      }
    }

    /// <summary>Field number for the "BookName" field.</summary>
    public const int BookNameFieldNumber = 3;
    private string bookName_ = "";
    /// <summary>
    /// 书名
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public string BookName {
      get { return bookName_; }
      set {
        bookName_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "IsbnCode" field.</summary>
    public const int IsbnCodeFieldNumber = 4;
    private string isbnCode_ = "";
    /// <summary>
    /// 书isbn码
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public string IsbnCode {
      get { return isbnCode_; }
      set {
        isbnCode_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "BookCoverImage" field.</summary>
    public const int BookCoverImageFieldNumber = 5;
    private pb::ByteString bookCoverImage_ = pb::ByteString.Empty;
    /// <summary>
    /// 封面
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public pb::ByteString BookCoverImage {
      get { return bookCoverImage_; }
      set {
        bookCoverImage_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "BookCoverImageType" field.</summary>
    public const int BookCoverImageTypeFieldNumber = 6;
    private string bookCoverImageType_ = "";
    /// <summary>
    /// 封面图片类型
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public string BookCoverImageType {
      get { return bookCoverImageType_; }
      set {
        bookCoverImageType_ = pb::ProtoPreconditions.CheckNotNull(value, "value");
      }
    }

    /// <summary>Field number for the "Price" field.</summary>
    public const int PriceFieldNumber = 7;
    private ulong price_;
    /// <summary>
    /// 价格(单位分)
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ulong Price {
      get { return price_; }
      set {
        price_ = value;
      }
    }

    /// <summary>Field number for the "IsOnShelves" field.</summary>
    public const int IsOnShelvesFieldNumber = 8;
    private int isOnShelves_;
    /// <summary>
    /// 是否上架
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public int IsOnShelves {
      get { return isOnShelves_; }
      set {
        isOnShelves_ = value;
      }
    }

    /// <summary>Field number for the "Count" field.</summary>
    public const int CountFieldNumber = 9;
    private ulong count_;
    /// <summary>
    /// 库存
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ulong Count {
      get { return count_; }
      set {
        count_ = value;
      }
    }

    /// <summary>Field number for the "BorrowedCount" field.</summary>
    public const int BorrowedCountFieldNumber = 10;
    private ulong borrowedCount_;
    /// <summary>
    /// 被借数量
    /// </summary>
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public ulong BorrowedCount {
      get { return borrowedCount_; }
      set {
        borrowedCount_ = value;
      }
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override bool Equals(object other) {
      return Equals(other as BookInfo);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public bool Equals(BookInfo other) {
      if (ReferenceEquals(other, null)) {
        return false;
      }
      if (ReferenceEquals(other, this)) {
        return true;
      }
      if (Id != other.Id) return false;
      if (BookType != other.BookType) return false;
      if (BookName != other.BookName) return false;
      if (IsbnCode != other.IsbnCode) return false;
      if (BookCoverImage != other.BookCoverImage) return false;
      if (BookCoverImageType != other.BookCoverImageType) return false;
      if (Price != other.Price) return false;
      if (IsOnShelves != other.IsOnShelves) return false;
      if (Count != other.Count) return false;
      if (BorrowedCount != other.BorrowedCount) return false;
      return Equals(_unknownFields, other._unknownFields);
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public override int GetHashCode() {
      int hash = 1;
      if (Id != 0UL) hash ^= Id.GetHashCode();
      if (BookType != 0) hash ^= BookType.GetHashCode();
      if (BookName.Length != 0) hash ^= BookName.GetHashCode();
      if (IsbnCode.Length != 0) hash ^= IsbnCode.GetHashCode();
      if (BookCoverImage.Length != 0) hash ^= BookCoverImage.GetHashCode();
      if (BookCoverImageType.Length != 0) hash ^= BookCoverImageType.GetHashCode();
      if (Price != 0UL) hash ^= Price.GetHashCode();
      if (IsOnShelves != 0) hash ^= IsOnShelves.GetHashCode();
      if (Count != 0UL) hash ^= Count.GetHashCode();
      if (BorrowedCount != 0UL) hash ^= BorrowedCount.GetHashCode();
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
      if (Id != 0UL) {
        output.WriteRawTag(8);
        output.WriteUInt64(Id);
      }
      if (BookType != 0) {
        output.WriteRawTag(16);
        output.WriteSInt32(BookType);
      }
      if (BookName.Length != 0) {
        output.WriteRawTag(26);
        output.WriteString(BookName);
      }
      if (IsbnCode.Length != 0) {
        output.WriteRawTag(34);
        output.WriteString(IsbnCode);
      }
      if (BookCoverImage.Length != 0) {
        output.WriteRawTag(42);
        output.WriteBytes(BookCoverImage);
      }
      if (BookCoverImageType.Length != 0) {
        output.WriteRawTag(50);
        output.WriteString(BookCoverImageType);
      }
      if (Price != 0UL) {
        output.WriteRawTag(56);
        output.WriteUInt64(Price);
      }
      if (IsOnShelves != 0) {
        output.WriteRawTag(64);
        output.WriteSInt32(IsOnShelves);
      }
      if (Count != 0UL) {
        output.WriteRawTag(72);
        output.WriteUInt64(Count);
      }
      if (BorrowedCount != 0UL) {
        output.WriteRawTag(80);
        output.WriteUInt64(BorrowedCount);
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
      if (Id != 0UL) {
        output.WriteRawTag(8);
        output.WriteUInt64(Id);
      }
      if (BookType != 0) {
        output.WriteRawTag(16);
        output.WriteSInt32(BookType);
      }
      if (BookName.Length != 0) {
        output.WriteRawTag(26);
        output.WriteString(BookName);
      }
      if (IsbnCode.Length != 0) {
        output.WriteRawTag(34);
        output.WriteString(IsbnCode);
      }
      if (BookCoverImage.Length != 0) {
        output.WriteRawTag(42);
        output.WriteBytes(BookCoverImage);
      }
      if (BookCoverImageType.Length != 0) {
        output.WriteRawTag(50);
        output.WriteString(BookCoverImageType);
      }
      if (Price != 0UL) {
        output.WriteRawTag(56);
        output.WriteUInt64(Price);
      }
      if (IsOnShelves != 0) {
        output.WriteRawTag(64);
        output.WriteSInt32(IsOnShelves);
      }
      if (Count != 0UL) {
        output.WriteRawTag(72);
        output.WriteUInt64(Count);
      }
      if (BorrowedCount != 0UL) {
        output.WriteRawTag(80);
        output.WriteUInt64(BorrowedCount);
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
      if (Id != 0UL) {
        size += 1 + pb::CodedOutputStream.ComputeUInt64Size(Id);
      }
      if (BookType != 0) {
        size += 1 + pb::CodedOutputStream.ComputeSInt32Size(BookType);
      }
      if (BookName.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeStringSize(BookName);
      }
      if (IsbnCode.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeStringSize(IsbnCode);
      }
      if (BookCoverImage.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeBytesSize(BookCoverImage);
      }
      if (BookCoverImageType.Length != 0) {
        size += 1 + pb::CodedOutputStream.ComputeStringSize(BookCoverImageType);
      }
      if (Price != 0UL) {
        size += 1 + pb::CodedOutputStream.ComputeUInt64Size(Price);
      }
      if (IsOnShelves != 0) {
        size += 1 + pb::CodedOutputStream.ComputeSInt32Size(IsOnShelves);
      }
      if (Count != 0UL) {
        size += 1 + pb::CodedOutputStream.ComputeUInt64Size(Count);
      }
      if (BorrowedCount != 0UL) {
        size += 1 + pb::CodedOutputStream.ComputeUInt64Size(BorrowedCount);
      }
      if (_unknownFields != null) {
        size += _unknownFields.CalculateSize();
      }
      return size;
    }

    [global::System.Diagnostics.DebuggerNonUserCodeAttribute]
    [global::System.CodeDom.Compiler.GeneratedCode("protoc", null)]
    public void MergeFrom(BookInfo other) {
      if (other == null) {
        return;
      }
      if (other.Id != 0UL) {
        Id = other.Id;
      }
      if (other.BookType != 0) {
        BookType = other.BookType;
      }
      if (other.BookName.Length != 0) {
        BookName = other.BookName;
      }
      if (other.IsbnCode.Length != 0) {
        IsbnCode = other.IsbnCode;
      }
      if (other.BookCoverImage.Length != 0) {
        BookCoverImage = other.BookCoverImage;
      }
      if (other.BookCoverImageType.Length != 0) {
        BookCoverImageType = other.BookCoverImageType;
      }
      if (other.Price != 0UL) {
        Price = other.Price;
      }
      if (other.IsOnShelves != 0) {
        IsOnShelves = other.IsOnShelves;
      }
      if (other.Count != 0UL) {
        Count = other.Count;
      }
      if (other.BorrowedCount != 0UL) {
        BorrowedCount = other.BorrowedCount;
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
            Id = input.ReadUInt64();
            break;
          }
          case 16: {
            BookType = input.ReadSInt32();
            break;
          }
          case 26: {
            BookName = input.ReadString();
            break;
          }
          case 34: {
            IsbnCode = input.ReadString();
            break;
          }
          case 42: {
            BookCoverImage = input.ReadBytes();
            break;
          }
          case 50: {
            BookCoverImageType = input.ReadString();
            break;
          }
          case 56: {
            Price = input.ReadUInt64();
            break;
          }
          case 64: {
            IsOnShelves = input.ReadSInt32();
            break;
          }
          case 72: {
            Count = input.ReadUInt64();
            break;
          }
          case 80: {
            BorrowedCount = input.ReadUInt64();
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
            Id = input.ReadUInt64();
            break;
          }
          case 16: {
            BookType = input.ReadSInt32();
            break;
          }
          case 26: {
            BookName = input.ReadString();
            break;
          }
          case 34: {
            IsbnCode = input.ReadString();
            break;
          }
          case 42: {
            BookCoverImage = input.ReadBytes();
            break;
          }
          case 50: {
            BookCoverImageType = input.ReadString();
            break;
          }
          case 56: {
            Price = input.ReadUInt64();
            break;
          }
          case 64: {
            IsOnShelves = input.ReadSInt32();
            break;
          }
          case 72: {
            Count = input.ReadUInt64();
            break;
          }
          case 80: {
            BorrowedCount = input.ReadUInt64();
            break;
          }
        }
      }
    }
    #endif

  }

  #endregion

}

#endregion Designer generated code