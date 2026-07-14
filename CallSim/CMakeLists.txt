cmake_minimum_required(VERSION 3.14)
project(CallSim CXX)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


find_package(protobuf CONFIG QUIET)
if(NOT protobuf_FOUND)
    find_package(Protobuf REQUIRED)
    if(TARGET Protobuf::libprotobuf AND NOT TARGET protobuf::libprotobuf)
        add_library(protobuf::libprotobuf ALIAS Protobuf::libprotobuf)
    endif()
else()
    message(STATUS "Protobuf found via CONFIG mode")
endif()

find_package(Boost 1.70 REQUIRED)
find_package(Threads REQUIRED)

enable_testing()

add_subdirectory(proto)
add_subdirectory(Client)
add_subdirectory(Server)