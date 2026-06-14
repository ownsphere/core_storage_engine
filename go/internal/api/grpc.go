package api

import (
	"context"
	"errors"

	"github.com/ownsphere/core_storage_engine/go/internal/api/storagepb"
	servicepkg "github.com/ownsphere/core_storage_engine/go/internal/service"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	emptypb "google.golang.org/protobuf/types/known/emptypb"
)

var ErrNilStorageService = errors.New("api: storage service is required")

type storageWorkflowService interface {
	StoreFile(ctx context.Context, sourcePath, fileID string) error
	RetrieveFile(ctx context.Context, fileID, outputPath string) error
	ListFiles(ctx context.Context) ([]string, error)
	DeleteFile(ctx context.Context, fileID string) error
	DeleteAllFiles(ctx context.Context) error
	Progress(ctx context.Context, fileID string) (int, error)
	WaitForBackgroundTasks(ctx context.Context) error
}

// GRPCServer exposes the storage workflow service over gRPC.
type GRPCServer struct {
	storagepb.UnimplementedStorageEngineServiceServer
	service storageWorkflowService
}

// NewGRPCServer creates a gRPC handler backed by the storage service layer.
func NewGRPCServer(service storageWorkflowService) (*GRPCServer, error) {
	if service == nil {
		return nil, ErrNilStorageService
	}
	return &GRPCServer{service: service}, nil
}

func (s *GRPCServer) StoreFile(ctx context.Context, req *storagepb.StoreFileRequest) (*emptypb.Empty, error) {
	if err := s.service.StoreFile(ctx, req.GetSourcePath(), req.GetFileId()); err != nil {
		return nil, mapError(err)
	}
	return &emptypb.Empty{}, nil
}

func (s *GRPCServer) RetrieveFile(ctx context.Context, req *storagepb.RetrieveFileRequest) (*emptypb.Empty, error) {
	if err := s.service.RetrieveFile(ctx, req.GetFileId(), req.GetOutputPath()); err != nil {
		return nil, mapError(err)
	}
	return &emptypb.Empty{}, nil
}

func (s *GRPCServer) ListFiles(ctx context.Context, _ *storagepb.ListFilesRequest) (*storagepb.ListFilesResponse, error) {
	files, err := s.service.ListFiles(ctx)
	if err != nil {
		return nil, mapError(err)
	}
	return &storagepb.ListFilesResponse{FileIds: files}, nil
}

func (s *GRPCServer) DeleteFile(ctx context.Context, req *storagepb.DeleteFileRequest) (*emptypb.Empty, error) {
	if err := s.service.DeleteFile(ctx, req.GetFileId()); err != nil {
		return nil, mapError(err)
	}
	return &emptypb.Empty{}, nil
}

func (s *GRPCServer) DeleteAllFiles(ctx context.Context, _ *emptypb.Empty) (*emptypb.Empty, error) {
	if err := s.service.DeleteAllFiles(ctx); err != nil {
		return nil, mapError(err)
	}
	return &emptypb.Empty{}, nil
}

func (s *GRPCServer) GetProgress(ctx context.Context, req *storagepb.GetProgressRequest) (*storagepb.GetProgressResponse, error) {
	progress, err := s.service.Progress(ctx, req.GetFileId())
	if err != nil {
		return nil, mapError(err)
	}
	return &storagepb.GetProgressResponse{Percent: int32(progress)}, nil
}

func (s *GRPCServer) WaitForBackgroundTasks(ctx context.Context, _ *storagepb.WaitForBackgroundTasksRequest) (*emptypb.Empty, error) {
	if err := s.service.WaitForBackgroundTasks(ctx); err != nil {
		return nil, mapError(err)
	}
	return &emptypb.Empty{}, nil
}

func mapError(err error) error {
	switch {
	case err == nil:
		return nil
	case errors.Is(err, context.Canceled):
		return status.Error(codes.Canceled, err.Error())
	case errors.Is(err, context.DeadlineExceeded):
		return status.Error(codes.DeadlineExceeded, err.Error())
	case errors.Is(err, servicepkg.ErrEmptyFileID),
		errors.Is(err, servicepkg.ErrEmptySourcePath),
		errors.Is(err, servicepkg.ErrEmptyOutputPath):
		return status.Error(codes.InvalidArgument, err.Error())
	case errors.Is(err, servicepkg.ErrServiceClosed):
		return status.Error(codes.FailedPrecondition, err.Error())
	default:
		return status.Error(codes.Internal, err.Error())
	}
}
