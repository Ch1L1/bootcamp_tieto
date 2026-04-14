add_executable(server src/server.cpp)

target_include_directories(server PRIVATE inc)

target_link_libraries(server PRIVATE callsim_proto)

find_package(GTest REQUIRED)

add_executable(server_test test/proto_server_test.cpp)

target_link_libraries(server_test 
    PRIVATE 
    callsim_proto 
    GTest::gtest 
    GTest::gtest_main
)

add_test(NAME ServerProtoTest COMMAND server_test)