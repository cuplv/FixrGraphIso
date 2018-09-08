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
set(FIXRISO_PROTOBUF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protobuf")
message("${FIXRISO_PROTOBUF_DIR}/proto_acdfg_bin.proto")

PROTOBUF_GENERATE_CPP(PROTO_SRCS_ACDFG PROTO_HDRS_ACDFG ${PROTOBUF_IMPORT_DIRS}/proto_acdfg.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS_ISO PROTO_HDRS_ISO ${PROTOBUF_IMPORT_DIRS}/proto_iso.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS_UNWEIGHTEDISO PROTO_HDRS_UNWEIGHTEDISO ${FIXRISO_PROTOBUF_DIR}/proto_unweighted_iso.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS_ACDFGBIN PROTO_HDRS_ACDFGBIN ${FIXRISO_PROTOBUF_DIR}/proto_acdfg_bin.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS_SEARCH PROTO_HDRS_SEARCH ${FIXRISO_PROTOBUF_DIR}/proto_search.proto)

if (NOT GUROBI_FOUND)
   set(LP_LIBRARY ${GLPK_LIBRARY})
else()
   set(LP_LIBRARY ${GLPK_LIBRARY} ${GUROBI_CPP_LIBRARY} ${GUROBI_C_LIBRARY})
endif(NOT GUROBI_FOUND)


add_executable(fixrgraphiso
  main.cpp
  explicitTests.cpp
  acdfg.cpp
  serialization.cpp
  ilpApproxIsomorphismEncoder.cpp
  milpProblem.cpp
  ${PROTO_SRCS_ACDFG}
  ${PROTO_SRCS_ISO}
)
target_compile_features(fixrgraphiso PRIVATE cxx_range_for)

add_executable(frequentitemsets
  frequentItemSetsMain.cpp
  itemSetDB.cpp
  ${PROTO_SRCS_ISO}
  ${PROTO_SRCS_ACDFG}
)

target_compile_features(frequentitemsets PRIVATE cxx_range_for)
target_link_libraries(frequentitemsets
  ${PROTOBUF_LIBRARY}
    )

set(CMAKE_MACOSX_RPATH 1)
add_library(frequentsubgraphs_library SHARED
   acdfg.cpp
   acdfgBin.cpp
   frequentSubgraphs.cpp
   isomorphismClass.cpp
   serialization.cpp
   serializationLattice.cpp
   searchLattice.cpp
   ilpApproxIsomorphismEncoder.cpp
   milpProblem.cpp
   ${PROTO_SRCS_ISO}
   ${PROTO_SRCS_ACDFG}
   ${PROTO_SRCS_ACDFGBIN}
   ${PROTO_SRCS_UNWEIGHTEDISO}
   ${PROTO_SRCS_SEARCH}
)
target_compile_features(frequentsubgraphs_library PRIVATE cxx_range_for)
target_link_libraries(frequentsubgraphs_library
  ${LP_LIBRARY}
  ${PROTOBUF_LIBRARY}
  ${Z3_LIBRARY})

add_executable(frequentsubgraphs
   frequentSubgraphsMain.cpp
)
target_compile_features(frequentsubgraphs PRIVATE cxx_range_for)

add_executable(searchlattice
   searchLatticeMain.cpp
)
target_compile_features(searchlattice PRIVATE cxx_range_for)


target_link_libraries(fixrgraphiso
  ${FIXR_GRAPH_ISO_LINK_LIBS}
  ${PROTOBUF_LIBRARY}
  ${LP_LIBRARY}
)

target_link_libraries(frequentsubgraphs
  frequentsubgraphs_library
  ${LP_LIBRARY}
  ${PROTOBUF_LIBRARY}
  ${Z3_LIBRARY}
)

message(${LP_LIBRARY})

target_link_libraries(searchlattice
  frequentsubgraphs_library
  ${LP_LIBRARY}
  ${PROTOBUF_LIBRARY}
  ${Z3_LIBRARY}
)


