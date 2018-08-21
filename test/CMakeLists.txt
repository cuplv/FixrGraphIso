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

add_subdirectory(iso)
include_directories(${GTEST_INCLUDE_DIR},
                    ${source_dir}/test/iso)


