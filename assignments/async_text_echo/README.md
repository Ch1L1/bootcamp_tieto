# async_text_echo

A simple asynchronous client-server text echo

## Features
- Client reads text from stdin, sends to server, prints server reply.
- Server prints received text, replies with "Re:" prefix.

## Usage

```sh
# Build
./async_text_echo.sh --build

# Run server
./async_text_echo.sh --run-server

# Run client
./async_text_echo.sh --run-client

# Clean build
./async_text_echo.sh --clean
```

## Requirements
- CMake >= 3.10
- Boost (libboost-all-dev)
- C++17 compiler
