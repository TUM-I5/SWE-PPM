#!/bin/bash
#NAME=multi_node_weak_scaling
#SBATCH -o /home/hpc/pr63so/ga84zod2/jobs/output/multi_node_weak_scaling.%j.%N.out
#SBATCH -D swe-benchmark
#SBATCH -J multi_node_weak_scaling
#SBATCH --get-user-env
#SBATCH --clusters=mpp2
#SBATCH --nodes=8
#SBATCH --get-user-env
#SBATCH --mail-type=NONE
#SBATCH --export=NONE
#SBATCH --time=30:00:00

source /etc/profile.d/modules.sh
source ~/init.sh

export GASNET_PHYSMEM_MAX='55 GB'
NAME=multi_node_weak_scaling
TIME=5000
CP=80

NODECOUNT=(1 2 4 8)
XSIZE=(2000 3200 4000 6400)
YSIZE=(2000 2500 4000 5000)
BATH=~/data/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc
DISPL=~/data/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc
OUTPUT=$WORK/jobs/output/$NAME
REG_TIME="Wall\): ([0-9]*\.[0-9]*)s"
REG_FLOPS="Flops\(Total\): ([0-9]*\.[0-9]*)GFLOPS"
CORESIZE=28
echo "PROCESSES;CHARM;UPCXX;MPI" >> ~/jobs/output/${NAME}_time.csv
echo "PROCESSES;CHARM;UPCXX;MPI" >> ~/jobs/output/${NAME}_flops.csv
for index in {0..3}
do


PARSTRING="-x ${XSIZE[$index]} -y ${YSIZE[$index]} -o $OUTPUT -b $BATH -d $DISPL -t $TIME -n $CP"

CHARM_COUNT_TIME=0
MPI_COUNT_TIME=0
UPCXX_COUNT_TIME=0


CHARM_COUNT_FLOPS=0
MPI_COUNT_FLOPS=0
UPCXX_COUNT_FLOPS=0
MAX=10
for ((i=1;i<=MAX;i++)); do
echo "#####################################################################################"
echo "######################Simulate with $NODECOUNT Nodes #$i Time !######################"
echo "#####################################################################################"

PROCESSES=$((NODECOUNT[$index]*CORESIZE))

CHARM_OUTPUT=$(~/charm/mpi-linux-x86_64-mpicxx/bin/charmrun +p$PROCESSES  ~/swe-benchmark/build/SWE_gnu_release_charm_hybrid_vec $PARSTRING)
echo $CHARM_OUTPUT
if [[ $CHARM_OUTPUT =~ $REG_TIME ]]; then CHARM_TIME=${BASH_REMATCH[1]}; fi
if [[ $CHARM_OUTPUT =~ $REG_FLOPS ]]; then CHARM_FLOPS=${BASH_REMATCH[1]}; fi

UPCXX_OUTPUT=$(~/upcxx/ibv/bin/upcxx-run -n $PROCESSES ~/swe-benchmark/build/SWE_gnu_release_upcxx_hybrid_vec $PARSTRING)
echo $UPCXX_OUTPUT
if [[ $UPCXX_OUTPUT =~ $REG_TIME ]]; then UPCXX_TIME=${BASH_REMATCH[1]}; fi
if [[ $UPCXX_OUTPUT =~ $REG_FLOPS ]]; then UPCXX_FLOPS=${BASH_REMATCH[1]}; fi

MPI_OUTPUT=$(mpirun -n $PROCESSES ~/swe-benchmark/build/SWE_gnu_release_mpi_hybrid_vec $PARSTRING)
echo $MPI_OUTPUT
if [[ $MPI_OUTPUT =~ $REG_TIME ]]; then MPI_TIME=${BASH_REMATCH[1]}; fi
if [[ $MPI_OUTPUT =~ $REG_FLOPS ]]; then MPI_FLOPS=${BASH_REMATCH[1]}; fi


UPCXX_COUNT_TIME=$(bc -l <<< "$UPCXX_COUNT_TIME+$UPCXX_TIME")
CHARM_COUNT_TIME=$(bc -l <<< "$CHARM_COUNT_TIME+$CHARM_TIME")
MPI_COUNT_TIME=$(bc -l <<< "$MPI_COUNT_TIME+$MPI_TIME")

UPCXX_COUNT_FLOPS=$(bc -l <<< "$UPCXX_COUNT_FLOPS+$UPCXX_FLOPS")
CHARM_COUNT_FLOPS=$(bc -l <<< "$CHARM_COUNT_FLOPS+$CHARM_FLOPS")
MPI_COUNT_FLOPS=$(bc -l <<< "$MPI_COUNT_FLOPS+$MPI_FLOPS")

done 

UPCXX_TIME=$(bc -l <<< "$UPCXX_COUNT_TIME/$MAX")
CHARM_TIME=$(bc -l <<< "$CHARM_COUNT_TIME/$MAX")
MPI_TIME=$(bc -l <<< "$MPI_COUNT_TIME/$MAX")

UPCXX_FLOPS=$(bc -l <<< "$UPCXX_COUNT_FLOPS/$MAX")
CHARM_FLOPS=$(bc -l <<< "$CHARM_COUNT_FLOPS/$MAX")
MPI_FLOPS=$(bc -l <<< "$MPI_COUNT_FLOPS/$MAX")

echo "$PROCESSES;$CHARM_TIME;$UPCXX_TIME;$MPI_TIME;" >> ~/jobs/output/${NAME}_time.csv
echo "$PROCESSES;$CHARM_FLOPS;$UPCXX_FLOPS;$MPI_FLOPS" >> ~/jobs/output/${NAME}_flops.csv

done 
