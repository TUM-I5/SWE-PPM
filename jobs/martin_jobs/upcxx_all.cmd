#!/bin/bash
#NAME=upcxx_all
#SBATCH -o /dss/dsshome1/lxc01/ga84zod2/jobs/output/upcxx_all.%j.%N.out
#SBATCH -D /dss/dsshome1/lxc01/ga84zod2/swe-benchmark
#SBATCH -J upcxx_all
#SBATCH --get-user-env
#SBATCH --clusters=cm2
#SBATCH --partition=cm2_std
#SBATCH --nodes=4
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=28
#SBATCH --get-user-env
#SBATCH --mail-type=NONE
#SBATCH --export=NONE
#SBATCH --time=24:00:00
module load slurm_setup
source /etc/profile.d/modules.sh
source ~/init.sh
unset KMP_AFFINITY
export GASNET_PHYSMEM_MAX='55 GB'

XSIZE_GTS=(2048 2048 2048)
YSIZE_GTS=(2048 2048 2048)

XSIZE_WS_S=(1024 1448 2048)
YSIZE_WS_S=(1024 1448 2048)


XSIZE_WS_B=(2048 2896 4096)
YSIZE_WS_B=(2048 2896 4096)
NODES=(1 2 4)
NAME=upcxx_all
TIME=40
CP=40
OUTPUT="${HOME}/jobs/output/${NAME}"
CORESIZE=28
for index in 0 1 2  
do

MAX=10
for ((i=1;i<=MAX;i++)); do
echo "#####################################################################################"
echo "######################Simulate with $NODECOUNT Nodes #$i Time !######################"
echo "#####################################################################################"
NODECOUNT=${NODES[$index]}
PROCESSES=$((NODECOUNT*CORESIZE))

XSIZE=${XSIZE_GTS[$index]}
YSIZE=${YSIZE_GTS[$index]}
PARSTRING="-x $XSIZE -y $YSIZE -t ${TIME}  -l 0 -n $CP -o ${OUTPUT}"
$UPCXX_PATH/bin/upcxx-run -n $PROCESSES ~/test/swe-benchmark/build/swe_benchmark_upcxx ${PARSTRING}_upcxx_gts

XSIZE=${XSIZE_WS_S[$index]}
YSIZE=${YSIZE_WS_S[$index]}
PARSTRING="-x $XSIZE -y $YSIZE -t ${TIME}  -l 0 -n $CP -o ${OUTPUT}"
$UPCXX_PATH/bin/upcxx-run -n $PROCESSES ~/test/swe-benchmark/build/swe_benchmark_upcxx ${PARSTRING}_upcxx_ws_s

XSIZE=${XSIZE_WS_B[$index]}
YSIZE=${YSIZE_WS_B[$index]}
PARSTRING="-x $XSIZE -y $YSIZE -t ${TIME}  -l 0 -n $CP -o ${OUTPUT}"
$UPCXX_PATH/bin/upcxx-run -n $PROCESSES ~/test/swe-benchmark/build/swe_benchmark_upcxx ${PARSTRING}_upcxx_ws_b

done 
done 
