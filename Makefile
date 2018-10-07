all: test clean

simulate_smp:
	./build/SWE_gnu_release_none_omp_hybrid -t 3600 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/tohu_1m_new -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_upcxx:
	${UPCXX_PATH}/bin/upcxx-run -n 4 ./build/SWE_gnu_release_upcxx_hybrid -t 3600 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/upcxx -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_upcxx_test:
	${UPCXX_PATH}/bin/upcxx-run -n 4 ./build/SWE_gnu_release_upcxx_hybrid -t 60 -n 10 -x 10 -y 10 -o ~/storage/tsunami/simulation/radial_upcxx

simulate_mpi:
	OMP_NUM_THREADS=1 mpirun -np 4 ./build/SWE_gnu_release_mpi_hybrid -t 3600 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/mpi -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_ampi:
	./charmrun +p4 ./build/SWE_gnu_release_mpi_hybrid -t 3600 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/mpi -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_charm:
	./charmrun +p4 ./build/SWE_gnu_release_charm_omp_hybrid -t 3600 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/charm -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_charm_test:
	./charmrun +p4 ./build/SWE_gnu_release_charm_hybrid -t 60 -n 10 -x 10 -y 10 -o ~/storage/tsunami/simulation/radial_charm

debug_charm_test:
	/home/jurek/repository/tum/ccs_tools/bin/charmdebug +p4 ./build/SWE_gnu_release_charm_hybrid -t 60 -n 10 -x 10 -y 10 -o ~/storage/tsunami/simulation/charm

#ampi:
#	scons writeNetCDF=True openmp=False solver=hybrid parallelization=ampi

smp:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=none asagi=true asagiDir=${ASAGI_PATH}

mpi_hybrid:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=mpi asagi=true asagiDir=${ASAGI_PATH} netCDFDir=${NETCDF_BASE}
mpi:
	scons writeNetCDF=True openmp=false solver=hybrid parallelization=mpi asagi=true asagiDir=${ASAGI_PATH}

upcxx_hybrid:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=upcxx asagi=true asagiDir=${ASAGI_PATH} netCDFDir=${NETCDF_BASE}
upcxx:
	scons writeNetCDF=True openmp=false solver=hybrid parallelization=upcxx asagi=true asagiDir=${ASAGI_PATH}

charm:
	scons writeNetCDF=True openmp=true solver=hybrid parallelization=charm asagi=true asagiDir=${ASAGI_PATH}






test: test/runner ncgen_test
	-./test/runner

valgrind: test/runner.cpp
	g++ -o test/mem -I$CXXTEST -Isrc -g -O0 test/runner.cpp src/blocks/SWE_DimensionalSplitting.cpp src/blocks/SWE_Block.cpp src/reader/netCdfReader.cpp -lnetcdf -fopenmp
	valgrind --leak-check=yes ./test/mem 2> ./test/valgrind.output
	rm -f test/mem

gdb: test/runner.cpp
	g++ -o test/gdb -I$CXXTEST -Isrc -g -O0 test/runner.cpp src/blocks/SWE_DimensionalSplitting.cpp src/blocks/SWE_Block.cpp src/reader/netCdfReader.cpp -lnetcdf -fopenmp

test/runner: test/runner.cpp
	g++ -o test/runner -I$CXXTEST -Isrc test/runner.cpp src/blocks/SWE_DimensionalSplitting.cpp src/blocks/SWE_Block.cpp src/reader/netCdfReader.cpp -lnetcdf -fopenmp

test/runner.cpp: test/*.h
	cxxtestgen --error-printer -o test/runner.cpp test/*.h

ncgen_test:
	ncgen -o test/testZero.nc test/res/testZero.cdl
	ncgen -o test/testOne.nc test/res/testOne.cdl
	ncgen -o test/testData.nc test/res/testData.cdl
	ncgen -o test/testDataLarge.nc test/res/testDataLarge.cdl
	ncgen -o test/testData_displacementZero.nc test/res/testData_displacementZero.cdl
	ncgen -o test/testSnapToGrid_displacementZero.nc test/res/testSnapToGrid_displacementZero.cdl
	ncgen -o test/testSnapToGrid.nc test/res/testSnapToGrid.cdl
	ncgen -o test/testData_displacementNotZero.nc test/res/testData_displacementNotZero.cdl
	ncgen -o test/testData_displacementUnequalToBath.nc test/res/testData_displacementUnequalToBath.cdl
	ncgen -o test/testDisplacedGrid.nc test/res/testDisplacedGrid.cdl
	ncgen -o test/testResumeable.nc test/res/testResumeable.cdl
	ncgen -o test/testFriction.nc test/res/testFriction.cdl

clean:
	rm -f test/runner
	rm -f test/gdb
	rm -f test/*.cpp
	rm -f test/*.nc

.PHONY: clean valgrind ncgen_test artificial netcdf
