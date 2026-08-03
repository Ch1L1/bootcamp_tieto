# CallSim

A small client-server call simulation built with C++ and CMake. The project uses Google Protocol Buffers for message exchange and a simple client state machine for the interactive call flow.

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

### Prerequisites
- CMake (>= 3.10)
- C++17-capable compiler
- Protobuf (Google Protocol Buffers)

**macOS Installation (via Homebrew):**

```bash
brew install cmake protobuf
```

**Ubuntu/Debian Installation (via APT):**
```bash
sudo apt update && sudo apt install -y build-essential cmake libprotobuf-dev protobuf-compiler
```

## Quick Start

```bash
# Build the project
./callsim.sh --build

# Run the server (starts listening on localhost:8080)
./callsim.sh --run-server

# Run the client
./callsim.sh --run-client

# Remove all build artifacts
./callsim.sh --clean
```

**Manual Commands**

```bash
# Configure and build
cmake -S . -B build
cmake --build build

# Run executables
./build/Server/server_bin
./build/Client/client_bin

# Clean
rm -rf build/
```

## Run server

Starts the server and listens for incoming client connections on localhost:8080.

```bash
./callsim.sh --run-server
```

## Run client

Starts an interactive client that connects to the server and waits for commands on standard input. The client binary accepts an optional client ID argument. When launched through the helper script, you can pass that ID after the `--run-client` flag.

```bash
./callsim.sh --run-client
```

### Available commands

Once the client is running, it supports the following commands:

- `/call <client_id>` - Start a call with another client
- `/answer` - Accept an incoming call
- `/reject` - Decline an incoming call
- `/help` - Show the available commands
- `/exit` - Disconnect from the server


### Example flow

```bash
./callsim.sh --run-server
./callsim.sh --run-client Bob
```

Then in the client console:

```text
/call Lora
/answer
/reject
```

## Run tests

```bash
./callsim.sh --test
```

## Clean build artifacts

```bash
./callsim.sh --clean
```

## Notes

- The current phase focuses on the interactive call flow and basic client/server state handling.
- TODO: HangUp call
