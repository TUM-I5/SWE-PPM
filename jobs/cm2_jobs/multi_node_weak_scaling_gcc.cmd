#!/bin/bash
#NAME=multi_node_weak_scaling_gcc
#SBATCH -o /home/hpc/pr63so/ga84zod2/jobs/output/multi_node_weak_scaling_gcc.%j.%N.out
#SBATCH -D swe_gcc
#SBATCH -J multi_node_weak_scaling_gcc
#SBATCH --get-user-env
#SBATCH --clusters=mpp2
#SBATCH --nodes=8
#SBATCH --get-user-env
#SBATCH --mail-type=NONE
#SBATCH --export=NONE
#SBATCH --time=48:00:00

source /etc/profile.d/modules.sh
source ~/init.sh

export GASNET_PHYSMEM_MAX='55 GB'
XSIZE=(2000 3200 4000 6400)
YSIZE=(2000 2500 4000 5000)
NODECOUNT=(1 2 4 8)
NAME=multi_node_weak_scaling_gcc
TIME=5000
CP=80
BATH=~/data/tohoku_gebco_ucsb3_2000m_hawaii_bath.nc
DISPL=~/data/tohoku_gebco_ucsb3_2000m_hawaii_displ.nc
OUTPUT=$WORK/jobs/output/$NAME
PARSTRING="-x $XSIZE -y $YSIZE -o $OUTPUT -b $BATH -d $DISPL -t $TIME -n $CP"
PARSTRING_HPX="--resolution-horizontal $XSIZE --resolution-vertical $YSIZE -b $BATH -d $DISPL -e $TIME -n $CP --blocks 28"
REG_TIME="Wall\): ([0-9]*\.[0-9]*)s"

REG_COMM="Communication Time\(Total\): ([0-9]*\.[0-9]*)s"
REG_RED="Reduction Time\(Total\): ([0-9]*\.[0-9]*)s"
REG_FLOPS="Flops\(Total\): ([0-9]*\.[0-9]*)GFLOPS"
CORESIZE=28
echo "PROCESSES;CHARM;UPCXX;MPI;HPX" >> ~/jobs/output/${NAME}_comm.csv
echo "PROCESSES;CHARM;UPCXX;MPI;HPX" >> ~/jobs/output/${NAME}_time.csv
echo "PROCESSES;CHARM;UPCXX;MPI;HPX" >> ~/jobs/output/${NAME}_flops.csv
echo "PROCESSES;CHARM;UPCXX;MPI;HPX" >> ~/jobs/output/${NAME}_red.csv
for index in {0..3}
do

PARSTRING="-x ${XSIZE[$index]} -y ${YSIZE[$index]} -o $OUTPUT -b $BATH -d $DISPL -t $TIME -n $CP"

PARSTRING_HPX="--resolution-horizontal ${XSIZE[$index]} --resolution-vertical ${YSIZE[$index]} -b $BATH -d $DISPL -e $TIME -n $CP --blocks 28"
CHARM_COUNT_TIME=0
MPI_COUNT_TIME=0
UPCXX_COUNT_TIME=0


CHARM_COUNT_RED=0
MPI_COUNT_RED=0
UPCXX_COUNT_RED=0
HPX_COUNT_RED=0

CHARM_COUNT_COMM=0
MPI_COUNT_COMM=0
UPCXX_COUNT_COMM=0

CHARM_COUNT_FLOPS=0
MPI_COUNT_FLOPS=0
UPCXX_COUNT_FLOPS=0

HPX_COUNT_TIME=0
HPX_COUNT_FLOPS=0
HPX_COUNT_COMM=0
MAX=10
for ((i=1;i<=MAX;i++)); do
echo "#####################################################################################"
echo "######################Simulate with $NODECOUNT Nodes #$i Time !######################"
echo "#####################################################################################"


PROCESSES=$((NODECOUNT[$index]*CORESIZE))

CHARM_OUTPUT=$(mpirun -n $PROCESSES  ~/swe_gcc/build/SWE_gnu_release_charm_hybrid_vec $PARSTRING)
echo $CHARM_OUTPUT
if [[ $CHARM_OUTPUT =~ $REG_TIME ]]; then CHARM_TIME=${BASH_REMATCH[1]}; fi
if [[ $CHARM_OUTPUT =~ $REG_FLOPS ]]; then CHARM_FLOPS=${BASH_REMATCH[1]}; fi
if [[ $CHARM_OUTPUT =~ $REG_COMM ]]; then CHARM_COMM=${BASH_REMATCH[1]}; fi
if [[ $CHARM_OUTPUT =~ $REG_RED ]]; then CHARM_RED=${BASH_REMATCH[1]}; fi

UPCXX_OUTPUT=$(~/upcxx/gccibv/bin/upcxx-run -n $PROCESSES ~/swe_gcc/build/SWE_gnu_release_upcxx_hybrid_vec $PARSTRING)
echo $UPCXX_OUTPUT
if [[ $UPCXX_OUTPUT =~ $REG_TIME ]]; then UPCXX_TIME=${BASH_REMATCH[1]}; fi
if [[ $UPCXX_OUTPUT =~ $REG_FLOPS ]]; then UPCXX_FLOPS=${BASH_REMATCH[1]}; fi
if [[ $UPCXX_OUTPUT =~ $REG_COMM ]]; then UPCXX_COMM=${BASH_REMATCH[1]}; fi
if [[ $UPCXX_OUTPUT =~ $REG_RED ]]; then UPCXX_RED=${BASH_REMATCH[1]}; fi

