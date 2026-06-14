// Code generated manually to match storage.proto. DO NOT EDIT.

package storagepb

import (
	proto "google.golang.org/protobuf/proto"
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	descriptorpb "google.golang.org/protobuf/types/descriptorpb"
	emptypb "google.golang.org/protobuf/types/known/emptypb"
	reflect "reflect"
	sync "sync"
)

const (
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

type StoreFileRequest struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	SourcePath    string                 `protobuf:"bytes,1,opt,name=source_path,json=sourcePath,proto3" json:"source_path,omitempty"`
	FileId        string                 `protobuf:"bytes,2,opt,name=file_id,json=fileId,proto3" json:"file_id,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *StoreFileRequest) Reset() {
	*x = StoreFileRequest{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[0]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *StoreFileRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*StoreFileRequest) ProtoMessage()    {}
func (x *StoreFileRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[0]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*StoreFileRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{0}
}
func (x *StoreFileRequest) GetSourcePath() string {
	if x != nil {
		return x.SourcePath
	}
	return ""
}
func (x *StoreFileRequest) GetFileId() string {
	if x != nil {
		return x.FileId
	}
	return ""
}

type RetrieveFileRequest struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	FileId        string                 `protobuf:"bytes,1,opt,name=file_id,json=fileId,proto3" json:"file_id,omitempty"`
	OutputPath    string                 `protobuf:"bytes,2,opt,name=output_path,json=outputPath,proto3" json:"output_path,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *RetrieveFileRequest) Reset() {
	*x = RetrieveFileRequest{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[1]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *RetrieveFileRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*RetrieveFileRequest) ProtoMessage()    {}
func (x *RetrieveFileRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[1]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*RetrieveFileRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{1}
}
func (x *RetrieveFileRequest) GetFileId() string {
	if x != nil {
		return x.FileId
	}
	return ""
}
func (x *RetrieveFileRequest) GetOutputPath() string {
	if x != nil {
		return x.OutputPath
	}
	return ""
}

type ListFilesRequest struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *ListFilesRequest) Reset() {
	*x = ListFilesRequest{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[2]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *ListFilesRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*ListFilesRequest) ProtoMessage()    {}
func (x *ListFilesRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[2]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*ListFilesRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{2}
}

type ListFilesResponse struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	FileIds       []string               `protobuf:"bytes,1,rep,name=file_ids,json=fileIds,proto3" json:"file_ids,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *ListFilesResponse) Reset() {
	*x = ListFilesResponse{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[3]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *ListFilesResponse) String() string { return protoimpl.X.MessageStringOf(x) }
func (*ListFilesResponse) ProtoMessage()    {}
func (x *ListFilesResponse) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[3]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*ListFilesResponse) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{3}
}
func (x *ListFilesResponse) GetFileIds() []string {
	if x != nil {
		return x.FileIds
	}
	return nil
}

type DeleteFileRequest struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	FileId        string                 `protobuf:"bytes,1,opt,name=file_id,json=fileId,proto3" json:"file_id,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *DeleteFileRequest) Reset() {
	*x = DeleteFileRequest{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[4]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *DeleteFileRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*DeleteFileRequest) ProtoMessage()    {}
func (x *DeleteFileRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[4]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*DeleteFileRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{4}
}
func (x *DeleteFileRequest) GetFileId() string {
	if x != nil {
		return x.FileId
	}
	return ""
}

type GetProgressRequest struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	FileId        string                 `protobuf:"bytes,1,opt,name=file_id,json=fileId,proto3" json:"file_id,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *GetProgressRequest) Reset() {
	*x = GetProgressRequest{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[5]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *GetProgressRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*GetProgressRequest) ProtoMessage()    {}
func (x *GetProgressRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[5]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*GetProgressRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{5}
}
func (x *GetProgressRequest) GetFileId() string {
	if x != nil {
		return x.FileId
	}
	return ""
}

type GetProgressResponse struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	Percent       int32                  `protobuf:"varint,1,opt,name=percent,proto3" json:"percent,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *GetProgressResponse) Reset() {
	*x = GetProgressResponse{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[6]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *GetProgressResponse) String() string { return protoimpl.X.MessageStringOf(x) }
func (*GetProgressResponse) ProtoMessage()    {}
func (x *GetProgressResponse) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[6]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*GetProgressResponse) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{6}
}
func (x *GetProgressResponse) GetPercent() int32 {
	if x != nil {
		return x.Percent
	}
	return 0
}

