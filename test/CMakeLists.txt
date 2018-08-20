set(TEST_SUBDIRS
  iso
)

set(TEST_DATA_DIRS
  test_data
)

add_subdirectory(iso)
include_directories(${GTEST_INCLUDE_DIR},
                    ${source_dir}/test/iso)


