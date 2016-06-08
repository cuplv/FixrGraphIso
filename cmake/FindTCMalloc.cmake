# try to find the TCMalloc library
#
# use TCMALLOC_LIB_DIR to customize the search path
#
# use TCMALLOC_PREFER_SHARED to force searching for the shared version of the
# library if possible

set(_tcmalloc_name tcmalloc)
if(TCMALLOC_PREFER_SHARED)
    set(_tcmalloc_name 
        ${CMAKE_SHARED_LIBRARY_PREFIX}tcmalloc${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

find_library(TCMALLOC_FOUND NAMES ${_tcmalloc_name} tcmalloc
    PATHS "${TCMALLOC_LIB_DIR}")

if(TCMALLOC_FOUND)
    set(TCMALLOC_LIBRARIES ${TCMALLOC_FOUND})
    message(STATUS "using tcmalloc library in ${TCMALLOC_FOUND}")
        
    if("${TCMALLOC_FOUND}" MATCHES ".*${CMAKE_STATIC_LIBRARY_SUFFIX}$")
        find_library(_tcmalloc_pthreads NAMES pthread)
        if(_tcmalloc_pthreads)
            message(STATUS "adding -pthread link flag for tcmalloc")
            set(TCMALLOC_LINK_FLAGS -pthread)
        endif()
        find_library(_tcmalloc_libunwind unwind PATHS "${TCMALLOC_LIB_DIR}")
        if(_tcmalloc_libunwind)
            set(_tcmalloc_gcc_eh_ok 1)
            set(_tcmalloc_stdcxx_ok 1)
            
            if(CMAKE_COMPILER_IS_GNUCXX)
                execute_process(COMMAND "${CMAKE_CXX_COMPILER}"
                    "-print-file-name=${CMAKE_STATIC_LIBRARY_PREFIX}gcc_eh${CMAKE_STATIC_LIBRARY_SUFFIX}"
                    RESULT_VARIABLE _tcmalloc_gcc_eh_ok
                    OUTPUT_VARIABLE _tcmalloc_gcc_eh
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
                execute_process(COMMAND "${CMAKE_CXX_COMPILER}"
                    "-print-file-name=${CMAKE_STATIC_LIBRARY_PREFIX}stdc++${CMAKE_STATIC_LIBRARY_SUFFIX}"
                    RESULT_VARIABLE _tcmalloc_stdcxx_ok
                    OUTPUT_VARIABLE _tcmalloc_stdcxx
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
            endif()

            if(_tcmalloc_gcc_eh_ok EQUAL 0 AND EXISTS "${_tcmalloc_gcc_eh}")
                message(STATUS
                    "adding gcc_eh library in ${_tcmalloc_gcc_eh} "
                    "for tcmalloc libunwind")
                set(TCMALLOC_LIBRARIES ${TCMALLOC_LIBRARIES}
                    "${_tcmalloc_gcc_eh}")
            endif()            
            message(STATUS
                "adding unwind library in ${_tcmalloc_libunwind} for tcmalloc")
            set(TCMALLOC_LIBRARIES ${TCMALLOC_LIBRARIES}
                "${_tcmalloc_libunwind}")
            if(_tcmalloc_stdcxx_ok EQUAL 0 AND EXISTS "${_tcmalloc_stdcxx_eh}")
                message(STATUS
                    "adding stdc++ library in ${_tcmalloc_stdcxx} "
                    "for tcmalloc libunwind")
                set(TCMALLOC_LIBRARIES ${TCMALLOC_LIBRARIES}
                    "${_tcmalloc_stdcxx}")
            endif()
            find_library(_tcmalloc_liblzma lzma PATHS "${TCMALLOC_LIB_DIR}")
            if(_tcmalloc_liblzma)
                message(STATUS
                    "adding lzma library in ${_tcmalloc_liblzma} for tcmalloc")
                set(TCMALLOC_LIBRARIES ${TCMALLOC_LIBRARIES}
                    "${_tcmalloc_liblzma}")
            endif()
        endif()
    endif()
else()
    message(STATUS "tcmalloc library not found")
endif()

mark_as_advanced(
    _tcmalloc_pthreads
    _tcmalloc_libunwind
    _tcmalloc_liblzma
    )
