cmake_minimum_required(VERSION 3.15)
project(CallSim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED CONFIG)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

enable_testing()

add_subdirectory(proto)
add_subdirectory(Client)
add_subdirectory(Server)