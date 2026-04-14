include(FindProtobuf)
find_package(Protobuf REQUIRED)

# We want the root to be the directory containing the 'callsim' package
set(PROTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(PROTO_FILE ${PROTO_ROOT}/callsim/v1/message.proto)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Manually run protoc so it recreates the folder structure
add_custom_command(
    OUTPUT "${GEN_DIR}/callsim/v1/message.pb.cc" 
           "${GEN_DIR}/callsim/v1/message.pb.h"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}/callsim/v1"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${GEN_DIR} --proto_path=${PROTO_ROOT} ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ protocol files with directory structure"
    VERBATIM
)

set(PROTO_SRCS "${GEN_DIR}/callsim/v1/message.pb.cc")
set(PROTO_HDRS "${GEN_DIR}/callsim/v1/message.pb.h")

add_library(callsim_proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(callsim_proto PUBLIC ${Protobuf_LIBRARIES})

# This allows your code to find "callsim/v1/message.pb.h"
target_include_directories(callsim_proto PUBLIC ${GEN_DIR})