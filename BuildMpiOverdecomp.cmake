find_package(MPI REQUIRED)

list(APPEND mpi_include_directories ${MPI_CXX_INCLUDE_PATH})
list(APPEND mpi_compile_options ${MPI_CXX_COMPILE_FLAGS})
list(APPEND mpi_link_libraries ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})

set(BLOCK_FILES ${BLOCKS}/SWE_Block.hh ${BLOCKS}/SWE_DimensionalSplittingMPIOverdecomp.hh ${BLOCKS}/SWE_DimensionalSplittingMPIOverdecomp.cpp)
set(EXAMPLE_FILES ${EXAMPLES}/swe_mpi_overdecomp.cpp)
