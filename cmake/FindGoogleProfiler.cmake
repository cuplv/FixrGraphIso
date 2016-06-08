# try to find the Google Profiler
#
# if successful, set the following variables
#  GOOGLE_PROFILER_FOUND
#  GOOGLE_PROFILER_INCDIR
#  GOOGLE_PROFILER_LIBRARY

find_path(GOOGLE_PROFILER_INCDIR google/profiler.h
          ${GOOGLE_PROFILER_INCLUDE_DIR} # this to allow the user to provide a
                                         # custom location for it
          /usr/include /usr/local/include)

find_library(GOOGLE_PROFILER_LIBRARY
             NAMES profiler
             PATHS ${GOOGLE_PROFILER_LIB_DIR} # this to allow the user to
                                              # provide a custom location for it
                   /usr/lib /usr/local/lib)

if(GOOGLE_PROFILER_INCDIR AND GOOGLE_PROFILER_LIBRARY)
    set(GOOGLE_PROFILER_FOUND "YES")
endif()
