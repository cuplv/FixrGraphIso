set(TEST_SUBDIRS
  iso
)

set(TEST_DATA_DIRS
  test_data
)

set(TEST_LIBS
    ${Z3_LIBRARY}
    ${GTEST_MAIN_LIBRARIES}
    frequentsubgraphs_library)

file(COPY test_data DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR})

file(COPY search_data DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/test_data/produced_res)

include_directories(${GTEST_INCLUDE_DIR})

add_subdirectory(iso)
add_subdirectory(frequentsubgraph)
add_subdirectory(search)

