file(GLOB SRCS *.cpp)

add_executable(isoTest ${SRCS})
target_link_libraries(isoTest ${TEST_LIBS} GTest::GTest GTest::Main)
target_compile_features(isoTest PRIVATE cxx_range_for)

add_test(isoTest isoTest)

