cmake_minimum_required(VERSION 3.6)

project(swe_benchmark)



set(CMAKE_CXX_STANDARD 11)


#set(SOURCE_DIR src/blocks/*.hh src/scenarios/* src/solvers/* src/tools/* src/writer/* src/types/*)

#set(MAIN_FILE src/examples/swe_simple.cpp src/blocks/SWE_DimensionalSplitting.cpp)

#file(GLOB SOURCE_FILES ${SOURCE_DIR})
set(BUILDS Default)
include_directories(src)

include(FindNetCDF.cmake)

#add_definitions(-D_DECL_SWE_DimensionalSplittingCharm_H_ -D_DECL_swe_charm_H_ -DSWEDIMENSIONALSPLITTINGMPI_HH_)
list(APPEND SOURCE_FILES ${MAIN_FILE} )
MESSAGE(STATUS "${SOURCE_FILES}")




foreach( build_type ${BUILDS})
    include(Build${build_type}.cmake)
    string(TOLOWER ${build_type} build_type)
    add_executable(swe_benchmark_${build_type} ${SOURCE_FILES})
    target_link_libraries(swe_benchmark_${build_type} ${NETCDF_LIBRARIES})
endforeach(build_type ${BUILDS})


file(MAKE_DIRECTORY output)