include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS Message.proto)

add_library(callsim_proto STATIC 
    ${PROTO_SRCS} 
    ${PROTO_HDRS}
)

target_link_libraries(callsim_proto PUBLIC ${Protobuf_LIBRARIES})

target_include_directories(callsim_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})