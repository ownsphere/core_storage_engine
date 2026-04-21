#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/cpp/build}"
INFO_FILE="${REPO_ROOT}/coverage.info"
REPORT_DIR="${REPO_ROOT}/coverage-report"


if ! command -v lcov >/dev/null 2>&1; then
    echo "lcov is not installed or not available on PATH."
    echo "Install it with your package manager, for example: brew install lcov or sudo apt-get install -y lcov"
    exit 1
fi

if ! command -v genhtml >/dev/null 2>&1; then
    echo "genhtml is not installed or not available on PATH."
    echo "Install it with your package manager, for example: brew install lcov or sudo apt-get install -y lcov"
    exit 1
fi

if [[ ! -d "${BUILD_DIR}" ]]; then
    echo "Build directory not found: ${BUILD_DIR}"
    echo "Run ./scripts/build_cpp.sh first."
    exit 1
fi

ctest --output-on-failure --test-dir "${BUILD_DIR}"

rm -f "${INFO_FILE}"
rm -rf "${REPORT_DIR}"

lcov \
    --capture \
    --directory "${BUILD_DIR}" \
    --output-file "${INFO_FILE}" \
    --ignore-errors mismatch,inconsistent

lcov \
    --remove "${INFO_FILE}" \
    "/Applications/*" \
    "/Library/*" \
    "/opt/homebrew/*" \
    "/usr/*" \
    "*/tests/*" \
    "*/_deps/*" \
    --output-file "${INFO_FILE}" \
    --ignore-errors unused

genhtml "${INFO_FILE}" --output-directory "${REPORT_DIR}"

echo "Coverage report generated at ${REPORT_DIR}/index.html"
