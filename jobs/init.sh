#!/bin/bash 
MODULES=( netcdf-hdf5-all/4.7_hdf5-1.10-intel19-impi python/2.7_intel gcc/8  hwloc/1.11 cmake/3.15.4 scons) 
UNMODULES=()
ENVVARS=(UPCXX_PATH=$HOME/upcxx-2020.3.0/~/upcxx-2020.3.0/ibv/ LIBFABRIC_PATH=/dss/dsshome1/lrz/sys/spack/staging/20.1/opt/x86_64/intel/19.0.5-gcc-fcpqmql/compilers_and_libraries_2019.5.281/linux/mpi/intel64/libfabric/lib/libfabric.so
GPI_PATH=~/GPI-2/build/ CHAM_PATH=~/chameleon/cham_build/ CHARM_PATH=~/charm/mpi-linux-x86_64-mpicxx/  DATA_PATH=$HOME/data/ ASAGI_PATH=$HOME/ASAGI HPX_PATH=$HOME/hpx_icc_mpi/ BOOST_ROOT=$HOME/boost_1_71_0)
for mod in "${UNMODULES[@]}"
do
module unload $mod
done

for mod in "${MODULES[@]}"
do
module load $mod
done
for mod in "${ENVVARS[@]}"
do
export $mod
done


