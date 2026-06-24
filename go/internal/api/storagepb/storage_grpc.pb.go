// Code generated manually to match storage.proto. DO NOT EDIT.

package storagepb

import (
	context "context"

	grpc "google.golang.org/grpc"
	codes "google.golang.org/grpc/codes"
	status "google.golang.org/grpc/status"
	emptypb "google.golang.org/protobuf/types/known/emptypb"
)

const _ = grpc.SupportPackageIsVersion9

const (
	StorageEngineService_StoreFile_FullMethodName              = "/ownsphere.storage.v1.StorageEngineService/StoreFile"
	StorageEngineService_RetrieveFile_FullMethodName           = "/ownsphere.storage.v1.StorageEngineService/RetrieveFile"
	StorageEngineService_ListFiles_FullMethodName              = "/ownsphere.storage.v1.StorageEngineService/ListFiles"
	StorageEngineService_GetFileMetadata_FullMethodName        = "/ownsphere.storage.v1.StorageEngineService/GetFileMetadata"
	StorageEngineService_DeleteFile_FullMethodName             = "/ownsphere.storage.v1.StorageEngineService/DeleteFile"
	StorageEngineService_DeleteAllFiles_FullMethodName         = "/ownsphere.storage.v1.StorageEngineService/DeleteAllFiles"
	StorageEngineService_GetProgress_FullMethodName            = "/ownsphere.storage.v1.StorageEngineService/GetProgress"
	StorageEngineService_WaitForBackgroundTasks_FullMethodName = "/ownsphere.storage.v1.StorageEngineService/WaitForBackgroundTasks"
)

type StorageEngineServiceClient interface {
	StoreFile(ctx context.Context, in *StoreFileRequest, opts ...grpc.CallOption) (*emptypb.Empty, error)
	RetrieveFile(ctx context.Context, in *RetrieveFileRequest, opts ...grpc.CallOption) (*emptypb.Empty, error)
	ListFiles(ctx context.Context, in *ListFilesRequest, opts ...grpc.CallOption) (*ListFilesResponse, error)
	GetFileMetadata(ctx context.Context, in *GetFileMetadataRequest, opts ...grpc.CallOption) (*GetFileMetadataResponse, error)
	DeleteFile(ctx context.Context, in *DeleteFileRequest, opts ...grpc.CallOption) (*emptypb.Empty, error)
	DeleteAllFiles(ctx context.Context, in *emptypb.Empty, opts ...grpc.CallOption) (*emptypb.Empty, error)
	GetProgress(ctx context.Context, in *GetProgressRequest, opts ...grpc.CallOption) (*GetProgressResponse, error)
	WaitForBackgroundTasks(ctx context.Context, in *WaitForBackgroundTasksRequest, opts ...grpc.CallOption) (*emptypb.Empty, error)
}

type storageEngineServiceClient struct {
	cc grpc.ClientConnInterface
}

func NewStorageEngineServiceClient(cc grpc.ClientConnInterface) StorageEngineServiceClient {
	return &storageEngineServiceClient{cc: cc}
}

