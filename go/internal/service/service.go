package service

import (
	"context"
	"errors"
	"strings"
	"sync"

	clientpkg "github.com/ownsphere/core_storage_engine/go/pkg/client"
)

var (
	ErrNilStorage      = errors.New("service: storage client is required")
	ErrServiceClosed   = errors.New("service: storage service is closed")
	ErrEmptyFileID     = errors.New("service: file ID is required")
	ErrEmptySourcePath = errors.New("service: source path is required")
	ErrEmptyOutputPath = errors.New("service: output path is required")
)

// StorageService orchestrates storage workflows on top of the Go client wrapper.
type StorageService struct {
	mu      sync.RWMutex
	storage clientpkg.Storage
	closed  bool
}

// New creates a storage service backed by the provided storage client.
func New(storage clientpkg.Storage) (*StorageService, error) {
	if storage == nil {
		return nil, ErrNilStorage
	}

	return &StorageService{storage: storage}, nil
}

// Close prevents future work and releases the wrapped storage client.
func (s *StorageService) Close() error {
	if s == nil {
		return nil
	}

	s.mu.Lock()
	defer s.mu.Unlock()

	if s.closed {
		return nil
	}

	s.closed = true
	return s.storage.Close()
}

// StoreFile validates the request and stores a file under the provided file ID.
func (s *StorageService) StoreFile(ctx context.Context, sourcePath, fileID string) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if strings.TrimSpace(sourcePath) == "" {
		return ErrEmptySourcePath
	}
	if strings.TrimSpace(fileID) == "" {
		return ErrEmptyFileID
	}

	return s.withStorageRead(func(storage clientpkg.Storage) error {
		return storage.StoreFile(ctx, sourcePath, fileID)
	})
}

// RetrieveFile validates the request and reconstructs a stored file.
func (s *StorageService) RetrieveFile(ctx context.Context, fileID, outputPath string) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if strings.TrimSpace(fileID) == "" {
		return ErrEmptyFileID
	}
	if strings.TrimSpace(outputPath) == "" {
		return ErrEmptyOutputPath
	}

	return s.withStorageRead(func(storage clientpkg.Storage) error {
		return storage.RetrieveFile(ctx, fileID, outputPath)
	})
}

// ListFiles returns every tracked file ID.
func (s *StorageService) ListFiles(ctx context.Context) ([]string, error) {
	if err := ctxErr(ctx); err != nil {
		return nil, err
	}

	var files []string
	err := s.withStorageRead(func(storage clientpkg.Storage) error {
		var err error
		files, err = storage.ListFiles(ctx)
		return err
	})
	if err != nil {
		return nil, err
	}

	return files, nil
}

// DeleteFile validates the request and removes one stored file.
func (s *StorageService) DeleteFile(ctx context.Context, fileID string) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if strings.TrimSpace(fileID) == "" {
		return ErrEmptyFileID
	}

	return s.withStorageWrite(func(storage clientpkg.Storage) error {
		return storage.DeleteFile(ctx, fileID)
	})
}

// DeleteAllFiles removes all stored files while excluding concurrent mutations.
func (s *StorageService) DeleteAllFiles(ctx context.Context) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}

	return s.withStorageWrite(func(storage clientpkg.Storage) error {
		return storage.DeleteAllFiles(ctx)
	})
}

// Progress returns the latest known progress percentage for a file.
func (s *StorageService) Progress(ctx context.Context, fileID string) (int, error) {
	if err := ctxErr(ctx); err != nil {
		return 0, err
	}
	if strings.TrimSpace(fileID) == "" {
		return 0, ErrEmptyFileID
	}

	var progress int
	err := s.withStorageRead(func(storage clientpkg.Storage) error {
		var err error
		progress, err = storage.Progress(ctx, fileID)
		return err
	})
	if err != nil {
		return 0, err
	}

	return progress, nil
}

// WaitForBackgroundTasks waits for in-flight native background work to complete.
func (s *StorageService) WaitForBackgroundTasks(ctx context.Context) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}

	return s.withStorageRead(func(storage clientpkg.Storage) error {
		return storage.WaitForBackgroundTasks(ctx)
	})
}

func (s *StorageService) withStorageRead(fn func(storage clientpkg.Storage) error) error {
	if s == nil {
		return ErrServiceClosed
	}

	s.mu.RLock()
	defer s.mu.RUnlock()

	if s.closed || s.storage == nil {
		return ErrServiceClosed
	}

	return fn(s.storage)
}

func (s *StorageService) withStorageWrite(fn func(storage clientpkg.Storage) error) error {
	if s == nil {
		return ErrServiceClosed
	}

	s.mu.Lock()
	defer s.mu.Unlock()

	if s.closed || s.storage == nil {
		return ErrServiceClosed
	}

	return fn(s.storage)
}

func ctxErr(ctx context.Context) error {
	if ctx == nil {
		return nil
	}
	return ctx.Err()
}
