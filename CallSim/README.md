# CallSim

A client-server call simulation built with C++ and CMake.

---

## Project Structure

```
CallSim/
├── Client/          # Client application
├── Server/          # Server application
├── proto/           # Protobuf message definitions
├── callsim.sh       # Helper script
└── CMakeLists.txt   # CMake configuration
```

---

## Quick Start

```sh
# Build the project
./callsim.sh --build

# Run the server
./callsim.sh --run-server

# Run the client
./callsim.sh --run-client

# Remove all build artifacts
./callsim.sh --clean
```

---

## Manual Commands

```sh
# Configure and build
cmake -S . -B build
cmake --build build

# Run
./build/Server/server
./build/Client/client

# Clean
rm -rf build/
```

---

## Requirements

- CMake >= 3.10
- A C++17-capable compiler

