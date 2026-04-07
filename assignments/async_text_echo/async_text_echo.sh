#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

usage() {
    echo "Usage: $0 [option]"
    echo ""
    echo "Options:"
    echo "  --build        Configure and compile the project"
    echo "  --run-client   Run the client executable"
    echo "  --run-server   Run the server executable"
    echo "  --clean        Remove the build directory"
    echo "  --help         Show this help message"
}

build() {
    echo "==> Building async_text_echo..."
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
    cmake --build "$BUILD_DIR"
    echo "==> Build complete."
}

run_client() {
    if [[ ! -f "$BUILD_DIR/Client/client" ]]; then
        echo "Error: client binary not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Starting client..."
    "$BUILD_DIR/Client/client"
}

run_server() {
    if [[ ! -f "$BUILD_DIR/Server/server" ]]; then
        echo "Error: server binary not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Starting server..."
    "$BUILD_DIR/Server/server"
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
