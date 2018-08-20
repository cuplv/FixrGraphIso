file(GLOB SRCS *.cpp)

add_executable(isoTest ${SRCS})

# target_link_libraries(testiso
#     libfoo
#     libgtest
#     libgmock
# )

#install(TARGETS isoTest DESTINATION bin)

target_link_libraries(isoTest ${GTEST_MAIN_LIBRARIES} GTest::GTest GTest::Main)

message("Adding test")
add_test(isoTest isoTest)


