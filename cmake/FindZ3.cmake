# Try to find the Z3 libraries and C++ includes
#
# if successful, set the following variables
#  Z3_FOUND
#  Z3_INCLUDEDIR
#  Z3_LIBRARIES
#

find_path(Z3_header_PATH z3.h
          ${Z3_INCLUDE_DIR} # this to allow the user to provide a custom location for z3
          /usr/include /usr/local/include)

find_library(Z3_LIBRARY
             NAMES z3
             PATHS ${Z3_LIB_DIR} # this to allow the user to provide a custom location for z3
             /usr/lib /usr/local/lib)

if(Z3_header_PATH AND Z3_LIBRARY)
    set(Z3_FOUND "YES")
    set(Z3_INCLUDEDIR ${Z3_header_PATH})
    set(Z3_LIBRARIES ${Z3_LIBRARY})

endif(Z3_header_PATH AND Z3_LIBRARY)

mark_as_advanced(
  Z3_header_PATH
  Z3_LIBRARY
)

