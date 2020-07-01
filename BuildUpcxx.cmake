SET(UPCXX_DIR $ENV{UPCXX_PATH}/share/cmake/UPCXX/)
find_package(UPCXX REQUIRED)

set(upcxx_compile_options -DUPCXX)

list(APPEND upcxx_link_libraries  ${UPCXX_LIBRARIES} )

set(BLOCK_FILES ${BLOCKS}/SWE_Block.hh ${BLOCKS}/SWE_DimensionalSplittingUpcxx.hh ${BLOCKS}/SWE_DimensionalSplittingUpcxx.cpp)
set(EXAMPLE_FILES ${EXAMPLES}/swe_upcxx.cpp)

