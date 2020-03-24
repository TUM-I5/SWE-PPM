SET(HPX_DIR $ENV{HPX_PATH}/lib/cmake/HPX)

find_package(HPX REQUIRED)
set(BLOCK_FILES ${BLOCKS}/SWE_Block.hh ${BLOCKS}/SWE_DimensionalSplittingHpx.hh ${BLOCKS}/SWE_DimensionalSplittingHpx.cpp
                                        ${BLOCKS}/SWE_Hpx_No_Component.hpp ${BLOCKS}/SWE_Hpx_No_Component.cpp)
set(EXAMPLE_FILES ${EXAMPLES}/swe_hpx.cpp)

#add_hpx_component(block_component
 #       SOURCES ${BLOCKS}/SWE_DimensionalSplittingHpx.cpp
 #       HEADERS ${BLOCKS}/SWE_DimensionalSplittingHpx.hh
 #       COMPONENT_DEPENDENCIES iostreams)

#add_hpx_component(no_component
#        SOURCES ${BLOCKS}/SWE_Hpx_No_Component.cpp
#        HEADERS ${BLOCKS}/SWE_Hpx_No_Component.hpp
#        COMPONENT_DEPENDENCIES block_component iostreams)

#add_hpx_executable(swe_benchmark_hpx
 #       ESSENTIAL
#        SOURCES ${EXAMPLES}/swe_hpx.cpp ${BLOCK_FILES} ${SCENARIO_FILES} ${WRITER_FILES} ${TYPE_FILES} ${SOLVER_FILES}
 #       COMPONENT_DEPENDENCIES iostreams
 #       DEPENDENCIES ${NETCDF_LIBRARIES})