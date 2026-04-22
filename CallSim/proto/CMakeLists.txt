# CallSim/proto/CMakeLists.txt
find_program(PROTOC_EXECUTABLE protoc REQUIRED)

set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/Message.pb.cc")
set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/Message.pb.h")

add_custom_command(
    OUTPUT ${PROTO_SRC} ${PROTO_HDR}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS --cpp_out="${CMAKE_CURRENT_BINARY_DIR}" 
         -I"${CMAKE_CURRENT_SOURCE_DIR}" 
         "${CMAKE_CURRENT_SOURCE_DIR}/Message.proto"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Message.proto"
)

add_library(callsim_proto ${PROTO_SRC} ${PROTO_HDR})

target_link_libraries(callsim_proto PUBLIC protobuf::libprotobuf)

target_include_directories(callsim_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})