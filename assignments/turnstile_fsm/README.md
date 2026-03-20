# Turnstile FSM

A simple finite state machine (FSM) modelling a turnstile

## States & Transitions

| Current State | Event  | Next State | Action             |
|---------------|--------|------------|--------------------|
| Locked        | coin   | Unlocked   | "Unlocking"        |
| Locked        | push   | Locked     | "Still locked"     |
| Unlocked      | push   | Locked     | "Locking"          |
| Unlocked      | coin   | Unlocked   | "Already unlocked" |

## Build & Run

```sh
./turnstile.sh --build   # Configure and compile
./turnstile.sh --run     # Run the simulator
./turnstile.sh --clean   # Remove build artifacts
```

Or:

```sh
cmake -S . -B build
cmake --build build
./build/turnstile
```

## Usage

Once running, the program waits for inputs:

Valid inputs: `coin`, `push`, `exit` (to quit).

## Requirements

- CMake >= 3.16
- C++17-capable compiler
