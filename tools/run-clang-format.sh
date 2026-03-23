#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

if ! command -v clang-format &>/dev/null; then
    echo "Error: clang-format not found on PATH" >&2
    echo "Install with: sudo apt-get install clang-format" >&2
    exit 1
fi

FILES=$(find "$PROJECT_DIR/src" "$PROJECT_DIR/apps" "$PROJECT_DIR/test" \
    -name '*.c' -o -name '*.h')

if [ "${1:-}" = "--check" ]; then
    echo "Checking formatting..."
    clang-format --dry-run --Werror $FILES
else
    echo "Formatting files..."
    clang-format -i $FILES
    echo "Done."
fi
