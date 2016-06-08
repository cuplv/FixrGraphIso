#############################################################################
# build the sub-libraries
#############################################################################

# set(testprotobuf_SUBDIRS
#     utils
# )

include_directories(.)

#############################################################################
# process the protobuf file
#############################################################################
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTOBUF_IMPORT_DIRS}/proto_acdfg.proto)

#MESSAGE("PROTO SOURCES: ${PROTO_SRCS}")
#MESSAGE("PROTO HDRS: ${PROTO_HDRS}")

add_executable(fixrgraphiso
  main.cpp
  ${PROTO_SRCS}
)

target_link_libraries(fixrgraphiso
  ${PROTOBUF_LIBRARY}
)
