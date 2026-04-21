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

## Go Service Scaffold

The Go layer is now structured for:

- `cmd/server/` for the executable entrypoint
- `internal/api/` for handlers and transport logic
- `internal/service/` for orchestration/business logic
- `internal/bridge/` for integration with the native engine
- `pkg/client/` for any external client SDK

The current server is a lightweight scaffold so the directory layout is ready before the real API implementation lands.

## Build and Run

Build the C++ engine:

```bash
./scripts/build_cpp.sh
```

Run the Go server scaffold:

```bash
./scripts/run_server.sh
```

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