MPI_OUTPUT=$(mpirun -n $PROCESSES ~/swe_gcc/build/SWE_gnu_release_mpi_hybrid_vec $PARSTRING)
echo $MPI_OUTPUT
if [[ $MPI_OUTPUT =~ $REG_TIME ]]; then MPI_TIME=${BASH_REMATCH[1]}; fi
if [[ $MPI_OUTPUT =~ $REG_FLOPS ]]; then MPI_FLOPS=${BASH_REMATCH[1]}; fi
if [[ $MPI_OUTPUT =~ $REG_COMM ]]; then MPI_COMM=${BASH_REMATCH[1]}; fi
if [[ $MPI_OUTPUT =~ $REG_RED ]]; then MPI_RED=${BASH_REMATCH[1]}; fi
HPX_OUTPUT=$(srun -n ${NODECOUNT[$index]} -c $CORESIZE ~/swe_gcc/build/SWE_gnu_release_hpx_hybrid $PARSTRING_HPX)
echo $HPX_OUTPUT
if [[ $HPX_OUTPUT =~ $REG_TIME ]]; then HPX_TIME=${BASH_REMATCH[1]}; fi
if [[ $HPX_OUTPUT =~ $REG_FLOPS ]]; then HPX_FLOPS=${BASH_REMATCH[1]}; fi
if [[ $HPX_OUTPUT =~ $REG_COMM ]]; then HPX_COMM=${BASH_REMATCH[1]}; fi
if [[ $HPX_OUTPUT =~ $REG_RED ]]; then HPX_RED=${BASH_REMATCH[1]}; fi

HPX_COUNT_TIME=$(bc -l <<< "$HPX_COUNT_TIME+$HPX_TIME")
HPX_COUNT_FLOPS=$(bc -l <<< "$HPX_COUNT_FLOPS+$HPX_FLOPS")
HPX_COUNT_COMM=$(bc -l <<< "$HPX_COUNT_COMM+$HPX_COMM")

UPCXX_COUNT_TIME=$(bc -l <<< "$UPCXX_COUNT_TIME+$UPCXX_TIME")
CHARM_COUNT_TIME=$(bc -l <<< "$CHARM_COUNT_TIME+$CHARM_TIME")
MPI_COUNT_TIME=$(bc -l <<< "$MPI_COUNT_TIME+$MPI_TIME")

HPX_COUNT_RED=$(bc -l <<< "$HPX_COUNT_COMM+$HPX_COMM")
UPCXX_COUNT_RED=$(bc -l <<< "$UPCXX_COUNT_RED+$UPCXX_RED")
CHARM_COUNT_RED=$(bc -l <<< "$CHARM_COUNT_RED+$CHARM_RED")
MPI_COUNT_RED=$(bc -l <<< "$MPI_COUNT_RED+$MPI_RED")

UPCXX_COUNT_COMM=$(bc -l <<< "$UPCXX_COUNT_COMM+$UPCXX_COMM")
CHARM_COUNT_COMM=$(bc -l <<< "$CHARM_COUNT_COMM+$CHARM_COMM")
MPI_COUNT_COMM=$(bc -l <<< "$MPI_COUNT_COMM+$MPI_COMM")

UPCXX_COUNT_FLOPS=$(bc -l <<< "$UPCXX_COUNT_FLOPS+$UPCXX_FLOPS")
CHARM_COUNT_FLOPS=$(bc -l <<< "$CHARM_COUNT_FLOPS+$CHARM_FLOPS")
MPI_COUNT_FLOPS=$(bc -l <<< "$MPI_COUNT_FLOPS+$MPI_FLOPS")

done 

UPCXX_TIME=$(bc -l <<< "$UPCXX_COUNT_TIME/$MAX")
CHARM_TIME=$(bc -l <<< "$CHARM_COUNT_TIME/$MAX")
MPI_TIME=$(bc -l <<< "$MPI_COUNT_TIME/$MAX")

UPCXX_COMM=$(bc -l <<< "$UPCXX_COUNT_COMM/$MAX")
CHARM_COMM=$(bc -l <<< "$CHARM_COUNT_COMM/$MAX")
MPI_COMM=$(bc -l <<< "$MPI_COUNT_COMM/$MAX")

UPCXX_FLOPS=$(bc -l <<< "$UPCXX_COUNT_FLOPS/$MAX")
CHARM_FLOPS=$(bc -l <<< "$CHARM_COUNT_FLOPS/$MAX")
MPI_FLOPS=$(bc -l <<< "$MPI_COUNT_FLOPS/$MAX")

HPX_TIME=$(bc -l <<< "$HPX_COUNT_TIME/$MAX")
HPX_FLOPS=$(bc -l <<< "$HPX_COUNT_FLOPS/$MAX")
HPX_COMM=$(bc -l <<< "$HPX_COUNT_COMM/$MAX")


UPCXX_RED=$(bc -l <<< "$UPCXX_COUNT_RED/$MAX")
CHARM_RED=$(bc -l <<< "$CHARM_COUNT_RED/$MAX")
MPI_RED=$(bc -l <<< "$MPI_COUNT_RED/$MAX")
HPX_RED=$(bc -l <<< "$HPX_COUNT_RED/$MAX")

echo "$PROCESSES;$CHARM_TIME;$UPCXX_TIME;$MPI_TIME;$HPX_TIME" >> ~/jobs/output/${NAME}_time.csv
echo "$PROCESSES;$CHARM_COMM;$UPCXX_COMM;$MPI_COMM;$HPX_COMM" >> ~/jobs/output/${NAME}_comm.csv
echo "$PROCESSES;$CHARM_FLOPS;$UPCXX_FLOPS;$MPI_FLOPS;$HPX_FLOPS" >> ~/jobs/output/${NAME}_flops.csv
echo "$PROCESSES;$CHARM_RED;$UPCXX_RED;$MPI_RED;$HPX_RED" >> ~/jobs/output/${NAME}_red.csv

done 
