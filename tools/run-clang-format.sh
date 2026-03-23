#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

CLANG_FORMAT="${CLANG_FORMAT:-clang-format-18}"

if ! command -v "$CLANG_FORMAT" &>/dev/null; then
    echo "Error: $CLANG_FORMAT not found on PATH" >&2
    echo "Install with: sudo apt-get install clang-format-18" >&2
    exit 1
fi

FILES=$(find "$PROJECT_DIR/src" "$PROJECT_DIR/apps" "$PROJECT_DIR/test" \
    -name '*.c' -o -name '*.h')

if [ "${1:-}" = "--check" ]; then
    echo "Checking formatting..."
    $CLANG_FORMAT --dry-run --Werror $FILES
else
    echo "Formatting files..."
    $CLANG_FORMAT -i $FILES
    echo "Done."
fi
