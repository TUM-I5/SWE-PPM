#!/bin/bash
#NAME=lts_8192_8192_20_08
#SBATCH -o /dss/dsshome1/lxc0E/ga25qic2/jobs_mpioverdecomp_tasking/output/lts_8192_8192_20.%j.%N.out
#SBATCH -D /dss/dsshome1/lxc0E/ga25qic2/Codes/swe-benchmark
#SBATCH -J lts_8192_8192_20_08
#SBATCH --get-user-env
#SBATCH --clusters=cm2
#SBATCH --qos=cm2_std
#SBATCH --partition=cm2_std
#SBATCH --nodes=8
#SBATCH --cpus-per-task=28
#SBATCH --ntasks-per-node=1
#SBATCH --get-user-env
#SBATCH --mail-type=NONE
#SBATCH --export=NONE
#SBATCH --time=01:30:00

module use ~/.modules
module load slurm_setup
module load chameleon

source /etc/profile.d/modules.sh
source ~/Codes/swe-benchmark/jobs/init.sh
unset KMP_AFFINITY
#export MIN_ABS_LOAD_IMBALANCE_BEFORE_MIGRATION=56
export MIN_LOCAL_TASKS_IN_QUEUE_BEFORE_MIGRATION=56
export GASNET_PHYSMEM_MAX='55 GB'
export TAG_NBITS_TASK_ID=10
XSIZE=8192
YSIZE=8192
NAME=lts_8192_8192_20_08
TIME=20
CP=20
OUTPUT="${HOME}/jobsi_mpioverdecomp_tasking/output/${NAME}"
PARSTRING="-x $XSIZE -y $YSIZE -t ${TIME}  -l 1 -n $CP -o ${OUTPUT}"
PARSTRING_HPX="--resolution-horizontal $XSIZE --resolution-vertical $YSIZE -e $TIME -n $CP --local-timestepping 1 -o $OUTPUT"
CORESIZE=28
for NODECOUNT in 8
do

MAX=3
for ((i=1;i<=MAX;i++)); do
echo "#####################################################################################"
echo "######################Simulate with $NODECOUNT Nodes #$i Time !######################"
echo "#####################################################################################"
#mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_1 --blocks 1
#mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_32 --blocks 32 
#mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_64 --blocks 64
#mpirun -n $NODECOUNT  ~/swe-benchmark/build/swe_benchmark_hpx ${PARSTRING_HPX}_hpx_128 --blocks 128

export SLURM_CPUS_PER_TASK=56
#I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=28 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -n $NODECOUNT  ~/Codes/swe-benchmark/build/swe_benchmark_mpioverdecomp ${PARSTRING}_chameleon_32 --blocks 32
I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=28 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -n $NODECOUNT  ~/Codes/swe-benchmark/build/swe_benchmark_mpioverdecomp ${PARSTRING}_chameleon_64 --blocks 64
I_MPI_PIN=1 I_MPI_PIN_DOMAIN=auto OMP_NUM_THREADS=28 OMP_PLACES=cores OMP_PROC_BIND=close mpirun -n $NODECOUNT  ~/Codes/swe-benchmark/build/swe_benchmark_mpioverdecomp ${PARSTRING}_chameleon_128 --blocks 128

done 
done 
