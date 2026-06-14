package bridge

/*
#cgo CFLAGS: -I${SRCDIR}/../../../cpp/include
#cgo darwin LDFLAGS: -L${SRCDIR}/../../../cpp/build -Wl,-rpath,${SRCDIR}/../../../cpp/build -lengine_lib_shared -lc++
#cgo linux LDFLAGS: -L${SRCDIR}/../../../cpp/build -Wl,-rpath,${SRCDIR}/../../../cpp/build -lengine_lib_shared -lstdc++

#include <stdlib.h>
#include "bridge.h"
*/
import "C"

import (
	"errors"
	"runtime"
	"sync"
	"unsafe"
)

var errEngineClosed = errors.New("bridge: engine is closed")

// Engine owns a native StorageEngine handle and exposes it as a Go-friendly API.
type Engine struct {
	mu     sync.Mutex
	handle C.StorageEngineHandle
}

// New creates a native storage engine rooted at storageRoot.
// Passing an empty string uses the engine's default root.
func New(storageRoot string) (*Engine, error) {
	root := cStringOrNil(storageRoot)
	if root != nil {
		defer C.free(unsafe.Pointer(root))
	}

	handle := C.storage_engine_create(root)
	if handle == nil {
		return nil, errors.New("bridge: failed to create storage engine")
	}

	engine := &Engine{handle: handle}
	runtime.SetFinalizer(engine, func(e *Engine) {
		_ = e.Close()
	})

	return engine, nil
}

// Close releases the native storage engine handle.
func (e *Engine) Close() error {
	if e == nil {
		return nil
	}

	e.mu.Lock()
	defer e.mu.Unlock()

	if e.handle == nil {
		return nil
	}

	C.storage_engine_destroy(e.handle)
	e.handle = nil
	runtime.SetFinalizer(e, nil)
	return nil
}

// StoreFile stores a file in the native storage engine.
func (e *Engine) StoreFile(filePath, fileID string) error {
	return e.withHandle(func(handle C.StorageEngineHandle) error {
		cFilePath := C.CString(filePath)
		defer C.free(unsafe.Pointer(cFilePath))

		cFileID := C.CString(fileID)
		defer C.free(unsafe.Pointer(cFileID))

		ok := C.storage_engine_store_file(handle, cFilePath, cFileID, nil)
		return e.boolResult(handle, ok, "store file")
	})
}

// RetrieveFile reconstructs a stored file at outputPath.
func (e *Engine) RetrieveFile(fileID, outputPath string) error {
	return e.withHandle(func(handle C.StorageEngineHandle) error {
		cFileID := C.CString(fileID)
		defer C.free(unsafe.Pointer(cFileID))

		cOutputPath := C.CString(outputPath)
		defer C.free(unsafe.Pointer(cOutputPath))

		ok := C.storage_engine_retrieve_file(handle, cFileID, cOutputPath, nil)
		return e.boolResult(handle, ok, "retrieve file")
	})
}

// ListFiles returns every file ID currently tracked by the native engine.
func (e *Engine) ListFiles() ([]string, error) {
	var files []string

	err := e.withHandle(func(handle C.StorageEngineHandle) error {
		var count C.int
		list := C.storage_engine_list_files(handle, &count)
		if list == nil {
			if count == 0 {
				files = []string{}
				return nil
			}
			return e.lastError(handle, "list files")
		}
		defer C.storage_engine_free_file_list(list, count)

		length := int(count)
		files = make([]string, 0, length)
		items := unsafe.Slice((**C.char)(unsafe.Pointer(list)), length)
		for _, item := range items {
			files = append(files, C.GoString(item))
		}

		return nil
	})
	if err != nil {
		return nil, err
	}

	return files, nil
}

// DeleteFile removes one stored file and its metadata.
func (e *Engine) DeleteFile(fileID string) error {
	return e.withHandle(func(handle C.StorageEngineHandle) error {
		cFileID := C.CString(fileID)
		defer C.free(unsafe.Pointer(cFileID))

		ok := C.storage_engine_delete_file(handle, cFileID)
		return e.boolResult(handle, ok, "delete file")
	})
}

// DeleteAllFiles removes all stored files from the native engine.
func (e *Engine) DeleteAllFiles() error {
	return e.withHandle(func(handle C.StorageEngineHandle) error {
		ok := C.storage_engine_delete_all_files(handle)
		return e.boolResult(handle, ok, "delete all files")
	})
}

// Progress returns the latest recorded progress percentage for fileID.
func (e *Engine) Progress(fileID string) (int, error) {
	var progress int

	err := e.withHandle(func(handle C.StorageEngineHandle) error {
		cFileID := C.CString(fileID)
		defer C.free(unsafe.Pointer(cFileID))

		value := C.storage_engine_get_progress(handle, cFileID)
		if int(value) < 0 {
			return e.lastError(handle, "get progress")
		}

		progress = int(value)
		return nil
	})
	if err != nil {
		return 0, err
	}

	return progress, nil
}

// WaitForBackgroundTasks blocks until the native engine has drained background work.
func (e *Engine) WaitForBackgroundTasks() error {
	return e.withHandle(func(handle C.StorageEngineHandle) error {
		C.storage_engine_wait_for_background_tasks(handle)
		return e.lastError(handle, "")
	})
}

func (e *Engine) withHandle(fn func(handle C.StorageEngineHandle) error) error {
	if e == nil {
		return errEngineClosed
	}

	e.mu.Lock()
	defer e.mu.Unlock()

	if e.handle == nil {
		return errEngineClosed
	}

	return fn(e.handle)
}

func (e *Engine) boolResult(handle C.StorageEngineHandle, ok C.bool, action string) error {
	if ok {
		return nil
	}

	return e.lastError(handle, action)
}

func (e *Engine) lastError(handle C.StorageEngineHandle, action string) error {
	msg := C.storage_engine_get_error(handle)
	if msg == nil {
		if action == "" {
			return nil
		}
		return errors.New("bridge: " + action + " failed")
	}

	text := C.GoString(msg)
	if text == "" {
		if action == "" {
			return nil
		}
		return errors.New("bridge: " + action + " failed")
	}
	if action == "" {
		return errors.New("bridge: " + text)
	}
	return errors.New("bridge: " + action + ": " + text)
}

func cStringOrNil(value string) *C.char {
	if value == "" {
		return nil
	}
	return C.CString(value)
}
