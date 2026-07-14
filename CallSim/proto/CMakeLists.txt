add_library(proto_lib STATIC Message.proto)

protobuf_generate(TARGET proto_lib LANGUAGE cpp)

target_link_libraries(proto_lib PUBLIC protobuf::libprotobuf)
target_include_directories(proto_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})