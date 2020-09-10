#!/bin/bash
#NAME=charm1_migartion_count
#SBATCH -o /dss/dsshome1/lxc01/ga84zod2/jobs/output/charm1_migartion_count.%j.%N.out
#SBATCH -D /dss/dsshome1/lxc01/ga84zod2/swe-benchmark
#SBATCH -J charm1_migartion_count
#SBATCH --get-user-env
#SBATCH --clusters=cm2
#SBATCH --partition=cm2_std
#SBATCH --nodes=8
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=28
#SBATCH --get-user-env
#SBATCH --mail-type=NONE
#SBATCH --export=NONE
#SBATCH --time=1:00:00
module load slurm_setup
source /etc/profile.d/modules.sh
source ~/init.sh
unset KMP_AFFINITY
export GASNET_PHYSMEM_MAX='55 GB'
XSIZE=4096
YSIZE=4096
NAME=charm1_migartion_count
TIME=20
CP=20
OUTPUT="${HOME}/jobs/output/${NAME}"
PARSTRING="-x $XSIZE -y $YSIZE -t ${TIME}  -l 1 -n $CP -o ${OUTPUT}"
PARSTRING_GTS="-x $XSIZE -y $YSIZE -t ${TIME}  -l 0 -n $CP -o ${OUTPUT}"
CORESIZE=28
for NODECOUNT in 8
do

MAX=3
for ((i=1;i<=MAX;i++)); do
echo "#####################################################################################"
echo "######################Simulate with $NODECOUNT Nodes #$i Time !######################"
echo "#####################################################################################"

PROCESSES=$((NODECOUNT*CORESIZE))


CHARES=$((64*NODECOUNT))
$CHARM_PATH/bin/charmrun +p$PROCESSES  ~/swe-benchmark/build/SWE_intel_release_charm_hybrid_vec ${PARSTRING}_lts_greedy_charm_64 --chares $CHARES +balancer GreedyRefineLB +LBDebug1 > "${OUTPUT}_${NODECOUNT}_lts_greedy_charm_64"

CHARES=$((128*NODECOUNT))
$CHARM_PATH/bin/charmrun +p$PROCESSES  ~/swe-benchmark/build/SWE_intel_release_charm_hybrid_vec ${PARSTRING}_lts_greedy_charm_128 --chares $CHARES +balancer GreedyRefineLB  +LBDebug1 >  "${OUTPUT}_${NODECOUNT}_lts_greedy_charm_128"



CHARES=$((64*NODECOUNT))
$CHARM_PATH/bin/charmrun +p$PROCESSES  ~/swe-benchmark/build/SWE_intel_release_charm_hybrid_vec ${PARSTRING}_lts_refine_charm_64 --chares $CHARES +balancer RefineLB +LBDebug1 > "${OUTPUT}_${NODECOUNT}_lts_charm_64"

CHARES=$((128*NODECOUNT))
$CHARM_PATH/bin/charmrun +p$PROCESSES  ~/swe-benchmark/build/SWE_intel_release_charm_hybrid_vec ${PARSTRING}_lts_refine charm_128 --chares $CHARES +balancer RefineLB  +LBDebug1 >  "${OUTPUT}_${NODECOUNT}_lts_charm_128"




done 
done 
