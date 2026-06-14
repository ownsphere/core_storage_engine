package service

import (
	"context"
	"errors"
	"sync"
	"testing"
)

type stubStorage struct {
	mu             sync.Mutex
	closeCalls     int
	storeCalls     []storeCall
	retrieveCalls  []retrieveCall
	listCalls      int
	deleteCalls    []string
	deleteAllCalls int
	progressCalls  []string
	waitCalls      int
	listResult     []string
	progressResult int
	storeErr       error
	retrieveErr    error
	listErr        error
	deleteErr      error
	deleteAllErr   error
	progressErr    error
	waitErr        error
	closeErr       error
}

type storeCall struct {
	sourcePath string
	fileID     string
}

type retrieveCall struct {
	fileID     string
	outputPath string
}

func (s *stubStorage) Close() error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.closeCalls++
	return s.closeErr
}

func (s *stubStorage) StoreFile(_ context.Context, sourcePath, fileID string) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.storeCalls = append(s.storeCalls, storeCall{sourcePath: sourcePath, fileID: fileID})
	return s.storeErr
}

func (s *stubStorage) RetrieveFile(_ context.Context, fileID, outputPath string) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.retrieveCalls = append(s.retrieveCalls, retrieveCall{fileID: fileID, outputPath: outputPath})
	return s.retrieveErr
}

func (s *stubStorage) ListFiles(context.Context) ([]string, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.listCalls++
	if s.listErr != nil {
		return nil, s.listErr
	}
	out := make([]string, len(s.listResult))
	copy(out, s.listResult)
	return out, nil
}

func (s *stubStorage) DeleteFile(_ context.Context, fileID string) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.deleteCalls = append(s.deleteCalls, fileID)
	return s.deleteErr
}

func (s *stubStorage) DeleteAllFiles(context.Context) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.deleteAllCalls++
	return s.deleteAllErr
}

func (s *stubStorage) Progress(_ context.Context, fileID string) (int, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.progressCalls = append(s.progressCalls, fileID)
	return s.progressResult, s.progressErr
}

func (s *stubStorage) WaitForBackgroundTasks(context.Context) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.waitCalls++
	return s.waitErr
}

func TestNewRequiresStorage(t *testing.T) {
	t.Parallel()

	service, err := New(nil)
	if !errors.Is(err, ErrNilStorage) {
		t.Fatalf("expected ErrNilStorage, got %v", err)
	}
	if service != nil {
		t.Fatalf("expected nil service, got %#v", service)
	}
}

func TestStoreFileValidatesInput(t *testing.T) {
	t.Parallel()

	storage := &stubStorage{}
	service, err := New(storage)
	if err != nil {
		t.Fatalf("New() error = %v", err)
	}

	if err := service.StoreFile(context.Background(), " ", "file-1"); !errors.Is(err, ErrEmptySourcePath) {
		t.Fatalf("expected ErrEmptySourcePath, got %v", err)
	}
	if err := service.StoreFile(context.Background(), "/tmp/input", " "); !errors.Is(err, ErrEmptyFileID) {
		t.Fatalf("expected ErrEmptyFileID, got %v", err)
	}

	if got := len(storage.storeCalls); got != 0 {
		t.Fatalf("expected no storage calls, got %d", got)
	}
}

func TestStoreFileDelegatesToClient(t *testing.T) {
	t.Parallel()

	storage := &stubStorage{}
	service, err := New(storage)
	if err != nil {
		t.Fatalf("New() error = %v", err)
	}

	if err := service.StoreFile(context.Background(), "/tmp/input", "file-1"); err != nil {
		t.Fatalf("StoreFile() error = %v", err)
	}

	if got := len(storage.storeCalls); got != 1 {
		t.Fatalf("expected one store call, got %d", got)
	}
	if storage.storeCalls[0].sourcePath != "/tmp/input" || storage.storeCalls[0].fileID != "file-1" {
		t.Fatalf("unexpected store call: %#v", storage.storeCalls[0])
	}
}

