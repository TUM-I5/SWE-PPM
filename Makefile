all: test clean

simulate:
	./build/SWE_gnu_release_none_hybrid -t 40 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/tohu_1m_baseline

simulate_upcxx:
	${UPCXX_PATH}/bin/upcxx-run -n 4 ./build/SWE_gnu_release_upcxx_hybrid -t 3600 -n 80 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/tohu_1m_new -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_mpi:
	mpirun -np 2 ./build/SWE_gnu_release_mpi_hybrid -t 3600 -n 80 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/mpi -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_ampi:
	./charmrun +p4 ./build/SWE_gnu_release_mpi_hybrid -t 40 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/tohu_1m +vp4

baseline:
	scons writeNetCDF=True openmp=True solver=hybrid

upcxx:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=upcxx asagi=true asagiDir=${ASAGI_PATH}

mpi:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=mpi asagi=true asagiDir=${ASAGI_PATH}

ampi:
	scons writeNetCDF=True openmp=False solver=hybrid parallelization=ampi

netcdf_cluster_upcxx:
	scons readNetCDF_tsunami=True writeNetCDF=True openmp=True solver=hybrid parallelization=upcxx netCDFDir=${NETCDF_BASE}





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
