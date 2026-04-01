cmake_minimum_required(VERSION 3.10)
project(async_text_echo CXX)

find_package(Boost REQUIRED)
add_subdirectory(Client)
add_subdirectory(Server)
