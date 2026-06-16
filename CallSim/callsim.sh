#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

usage() {
    echo "Usage: $0 [option]"
    echo ""
    echo "Options:"
    echo "  --build              Configure and compile the project"
    echo "  --run-client [name]  Run the client executable (optional: assign unique ID)"
    echo "  --run-server         Run the server executable"
    echo "  --test               Run all Google Test suites via CTest"
    echo "  --clean              Remove the build directory"
    echo "  --help               Show this help message"
}

build() {
    echo "==> Building CallSim..."
    cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR"
    cmake --build "$BUILD_DIR"
    echo "==> Build complete."
}

run_client() {
    if [[ ! -f "$BUILD_DIR/Client/client_bin" ]]; then
        echo "Error: client binary not found. Run '$0 --build' first."
        exit 1
    fi
    
    local client_id="${1:-client_default}"
    echo "==> Starting client with ID: $client_id..."
    "$BUILD_DIR/Client/client_bin" "$client_id"
}

run_server() {
    if [[ ! -f "$BUILD_DIR/Server/server_bin" ]]; then
        echo "Error: server binary not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Starting server..."
    "$BUILD_DIR/Server/server_bin"
}

run_tests() {
    if [[ ! -d "$BUILD_DIR" ]]; then
        echo "Error: Build directory not found. Run '$0 --build' first."
        exit 1
    fi
    echo "==> Running automated test suites..."
    cd "$BUILD_DIR" && ctest --output-on-failure
}

clean() {
    echo "==> Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    echo "==> Done."
}

if [[ $# -lt 1 || $# -gt 2 ]]; then
    usage
    exit 1
fi

case "$1" in
    --build)       build ;;
    --run-client)  run_client "$2" ;;
    --run-server)  run_server ;;
    --test)        run_tests ;;
    --clean)       clean ;;
    --help)        usage ;;
    *)
        echo "Unknown option: $1"
        usage
        exit 1
        ;;
esac