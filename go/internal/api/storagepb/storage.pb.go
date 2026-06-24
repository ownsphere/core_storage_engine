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
	state            protoimpl.MessageState `protogen:"open.v1"`
	SourcePath       string                 `protobuf:"bytes,1,opt,name=source_path,json=sourcePath,proto3" json:"source_path,omitempty"`
	FileId           string                 `protobuf:"bytes,2,opt,name=file_id,json=fileId,proto3" json:"file_id,omitempty"`
	OriginalFilename string                 `protobuf:"bytes,3,opt,name=original_filename,json=originalFilename,proto3" json:"original_filename,omitempty"`
	Extension        string                 `protobuf:"bytes,4,opt,name=extension,proto3" json:"extension,omitempty"`
	ContentType      string                 `protobuf:"bytes,5,opt,name=content_type,json=contentType,proto3" json:"content_type,omitempty"`
	Checksum         string                 `protobuf:"bytes,6,opt,name=checksum,proto3" json:"checksum,omitempty"`
	UploadedAtUnixMs int64                  `protobuf:"varint,7,opt,name=uploaded_at_unix_ms,json=uploadedAtUnixMs,proto3" json:"uploaded_at_unix_ms,omitempty"`
	unknownFields    protoimpl.UnknownFields
	sizeCache        protoimpl.SizeCache
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
func (x *StoreFileRequest) GetOriginalFilename() string {
	if x != nil {
		return x.OriginalFilename
	}
	return ""
}
func (x *StoreFileRequest) GetExtension() string {
	if x != nil {
		return x.Extension
	}
	return ""
}
func (x *StoreFileRequest) GetContentType() string {
	if x != nil {
		return x.ContentType
	}
	return ""
}
func (x *StoreFileRequest) GetChecksum() string {
	if x != nil {
		return x.Checksum
	}
	return ""
}
func (x *StoreFileRequest) GetUploadedAtUnixMs() int64 {
	if x != nil {
		return x.UploadedAtUnixMs
	}
	return 0
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
	Files         []*FileMetadata        `protobuf:"bytes,2,rep,name=files,proto3" json:"files,omitempty"`
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
func (x *ListFilesResponse) GetFiles() []*FileMetadata {
	if x != nil {
		return x.Files
	}
	return nil
}

type FileMetadata struct {
	state            protoimpl.MessageState `protogen:"open.v1"`
	FileId           string                 `protobuf:"bytes,1,opt,name=file_id,json=fileId,proto3" json:"file_id,omitempty"`
	StorageKey       string                 `protobuf:"bytes,2,opt,name=storage_key,json=storageKey,proto3" json:"storage_key,omitempty"`
	OriginalFilename string                 `protobuf:"bytes,3,opt,name=original_filename,json=originalFilename,proto3" json:"original_filename,omitempty"`
	Extension        string                 `protobuf:"bytes,4,opt,name=extension,proto3" json:"extension,omitempty"`
	ContentType      string                 `protobuf:"bytes,5,opt,name=content_type,json=contentType,proto3" json:"content_type,omitempty"`
	FileSize         int64                  `protobuf:"varint,6,opt,name=file_size,json=fileSize,proto3" json:"file_size,omitempty"`
	Checksum         string                 `protobuf:"bytes,7,opt,name=checksum,proto3" json:"checksum,omitempty"`
	UploadedAtUnixMs int64                  `protobuf:"varint,8,opt,name=uploaded_at_unix_ms,json=uploadedAtUnixMs,proto3" json:"uploaded_at_unix_ms,omitempty"`
	unknownFields    protoimpl.UnknownFields
	sizeCache        protoimpl.SizeCache
}

func (x *FileMetadata) Reset() {
	*x = FileMetadata{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[4]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *FileMetadata) String() string { return protoimpl.X.MessageStringOf(x) }
func (*FileMetadata) ProtoMessage()    {}
func (x *FileMetadata) ProtoReflect() protoreflect.Message {
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
func (*FileMetadata) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{4}
}
func (x *FileMetadata) GetFileId() string {
	if x != nil {
		return x.FileId
	}
	return ""
}
func (x *FileMetadata) GetStorageKey() string {
	if x != nil {
		return x.StorageKey
	}
	return ""
}
func (x *FileMetadata) GetOriginalFilename() string {
	if x != nil {
		return x.OriginalFilename
	}
	return ""
}
func (x *FileMetadata) GetExtension() string {
	if x != nil {
		return x.Extension
	}
	return ""
}
func (x *FileMetadata) GetContentType() string {
	if x != nil {
		return x.ContentType
	}
	return ""
}
func (x *FileMetadata) GetFileSize() int64 {
	if x != nil {
		return x.FileSize
	}
	return 0
}
func (x *FileMetadata) GetChecksum() string {
	if x != nil {
		return x.Checksum
	}
	return ""
}
func (x *FileMetadata) GetUploadedAtUnixMs() int64 {
	if x != nil {
		return x.UploadedAtUnixMs
	}
	return 0
}

type GetFileMetadataRequest struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	FileId        string                 `protobuf:"bytes,1,opt,name=file_id,json=fileId,proto3" json:"file_id,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *GetFileMetadataRequest) Reset() {
	*x = GetFileMetadataRequest{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[5]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *GetFileMetadataRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*GetFileMetadataRequest) ProtoMessage()    {}
func (x *GetFileMetadataRequest) ProtoReflect() protoreflect.Message {
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
func (*GetFileMetadataRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{5}
}
func (x *GetFileMetadataRequest) GetFileId() string {
	if x != nil {
		return x.FileId
	}
	return ""
}

type GetFileMetadataResponse struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	Metadata      *FileMetadata          `protobuf:"bytes,1,opt,name=metadata,proto3" json:"metadata,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *GetFileMetadataResponse) Reset() {
	*x = GetFileMetadataResponse{}
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[6]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *GetFileMetadataResponse) String() string { return protoimpl.X.MessageStringOf(x) }
func (*GetFileMetadataResponse) ProtoMessage()    {}
func (x *GetFileMetadataResponse) ProtoReflect() protoreflect.Message {
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
func (*GetFileMetadataResponse) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{6}
}
func (x *GetFileMetadataResponse) GetMetadata() *FileMetadata {
	if x != nil {
		return x.Metadata
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
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[7]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *DeleteFileRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*DeleteFileRequest) ProtoMessage()    {}
func (x *DeleteFileRequest) ProtoReflect() protoreflect.Message {
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
func (*DeleteFileRequest) Descriptor() ([]byte, []int) {
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{7}
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
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[8]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *GetProgressRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*GetProgressRequest) ProtoMessage()    {}
func (x *GetProgressRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[8]
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
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{8}
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
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[9]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *GetProgressResponse) String() string { return protoimpl.X.MessageStringOf(x) }
func (*GetProgressResponse) ProtoMessage()    {}
func (x *GetProgressResponse) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[9]
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
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{9}
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
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[10]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *WaitForBackgroundTasksRequest) String() string { return protoimpl.X.MessageStringOf(x) }
func (*WaitForBackgroundTasksRequest) ProtoMessage()    {}
func (x *WaitForBackgroundTasksRequest) ProtoReflect() protoreflect.Message {
	mi := &file_internal_api_storagepb_storage_proto_msgTypes[10]
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
	return file_internal_api_storagepb_storage_proto_rawDescGZIP(), []int{10}
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

var file_internal_api_storagepb_storage_proto_msgTypes = make([]protoimpl.MessageInfo, 11)
var file_internal_api_storagepb_storage_proto_goTypes = []any{
	(*StoreFileRequest)(nil),
	(*RetrieveFileRequest)(nil),
	(*ListFilesRequest)(nil),
	(*ListFilesResponse)(nil),
	(*FileMetadata)(nil),
	(*GetFileMetadataRequest)(nil),
	(*GetFileMetadataResponse)(nil),
	(*DeleteFileRequest)(nil),
	(*GetProgressRequest)(nil),
	(*GetProgressResponse)(nil),
	(*WaitForBackgroundTasksRequest)(nil),
	(*emptypb.Empty)(nil),
}
var file_internal_api_storagepb_storage_proto_depIdxs = []int32{
	4,  // 0: ownsphere.storage.v1.ListFilesResponse.files:type_name -> ownsphere.storage.v1.FileMetadata
	4,  // 1: ownsphere.storage.v1.GetFileMetadataResponse.metadata:type_name -> ownsphere.storage.v1.FileMetadata
	0,  // 2: ownsphere.storage.v1.StorageEngineService.StoreFile:input_type -> ownsphere.storage.v1.StoreFileRequest
	1,  // 3: ownsphere.storage.v1.StorageEngineService.RetrieveFile:input_type -> ownsphere.storage.v1.RetrieveFileRequest
	2,  // 4: ownsphere.storage.v1.StorageEngineService.ListFiles:input_type -> ownsphere.storage.v1.ListFilesRequest
	5,  // 5: ownsphere.storage.v1.StorageEngineService.GetFileMetadata:input_type -> ownsphere.storage.v1.GetFileMetadataRequest
	7,  // 6: ownsphere.storage.v1.StorageEngineService.DeleteFile:input_type -> ownsphere.storage.v1.DeleteFileRequest
	11, // 7: ownsphere.storage.v1.StorageEngineService.DeleteAllFiles:input_type -> google.protobuf.Empty
	8,  // 8: ownsphere.storage.v1.StorageEngineService.GetProgress:input_type -> ownsphere.storage.v1.GetProgressRequest
	10, // 9: ownsphere.storage.v1.StorageEngineService.WaitForBackgroundTasks:input_type -> ownsphere.storage.v1.WaitForBackgroundTasksRequest
	11, // 10: ownsphere.storage.v1.StorageEngineService.StoreFile:output_type -> google.protobuf.Empty
	11, // 11: ownsphere.storage.v1.StorageEngineService.RetrieveFile:output_type -> google.protobuf.Empty
	3,  // 12: ownsphere.storage.v1.StorageEngineService.ListFiles:output_type -> ownsphere.storage.v1.ListFilesResponse
	6,  // 13: ownsphere.storage.v1.StorageEngineService.GetFileMetadata:output_type -> ownsphere.storage.v1.GetFileMetadataResponse
	11, // 14: ownsphere.storage.v1.StorageEngineService.DeleteFile:output_type -> google.protobuf.Empty
	11, // 15: ownsphere.storage.v1.StorageEngineService.DeleteAllFiles:output_type -> google.protobuf.Empty
	9,  // 16: ownsphere.storage.v1.StorageEngineService.GetProgress:output_type -> ownsphere.storage.v1.GetProgressResponse
	11, // 17: ownsphere.storage.v1.StorageEngineService.WaitForBackgroundTasks:output_type -> google.protobuf.Empty
	10, // [10:18] is the sub-list for method output_type
	2,  // [2:10] is the sub-list for method input_type
	0,  // [0:0] is the sub-list for extension type_name
	0,  // [0:0] is the sub-list for extension extendee
	0,  // [0:2] is the sub-list for field type_name
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
			switch v := v.(*FileMetadata); i {
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
			switch v := v.(*GetFileMetadataRequest); i {
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
			switch v := v.(*GetFileMetadataResponse); i {
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
		file_internal_api_storagepb_storage_proto_msgTypes[8].Exporter = func(v any, i int) any {
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
		file_internal_api_storagepb_storage_proto_msgTypes[9].Exporter = func(v any, i int) any {
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
		file_internal_api_storagepb_storage_proto_msgTypes[10].Exporter = func(v any, i int) any {
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
			NumMessages:   11,
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
			buildMessageDescriptor(
				"StoreFileRequest",
				stringField("source_path", 1),
				stringField("file_id", 2),
				stringField("original_filename", 3),
				stringField("extension", 4),
				stringField("content_type", 5),
				stringField("checksum", 6),
				int64Field("uploaded_at_unix_ms", 7),
			),
			buildMessageDescriptor("RetrieveFileRequest", stringField("file_id", 1), stringField("output_path", 2)),
			buildMessageDescriptor("ListFilesRequest"),
			buildMessageDescriptor("ListFilesResponse", repeatedStringField("file_ids", 1), repeatedMessageField("files", 2, ".ownsphere.storage.v1.FileMetadata")),
			buildMessageDescriptor(
				"FileMetadata",
				stringField("file_id", 1),
				stringField("storage_key", 2),
				stringField("original_filename", 3),
				stringField("extension", 4),
				stringField("content_type", 5),
				int64Field("file_size", 6),
				stringField("checksum", 7),
				int64Field("uploaded_at_unix_ms", 8),
			),
			buildMessageDescriptor("GetFileMetadataRequest", stringField("file_id", 1)),
			buildMessageDescriptor("GetFileMetadataResponse", messageField("metadata", 1, ".ownsphere.storage.v1.FileMetadata")),
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
				methodDescriptor("GetFileMetadata", ".ownsphere.storage.v1.GetFileMetadataRequest", ".ownsphere.storage.v1.GetFileMetadataResponse"),
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
func messageField(name string, number int32, typeName string) *descriptorpb.FieldDescriptorProto {
	return &descriptorpb.FieldDescriptorProto{
		Name:     protoString(name),
		Number:   protoInt32(number),
		Label:    descriptorpb.FieldDescriptorProto_LABEL_OPTIONAL.Enum(),
		Type:     descriptorpb.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
		TypeName: protoString(typeName),
	}
}
func repeatedMessageField(name string, number int32, typeName string) *descriptorpb.FieldDescriptorProto {
	return &descriptorpb.FieldDescriptorProto{
		Name:     protoString(name),
		Number:   protoInt32(number),
		Label:    descriptorpb.FieldDescriptorProto_LABEL_REPEATED.Enum(),
		Type:     descriptorpb.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
		TypeName: protoString(typeName),
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
func int64Field(name string, number int32) *descriptorpb.FieldDescriptorProto {
	return &descriptorpb.FieldDescriptorProto{
		Name:   protoString(name),
		Number: protoInt32(number),
		Label:  descriptorpb.FieldDescriptorProto_LABEL_OPTIONAL.Enum(),
		Type:   descriptorpb.FieldDescriptorProto_TYPE_INT64.Enum(),
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