func TestRetrieveListDeleteProgressAndWaitDelegate(t *testing.T) {
	t.Parallel()

	storage := &stubStorage{
		listResult:     []string{"file-1", "file-2"},
		progressResult: 73,
	}
	service, err := New(storage)
	if err != nil {
		t.Fatalf("New() error = %v", err)
	}

	if err := service.RetrieveFile(context.Background(), "file-1", "/tmp/output"); err != nil {
		t.Fatalf("RetrieveFile() error = %v", err)
	}

	files, err := service.ListFiles(context.Background())
	if err != nil {
		t.Fatalf("ListFiles() error = %v", err)
	}
	if len(files) != 2 || files[0] != "file-1" || files[1] != "file-2" {
		t.Fatalf("unexpected files: %#v", files)
	}

	if err := service.DeleteFile(context.Background(), "file-2"); err != nil {
		t.Fatalf("DeleteFile() error = %v", err)
	}
	if err := service.DeleteAllFiles(context.Background()); err != nil {
		t.Fatalf("DeleteAllFiles() error = %v", err)
	}

	progress, err := service.Progress(context.Background(), "file-1")
	if err != nil {
		t.Fatalf("Progress() error = %v", err)
	}
	if progress != 73 {
		t.Fatalf("expected progress 73, got %d", progress)
	}

	if err := service.WaitForBackgroundTasks(context.Background()); err != nil {
		t.Fatalf("WaitForBackgroundTasks() error = %v", err)
	}

	if len(storage.retrieveCalls) != 1 {
		t.Fatalf("expected one retrieve call, got %d", len(storage.retrieveCalls))
	}
	if storage.listCalls != 1 {
		t.Fatalf("expected one list call, got %d", storage.listCalls)
	}
	if len(storage.deleteCalls) != 1 || storage.deleteCalls[0] != "file-2" {
		t.Fatalf("unexpected delete calls: %#v", storage.deleteCalls)
	}
	if storage.deleteAllCalls != 1 {
		t.Fatalf("expected one delete-all call, got %d", storage.deleteAllCalls)
	}
	if len(storage.progressCalls) != 1 || storage.progressCalls[0] != "file-1" {
		t.Fatalf("unexpected progress calls: %#v", storage.progressCalls)
	}
	if storage.waitCalls != 1 {
		t.Fatalf("expected one wait call, got %d", storage.waitCalls)
	}
}

func TestClosePreventsFutureOperations(t *testing.T) {
	t.Parallel()

	storage := &stubStorage{}
	service, err := New(storage)
	if err != nil {
		t.Fatalf("New() error = %v", err)
	}

	if err := service.Close(); err != nil {
		t.Fatalf("Close() error = %v", err)
	}
	if err := service.Close(); err != nil {
		t.Fatalf("second Close() error = %v", err)
	}
	if storage.closeCalls != 1 {
		t.Fatalf("expected one close call, got %d", storage.closeCalls)
	}

	if err := service.StoreFile(context.Background(), "/tmp/input", "file-1"); !errors.Is(err, ErrServiceClosed) {
		t.Fatalf("expected ErrServiceClosed, got %v", err)
	}
	if _, err := service.ListFiles(context.Background()); !errors.Is(err, ErrServiceClosed) {
		t.Fatalf("expected ErrServiceClosed, got %v", err)
	}
}

func TestContextCancellationShortCircuits(t *testing.T) {
	t.Parallel()

	storage := &stubStorage{}
	service, err := New(storage)
	if err != nil {
		t.Fatalf("New() error = %v", err)
	}

	ctx, cancel := context.WithCancel(context.Background())
	cancel()

	if err := service.DeleteAllFiles(ctx); !errors.Is(err, context.Canceled) {
		t.Fatalf("expected context.Canceled, got %v", err)
	}
	if got := storage.deleteAllCalls; got != 0 {
		t.Fatalf("expected no delete-all calls, got %d", got)
	}
}
