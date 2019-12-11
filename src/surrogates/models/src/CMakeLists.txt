#  _______________________________________________________________________
#
#  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
#  Copyright 2014 Sandia Corporation.
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

SET(SOURCES GaussianProcess.cpp rol_object.cpp
      DataScaler.cpp)
SET(HEADERS GaussianProcess.hpp rol_object.hpp
	    teuchos_data_types.hpp
      DataScaler.hpp)

include_directories(${Surrogates_SOURCE_DIR}/models/src
	            ${DAKOTA_INCDIRS}
	            ${Teuchos_INCLUDE_DIRS}
              ${ROL_INCLUDE_DIRS})

add_library(surrogates_models ${SOURCES} ${HEADERS})

target_link_libraries(surrogates_models ${Surrogates_LIBS})
