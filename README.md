# OwnSphere Core Storage Engine

OwnSphere is a storage-engine repository organized around a native C++ core and a Go service layer.

## Repository Structure

```text
core_storage_engine/
├── cpp/
│   ├── src/
│   ├── include/
│   ├── tests/
│   └── CMakeLists.txt
├── go/
│   ├── cmd/server/
│   ├── internal/
│   │   ├── api/
│   │   ├── bridge/
│   │   └── service/
│   ├── pkg/client/
│   └── go.mod
└── scripts/
├── build_cpp.sh
    ├── generate_cpp_coverage.sh
    └── run_server.sh
```

## Layout Notes

- `cpp/` contains the storage engine, headers, and tests.
- `go/` is the service/API layer scaffold that will sit on top of the C++ core.
- `scripts/` contains repo-level helper scripts so day-to-day commands stay consistent.

## C++ Engine

Current capabilities in the native engine:

- File chunking
- Metadata management
- SHA-256 checksum validation
- AES-256 encryption/decryption
- Atomic metadata writes
- Rollback on failure
- File reconstruction, listing, and deletion
- Progress reporting
- GoogleTest-based test coverage

## Go Service Layer

The Go layer is now organized as:

- `cmd/server/` for the executable entrypoint
- `internal/api/` for gRPC transport handlers and protobuf bindings
- `internal/service/` for orchestration/business logic
- `internal/bridge/` for integration with the native engine
- `pkg/client/` for the Go-facing storage client wrapper

Current Go capabilities include:

- cgo bridge bindings for the native storage engine
- a Go client wrapper for storage operations
- a service layer for validation and orchestration
- a gRPC API for store, retrieve, list, delete, progress, and background task operations
- gRPC health checks and server reflection

## Build and Run

Build the C++ engine:

```bash
./scripts/build_cpp.sh
```

Run Go tests:

```bash
cd go
go test ./...
```

Run the Go gRPC server:

```bash
./scripts/run_server.sh
```

Optional environment variables:

- `PORT` to change the gRPC listen port. Default: `8080`
- `STORAGE_ROOT` to override the native engine storage root

Run C++ tests from the generated build directory:

```bash
cd cpp/build
ctest --output-on-failure
```

Generate the C++ HTML coverage report:

```bash
./scripts/generate_cpp_coverage.sh
```

This writes:

- `coverage.info`
- `coverage-report/index.html`

## Dependencies

For the C++ layer you will need:

- `cmake`
- A C++17 compiler
- OpenSSL development headers
- GoogleTest
- `lcov` and `genhtml` for HTML coverage reports

For the Go layer you will need:

- Go 1.22 or newer
- Access to Go modules for `google.golang.org/grpc` and `google.golang.org/protobuf`
