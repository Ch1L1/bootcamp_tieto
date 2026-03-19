#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

usage() {
    echo "Usage: $0 [option]"
    echo ""
    echo "Options:"
    echo "  --build   Configure and compile the project"
    echo "  --run     Run the turnstile simulator"
    echo "  --clean   Remove the build directory"
    echo "  --help    Show this help message"
}

build() {
    echo "==> Configuring and building Turnstile FSM..."
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
    cmake --build "$BUILD_DIR"
    echo "==> Build complete."
}

run() {
    if [[ ! -f "$BUILD_DIR/turnstile" ]]; then
        echo "Error: turnstile binary not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Starting Turnstile FSM..."
    "$BUILD_DIR/turnstile"
}

clean() {
    echo "==> Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    echo "==> Done."
}

if [[ $# -ne 1 ]]; then
    usage
    exit 1
fi

case "$1" in
    --build)  build ;;
    --run)    run ;;
    --clean)  clean ;;
    --help)   usage ;;
    *)
        echo "Unknown option: $1"
        usage
        exit 1
        ;;
esac
