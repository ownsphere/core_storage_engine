package client

import (
	"context"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"testing"
)

func TestClientStoreAndRetrieveFileIntegration(t *testing.T) {
	t.Parallel()

	if _, err := os.Stat(nativeLibraryPath()); err != nil {
		t.Skipf("native library not available: %v", err)
	}

	ctx := context.Background()
	storageRoot := t.TempDir()
	workDir := t.TempDir()

	sourcePath := filepath.Join(workDir, "source.txt")
	contents := []byte("OwnSphere Go integration test content.\n")
	if err := os.WriteFile(sourcePath, contents, 0o644); err != nil {
		t.Fatalf("WriteFile(source) error = %v", err)
	}

	client, err := New(storageRoot)
	if err != nil {
		t.Fatalf("New() error = %v", err)
	}
	defer func() {
		if err := client.Close(); err != nil {
			t.Fatalf("Close() error = %v", err)
		}
	}()

	const fileID = "integration-file"
	if err := client.StoreFile(ctx, sourcePath, fileID); err != nil {
		t.Fatalf("StoreFile() error = %v", err)
	}
	if err := client.WaitForBackgroundTasks(ctx); err != nil {
		t.Fatalf("WaitForBackgroundTasks() error = %v", err)
	}

	fileIDs, err := client.ListFiles(ctx)
	if err != nil {
		t.Fatalf("ListFiles() error = %v", err)
	}
	if !slices.Contains(fileIDs, fileID) {
		t.Fatalf("expected file ID %q in list %#v", fileID, fileIDs)
	}

	outputPath := filepath.Join(workDir, "output.txt")
	if err := client.RetrieveFile(ctx, fileID, outputPath); err != nil {
		t.Fatalf("RetrieveFile() error = %v", err)
	}
	if err := client.WaitForBackgroundTasks(ctx); err != nil {
		t.Fatalf("WaitForBackgroundTasks() after retrieve error = %v", err)
	}

	retrieved, err := os.ReadFile(outputPath)
	if err != nil {
		t.Fatalf("ReadFile(output) error = %v", err)
	}
	if string(retrieved) != string(contents) {
		t.Fatalf("retrieved content mismatch: got %q want %q", string(retrieved), string(contents))
	}
}

func nativeLibraryPath() string {
	fileName := "libengine_lib_shared.so"
	if runtime.GOOS == "darwin" {
		fileName = "libengine_lib_shared.dylib"
	}
	return filepath.Join("..", "..", "cpp", "build", fileName)
}
