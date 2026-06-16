include_directories(inc ${CMAKE_BINARY_DIR}/proto)

add_executable(client_bin src/client.cpp)
target_link_libraries(client_bin PRIVATE proto_lib)

add_executable(client_tests test/ClientTests.cpp)
target_link_libraries(client_tests PRIVATE proto_lib GTest::gtest GTest::gtest_main Threads::Threads)

add_test(NAME ClientFSMTests COMMAND client_tests)