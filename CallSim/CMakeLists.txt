cmake_minimum_required(VERSION 3.10)
project(CallSimProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf CONFIG REQUIRED)

add_subdirectory(proto)

add_subdirectory(Client)
add_subdirectory(Server)