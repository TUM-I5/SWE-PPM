cmake_minimum_required(VERSION 3.10)
#set(CMAKE_CXX_COMPILER "mpicc")


project(swe_benchmark)

set(CMAKE_CXX_STANDARD 14)

set(BUILDS Default Mpi Upcxx Hpx)

#set(BUILDS Hpx)

include_directories(src)
include(FindNetCDF.cmake)
include(Structure.cmake)

add_definitions(-DWRITENETCDF )#-DNDEBUG -DVECTORIZE
list(APPEND SOURCE_FILES ${MAIN_FILE} )

file(GLOB SCENARIO_FILES ${SCENARIOS}/*)
file(GLOB WRITER_FILES ${WRITER}/*)
file(GLOB TYPE_FILES ${TYPES}/*)


foreach( build_type ${BUILDS})
    unset(SOURCE_FILES)
    include(Build${build_type}.cmake)


    set(SOLVER_FILES ${SOLVERS}/HLLEFun.hpp)
    list(APPEND SOURCE_FILES ${BLOCK_FILES} ${EXAMPLE_FILES} ${SCENARIO_FILES} ${WRITER_FILES} ${TYPE_FILES} ${SOLVER_FILES})
    string(TOLOWER ${build_type} build_type)
    #
    if( "${build_type}" STREQUAL "hpx" )
        add_hpx_executable(swe_benchmark_hpx
                ESSENTIAL
                SOURCES ${SOURCE_FILES}
                COMPONENT_DEPENDENCIES iostreams
                DEPENDENCIES ${NETCDF_LIBRARIES})
    else()
        add_executable(swe_benchmark_${build_type} ${SOURCE_FILES})
        target_include_directories(swe_benchmark_${build_type} PUBLIC "${${build_type}_include_directories}" )
        target_link_libraries(swe_benchmark_${build_type} ${NETCDF_LIBRARIES} "${${build_type}_link_libraries}" )
    endif()
    #
    target_compile_options(swe_benchmark_${build_type} PUBLIC "${${build_type}_compile_options}" )#-O3 -std=c++14 -fmessage-length=0 -mtune=native -fstrict-aliasing -fargument-noalias)
    #

endforeach(build_type ${BUILDS})


file(MAKE_DIRECTORY output)
