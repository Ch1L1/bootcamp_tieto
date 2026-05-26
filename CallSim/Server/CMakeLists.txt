add_executable(server src/server.cpp)

target_include_directories(server PRIVATE inc)

target_link_libraries(server PRIVATE callsim_proto)