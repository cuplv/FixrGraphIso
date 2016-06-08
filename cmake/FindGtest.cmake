# try to find the Google Test framework
#
# if successful, set the following variables
#  GTEST_FOUND
#  GTEST_INCLUDE_DIR
#  GTEST_LIBRARY
#  GTEST_MAIN_LIBRARY

find_path(GTEST_INCLUDE_DIR gtest/gtest.h
          ${GTESTINCLUDEDIR} # this to allow the user to provide a custom
                             # location for Gtest
          /usr/include /usr/local/include)

find_library(GTEST_LIBRARY
             NAMES gtest
             PATHS ${GTESTLIBDIR} # this to allow the user to provide a
                                  # custom location for Gtest
                   /usr/lib /usr/local/lib)

find_library(GTEST_MAIN_LIBRARY
             NAMES gtest_main
             PATHS ${GTESTLIBDIR}
                   /usr/lib /usr/local/lib)

if(GTEST_INCLUDE_DIR AND GTEST_LIBRARY)
    set(GTEST_FOUND "YES")
endif(GTEST_INCLUDE_DIR AND GTEST_LIBRARY)
