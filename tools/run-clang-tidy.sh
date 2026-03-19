#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build-clang-tidy"

# Check for clang-tidy
if ! command -v clang-tidy &>/dev/null; then
    echo "Error: clang-tidy not found on PATH" >&2
    echo "Install with: sudo apt-get install clang-tidy" >&2
    exit 1
fi

# Generate compile_commands.json if not present
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Generating compile_commands.json in $BUILD_DIR ..."
    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_C_FLAGS="-std=gnu99"
fi

# Build file regex to match only project sources (exclude 3rdparty)
FILE_REGEX='(src|apps|test)/'

if command -v run-clang-tidy &>/dev/null; then
    run-clang-tidy -p "$BUILD_DIR" "$FILE_REGEX" "$@"
else
    echo "run-clang-tidy not found, falling back to manual invocation..." >&2
    find "$PROJECT_DIR/src" "$PROJECT_DIR/apps" "$PROJECT_DIR/test" \
        -name '*.c' -o -name '*.h' \
        | xargs -P "$(nproc)" -I{} clang-tidy -p "$BUILD_DIR" "$@" {}
fi
