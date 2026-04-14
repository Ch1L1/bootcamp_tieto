#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

usage() {
    echo "Usage: $0 [option]"
    echo ""
    echo "Options:"
    echo "  --build         Configure and compile the project"
    echo "  --test          Run all unit tests (CTest)"
    echo "  --run-client    Run the client executable"
    echo "  --run-server    Run the server executable"
    echo "  --clean         Remove the build directory"
    echo "  --help          Show this help message"
}

build() {
    echo "==> Building CallSim..."
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
    cmake --build "$BUILD_DIR" -j$(nproc)
    echo "==> Build complete."
}

run_test() {
    if [[ ! -d "$BUILD_DIR" ]]; then
        echo "Error: build directory not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Running tests..."
    cd "$BUILD_DIR" && ctest --output-on-failure
}

run_client() {
    # Path updated to lowercase 'client'
    if [[ ! -f "$BUILD_DIR/client/client" ]]; then
        echo "Error: client binary not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Starting client..."
    "$BUILD_DIR/client/client"
}

run_server() {
    # Path updated to lowercase 'server'
    if [[ ! -f "$BUILD_DIR/server/server" ]]; then
        echo "Error: server binary not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Starting server..."
    "$BUILD_DIR/server/server"
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
    --build)       build ;;
    --test)        run_test ;;
    --run-client)  run_client ;;
    --run-server)  run_server ;;
    --clean)       clean ;;
    --help)        usage ;;
    *)
        echo "Unknown option: $1"
        usage
        exit 1
        ;;
esac