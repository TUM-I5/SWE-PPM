SET(UPCXX_INSTALL $ENV{UPCXX_PATH})

include(FindUPCXX.cmake)  #@todo use find_package(UPCXX) instead (only works with UPCXX >= 2019.9)
set(upcxx_compile_options -DUPCXX)

list(APPEND upcxx_link_libraries  ${UPCXX_LIBRARIES} )

set(BLOCK_FILES ${BLOCKS}/SWE_Block.hh ${BLOCKS}/SWE_DimensionalSplittingUpcxx.hh ${BLOCKS}/SWE_DimensionalSplittingUpcxx.cpp)
set(EXAMPLE_FILES ${EXAMPLES}/swe_upcxx.cpp)

