add_executable(client src/client.cpp)
target_include_directories(client PRIVATE inc)
target_link_libraries(client PRIVATE callsim_proto)

find_package(GTest REQUIRED)

add_executable(client_test test/proto_test.cpp)

target_include_directories(client_test PRIVATE inc)

target_link_libraries(client_test 
    PRIVATE 
    callsim_proto 
    GTest::gtest 
    GTest::gtest_main
)

add_test(NAME ClientProtoTest COMMAND client_test)