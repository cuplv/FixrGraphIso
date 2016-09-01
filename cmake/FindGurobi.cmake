# Try to find the Gurobi libraries and the includes

find_path(GUROBI_header_PATH gurobi_c++.h
          /Library/gurobi652/mac64/include )

find_library(GUROBI_CPP_LIBRARY
	     NAMES gurobi_c++
	     PATHS  /Library/gurobi652/mac64/lib )


find_library(GUROBI_C_LIBRARY
	     NAMES gurobi65
	     PATHS  /Library/gurobi652/mac64/lib )
	     
if (GUROBI_header_PATH AND GUROBI_C_LIBRARY AND GUROBI_CPP_LIBRARY)
   set(GUROBI_FOUND "YES")
   set(GUROBI_INCLUDEDIR ${GUROBI_header_PATH})
   set(GUROBI_LIBRARIES "${GUROBI_CPP_LIBRARY} ${GUROBI_C_LIBRARY}")
endif(GUROBI_header_PATH AND GUROBI_C_LIBRARY AND GUROBI_CPP_LIBRARY)