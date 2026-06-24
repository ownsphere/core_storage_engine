package api

import (
	"context"
	"errors"
	"testing"
	"time"

	"github.com/ownsphere/core_storage_engine/go/internal/api/storagepb"
	servicepkg "github.com/ownsphere/core_storage_engine/go/internal/service"
	clientpkg "github.com/ownsphere/core_storage_engine/go/pkg/client"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	emptypb "google.golang.org/protobuf/types/known/emptypb"
)

type stubWorkflowService struct {
	storeSourcePath string
	storeFileID     string
	storeOptions    clientpkg.StoreFileOptions
	progressFileID  string
	metadataFileID  string

	listFilesResult    []string
	listMetadataResult []clientpkg.FileMetadata
	metadataResult     clientpkg.FileMetadata
	progressResult     int

	storeErr        error
	retrieveErr     error
	listFilesErr    error
	listMetadataErr error
	metadataErr     error
	deleteFileErr   error
	deleteAllErr    error
	progressErr     error
	waitErr         error
}

func (s *stubWorkflowService) StoreFile(_ context.Context, sourcePath, fileID string) error {
	s.storeSourcePath = sourcePath
	s.storeFileID = fileID
	return s.storeErr
}
func (s *stubWorkflowService) StoreFileWithMetadata(_ context.Context, sourcePath, fileID string, options clientpkg.StoreFileOptions) (clientpkg.FileMetadata, error) {
	s.storeSourcePath = sourcePath
	s.storeFileID = fileID
	s.storeOptions = options
	return s.metadataResult, s.storeErr
}
func (s *stubWorkflowService) RetrieveFile(context.Context, string, string) error {
	return s.retrieveErr
}
func (s *stubWorkflowService) ListFiles(context.Context) ([]string, error) {
	return s.listFilesResult, s.listFilesErr
}
func (s *stubWorkflowService) ListFileMetadata(context.Context) ([]clientpkg.FileMetadata, error) {
	return s.listMetadataResult, s.listMetadataErr
}
func (s *stubWorkflowService) GetFileMetadata(_ context.Context, fileID string) (clientpkg.FileMetadata, error) {
	s.metadataFileID = fileID
	return s.metadataResult, s.metadataErr
}
func (s *stubWorkflowService) DeleteFile(context.Context, string) error { return s.deleteFileErr }
func (s *stubWorkflowService) DeleteAllFiles(context.Context) error     { return s.deleteAllErr }
func (s *stubWorkflowService) Progress(_ context.Context, fileID string) (int, error) {
	s.progressFileID = fileID
	return s.progressResult, s.progressErr
}
func (s *stubWorkflowService) WaitForBackgroundTasks(context.Context) error { return s.waitErr }

func TestNewGRPCServerRequiresService(t *testing.T) {
	t.Parallel()

	server, err := NewGRPCServer(nil)
	if !errors.Is(err, ErrNilStorageService) {
		t.Fatalf("expected ErrNilStorageService, got %v", err)
	}
	if server != nil {
		t.Fatalf("expected nil server, got %#v", server)
	}
}

func TestStoreFileDelegatesToService(t *testing.T) {
	t.Parallel()

	stub := &stubWorkflowService{}
	server, err := NewGRPCServer(stub)
	if err != nil {
		t.Fatalf("NewGRPCServer() error = %v", err)
	}

	_, err = server.StoreFile(context.Background(), &storagepb.StoreFileRequest{
		SourcePath:       "/tmp/input",
		FileId:           "file-1",
		OriginalFilename: "report.png",
		ContentType:      "image/png",
		UploadedAtUnixMs: 1710000000000,
	})
	if err != nil {
		t.Fatalf("StoreFile() error = %v", err)
	}
	if stub.storeSourcePath != "/tmp/input" || stub.storeFileID != "file-1" {
		t.Fatalf("unexpected store args: %q %q", stub.storeSourcePath, stub.storeFileID)
	}
	if stub.storeOptions.OriginalFilename != "report.png" || stub.storeOptions.ContentType != "image/png" {
		t.Fatalf("unexpected store options: %#v", stub.storeOptions)
	}
	if !stub.storeOptions.UploadedAt.Equal(time.UnixMilli(1710000000000).UTC()) {
		t.Fatalf("unexpected uploaded at: %v", stub.storeOptions.UploadedAt)
	}
}

