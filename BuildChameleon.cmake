find_package(MPI REQUIRED)

find_library(CHAMELEON_LIB chameleon HINTS "$ENV{CHAM_PATH}/lib")

list(APPEND chameleon_include_directories $ENV{CHAM_PATH}/include)

list(APPEND chameleon_link_libraries ${CHAMELEON_LIB})


list(APPEND chameleon_include_directories ${MPI_CXX_INCLUDE_PATH})
list(APPEND chameleon_compile_options ${MPI_CXX_COMPILE_FLAGS})
list(APPEND chameleon_link_libraries ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})

set(BLOCK_FILES ${BLOCKS}/SWE_Block.hh ${BLOCKS}/SWE_DimensionalSplittingChameleon.hh ${BLOCKS}/SWE_DimensionalSplittingChameleon.cpp)
set(EXAMPLE_FILES ${EXAMPLES}/swe_chameleon_new.cpp)
