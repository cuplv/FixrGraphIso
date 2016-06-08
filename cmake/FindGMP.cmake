# try to find GMP (also with the C++ bindings)
#
# if successful, set the following variables
#  GMP_FOUND
#  GMPXX_FOUND
#  GMP_INCLUDEDIR
#  GMP_LIBRARIES
#  GMPXX_LIBRARIES

find_path(GMP_c_header_PATH gmp.h
          ${GMP_INCLUDE_DIR} # this to allow the user to provide a custom
                             # location for GMP
          /usr/include /usr/local/include)
find_path(GMP_cxx_header_PATH gmpxx.h
          ${GMP_INCLUDE_DIR}
          /usr/include /usr/local/include)

find_library(GMP_c_LIBRARY
             NAMES gmp
             PATHS ${GMP_LIB_DIR} # this to allow the user to provide a
                                  # custom location for GMP
                   /usr/lib /usr/local/lib)

find_library(GMP_cxx_LIBRARY
             NAMES gmpxx
             PATHS ${GMP_LIB_DIR}
                   /usr/lib /usr/local/lib)

if(GMP_c_header_PATH AND GMP_c_LIBRARY)
    set(GMP_FOUND "YES")
    set(GMP_INCLUDEDIR ${GMP_header_PATH})
    set(GMP_LIBRARIES ${GMP_c_LIBRARY})

    if(GMP_cxx_header_PATH AND GMP_cxx_LIBRARY)
        set(GMPXX_FOUND "YES")
        set(GMP_INCLUDEDIR ${GMP_INCLUDEDIR} ${GMP_cxx_header_PATH})
        set(GMPXX_LIBRARIES ${GMP_cxx_LIBRARY})
    endif(GMP_cxx_header_PATH AND GMP_cxx_LIBRARY)
endif(GMP_c_header_PATH AND GMP_c_LIBRARY)

mark_as_advanced(
  GMP_c_header_PATH
  GMP_cxx_header_PATH
  GMP_c_LIBRARY
  GMP_cxx_LIBRARY
)
