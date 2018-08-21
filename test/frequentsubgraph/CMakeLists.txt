file(GLOB SRCS *.cpp)

add_executable(frequentSubgraphTest ${SRCS})
target_link_libraries(frequentSubgraphTest ${TEST_LIBS} GTest::GTest GTest::Main)
target_compile_features(frequentSubgraphTest PRIVATE cxx_range_for)

add_test(frequentSubgraphTest frequentSubgraphTest)

