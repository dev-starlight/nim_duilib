#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build/coverage"
REPORT_DIR="${BUILD_DIR}/report"

command -v cmake >/dev/null 2>&1 || { echo "cmake not found"; exit 1; }
command -v gcovr >/dev/null 2>&1 || { echo "gcovr not found"; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo "gcc not found"; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo "g++ not found"; exit 1; }

cmake -S "${ROOT_DIR}/tests" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_FLAGS="--coverage -O0 -g" \
    -DCMAKE_CXX_FLAGS="--coverage -O0 -g"

cmake --build "${BUILD_DIR}" --config Debug
ctest --test-dir "${BUILD_DIR}" --output-on-failure

mkdir -p "${REPORT_DIR}"

gcovr \
    --root "${ROOT_DIR}" \
    --object-directory "${BUILD_DIR}" \
    --exclude ".*duilib/third_party/.*" \
    --exclude ".*tests/.*" \
    --xml-pretty "${REPORT_DIR}/coverage.xml" \
    --html-details "${REPORT_DIR}/coverage.html" \
    --txt "${REPORT_DIR}/coverage.txt" \
    --print-summary

echo "Coverage reports generated:"
echo "  ${REPORT_DIR}/coverage.xml"
echo "  ${REPORT_DIR}/coverage.html"
echo "  ${REPORT_DIR}/coverage.txt"
