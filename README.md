# OwnSphere Core Storage Engine

OwnSphere is a high-performance storage system built from scratch, inspired by modern object storage and NAS systems.

The project focuses on building core storage fundamentals such as chunking, data integrity, and failure handling, with a long-term goal of evolving into a distributed storage system.

---

## Overview

OwnSphere is designed with a layered architecture:

- **C++ Core Engine** → Handles storage, chunking, and data integrity

This separation ensures:
- High performance (C++)
- Easy integration and scalability (Go)

---

## Features

### Core Storage Engine (C++)
- File chunking (fixed-size chunks)
- Metadata management
- SHA-256 checksum-based data integrity
- AES-256 encryption/decryption layer
- Atomic metadata writes (crash-safe)
- Rollback on failure (prevents partial writes)
- File reconstruction from chunks
- File listing and deletion
- Progress tracking

---

### Testing & Quality
- Unit testing using GoogleTest
- Integration testing (store/retrieve/delete)
- Failure testing (missing chunk, corruption)
- Concurrency testing
- Code coverage using gcov/lcov

---

### Logging System
- Structured logging
- Log levels: INFO, ERROR, DEBUG
- Console + file logging
- Thread-safe logging

---

## What is Handled

- File storage and retrieval
- Chunk-based storage architecture
- Metadata consistency (atomic writes)
- Failure detection:
  - Missing chunk
  - Corrupted chunk via SHA-256 verification
- Rollback on failure
- Empty file handling
- File overwrite handling
- CLI-based interaction
- API-based interaction (Go agent)
- Logging and observability
- Test coverage and validation

---

## Not Handled Yet

### Data Safety
- Crash recovery system
- Write-Ahead Logging (WAL)
- Atomic chunk writes

### Reliability
- Data replication (RAID / multi-copy)
- Garbage collection (orphan chunk cleanup)
- Metadata corruption detection

### Concurrency
- Full thread-safe storage operations

### Security
- External key management / secret rotation
- Authentication / access control

### Scalability
- Distributed storage (multi-node)
- Sharding and load balancing

### Observability
- Metrics (latency, storage usage, failure rate)
- Monitoring system

---

## Setup & Installation

### System Dependencies (Ubuntu)

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    g++ \
    git \
    lcov \
    gcovr
