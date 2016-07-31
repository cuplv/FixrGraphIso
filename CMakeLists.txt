project(fixrgraphiso)
cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
if(NOT CMAKE_VERSION VERSION_LESS 2.8.12)
    cmake_policy(SET CMP0022 OLD)
endif()
if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0037 OLD)
endif()


#############################################################################
# the external libraries

# check for protobuf
include(FindProtobuf)
if (NOT PROTOBUF_FOUND)
  message(FATAL_ERROR "ERROR: protobuf not found")
endif(NOT PROTOBUF_FOUND)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

# check for GLPK
include(FindGLPK)
if (NOT GLPK_FOUND)
  message(FATAL_ERROR "ERROR: glpk not found")
endif(NOT GLPK_FOUND)
include_directories(${GLPK_INCLUDEDIR})


#############################################################################
# external dependencies

# protobuf from https://github.com/cuplv/FixrGraphExtractor
if (NOT FIXR_GRAPH_EXTRACTOR_DIRECTORY)
     message(FATAL_ERROR "Please specify -DFIXR_GRAPH_EXTRACTOR_DIRECTORY=<path to the FixrGraphExtractor project>")
endif()
set(FIXR_GRAPH_EXTRACTOR_DIRECTORY_PROTOBUF "${FIXR_GRAPH_EXTRACTOR_DIRECTORY}/src/main/protobuf")

message("Setting ${PROTOBUF_IMPORT_DIRS} to: ${FIXR_GRAPH_EXTRACTOR_DIRECTORY_PROTOBUF}")
set(PROTOBUF_IMPORT_DIRS "${FIXR_GRAPH_EXTRACTOR_DIRECTORY_PROTOBUF}")

#############################################################################
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_BINARY_DIR}/src)
add_subdirectory(src/fixrgraphiso)

#############################################################################
# Write the config file
# message("Writing configuration file")

set(incdirs
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_BINARY_DIR}/fixrgraphiso"
  "${PROJECT_SOURCE_DIR}/src/fixrgraphiso"
)  


