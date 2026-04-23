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

# Build file regex to match only project sources (exclude 3rdparty).
# The lookahead rejects paths containing "/3rdparty/" — needed because the
# compile_commands.json has absolute paths where "src/" also appears as
# "3rdparty/libtomcrypt/src/...".
FILE_REGEX='^(?!.*/3rdparty/).*/(src|apps|test)/'

# Anchor the header filter on the project root so headers under 3rdparty/
# (e.g. libtomcrypt) aren't scanned. Overrides HeaderFilterRegex from
# .clang-tidy — we can't express this exclusion there since llvm::Regex
# has no negative lookahead and older clang-tidy lacks ExcludeHeaderFilterRegex.
HEADER_FILTER="^$PROJECT_DIR/(src|apps|test)/"

if command -v run-clang-tidy &>/dev/null; then
    run-clang-tidy -p "$BUILD_DIR" -warnings-as-errors='*' -header-filter="$HEADER_FILTER" "$FILE_REGEX" "$@"
else
    echo "run-clang-tidy not found, falling back to manual invocation..." >&2
    find "$PROJECT_DIR/src" "$PROJECT_DIR/apps" "$PROJECT_DIR/test" \
        -name '*.c' -o -name '*.h' \
        | xargs -P "$(nproc)" -I{} clang-tidy -p "$BUILD_DIR" -warnings-as-errors='*' -header-filter="$HEADER_FILTER" "$@" {}
fi
