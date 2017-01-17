# Try to find the Gurobi libraries and the includes


find_path(GUROBI_header_PATH
          NAMES gurobi_c++.h
          PATHS /home/ubuntu/gurobi701/linux64/include  /Library/gurobi701/mac64/include )



find_library(GUROBI_C_LIBRARY
	     NAMES gurobi70
	     PATHS  /Library/gurobi701/mac64 /home/ubuntu/gurobi701/linux64/lib )

find_library(GUROBI_CPP_LIBRARY
             NAMES gurobi_c++
             PATHS /Library/gurobi701/mac64 /home/ubuntu/gurobi701/linux64/lib )

	     
if (GUROBI_header_PATH AND GUROBI_C_LIBRARY AND GUROBI_CPP_LIBRARY)
   set(GUROBI_FOUND "YES")
   set(GUROBI_INCLUDEDIR ${GUROBI_header_PATH})
   set(GUROBI_LIBRARIES "${GUROBI_CPP_LIBRARY} ${GUROBI_C_LIBRARY}")
endif(GUROBI_header_PATH AND GUROBI_C_LIBRARY AND GUROBI_CPP_LIBRARY)