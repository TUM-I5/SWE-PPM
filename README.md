SWE-PPM
===

Parallel implementations of Shallow Water Equations based on the teaching code(https://github.com/TUM-I5/SWE).

Documentation
-------------

The documentation is available in the [Wiki](https://github.com/TUM-I5/SWE/wiki)

System Requirements
------------
- CMake >= 3.10
- NetCDF
- OpenMP
- HPX >= 1.4 (https://github.com/STEllAR-GROUP/hpx)
- Charm++ (https://github.com/UIUC-PPL/charm)
- UPC++ >= 2020.3.0 (https://bitbucket.org/berkeleylab/upcxx)
- ASAGI (only required for usage with geographic scenarios) (https://github.com/TUM-I5/ASAGI)

Getting SWE-PPM
------------------
1. Clone the repository with following command: \
`git clone --recursive`
2. Checkout the *SC\_PAW-ATM\_Workshop\_submission* : \
`git checkout tags/SC_PAW-ATM_Workshop_submission -b master`

Installation
------------
1. Create build directory\
    `mkdir build && cd build`
2. Set necessary environment variables\
UPC++: \
`export UPCXX_PATH=\path\to\upcxx`\
Chameleon: \
`export CHAMELEON_PATH=\path\to\chameleon`
HPX: \
`export HPX_PATH=\path\to\hpx`\
3. Specify cmake targets and compile options\
    `cmake \-DBUILD_SWE_MPI=On \-DBUILD_SWE_UPCXX=On \-DBUILD_SWE_CHAMELEON=On \-DBUILD_SWE_HPX=On  ..`\
    For a complete list of configurable options call \
     `cmake -LAH ..`
4. Compile targets\
    `make` or `cmake --build . --target swe_benchmark_<target>`

Charm++ Installation
--------------
Unfortunately Charm++ still requires the old scons build system. 

1. Set environment variable \
`export CHARM_PATH=\path\to\charm`
2. Compile using scons \
`scons writeNetCDF=True compiler=intel solver=hybrid openmp=false parallelization=charm asagi=false copyenv=true vectorize=true`

Execution
------------
Note, that the below provided examples may vary depending on the system architecture and configuration of the frameworks.
The examples execute the compiled scenario with a **2048x2048 cell resolution**,80 seconds simulation duration, 20 checkpoints, **global time stepping** and file output enabled. 
- MPI: \
`mpirun -np 56 ./build/swe_benchmark_mpi --simulation-duration 80 --checkpoint-count 20 --resolution-horizontal 2048 --resolution-vertical 2048 --output-basepath ./output/mpi_gts --local-timestepping 0 --write 1`
- UPC++: \
`$UPCXX_PATH/bin/upcxx-run -np 56 ./build/swe_benchmark_upcxx --simulation-duration 80 --checkpoint-count 20 --resolution-horizontal 2048 --resolution-vertical 2048 --output-basepath ./output/upcxx_gts --local-timestepping 0 --write 1`
- Charm++:\
`$CHARM_PATH/bin/charmrun +p56 ./build/SWE_intel_release_charm_hybrid_vec --simulation-duration 80 --checkpoint-count 20 --resolution-horizontal 2048 --resolution-vertical 2048 --output-basepath ./output/charm_gts --local-timestepping 0 --write 1 --chares 56`
- Chameleon: \
`I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=27 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -np 56 ./build/swe_benchmark_upcxx --simulation-duration 80 --checkpoint-count 20 --resolution-horizontal 2048 --resolution-vertical 2048 --output-basepath ./output/chameleon_gts --local-timestepping 0 --write 1 --blocks 28`
- HPX: \
`mpirun -np 56 ./build/swe_benchmark_hpx --simulation-duration 80 --checkpoint-count 20 --resolution-horizontal 2048 --resolution-vertical 2048 --output-basepath ./output/hpx_gts --local-timestepping 0 --write 1 --blocks 28`

More examples of execution scripts can be found at https://github.com/TUM-I5/SWE-PPM/tree/master/jobs

Contributors
-------------
- Michael Bader
- Martin Bogusz
- Philipp Samfaß
- Alexander Pöppl

License
-------

SWE is release unter GPLv3 (see [gpl.txt](gpl.txt))
