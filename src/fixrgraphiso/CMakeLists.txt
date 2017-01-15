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
PROTOBUF_GENERATE_CPP(PROTO_SRCS_ACDFG PROTO_HDRS_ACDFG ${PROTOBUF_IMPORT_DIRS}/proto_acdfg.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS_ISO PROTO_HDRS_ISO ${PROTOBUF_IMPORT_DIRS}/proto_iso.proto)

#MESSAGE("PROTO SOURCES: ${PROTO_SRCS}")
#MESSAGE("PROTO HDRS: ${PROTO_HDRS}")

# TODO: create a library for reuse


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
	
add_executable(frequentsubgraphs
   frequentSubgraphsMain.cpp
   isomorphismClass.cpp
   acdfg.cpp
   serialization.cpp
   ilpApproxIsomorphismEncoder.cpp
   milpProblem.cpp
   ${PROTO_SRCS_ISO}
   ${PROTO_SRCS_ACDFG}
)
target_compile_features(frequentsubgraphs PRIVATE cxx_range_for)


if (NOT GUROBI_FOUND)
   target_link_libraries(fixrgraphiso
    ${FIXR_GRAPH_ISO_LINK_LIBS}
    ${PROTOBUF_LIBRARY}
    ${GLPK_LIBRARY}
    )

    target_link_libraries(frequentsubgraphs
    ${GLPK_LIBRARY}
    ${PROTOBUF_LIBRARY}
    ${Z3_LIBRARY}
    )   
 
endif(NOT GUROBI_FOUND)

if (GUROBI_FOUND)
   target_link_libraries(fixrgraphiso
   ${FIXR_GRAPH_ISO_LINK_LIBS}
   ${PROTOBUF_LIBRARY}
   ${GLPK_LIBRARY}
   ${GUROBI_CPP_LIBRARY}
   ${GUROBI_C_LIBRARY}
   )

target_link_libraries(frequentsubgraphs
    ${GLPK_LIBRARY}	
   ${GUROBI_CPP_LIBRARY}	
   ${GUROBI_C_LIBRARY}
    ${PROTOBUF_LIBRARY}
    ${Z3_LIBRARY}
    ) 

endif(GUROBI_FOUND)

