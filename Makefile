

all: 	mpi upcxx charm chameleon_asagi hpx

simulate_smp:
	./build/SWE_gnu_release_none_omp_hybrid -t 3600 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/tohu_1m_new -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc
simulate_upcxx:
	${UPCXX_PATH}/bin/upcxx-run -n 16 ./build/SWE_gnu_release_upcxx_hybrid_vec -t 1000 -n 20 -x 1000 -y 1000 -o ./mpi -b ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc -d ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc -l 1

simulate_upcxx_rad:
	${UPCXX_PATH}/bin/upcxx-run -n 56 ./build/SWE_gnu_release_upcxx_hybrid_vec -t 20 -n 20 -x 1024 -y 1024 -o ./output/upcxx -l 0 -w 1
simulate_upcxx_test:
	${UPCXX_PATH}/bin/upcxx-run -n 4 ./build/SWE_gnu_release_upcxx_hybrid -t 60 -n 10 -x 10 -y 10 -o ~/storage/tsunami/simulation/radial_upcxx

simulate_mpi:
	mpirun -np 4 ./build/SWE_gnu_release_mpi_hybrid_vec -t 1000 -n 20 -x 1000 -y 1000 -o ./mpi -b ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc -d ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc -l 1

simulate_mpi_lts:
	mpirun -np 56 ./build/SWE_intel_release_mpi_hybrid_vec -t 20 -n 20 -x 2048 -y 2048 -o ./output/mpi_lts -l 1 -w 1
simulate_mpi_gts:
	mpirun -np 56 ./build/SWE_intel_release_mpi_hybrid_vec -t 40 -n 40 -x 1024 -y 1024 -o ./output/mpi_gts -l 0 -w 1
simulate_hpx:
	./build/SWE_gnu_release_hpx_hybrid_vec -e 16000 -n 20 --resolution-horizontal 1024 --resolution-vertical 1024 -o ./mpi -b ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc -d ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc  --blocks 64 --local-timestepping 12 --write 1

simulate_hpx_gts:
	 ./build/SWE_intel_release_hpx_hybrid_vec -e 40 -n 20 --resolution-horizontal 2048 --resolution-vertical 2048 -o ./output/hpx --blocks 32 --local-timestepping 0
simulate_hpx_lts:
	 ./build/SWE_intel_release_hpx_hybrid_vec -e 20 -n 20 --resolution-horizontal 1024 --resolution-vertical 1024 -o ./output/hpx --blocks 32 --local-timestepping 1

simulate_chameleon:
	I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=11 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -np 2 ./build/SWE_gnu_release_chameleon_omp_hybrid_vec -t 1000 -n 20 -x 1000 -y 1000 -u 1 -v 1 -o ./mpi -b ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc -d ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc -l 1

simulate_chameleon_rad:
	I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=1 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -np 4 ./build/SWE_gnu_release_chameleon_omp_hybrid -t 20 -n 20 -x 1024 -y 1024 -l 1 -o ./output/chameleon  -i 8 -j 8 
simulate_ampi:
	./charmrun +p4 ./build/SWE_gnu_release_mpi_hybrid -t 3600 -n 20 -x 1000 -y 1000 -o ~/storage/tsunami/simulation/mpi -b /home/jurek/storage/tsunami/tohu_bath.nc -d /home/jurek/storage/tsunami/tohu_displ.nc

simulate_charm:
	mpirun -n 4  ./build/SWE_gnu_release_charm_hybrid_vec -t 1000 -n 20 -x 1000 -y 1000 -o ./mpi -b ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc -d ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc -l 1

simulate_charm_rad:
	${CHARM_PATH}/bin/charmrun +p4 ./build/SWE_gnu_release_charm_hybrid_vec -t 20 -n 20 -x 1024 -y 1024 -o ./charm -l 0 -w 0 --chares 32
simulate_charm_test:
	./charmrun +p4 ./build/SWE_gnu_release_charm_hybrid -t 60 -n 10 -x 10 -y 10 -o ~/storage/tsunami/simulation/radial_charm

debug_charm_test:
	/home/jurek/repository/tum/ccs_tools/bin/charmdebug +p4 ./build/SWE_gnu_release_charm_hybrid -t 60 -n 10 -x 10 -y 10 -o ~/storage/tsunami/simulation/charm

run_chameleon:
	OMP_NUM_THREADS=14  mpirun -np 2 ./build/SWE_gnu_release_chameleon_omp_hybrid -t 0.1 -n 1 -x 4096 -y 4096 -o ./output/test -i 200 -l 1

run_chameleon_asagi:
	OMP_NUM_THREADS=11 mpirun -np 2 ./build/SWE_gnu_release_chameleon_omp_hybrid_vec -t 0.1 -n 1 -x 4096 -y 4096 -o ./output/test -u 1 -v 1 -b ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc -d ${DATA_PATH}/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc -l 0

