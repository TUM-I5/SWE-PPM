#!/bin/bash
#NAME=lts_8192_8192_20
#SBATCH -o /dss/dsshome1/lxc03/di73gub/jobs/output/lts_8192_8192_20_04_charevariations.%j.%N.out
#SBATCH -D /dss/dsshome1/lxc03/di73gub/swe-benchmark
#SBATCH -J lts_8192_8192_20_04_cv
#SBATCH --get-user-env
#SBATCH --clusters=cm2_tiny
#SBATCH --partition=cm2_tiny
#SBATCH --nodes=4
#SBATCH --cpus-per-task=28
#SBATCH --ntasks-per-node=1
#SBATCH --get-user-env
#SBATCH --mail-type=NONE
#SBATCH --export=NONE
#SBATCH --time=03:00:00

module use ~/.modules
module load slurm_setup
#module load chameleon

source /etc/profile.d/modules.sh
source ~/swe-benchmark/jobs/init.sh
unset KMP_AFFINITY
#export MIN_ABS_LOAD_IMBALANCE_BEFORE_MIGRATION=56
export MIN_LOCAL_TASKS_IN_QUEUE_BEFORE_MIGRATION=56
export GASNET_PHYSMEM_MAX='55 GB'
export TAG_NBITS_TASK_ID=10
XSIZE=8192
YSIZE=8192
NAME=lts_8192_8192_20_04
TIME=2
CP=2
OUTPUT="/dss/dsshome1/lxc03/di73gub/jobs/output/${NAME}"
PARSTRING="-x $XSIZE -y $YSIZE -t ${TIME}  -l 1 -n $CP -o ${OUTPUT}"
PARSTRING_HPX="--resolution-horizontal $XSIZE --resolution-vertical $YSIZE -e $TIME -n $CP --local-timestepping 1 -o $OUTPUT"
CORESIZE=28
for NODECOUNT in 4
do
NUM_PES=$((CORESIZE*NODECOUNT))
CHARES_L=$((4*128))

echo "Running SWE on ${NUM_PES} with ${CHARES_S} and ${CHARES_L} chares."

MAX=3
for ((i=1;i<=MAX;i++)); do
echo "#####################################################################################"
echo "######################Simulate with $NODECOUNT Nodes #$i Time !######################"
echo "#####################################################################################"

for CHARES_PER_NODE in 32 64 128 256 512 1024 2048 4096; do
CHARES=$((${NODECOUNT} * ${CHARES_PER_NODE}))
/dss/dsshome1/lxc03/di73gub/charm/verbs-linux-x86_64-icc/bin/charmrun ++mpiexec ++p {NUM_PES} /dss/dsshome1/lxc03/di73gub/swe-benchmark/build/SWE_intel_release_charm_augrie_vec ${PARSTRING}_charmpp_${CHARES} -c ${CHARES}
done

done 
done 
