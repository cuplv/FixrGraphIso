file(GLOB SRCS *.cpp)

add_executable(isoTest ${SRCS})

message("${TEST_LIBS}")
target_link_libraries(isoTest ${TEST_LIBS} GTest::GTest GTest::Main)
target_compile_features(isoTest PRIVATE cxx_range_for)

message("Adding test")
add_test(isoTest isoTest --acdfg-path=${CMAKE_CURRENT_BINARY_DIR})


