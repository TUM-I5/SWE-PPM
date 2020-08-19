cmake_minimum_required(VERSION 3.10)
#set(CMAKE_CXX_COMPILER "mpicc")


project(swe_benchmark)

set(CMAKE_CXX_STANDARD 14)

#set(BUILDS Hpx Chameleon Upcxx Mpi)
set(BUILDS MpiOverdecomp)

#set(BUILDS Hpx Mpi)

include_directories(src)
include(FindNetCDF.cmake)
include(Structure.cmake)

add_definitions(-DWRITENETCDF) #-DVECTORIZE
list(APPEND SOURCE_FILES ${MAIN_FILE} )

file(GLOB SCENARIO_FILES ${SCENARIOS}/*)
file(GLOB WRITER_FILES ${WRITER}/*)
file(GLOB TYPE_FILES ${TYPES}/*)

option(ENABLE_MEMORY_SANITATION     "Enable GCC Address sanitation. Only supported with GCC toolchain." OFF)
option(ENABLE_VECTORIZATION         "Enable auto-vectorization of the block iteration on the CPU (only supported solvers), and in Release Mode." ON)
option(ENABLE_VECTORIZATION_VERBOSE "Enable verbose auto-vectorization reporting." OFF)

find_package(OpenMP REQUIRED)


foreach( build_type ${BUILDS})
    unset(SOURCE_FILES)
    include(Build${build_type}.cmake)



    set(SOLVER_FILES ${SOLVERS}/HLLEFun.hpp)
    list(APPEND SOURCE_FILES ${BLOCK_FILES} ${EXAMPLE_FILES} ${SCENARIO_FILES} ${WRITER_FILES} ${TYPE_FILES} ${SOLVER_FILES})
    string(TOLOWER ${build_type} build_type)
    #


    if( "${build_type}" STREQUAL "hpx" )
        add_hpx_executable(swe_bencchmark_hpx
                ESSENTIAL
                SOURCES ${SOURCE_FILES}
                COMPONENT_DEPENDENCIES iostreams
                DEPENDENCIES ${NETCDF_LIBRARIES} -lsvml -limf -lintlc)
    else()
        add_executable(swe_benchmark_${build_type} ${SOURCE_FILES})
        target_link_libraries(swe_benchmark_${build_type} PUBLIC ${NETCDF_LIBRARIES} "${${build_type}_link_libraries}" -lsvml -limf -lintlc)
    endif()
    #

    if (ENABLE_VECTORIZATION)
        # target_link_libraries(swe_benchmark_${build_type} PRIVATE OpenMP::OpenMP_CXX)
        target_compile_options(swe_benchmark_${build_type} PRIVATE 
            $<$<CXX_COMPILER_ID:Clang>: -march=native -fno-math-errno>
            $<$<CXX_COMPILER_ID:AppleClang>: -march=native -fno-math-errno>
            $<$<CXX_COMPILER_ID:Intel>: -xHost>
            $<$<CXX_COMPILER_ID:GNU>: -march=native -ffast-math -fassociative-math>
        )
        if(ENABLE_VECTORIZATION_VERBOSE)
            target_compile_options(swe_benchmark_${build_type} PRIVATE
                $<$<CXX_COMPILER_ID:AppleClang>: -Rpass-analysis=loop-vectorize>
                $<$<CXX_COMPILER_ID:Clang>: -Rpass-analysis=loop-vectorize>
                $<$<CXX_COMPILER_ID:Intel>: -qopt-report-phase=vec -qopt-report=2>
                $<$<CXX_COMPILER_ID:GNU>: -ftree-vectorizer-verbose=4 -fopt-info-all-vec>
            )
        endif()
        message(STATUS "Vectorization is enabled (Only for Release Mode).")
    endif()

    if (ENABLE_MEMORY_SANITATION AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        message(STATUS "Memory sanitation enabled. Linking appropriate libraries.")
        target_compile_options(swe_benchmark_${build_type} PUBLIC -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize-address-use-after-scope -Wuninitialized -Wall -Wextra -pedantic -g)
        target_link_libraries(swe_benchmark_${build_type} PUBLIC -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -g)
        # For some old Ubuntu machines, you may need to use a different linker. Set the following variable: CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold
    elseif(ENABLE_MEMORY_SANITATION)
        message(STATUS "Full Memory sanitation only supported on GNU Environment. Trying to use intel fallback.")
        target_compile_options(swe_benchmark_${build_type} PUBLIC -g -traceback -check-uninit)
        target_link_libraries(swe_benchmark_${build_type} PUBLIC -g -traceback -check-uninit)
        
    endif()

    target_include_directories(swe_benchmark_${build_type} PUBLIC "${${build_type}_include_directories}" ${NETCDF_INCLUDE_DIRS} )
    if(OpenMP_FOUND)
       target_compile_options(swe_benchmark_${build_type} PRIVATE 
          $<$<CXX_COMPILER_ID:Intel>: -qopenmp>
          $<$<CXX_COMPILER_ID:GNU>: -fopenmp>
       )
 
      if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        target_link_libraries(swe_benchmark_${build_type} PRIVATE -fopenmp )
      elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
        target_link_libraries(swe_benchmark_${build_type} PRIVATE -qopenmp )	    
      endif()
    endif()
    target_compile_options(swe_benchmark_${build_type} PRIVATE "${${build_type}_compile_options}" -g -O2 -std=c++14 -fmessage-length=0  )

endforeach(build_type ${BUILDS})


file(MAKE_DIRECTORY output)