func TestListFilesAndGetProgressResponses(t *testing.T) {
	t.Parallel()

	stub := &stubWorkflowService{
		listFilesResult: []string{"file-1", "file-2"},
		listMetadataResult: []clientpkg.FileMetadata{
			{FileID: "file-1", OriginalFilename: "one.txt"},
			{FileID: "file-2", OriginalFilename: "two.txt"},
		},
		metadataResult: clientpkg.FileMetadata{
			FileID:           "file-1",
			StorageKey:       "file-1",
			OriginalFilename: "one.txt",
			ContentType:      "text/plain",
			FileSize:         42,
			Checksum:         "abc123",
			UploadedAt:       time.UnixMilli(1710000000000).UTC(),
		},
		progressResult: 88,
	}
	server, err := NewGRPCServer(stub)
	if err != nil {
		t.Fatalf("NewGRPCServer() error = %v", err)
	}

	listResp, err := server.ListFiles(context.Background(), &storagepb.ListFilesRequest{})
	if err != nil {
		t.Fatalf("ListFiles() error = %v", err)
	}
	if len(listResp.FileIds) != 2 || listResp.FileIds[0] != "file-1" || listResp.FileIds[1] != "file-2" {
		t.Fatalf("unexpected list response: %#v", listResp)
	}
	if len(listResp.Files) != 2 || listResp.Files[0].GetOriginalFilename() != "one.txt" {
		t.Fatalf("unexpected list metadata response: %#v", listResp.Files)
	}

	metadataResp, err := server.GetFileMetadata(context.Background(), &storagepb.GetFileMetadataRequest{FileId: "file-1"})
	if err != nil {
		t.Fatalf("GetFileMetadata() error = %v", err)
	}
	if metadataResp.GetMetadata().GetChecksum() != "abc123" {
		t.Fatalf("unexpected metadata response: %#v", metadataResp)
	}
	if stub.metadataFileID != "file-1" {
		t.Fatalf("unexpected metadata file id: %q", stub.metadataFileID)
	}

	progressResp, err := server.GetProgress(context.Background(), &storagepb.GetProgressRequest{FileId: "file-1"})
	if err != nil {
		t.Fatalf("GetProgress() error = %v", err)
	}
	if progressResp.Percent != 88 {
		t.Fatalf("expected percent 88, got %d", progressResp.Percent)
	}
	if stub.progressFileID != "file-1" {
		t.Fatalf("unexpected progress file id: %q", stub.progressFileID)
	}
}

func TestUnaryHandlersMapServiceErrors(t *testing.T) {
	t.Parallel()

	testCases := []struct {
		name string
		err  error
		code codes.Code
		call func(server *GRPCServer) error
	}{
		{
			name: "invalid argument",
			err:  servicepkg.ErrEmptyFileID,
			code: codes.InvalidArgument,
			call: func(server *GRPCServer) error {
				_, err := server.DeleteFile(context.Background(), &storagepb.DeleteFileRequest{})
				return err
			},
		},
		{
			name: "failed precondition",
			err:  servicepkg.ErrServiceClosed,
			code: codes.FailedPrecondition,
			call: func(server *GRPCServer) error {
				_, err := server.WaitForBackgroundTasks(context.Background(), &storagepb.WaitForBackgroundTasksRequest{})
				return err
			},
		},
		{
			name: "canceled",
			err:  context.Canceled,
			code: codes.Canceled,
			call: func(server *GRPCServer) error {
				_, err := server.DeleteAllFiles(context.Background(), &emptypb.Empty{})
				return err
			},
		},
		{
			name: "internal",
			err:  errors.New("boom"),
			code: codes.Internal,
			call: func(server *GRPCServer) error {
				_, err := server.RetrieveFile(context.Background(), &storagepb.RetrieveFileRequest{FileId: "file-1", OutputPath: "/tmp/out"})
				return err
			},
		},
	}

	for _, tc := range testCases {
		tc := tc
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()

			stub := &stubWorkflowService{
				retrieveErr:   tc.err,
				deleteFileErr: tc.err,
				deleteAllErr:  tc.err,
				waitErr:       tc.err,
			}
			server, err := NewGRPCServer(stub)
			if err != nil {
				t.Fatalf("NewGRPCServer() error = %v", err)
			}

			gotErr := tc.call(server)
			if status.Code(gotErr) != tc.code {
				t.Fatalf("expected code %s, got %s (%v)", tc.code, status.Code(gotErr), gotErr)
			}
		})
	}
}
