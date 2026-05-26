# CallSim

A client-server call simulation built with C++ and CMake. This project uses Google Protocol Buffers (Protobuf) to define the communication protocol and state machines.

---

## Project Structure

```text
CallSim/
├── Client/          # Client application
├── Server/          # Server application
├── proto/           # Protobuf message definitions
├── callsim.sh       # Helper script for building and running
└── CMakeLists.txt   # Root CMake configuration
```
## Prerequisites
- CMake (>= 3.10)

- C++17-capable compiler

- Protobuf (Google Protocol Buffers)

**macOS Installation (via Homebrew):**

```Bash
brew install cmake protobuf
```

**Ubuntu/Debian Installation (via APT):**
```sh
sudo apt update && sudo apt install -y build-essential cmake libprotobuf-dev protobuf-compiler
```
## Quick Start

```Bash
# Build the project (Generates C++ API from .proto and compiles)
./callsim.sh --build

# Run the server sanity check
./callsim.sh --run-server

# Run the client sanity check (Run in a separate terminal)
./callsim.sh --run-client

# Remove all build artifacts
./callsim.sh --clean
```
## Manual Commands
If you prefer to build manually without the helper script:

```Bash
# Configure and build
cmake -S . -B build
cmake --build build

# Run executables
./build/Server/server
./build/Client/client

# Clean
rm -rf build/
```