type WaitForBackgroundTasksRequest struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *WaitForBackgroundTasksRequest) Reset() {
	*x = WaitForBackgroundTasksRequest{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[7]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *WaitForBackgroundTasksRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*WaitForBackgroundTasksRequest) ProtoMessage()    {}
func (x *WaitForBackgroundTasksRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[7]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}
func (*WaitForBackgroundTasksRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{7}
}

var File_internal_api_storagepb_storage_proto protoreflect.FileDescriptor

var (
	file_internal_api_storagepb_storage_proto_rawDescOnce sync.Once
	file_internal_api_storagepb_storage_proto_rawDescData = buildFileInternalAPIStoragepbStorageProtoRawDesc()
)

func file_internal_api_storagepb_storage_proto_rawDescGZIP() []byte {
	file_internal_api_storagepb_storage_proto_rawDescOnce.Do(func() {
		file_internal_api_storagepb_storage_proto_rawDescData = protoimpl.X.CompressGZIP(file_internal_api_storagepb_storage_proto_rawDescData)
	})
	return file_internal_api_storagepb_storage_proto_rawDescData
}

var file_internal_api_storagepb_storage_proto_msgTypes = make([]protoimpl.MessageInfo, 8)
var file_internal_api_storagepb_storage_proto_goTypes = []any{
	(*StoreFileRequest)(nil),
	(*RetrieveFileRequest)(nil),
	(*ListFilesRequest)(nil),
	(*ListFilesResponse)(nil),
	(*DeleteFileRequest)(nil),
	(*GetProgressRequest)(nil),
	(*GetProgressResponse)(nil),
	(*WaitForBackgroundTasksRequest)(nil),
	(*emptypb.Empty)(nil),
}
var file_internal_api_storagepb_storage_proto_depIdxs = []int32{
	0, 1, 2, 4, 8, 5, 7,
	8, 8, 3, 8, 8, 6, 8,
	7,
	0,
	0,
	0,
	0,
}

func init() { file_internal_api_storagepb_storage_proto_init() }

func file_internal_api_storagepb_storage_proto_init() {
	if File_internal_api_storagepb_storage_proto != nil {
		return
	}
	if !protoimpl.UnsafeEnabled {
		file_internal_api_storagepb_storage_proto_msgTypes[0].Exporter = func(v any, i int) any {
			switch v := v.(*StoreFileRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_internal_api_storagepb_storage_proto_msgTypes[1].Exporter = func(v any, i int) any {
			switch v := v.(*RetrieveFileRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_internal_api_storagepb_storage_proto_msgTypes[2].Exporter = func(v any, i int) any {
			switch v := v.(*ListFilesRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_internal_api_storagepb_storage_proto_msgTypes[3].Exporter = func(v any, i int) any {
			switch v := v.(*ListFilesResponse); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_internal_api_storagepb_storage_proto_msgTypes[4].Exporter = func(v any, i int) any {
			switch v := v.(*DeleteFileRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_internal_api_storagepb_storage_proto_msgTypes[5].Exporter = func(v any, i int) any {
			switch v := v.(*GetProgressRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_internal_api_storagepb_storage_proto_msgTypes[6].Exporter = func(v any, i int) any {
			switch v := v.(*GetProgressResponse); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
		file_internal_api_storagepb_storage_proto_msgTypes[7].Exporter = func(v any, i int) any {
			switch v := v.(*WaitForBackgroundTasksRequest); i {
			case 0:
				return &v.state
			case 1:
				return &v.sizeCache
			case 2:
				return &v.unknownFields
			default:
				return nil
			}
		}
	}

	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_internal_api_storagepb_storage_proto_rawDescData,
			NumEnums:      0,
			NumMessages:   8,
			NumExtensions: 0,
			NumServices:   1,
		},
		GoTypes:           file_internal_api_storagepb_storage_proto_goTypes,
		DependencyIndexes: file_internal_api_storagepb_storage_proto_depIdxs,
		MessageInfos:      file_internal_api_storagepb_storage_proto_msgTypes,
	}.Build()
	File_internal_api_storagepb_storage_proto = out.File
	file_internal_api_storagepb_storage_proto_goTypes = nil
	file_internal_api_storagepb_storage_proto_depIdxs = nil
}

func buildFileInternalAPIStoragepbStorageProtoRawDesc() []byte {
	fd := &descriptorpb.FileDescriptorProto{
		Syntax:     protoString("proto3"),
		Name:       protoString("internal/api/storagepb/storage.proto"),
		Package:    protoString("ownsphere.storage.v1"),
		Dependency: []string{"google/protobuf/empty.proto"},
		Options: &descriptorpb.FileOptions{
			GoPackage: protoString("github.com/ownsphere/core_storage_engine/go/internal/api/storagepb;storagepb"),
		},
		MessageType: []*descriptorpb.DescriptorProto{
			buildMessageDescriptor("StoreFileRequest", stringField("source_path", 1), stringField("file_id", 2)),
			buildMessageDescriptor("RetrieveFileRequest", stringField("file_id", 1), stringField("output_path", 2)),
			buildMessageDescriptor("ListFilesRequest"),
			buildMessageDescriptor("ListFilesResponse", repeatedStringField("file_ids", 1)),
			buildMessageDescriptor("DeleteFileRequest", stringField("file_id", 1)),
			buildMessageDescriptor("GetProgressRequest", stringField("file_id", 1)),
			buildMessageDescriptor("GetProgressResponse", int32Field("percent", 1)),
			buildMessageDescriptor("WaitForBackgroundTasksRequest"),
		},
		Service: []*descriptorpb.ServiceDescriptorProto{{
			Name: protoString("StorageEngineService"),
			Method: []*descriptorpb.MethodDescriptorProto{
				methodDescriptor("StoreFile", ".ownsphere.storage.v1.StoreFileRequest", ".google.protobuf.Empty"),
				methodDescriptor("RetrieveFile", ".ownsphere.storage.v1.RetrieveFileRequest", ".google.protobuf.Empty"),
				methodDescriptor("ListFiles", ".ownsphere.storage.v1.ListFilesRequest", ".ownsphere.storage.v1.ListFilesResponse"),
				methodDescriptor("DeleteFile", ".ownsphere.storage.v1.DeleteFileRequest", ".google.protobuf.Empty"),
				methodDescriptor("DeleteAllFiles", ".google.protobuf.Empty", ".google.protobuf.Empty"),
				methodDescriptor("GetProgress", ".ownsphere.storage.v1.GetProgressRequest", ".ownsphere.storage.v1.GetProgressResponse"),
				methodDescriptor("WaitForBackgroundTasks", ".ownsphere.storage.v1.WaitForBackgroundTasksRequest", ".google.protobuf.Empty"),
			},
		}},
	}

	raw, err := proto.Marshal(fd)
	if err != nil {
		panic(err)
	}
	return raw
}

func buildMessageDescriptor(name string, fields ...*descriptorpb.FieldDescriptorProto) *descriptorpb.DescriptorProto {
	return &descriptorpb.DescriptorProto{Name: protoString(name), Field: fields}
}
func stringField(name string, number int32) *descriptorpb.FieldDescriptorProto {
	return &descriptorpb.FieldDescriptorProto{
		Name:   protoString(name),
		Number: protoInt32(number),
		Label:  descriptorpb.FieldDescriptorProto_LABEL_OPTIONAL.Enum(),
		Type:   descriptorpb.FieldDescriptorProto_TYPE_STRING.Enum(),
	}
}
func repeatedStringField(name string, number int32) *descriptorpb.FieldDescriptorProto {
	return &descriptorpb.FieldDescriptorProto{
		Name:   protoString(name),
		Number: protoInt32(number),
		Label:  descriptorpb.FieldDescriptorProto_LABEL_REPEATED.Enum(),
		Type:   descriptorpb.FieldDescriptorProto_TYPE_STRING.Enum(),
	}
}
func int32Field(name string, number int32) *descriptorpb.FieldDescriptorProto {
	return &descriptorpb.FieldDescriptorProto{
		Name:   protoString(name),
		Number: protoInt32(number),
		Label:  descriptorpb.FieldDescriptorProto_LABEL_OPTIONAL.Enum(),
		Type:   descriptorpb.FieldDescriptorProto_TYPE_INT32.Enum(),
	}
}
func methodDescriptor(name, inputType, outputType string) *descriptorpb.MethodDescriptorProto {
	return &descriptorpb.MethodDescriptorProto{
		Name:       protoString(name),
		InputType:  protoString(inputType),
		OutputType: protoString(outputType),
	}
}
func protoString(value string) *string { return &value }
func protoInt32(value int32) *int32    { return &value }
