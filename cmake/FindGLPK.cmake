# Try to find the GLPK libraries and C++ includes
#
# if successful, set the following variables
#  GKPK_FOUND
#  GLPK_INCLUDEDIR
#  GLPK_LIBRARIES
#

find_path(GLPK_header_PATH glpk.h
          ${Z3_INCLUDE_DIR} # this to allow the user to provide a custom location for z3
          /usr/include /usr/local/include)

find_library(GLPK_LIBRARY
             NAMES glpk
             PATHS ${GLPK_LIB_DIR} # this to allow the user to provide a custom location for z3
             /usr/lib /usr/local/lib)

if(GLPK_header_PATH AND GLPK_LIBRARY)
    set(GLPK_FOUND "YES")
    set(GLPK_INCLUDEDIR ${GLPK_header_PATH})
    set(GLPK_LIBRARIES ${GLPK_LIBRARY})

endif(GLPK_header_PATH AND GLPK_LIBRARY)

mark_as_advanced(
  GLPK_header_PATH
  GLPK_LIBRARY
)