run_chameleon_test:
	I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=11 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -np 2 ./build/SWE_gnu_release_chameleon_omp_hybrid -t 1.0 -n 1 -x 320 -y 320 -l 1 -u 1 -v 1 -o ./output/test


#ampi:
#	scons writeNetCDF=True openmp=False solver=hybrid parallelization=ampi

smp:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=none asagi=true asagiDir=${ASAGI_PATH}

mpi_hybrid:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=mpi asagi=true asagiDir=${ASAGI_PATH} netCDFDir=${NETCDF_BASE} copyenv=true
mpi:
	scons writeNetCDF=True openmp=false solver=hybrid parallelization=mpi asagi=true asagiDir=${ASAGI_PATH} copyenv=true vectorize=true

mpi_load:
	scons writeNetCDF=True openmp=false solver=augrie parallelization=mpi asagi=false asagiDir=${ASAGI_PATH} copyenv=true
mpi_rad:
	scons writeNetCDF=True compiler=intel openmp=false solver=hybrid parallelization=mpi asagi=false copyenv=true showVectorization=true vectorize=true
mpi_debug:
	scons compileMode=debug writeNetCDF=True openmp=false solver=hybrid parallelization=mpi asagi=true asagiDir=${ASAGI_PATH} copyenv=true vectorize=true

upcxx_hybrid:
	scons writeNetCDF=True openmp=True solver=hybrid parallelization=upcxx asagi=true asagiDir=${ASAGI_PATH} netCDFDir=${NETCDF_BASE} copyenv=true
upcxx:
	scons writeNetCDF=True compiler=intel openmp=false solver=hybrid parallelization=upcxx asagi=true asagiDir=${ASAGI_PATH} copyenv=true showVectorization=true vectorize=true 

upcxx_load:
	scons writeNetCDF=True openmp=false solver=augrie parallelization=upcxx asagi=false copyenv=true showVectorization=true vectorize=true 
upcxx_rad:
	scons writeNetCDF=True compiler=intel openmp=false solver=hybrid parallelization=upcxx asagi=false copyenv=true showVectorization=true vectorize=true 
upcxx_debug:
	scons compileMode=debug writeNetCDF=True openmp=false solver=hybrid parallelization=upcxx asagi=true asagiDir=${ASAGI_PATH} copyenv=true
hpx:
	scons writeNetCDF=True openmp=false solver=hybrid parallelization=hpx asagi=true asagiDir=${ASAGI_PATH} copyenv=true vectorize=true
hpx_rad:
	scons writeNetCDF=True compiler=intel openmp=false solver=hybrid parallelization=hpx asagi=false asagiDir=${ASAGI_PATH} copyenv=true showVectorization=true vectorize=true
hpx_load:
	scons writeNetCDF=False openmp=false solver=augrie parallelization=hpx asagi=false asagiDir=${ASAGI_PATH} copyenv=true
charm:
	scons writeNetCDF=True openmp=false solver=hybrid parallelization=charm asagi=true asagiDir=${ASAGI_PATH}  copyenv=true vectorize=true

charm_hybrid:
	scons writeNetCDF=True openmp=true solver=hybrid parallelization=charm asagi=true asagiDir=${ASAGI_PATH}  copyenv=true

charm_load:
	scons writeNetCDF=True openmp=false solver=augrie parallelization=charm asagi=false asagiDir=${ASAGI_PATH}  copyenv=true

charm_rad:
	scons writeNetCDF=True compiler=intel openmp=false solver=hybrid parallelization=charm asagi=false  copyenv=true showVectorization=true vectorize=true
chameleon_rad:
	scons writeNetCDF=True compiler=intel solver=hybrid openmp=true parallelization=chameleon copyenv=true showVectorization=true vectorize=true

chameleon_gnu:
	scons writeNetCDF=True compiler=gnu solver=hybrid openmp=false parallelization=chameleon

chameleon_asagi:
	scons writeNetCDF=True solver=hybrid openmp=true parallelization=chameleon asagi=true asagiDir=${ASAGI_PATH} copyenv=true vectorize=true

chameleon_debug:
	scons writeNetCDF=True compiler=intel openmp=true parallelization=chameleon compileMode=debug

starpu:
	scons writeNetCDF=True compiler=intel openmp=true solver=hybrid parallelization=starpu
starpu_debug:
	scons writeNetCDF=True compiler=intel openmp=true solver=hybrid parallelization=starpu compileMode=debug
default:
	scons writeNetCDF=True solver=fwave



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
	rm -r -f build/*
	rm -f test/runner
	rm -f test/gdb
	rm -f test/*.cpp
	rm -f test/*.nc

.PHONY: clean valgrind ncgen_test artificial netcdf
