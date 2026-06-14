package client

import (
	"context"
	"errors"
	"strings"

	bridgepkg "github.com/ownsphere/core_storage_engine/go/internal/bridge"
)

var (
	ErrEmptyFileID     = errors.New("client: file ID is required")
	ErrEmptySourcePath = errors.New("client: source path is required")
	ErrEmptyOutputPath = errors.New("client: output path is required")
	ErrNotInitialized  = errors.New("client: storage client is not initialized")
)

// Storage defines the operations the service layer needs from the storage client.
type Storage interface {
	Close() error
	StoreFile(ctx context.Context, sourcePath, fileID string) error
	RetrieveFile(ctx context.Context, fileID, outputPath string) error
	ListFiles(ctx context.Context) ([]string, error)
	DeleteFile(ctx context.Context, fileID string) error
	DeleteAllFiles(ctx context.Context) error
	Progress(ctx context.Context, fileID string) (int, error)
	WaitForBackgroundTasks(ctx context.Context) error
}

type engine interface {
	Close() error
	StoreFile(filePath, fileID string) error
	RetrieveFile(fileID, outputPath string) error
	ListFiles() ([]string, error)
	DeleteFile(fileID string) error
	DeleteAllFiles() error
	Progress(fileID string) (int, error)
	WaitForBackgroundTasks() error
}

// Config controls how the storage client initializes the native engine.
type Config struct {
	StorageRoot string
}

// Client is the Go-facing wrapper around the native storage engine bridge.
type Client struct {
	engine engine
}

// New creates a client backed by the native storage engine.
func New(storageRoot string) (*Client, error) {
	return NewWithConfig(Config{StorageRoot: storageRoot})
}

// NewWithConfig creates a client backed by the native storage engine.
func NewWithConfig(cfg Config) (*Client, error) {
	nativeEngine, err := bridgepkg.New(strings.TrimSpace(cfg.StorageRoot))
	if err != nil {
		return nil, err
	}

	return &Client{engine: nativeEngine}, nil
}

// Close releases the underlying native engine resources.
func (c *Client) Close() error {
	if c == nil || c.engine == nil {
		return nil
	}
	return c.engine.Close()
}

// StoreFile persists a source file under the provided file ID.
func (c *Client) StoreFile(ctx context.Context, sourcePath, fileID string) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if err := c.ready(); err != nil {
		return err
	}
	if strings.TrimSpace(sourcePath) == "" {
		return ErrEmptySourcePath
	}
	if strings.TrimSpace(fileID) == "" {
		return ErrEmptyFileID
	}

	if err := c.engine.StoreFile(sourcePath, fileID); err != nil {
		return err
	}

	return ctxErr(ctx)
}

// RetrieveFile reconstructs a stored file at the provided output path.
func (c *Client) RetrieveFile(ctx context.Context, fileID, outputPath string) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if err := c.ready(); err != nil {
		return err
	}
	if strings.TrimSpace(fileID) == "" {
		return ErrEmptyFileID
	}
	if strings.TrimSpace(outputPath) == "" {
		return ErrEmptyOutputPath
	}

	if err := c.engine.RetrieveFile(fileID, outputPath); err != nil {
		return err
	}

	return ctxErr(ctx)
}

// ListFiles returns the current file IDs known to the storage engine.
func (c *Client) ListFiles(ctx context.Context) ([]string, error) {
	if err := ctxErr(ctx); err != nil {
		return nil, err
	}
	if err := c.ready(); err != nil {
		return nil, err
	}

	files, err := c.engine.ListFiles()
	if err != nil {
		return nil, err
	}

	return files, ctxErr(ctx)
}

// DeleteFile removes a stored file and its metadata.
func (c *Client) DeleteFile(ctx context.Context, fileID string) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if err := c.ready(); err != nil {
		return err
	}
	if strings.TrimSpace(fileID) == "" {
		return ErrEmptyFileID
	}

	if err := c.engine.DeleteFile(fileID); err != nil {
		return err
	}

	return ctxErr(ctx)
}

// DeleteAllFiles removes all stored files.
func (c *Client) DeleteAllFiles(ctx context.Context) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if err := c.ready(); err != nil {
		return err
	}

	if err := c.engine.DeleteAllFiles(); err != nil {
		return err
	}

	return ctxErr(ctx)
}

// Progress returns the latest progress percentage for the given file ID.
func (c *Client) Progress(ctx context.Context, fileID string) (int, error) {
	if err := ctxErr(ctx); err != nil {
		return 0, err
	}
	if err := c.ready(); err != nil {
		return 0, err
	}
	if strings.TrimSpace(fileID) == "" {
		return 0, ErrEmptyFileID
	}

	progress, err := c.engine.Progress(fileID)
	if err != nil {
		return 0, err
	}

	return progress, ctxErr(ctx)
}

// WaitForBackgroundTasks blocks until background native work completes.
func (c *Client) WaitForBackgroundTasks(ctx context.Context) error {
	if err := ctxErr(ctx); err != nil {
		return err
	}
	if err := c.ready(); err != nil {
		return err
	}

	if err := c.engine.WaitForBackgroundTasks(); err != nil {
		return err
	}

	return ctxErr(ctx)
}

func ctxErr(ctx context.Context) error {
	if ctx == nil {
		return nil
	}
	return ctx.Err()
}

func (c *Client) ready() error {
	if c == nil || c.engine == nil {
		return ErrNotInitialized
	}
	return nil
}
