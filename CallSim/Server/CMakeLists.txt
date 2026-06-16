include_directories(inc ${CMAKE_BINARY_DIR}/proto)

add_executable(server_bin src/server.cpp)
target_link_libraries(server_bin PRIVATE proto_lib)

add_executable(server_tests test/ServerTests.cpp)
target_link_libraries(server_tests PRIVATE proto_lib GTest::gtest GTest::gtest_main Threads::Threads)

add_test(NAME ServerFSMTests COMMAND server_tests)