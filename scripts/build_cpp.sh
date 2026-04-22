#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/cpp/build}"

cmake -S "${REPO_ROOT}/cpp" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}"