func (c *storageEngineServiceClient) StoreFile(ctx context.Context, in *StoreFileRequest, opts ...grpc.CallOption) (*emptypb.Empty, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(emptypb.Empty)
	err := c.cc.Invoke(ctx, StorageEngineService_StoreFile_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *storageEngineServiceClient) RetrieveFile(ctx context.Context, in *RetrieveFileRequest, opts ...grpc.CallOption) (*emptypb.Empty, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(emptypb.Empty)
	err := c.cc.Invoke(ctx, StorageEngineService_RetrieveFile_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *storageEngineServiceClient) ListFiles(ctx context.Context, in *ListFilesRequest, opts ...grpc.CallOption) (*ListFilesResponse, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(ListFilesResponse)
	err := c.cc.Invoke(ctx, StorageEngineService_ListFiles_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *storageEngineServiceClient) GetFileMetadata(ctx context.Context, in *GetFileMetadataRequest, opts ...grpc.CallOption) (*GetFileMetadataResponse, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(GetFileMetadataResponse)
	err := c.cc.Invoke(ctx, StorageEngineService_GetFileMetadata_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *storageEngineServiceClient) DeleteFile(ctx context.Context, in *DeleteFileRequest, opts ...grpc.CallOption) (*emptypb.Empty, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(emptypb.Empty)
	err := c.cc.Invoke(ctx, StorageEngineService_DeleteFile_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *storageEngineServiceClient) DeleteAllFiles(ctx context.Context, in *emptypb.Empty, opts ...grpc.CallOption) (*emptypb.Empty, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(emptypb.Empty)
	err := c.cc.Invoke(ctx, StorageEngineService_DeleteAllFiles_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *storageEngineServiceClient) GetProgress(ctx context.Context, in *GetProgressRequest, opts ...grpc.CallOption) (*GetProgressResponse, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(GetProgressResponse)
	err := c.cc.Invoke(ctx, StorageEngineService_GetProgress_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *storageEngineServiceClient) WaitForBackgroundTasks(ctx context.Context, in *WaitForBackgroundTasksRequest, opts ...grpc.CallOption) (*emptypb.Empty, error) {
	cOpts := append([]grpc.CallOption{grpc.StaticMethod()}, opts...)
	out := new(emptypb.Empty)
	err := c.cc.Invoke(ctx, StorageEngineService_WaitForBackgroundTasks_FullMethodName, in, out, cOpts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

type StorageEngineServiceServer interface {
	StoreFile(context.Context, *StoreFileRequest) (*emptypb.Empty, error)
	RetrieveFile(context.Context, *RetrieveFileRequest) (*emptypb.Empty, error)
	ListFiles(context.Context, *ListFilesRequest) (*ListFilesResponse, error)
	GetFileMetadata(context.Context, *GetFileMetadataRequest) (*GetFileMetadataResponse, error)
	DeleteFile(context.Context, *DeleteFileRequest) (*emptypb.Empty, error)
	DeleteAllFiles(context.Context, *emptypb.Empty) (*emptypb.Empty, error)
	GetProgress(context.Context, *GetProgressRequest) (*GetProgressResponse, error)
	WaitForBackgroundTasks(context.Context, *WaitForBackgroundTasksRequest) (*emptypb.Empty, error)
	mustEmbedUnimplementedStorageEngineServiceServer()
}

type UnimplementedStorageEngineServiceServer struct{}

func (UnimplementedStorageEngineServiceServer) StoreFile(context.Context, *StoreFileRequest) (*emptypb.Empty, error) {
	return nil, status.Error(codes.Unimplemented, "method StoreFile not implemented")
}
func (UnimplementedStorageEngineServiceServer) RetrieveFile(context.Context, *RetrieveFileRequest) (*emptypb.Empty, error) {
	return nil, status.Error(codes.Unimplemented, "method RetrieveFile not implemented")
}
func (UnimplementedStorageEngineServiceServer) ListFiles(context.Context, *ListFilesRequest) (*ListFilesResponse, error) {
	return nil, status.Error(codes.Unimplemented, "method ListFiles not implemented")
}
func (UnimplementedStorageEngineServiceServer) GetFileMetadata(context.Context, *GetFileMetadataRequest) (*GetFileMetadataResponse, error) {
	return nil, status.Error(codes.Unimplemented, "method GetFileMetadata not implemented")
}
func (UnimplementedStorageEngineServiceServer) DeleteFile(context.Context, *DeleteFileRequest) (*emptypb.Empty, error) {
	return nil, status.Error(codes.Unimplemented, "method DeleteFile not implemented")
}
func (UnimplementedStorageEngineServiceServer) DeleteAllFiles(context.Context, *emptypb.Empty) (*emptypb.Empty, error) {
	return nil, status.Error(codes.Unimplemented, "method DeleteAllFiles not implemented")
}
func (UnimplementedStorageEngineServiceServer) GetProgress(context.Context, *GetProgressRequest) (*GetProgressResponse, error) {
	return nil, status.Error(codes.Unimplemented, "method GetProgress not implemented")
}
func (UnimplementedStorageEngineServiceServer) WaitForBackgroundTasks(context.Context, *WaitForBackgroundTasksRequest) (*emptypb.Empty, error) {
	return nil, status.Error(codes.Unimplemented, "method WaitForBackgroundTasks not implemented")
}
func (UnimplementedStorageEngineServiceServer) mustEmbedUnimplementedStorageEngineServiceServer() {}
func (UnimplementedStorageEngineServiceServer) testEmbeddedByValue()                              {}

type UnsafeStorageEngineServiceServer interface {
	mustEmbedUnimplementedStorageEngineServiceServer()
}

func RegisterStorageEngineServiceServer(s grpc.ServiceRegistrar, srv StorageEngineServiceServer) {
	if t, ok := srv.(interface{ testEmbeddedByValue() }); ok {
		t.testEmbeddedByValue()
	}
	s.RegisterService(&StorageEngineService_ServiceDesc, srv)
}

func _StorageEngineService_StoreFile_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(StoreFileRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).StoreFile(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_StoreFile_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).StoreFile(ctx, req.(*StoreFileRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _StorageEngineService_RetrieveFile_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(RetrieveFileRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).RetrieveFile(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_RetrieveFile_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).RetrieveFile(ctx, req.(*RetrieveFileRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _StorageEngineService_ListFiles_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(ListFilesRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).ListFiles(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_ListFiles_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).ListFiles(ctx, req.(*ListFilesRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _StorageEngineService_GetFileMetadata_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(GetFileMetadataRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).GetFileMetadata(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_GetFileMetadata_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).GetFileMetadata(ctx, req.(*GetFileMetadataRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _StorageEngineService_DeleteFile_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(DeleteFileRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).DeleteFile(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_DeleteFile_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).DeleteFile(ctx, req.(*DeleteFileRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _StorageEngineService_DeleteAllFiles_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(emptypb.Empty)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).DeleteAllFiles(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_DeleteAllFiles_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).DeleteAllFiles(ctx, req.(*emptypb.Empty))
	}
	return interceptor(ctx, in, info, handler)
}

func _StorageEngineService_GetProgress_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(GetProgressRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).GetProgress(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_GetProgress_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).GetProgress(ctx, req.(*GetProgressRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _StorageEngineService_WaitForBackgroundTasks_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(WaitForBackgroundTasksRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(StorageEngineServiceServer).WaitForBackgroundTasks(ctx, in)
	}
	info := &grpc.UnaryServerInfo{Server: srv, FullMethod: StorageEngineService_WaitForBackgroundTasks_FullMethodName}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(StorageEngineServiceServer).WaitForBackgroundTasks(ctx, req.(*WaitForBackgroundTasksRequest))
	}
	return interceptor(ctx, in, info, handler)
}

var StorageEngineService_ServiceDesc = grpc.ServiceDesc{
	ServiceName: "ownsphere.storage.v1.StorageEngineService",
	HandlerType: (*StorageEngineServiceServer)(nil),
	Methods: []grpc.MethodDesc{
		{MethodName: "StoreFile", Handler: _StorageEngineService_StoreFile_Handler},
		{MethodName: "RetrieveFile", Handler: _StorageEngineService_RetrieveFile_Handler},
		{MethodName: "ListFiles", Handler: _StorageEngineService_ListFiles_Handler},
		{MethodName: "GetFileMetadata", Handler: _StorageEngineService_GetFileMetadata_Handler},
		{MethodName: "DeleteFile", Handler: _StorageEngineService_DeleteFile_Handler},
		{MethodName: "DeleteAllFiles", Handler: _StorageEngineService_DeleteAllFiles_Handler},
		{MethodName: "GetProgress", Handler: _StorageEngineService_GetProgress_Handler},
		{MethodName: "WaitForBackgroundTasks", Handler: _StorageEngineService_WaitForBackgroundTasks_Handler},
	},
	Streams:  []grpc.StreamDesc{},
	Metadata: "internal/api/storagepb/storage.proto",
}
