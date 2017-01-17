# Try to find the Gurobi libraries and the includes


find_path(GUROBI_header_PATH gurobi_c++.h
          /home/ubuntu/gurobi701/linux64/include )

find_library(GUROBI_C_LIBRARY
	     NAMES gurobi70
	     PATHS  /home/ubuntu/gurobi701/linux64/lib )

find_library(GUROBI_CPP_LIBRARY
             NAMES gurobi_c++
             PATHS /home/ubuntu/gurobi701/linux64/lib )

	     
if (GUROBI_header_PATH AND GUROBI_C_LIBRARY AND GUROBI_CPP_LIBRARY)
   set(GUROBI_FOUND "YES")
   set(GUROBI_INCLUDEDIR ${GUROBI_header_PATH})
   set(GUROBI_LIBRARIES "${GUROBI_CPP_LIBRARY} ${GUROBI_C_LIBRARY}")
endif(GUROBI_header_PATH AND GUROBI_C_LIBRARY AND GUROBI_CPP_LIBRARY)