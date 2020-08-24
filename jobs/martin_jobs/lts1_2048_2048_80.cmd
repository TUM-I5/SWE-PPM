#!/bin/bash
#NAME=lts_2048_2048_80
#SBATCH -o /dss/dsshome1/lxc01/ga84zod2/jobs/output/lts_2048_2048_80.%j.%N.out
#SBATCH -D /dss/dsshome1/lxc01/ga84zod2/swe-benchmark
#SBATCH -J lts_2048_2048_80
#SBATCH --get-user-env
#SBATCH --clusters=cm2
#SBATCH --partition=cm2_std
#SBATCH --nodes=8
#SBATCH --cpus-per-task=28
#SBATCH --ntasks-per-node=1
#SBATCH --get-user-env
#SBATCH --mail-type=NONE
#SBATCH --export=NONE
#SBATCH --time=24:00:00
module load slurm_setup
source /etc/profile.d/modules.sh
source ~/init.sh
unset KMP_AFFINITY
export MIN_ABS_LOAD_IMBALANCE_BEFORE_MIGRATION=56
export GASNET_PHYSMEM_MAX='55 GB'
XSIZE=2048
YSIZE=2048
NAME=lts_2048_2048_80
TIME=80
CP=20
OUTPUT="${HOME}/jobs/output/${NAME}"
PARSTRING="-x $XSIZE -y $YSIZE -t ${TIME}  -l 0 -n $CP -o ${OUTPUT}"
PARSTRING_HPX="--resolution-horizontal $XSIZE --resolution-vertical $YSIZE -e $TIME -n $CP --local-timestepping 1 -o $OUTPUT"
CORESIZE=28
for NODECOUNT in 1 2 4 8 
do

MAX=10
for ((i=1;i<=MAX;i++)); do
echo "#####################################################################################"
echo "######################Simulate with $NODECOUNT Nodes #$i Time !######################"
echo "#####################################################################################"
export SLURM_CPUS_PER_TASK=28
mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_1 --blocks 1
#mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_32 --blocks 32 
#mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_64 --blocks 64
#mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_128 --blocks 128

export SLURM_CPUS_PER_TASK=56
I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=27 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_chameleon ${PARSTRING}_chameleon_32 --blocks 32
I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=27 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_chameleon ${PARSTRING}_chameleon_64 --blocks 64
I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=27 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_chameleon ${PARSTRING}_chameleon_128 --blocks 128

done 
done 
