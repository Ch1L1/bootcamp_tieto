add_executable(client src/client.cpp)

target_include_directories(client PRIVATE inc)

target_link_libraries(client PRIVATE callsim_proto)
