file(GLOB SRCS *.cpp)

add_executable(searchTest ${SRCS})
target_link_libraries(searchTest ${TEST_LIBS} GTest::GTest GTest::Main)
target_compile_features(searchTest PRIVATE cxx_range_for)

add_test(searchTest searchTest)

