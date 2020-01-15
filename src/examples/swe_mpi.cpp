
/**
 * @file
 * This file is part of SWE.
 *
 * @author Jurek Olden (jurek.olden AT in.tum.de)
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 * This is a simple frontend for the SWE framework, which uses netCDF input files for bathymetry and displacement.
 * It then simulates this scenario according to the command line arguments it gets:
 *  - boundary condition
 *  - duration of the simulation
 *  - number of output checkpoints
 *  - resolution
 */

#include <cassert>
#include <string>
#include <ctime>
#include <time.h>
#include <unistd.h>
#include <limits.h>

#include "tools/args.hh"

#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#else
#include "writer/VtkWriter.hh"
#endif

#ifdef ASAGI
#include "scenarios/SWE_AsagiScenario.hh"
#else
#include "scenarios/SWE_simple_scenarios.hh"
#endif

#include "blocks/SWE_DimensionalSplittingMpi.hh"
#include <mpi.h>

int main(int argc, char** argv) {


	/**************
	 * INIT INPUT *
	 **************/


	// Define command line arguments
	tools::Args args;

#ifdef ASAGI
	args.addOption("bathymetry-file", 'b', "File containing the bathymetry");
	args.addOption("displacement-file", 'd', "File containing the displacement");
#endif
	args.addOption("simulation-duration", 't', "Time in seconds to simulate");
	args.addOption("checkpoint-count", 'n', "Number of simulation snapshots to be written");
	args.addOption("resolution-horizontal", 'x', "Number of simulation cells in horizontal direction");
	args.addOption("resolution-vertical", 'y', "Number of simulated cells in y-direction");
	args.addOption("output-basepath", 'o', "Output base file name");

    args.addOption("local-timestepping", 'l', "Activate local timestepping", tools::Args::Required, false);
	// Declare the variables needed to hold command line input
	float simulationDuration;
	int numberOfCheckPoints;
	int nxRequested;
	int nyRequested;
	bool  localTimestepping = false;
	std::string outputBaseName;



	// Declare variables for the output and the simulation time
	std::string outputFileName;
	float t = 0.;

	// Parse command line arguments
	tools::Args::Result ret = args.parse(argc, argv);
	switch (ret)
	{
		case tools::Args::Error:
			return 1;
		case tools::Args::Help:
			return 0;
		case tools::Args::Success:
			break;
	}

    if(args.isSet("local-timestepping") && args.getArgument<int>("local-timestepping") == 1){
        localTimestepping = true;

    }
	// Read in command line arguments
	simulationDuration = args.getArgument<float>("simulation-duration");
	numberOfCheckPoints = args.getArgument<int>("checkpoint-count");
	nxRequested = args.getArgument<int>("resolution-horizontal");
	nyRequested = args.getArgument<int>("resolution-vertical");
	outputBaseName = args.getArgument<std::string>("output-basepath");

	// Initialize scenario
#ifdef ASAGI
	SWE_AsagiScenario scenario(args.getArgument<std::string>("bathymetry-file"), args.getArgument<std::string>("displacement-file"));
#else
   // SWE_HalfDomainDry scenario;
	SWE_RadialDamBreakScenario scenario;
#endif

	// Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
	float* checkpointInstantOfTime = new float[numberOfCheckPoints];
	// Time delta is the time between any two checkpoints
	float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
	// The first checkpoint is reached after 0 + delta t
	checkpointInstantOfTime[0] = checkpointTimeDelta;
	for(int i = 1; i < numberOfCheckPoints; i++) {
		checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
	}


	/**********************************
	 * INIT MPI & SIMULATION BLOCKS *
	 **********************************/


	/*
	 * Calculate the cell widths of the grid used by the simulation:
	 * Get the size of the actual domain and divide it by the requested resolution.
	 *
	 * We use simple scenarios for testing, so we assume the position of BND_BOTTOM and BND_LEFT are at 0 respectively
	 */
	int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
	int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);
	float dxSimulation = (float) widthScenario / nxRequested;
	float dySimulation = (float) heightScenario/ nyRequested;

	// initialize MPI
	if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
		std::cerr << "MPI_Init failed." << std::endl;
	}

	int myMpiRank;
	int totalMpiRanks;
	MPI_Comm_rank(MPI_COMM_WORLD, &myMpiRank);
	MPI_Comm_size(MPI_COMM_WORLD, &totalMpiRanks);

	// Print status
	char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);

	printf("%i Spawned at %s\n", myMpiRank, hostname);

	/*
	 * determine the layout of UPC++ ranks:
	 * one block per process;
	 * if the number of processes is a square number, l_blockCountX = l_blockCountY,
	 * else l_blockCountX > l_blockCountY
	 */
	// number of SWE-Blocks in x- and y-direction
	int blockCountY = std::sqrt(totalMpiRanks);
	while (totalMpiRanks % blockCountY != 0) blockCountY--;
	int blockCountX = totalMpiRanks / blockCountY;

	// determine the local block position of each SWE_Block
	int localBlockPositionX = myMpiRank / blockCountY;
	int localBlockPositionY = myMpiRank % blockCountY;

	// compute local number of cells for each SWE_Block w.r.t. the simulation domain
	// (particularly not the original scenario domain, which might be finer in resolution)
	// (blocks at the domain boundary are assigned the "remainder" of cells)
	int nxBlockSimulation = nxRequested / blockCountX;
	int nxRemainderSimulation = nxRequested - (blockCountX - 1) * (nxRequested / blockCountX);
	int nyBlockSimulation = nyRequested / blockCountY;
	int nyRemainderSimulation = nyRequested - (blockCountY - 1) * (nyRequested / blockCountY);

	int nxLocal = (localBlockPositionX < blockCountX - 1) ? nxBlockSimulation : nxRemainderSimulation;
	int nyLocal = (localBlockPositionY < blockCountY - 1) ? nyBlockSimulation : nyRemainderSimulation;

	// Compute the origin of the local simulation block w.r.t. the original scenario domain.
	float localOriginX = scenario.getBoundaryPos(BND_LEFT) + localBlockPositionX * dxSimulation * nxBlockSimulation;
	float localOriginY = scenario.getBoundaryPos(BND_BOTTOM) + localBlockPositionY * dySimulation * nyBlockSimulation;

	// Determine the boundary types for the SWE_Block:
	// block boundaries bordering other blocks have a CONNECT boundary,
	// block boundaries bordering the entire scenario have the respective scenario boundary type
	BoundaryType boundaries[4];

	boundaries[BND_LEFT] = (localBlockPositionX > 0) ? CONNECT : scenario.getBoundaryType(BND_LEFT);
	boundaries[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? CONNECT : scenario.getBoundaryType(BND_RIGHT);
	boundaries[BND_BOTTOM] = (localBlockPositionY > 0) ? CONNECT : scenario.getBoundaryType(BND_BOTTOM);
	boundaries[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? CONNECT : scenario.getBoundaryType(BND_TOP);

	// Initialize the simulation block according to the scenario
	SWE_DimensionalSplittingMpi simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY,localTimestepping);
	simulation.initScenario(scenario, boundaries);

	// calculate neighbours to the current ranks simulation block
	int myNeighbours[4];
	myNeighbours[BND_LEFT] = (localBlockPositionX > 0) ? myMpiRank - blockCountY : -1;
	myNeighbours[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? myMpiRank + blockCountY : -1;
	myNeighbours[BND_BOTTOM] = (localBlockPositionY > 0) ? myMpiRank - 1 : -1;
	myNeighbours[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? myMpiRank + 1 : -1;
	simulation.connectNeighbours(myNeighbours);

	simulation.exchangeBathymetry();


	/***************
	 * INIT OUTPUT *
	 ***************/


	// Initialize boundary size of the ghost layers
	BoundarySize boundarySize = {{1, 1, 1, 1}};
	outputFileName = generateBaseFileName(outputBaseName, localBlockPositionX, localBlockPositionY);
#ifdef WRITENETCDF
	// Construct a netCDF writer
	NetCdfWriter writer(
			outputFileName,
			simulation.getBathymetry(),
			boundarySize,
			nxLocal,
			nyLocal,
			dxSimulation,
			dySimulation,
			simulation.getOriginX(),
			simulation.getOriginY());
#else
	// Construct a vtk writer
	VtkWriter writer(
			outputFileName,
			simulation.getBathymetry(),
			boundarySize,
			nxLocal,
			nyLocal,
			dxSimulation,
			dySimulation);
#endif // WRITENETCDF

	// Write the output at t = 0
	writer.writeTimeStep(
			simulation.getWaterHeight(),
			simulation.getMomentumHorizontal(),
			simulation.getMomentumVertical(),
			(float) 0.);


	/********************
	 * START SIMULATION *
	 ********************/
	float maxLocalTimestep;
    if(localTimestepping){
        simulation.computeMaxTimestep( 0.01,0.4);
        float localTimestep = simulation.getMaxTimestep();
        // reduce over all ranks
        MPI_Allreduce(&localTimestep, &maxLocalTimestep, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
        //maxLocalTimestep = 3*5.17476;
        simulation.setMaxLocalTimestep(maxLocalTimestep);
        std::cout << "Max local Timestep: " << maxLocalTimestep << std::endl;
        maxLocalTimestep = 3*5.17476;
    }

	// Initialize wall timer
	struct timespec startTime;
	struct timespec endTime;
    struct timespec barStartTime;
	float wallTime = 0.;
    float barrierTime = 0.;
	t = 0.0;

	float timestep;

	// loop over the count of requested checkpoints
	for(int i = 0; i < numberOfCheckPoints; i++) {
		// Simulate until the checkpoint is reached
		while(t < checkpointInstantOfTime[i]) {
		    do{
                // Start measurement
                clock_gettime(CLOCK_MONOTONIC, &startTime);

                // this is an implicit block (mpi recv in setGhostLayer()
                simulation.setGhostLayer();

                // compute numerical flux on each edge
                simulation.computeNumericalFluxes();

                // max timestep has been reduced over all ranks in computeNumericalFluxes()
                timestep = simulation.getMaxTimestep();


            //
                // update the cell values
                simulation.updateUnknowns(timestep);

                // Accumulate wall time
                clock_gettime(CLOCK_MONOTONIC, &endTime);
                wallTime += (endTime.tv_sec - startTime.tv_sec);
                wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

                // update simulation time with time step width.


                clock_gettime(CLOCK_MONOTONIC, &barStartTime);
                //MPI_Barrier(MPI_COMM_WORLD);
                barrierTime += (endTime.tv_sec - barStartTime.tv_sec);
                barrierTime += (float) (endTime.tv_nsec - barStartTime.tv_nsec) / 1E9;



            }while(localTimestepping && !simulation.hasMaxLocalTimestep());

            t += localTimestepping?maxLocalTimestep:timestep;
        }

		if(myMpiRank == 0) {
			printf("Write timestep (%fs)\n", t);
		}

		// write output
		writer.writeTimeStep(
				simulation.getWaterHeight(),
				simulation.getMomentumHorizontal(),
				simulation.getMomentumVertical(),
				t);
	}


	/************
	 * FINALIZE *
	 ************/

	printf("Rank %i : Compute Time (CPU): %fs - (WALL): %fs | Total Time (Wall): %fs Communication Time: %fs\n", myMpiRank, simulation.computeTime, simulation.computeTimeWall, wallTime, simulation.communicationTime);
    float flop = simulation.getFlops();
    float commTime = simulation.communicationTime;
    float reductionTime = simulation.reductionTime;
    float sumFlops;
	float sumCommTime;
	float sumReductionTime;
    float sumBarrierTime;
    MPI_Allreduce(&flop, &sumFlops, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&commTime, &sumCommTime, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&reductionTime, &sumReductionTime, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&barrierTime, &sumBarrierTime, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    if(myMpiRank == 0){
        std::cout   << "Rank: " << myMpiRank << std::endl
                    << "Flop count: " << sumFlops << std::endl
                    << "Flops(Total): " << ((float)sumFlops)/(wallTime*1000000000) << "GFLOPS"<< std::endl
                    << "Flops(Single): "<< ((float)simulation.getFlops())/(wallTime*1000000000) << std::endl
                    << "Communication Time(Total): " << sumCommTime << "s" << std::endl
                    << "Reduction Time(Total): " << sumReductionTime << "s" << std::endl
                    << "Barrier Time(Total): " << sumBarrierTime << "s" << std::endl;
    }
	simulation.freeMpiType();
	MPI_Finalize();

	return 0;
}
