# CallSim

A high-performance client-server call simulation built with C++, Protobuf, and GoogleTest.

---

## Project Structure

```text
callsim/
├── callsim/v1/      # Protobuf message definitions
├── client/          # Client application
├── server/          # Server application
├── proto/           # CMake logic for Protobuf generation
├── build/           # Build artifacts 
├── buf.yaml         # Protobuf linter configuration
└── CMakeLists.txt   # Root CMake configuration
```
## Requirements
- CMake >= 3.28
- C++17 capable compiler (GCC 13.3+ recommended)
- Protobuf Compiler (protoc)
- GoogleTest (libgtest-dev)
- Buf CLI (optional, for linting)

## Quick Start
1. Build the project using helper script
```Bash
./callsim.sh --build
```
Or manual build:
```Bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```
2. Run Tests
```Bash
./callsim.sh --test
```
3. Run Applications
```Bash
# Start the server
./callsim.sh --run-server
# Start the client (in a new terminal)
./callsim.sh --run-client
```
### Protocol Management
This project uses Protobuf for structured communication.

### Linting
To ensure the API follows standard naming conventions, run:

``` Bash
buf lint
```