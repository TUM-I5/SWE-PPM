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

#include "blocks/SWE_DimensionalSplittingUpcxx.hh"
#include <upcxx/upcxx.hpp>

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


	// Declare the variables needed to hold command line input
	float simulationDuration;
	int numberOfCheckPoints;
	int nxRequested;
	int nyRequested;
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

	// Read in command line arguments
	simulationDuration = args.getArgument<float>("simulation-duration");
	numberOfCheckPoints = args.getArgument<int>("checkpoint-count");
	nxRequested = args.getArgument<int>("resolution-horizontal");
	nyRequested = args.getArgument<int>("resolution-vertical");
	outputBaseName = args.getArgument<std::string>("output-basepath");

	// Initialize Scenario
#ifdef ASAGI
	SWE_AsagiScenario scenario(args.getArgument<std::string>("bathymetry-file"), args.getArgument<std::string>("displacement-file"));
#else
    SWE_HalfDomainDry scenario;
    //SWE_RadialDamBreakScenario scenario;
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
	 * INIT UPCXX & SIMULATION BLOCKS *
	 **********************************/


	/*
	 * Calculate the simulation grid layout.
	 * The cell count of the scenario as well as the scenario size is fixed, 
	 * Get the size of the actual domain and divide it by the requested resolution.
	 */
	int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
	int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);
	float dxSimulation = (float) widthScenario / nxRequested;
	float dySimulation = (float) heightScenario / nyRequested;

	// start parallel runtime (all global inits are done at this point)
	upcxx::init();

	auto myUpcxxRank = upcxx::rank_me();
	auto totalUpcxxRanks = upcxx::rank_n();

	// Print status
	char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);

	printf("%i Spawned at %s\n", myUpcxxRank, hostname);

	/*
	 * determine the layout of UPC++ ranks:
	 * one block per process;
	 * if the number of processes is a square number, l_blockCountX = l_blockCountY,
	 * else l_blockCountX > l_blockCountY
	 */
	// number of SWE-Blocks in x- and y-direction
	int blockCountY = std::sqrt(totalUpcxxRanks);
	while (totalUpcxxRanks % blockCountY != 0) blockCountY--;
	int blockCountX = totalUpcxxRanks / blockCountY;

	// determine the local block position of each SWE_Block
	int localBlockPositionX = myUpcxxRank / blockCountY;
	int localBlockPositionY = myUpcxxRank % blockCountY;

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
	SWE_DimensionalSplittingUpcxx simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY);
	simulation.initScenario(scenario, boundaries);

	// calculate neighbours to the current ranks simulation block
	int leftNeighborRank = (localBlockPositionX > 0) ? myUpcxxRank - blockCountY : -1;
	int rightNeighborRank = (localBlockPositionX < blockCountX - 1) ? myUpcxxRank + blockCountY : -1;
	int bottomNeighborRank = (localBlockPositionY > 0) ? myUpcxxRank - 1 : -1;
	int topNeighborRank = (localBlockPositionY < blockCountY - 1) ? myUpcxxRank + 1 : -1;

	/****************************************
	 * BROADCAST COPY LAYER GLOBAL POINTERS *
	 ****************************************/


	typedef upcxx::global_ptr<BlockConnectInterface<upcxx::global_ptr<float>>> InterfaceRegistry;
	typedef BlockConnectInterface<upcxx::global_ptr<float>> Interface;

	InterfaceRegistry leftInterfaceRegistry = nullptr;
	InterfaceRegistry rightInterfaceRegistry = nullptr;
	InterfaceRegistry bottomInterfaceRegistry = nullptr;
	InterfaceRegistry topInterfaceRegistry = nullptr;

	if (myUpcxxRank == 0) {
		leftInterfaceRegistry = upcxx::new_array<Interface>(totalUpcxxRanks);
		rightInterfaceRegistry = upcxx::new_array<Interface>(totalUpcxxRanks);
		bottomInterfaceRegistry = upcxx::new_array<Interface>(totalUpcxxRanks);
		topInterfaceRegistry = upcxx::new_array<Interface>(totalUpcxxRanks);
	}

	leftInterfaceRegistry = upcxx::broadcast(leftInterfaceRegistry, 0).wait();
	rightInterfaceRegistry = upcxx::broadcast(rightInterfaceRegistry, 0).wait();
	bottomInterfaceRegistry = upcxx::broadcast(bottomInterfaceRegistry, 0).wait();
	topInterfaceRegistry = upcxx::broadcast(topInterfaceRegistry, 0).wait();

	upcxx::rput(simulation.getCopyLayer(BND_RIGHT), leftInterfaceRegistry + myUpcxxRank).wait();
	upcxx::rput(simulation.getCopyLayer(BND_LEFT), rightInterfaceRegistry + myUpcxxRank).wait();
	upcxx::rput(simulation.getCopyLayer(BND_TOP), bottomInterfaceRegistry + myUpcxxRank).wait();
	upcxx::rput(simulation.getCopyLayer(BND_BOTTOM), topInterfaceRegistry + myUpcxxRank).wait();
	upcxx::barrier();

	Interface interfaces[4];
	if (leftNeighborRank > -1)
		interfaces[BND_LEFT] = upcxx::rget(leftInterfaceRegistry + leftNeighborRank).wait();
	if (rightNeighborRank > -1)
		interfaces[BND_RIGHT] = upcxx::rget(rightInterfaceRegistry + rightNeighborRank).wait();
	if (bottomNeighborRank > -1)
		interfaces[BND_BOTTOM] = upcxx::rget(bottomInterfaceRegistry + bottomNeighborRank).wait();
	if (topNeighborRank > -1)
		interfaces[BND_TOP] = upcxx::rget(topInterfaceRegistry + topNeighborRank).wait();

	simulation.connectBoundaries(interfaces);
	simulation.exchangeBathymetry();
	upcxx::barrier();

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


	// Initialize wall timer
	struct timespec startTime;
	struct timespec endTime;

	float wallTime = 0.;

	t = 0.0;

	float timestep;
	unsigned int iterations = 0;
	// loop over the count of requested checkpoints
	for(int i = 0; i < numberOfCheckPoints; i++) {
		// Simulate until the checkpoint is reached
		while(t < checkpointInstantOfTime[i]) {
			// Start measurement
			clock_gettime(CLOCK_MONOTONIC, &startTime);

			// set values in ghost cells.
			// this function blocks until everything has been received
			simulation.setGhostLayer();

			// compute numerical flux on each edge
			simulation.computeNumericalFluxes();

			// max timestep has been reduced over all ranks in computeNumericalFluxes()
			timestep = simulation.getMaxTimestep();

			// update the cell values
			simulation.updateUnknowns(timestep);

			// Accumulate wall time
			clock_gettime(CLOCK_MONOTONIC, &endTime);
			wallTime += (endTime.tv_sec - startTime.tv_sec);
			wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

			// update simulation time with time step width.
			t += timestep;
			iterations++;
			upcxx::barrier();
		}

		if(myUpcxxRank == 0) {
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


    printf("Rank %i : Compute Time (CPU): %fs - (WALL): %fs | Total Time (Wall): %fs Communication Time: %fs\n", myUpcxxRank, simulation.computeTime, simulation.computeTimeWall, wallTime, simulation.communicationTime);
    float sumFlops = upcxx::reduce_all(simulation.getFlops(), upcxx::op_fast_add).wait();
    float sumCommTime = upcxx::reduce_all(simulation.communicationTime, upcxx::op_fast_add).wait();
    if(myUpcxxRank == 0){
    std::cout   << "Rank: " << myUpcxxRank << std::endl
            << "Flop count: " << sumFlops << std::endl
            << "Flops(Total): " << ((float)sumFlops)/(wallTime*1000000000) << "GFLOPS"<< std::endl
            << "Flops(Single): "<< ((float)simulation.getFlops())/(wallTime*1000000000) << std::endl
            << "Communication Time(Total): " << sumCommTime << "s" << std::endl;
}
    upcxx::finalize();

	return 0;
}
