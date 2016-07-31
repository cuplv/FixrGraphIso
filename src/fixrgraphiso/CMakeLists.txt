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

# TODO: create a library for reuse


add_executable(fixrgraphiso
  main.cpp
  acdfg.cpp
  serialization.cpp
  ilpApproxIsomorphismEncoder.cpp
  milpProblem.cpp
  ${PROTO_SRCS}
)

target_link_libraries(fixrgraphiso
  ${FIXR_GRAPH_ISO_LINK_LIBS}
  ${PROTOBUF_LIBRARY}
  ${GLPK_LIBRARY}
)
