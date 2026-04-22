#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

if ! command -v go >/dev/null 2>&1; then
    echo "Go is not installed or not available on PATH."
    exit 1
fi

cd "${REPO_ROOT}/go"
go run ./cmd/server
