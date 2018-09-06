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

#include "main.decl.h"
#include "hello.decl.h"

#include <cassert>
#include <string>

#include "blocks/SWE_DimensionalSplittingUpcxx.hh"

#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#else
#include "writer/VtkWriter.hh"
#endif

#include "tools/args.hh"
#include "tools/Float2D.hh"
#include "tools/Logger.hh"
#include "tools/ProgressBar.hh"
#include "scenarios/SWE_simple_scenarios.hh"

#include <upcxx/upcxx.hpp>

/**
 * Compute the number of block rows from the total number of processes.
 *
 * The number of rows is determined as the square root of the
 * number of processes, if this is a square number;
 * otherwise, we use the largest number that is smaller than the square
 * root and still a divisor of the number of processes.
 *
 *
 * @param numProcs number of process.
 * @return number of block rows
 */
int computeNumberOfBlockRows(int i_numberOfProcesses) {
	int l_numberOfRows = std::sqrt(i_numberOfProcesses);
	while (i_numberOfProcesses % l_numberOfRows != 0) l_numberOfRows--;
	return l_numberOfRows;
};

Main::Main(CkMigrateMessage *msg) {}

Main::Main(CkArgMsg *msg) {


	/**************
	 * INIT INPUT *
	 **************/


	// Define command line arguments
	tools::Args args;

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
	tools::Args::Result ret = args.parse(msg->argc, msg->argv);
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

	// Initialize netCDF scenario which reads the input files
	SWE_RadialDamBreakScenario scenario;

	// Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
	float* checkpointInstantOfTime = new float[numberOfCheckPoints];
	// Time delta is the time between any two checkpoints
	float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
	// The first checkpoint is reached after 0 + delta t
	checkpointInstantOfTime[0] = checkpointTimeDelta;
	for(int i = 1; i < numberOfCheckPoints; i++) {
		checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
	}


	/****************************************
	 * INIT WORK CHARES / SIMULATION BLOCKS *
	 ****************************************/


	/*
	 * Calculate the cell widths of the grid used by the simulation:
	 * Get the size of the actual domain and divide it by the requested resolution.
	 *
	 * We use simple scenarios for testing, so we assume the position of BND_BOTTOM and BND_LEFT are at 0 respectively
	 */
	assert(scenario.getBoundaryPos(BND_BOTTOM) == 0);
	assert(scenario.getBoundaryPos(BND_LEFT) == 0);
	int nxScenario = scenario.getBoundaryPos(BND_RIGHT);
	int nyScenario = scenario.getBoundaryPos(BND_TOP);
	//float dxSimulation = nxScenario / nxRequested;
	//float dySimulation = nyScenario/ nyRequested;
	float dxSimulation = 1;
	float dySimulation = 1;

	/*
	 * determine the layout of UPC++ ranks:
	 * one block per process;
	 * if the number of processes is a square number, l_blockCountX = l_blockCountY,
	 * else l_blockCountX > l_blockCountY
	 */
	auto myUpcxxRank = upcxx::rank_me();
	auto totalUpcxxRanks = upcxx::rank_n();

	// number of SWE-Blocks in x- and y-direction
	int blockCountY = computeNumberOfBlockRows(totalUpcxxRanks);
	int blockCountX = totalUpcxxRanks / blockCountY;

	// determine the local block coordinates of each SWE_Block
	int localBlockPositionX = myUpcxxRank / blockCountY;
	int localBlockPositionY = myUpcxxRank % blockCountY;

	// compute local number of cells for each SWE_Block w.r.t. to the original scenario domain
	// (particularly not the simulation domain, which might be smaller in resolution)
	// (blocks at the domain boundary are assigned the "remainder" of cells)
	int nxBlock = nxScenario / blockCountX;
	int nxRemainder = nxScenario - (blockCountX - 1) * (nxScenario / blockCountX);
	int nyBlock = nyScenario / blockCountY;
	int nyRemainder = nyScenario - (blockCountY - 1) * (nyScenario / blockCountY);

	int nxLocal = (localBlockPositionX < blockCountX - 1) ? nxBlock : nxRemainder;
	int nyLocal = (localBlockPositionY < blockCountY - 1) ? nyBlock : nyRemainder;

	// Initialize the simulation block according to the scenario
	SWE_DimensionalSplittingUpcxx simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localBlockPositionX * nxBlock, localBlockPositionY * nyBlock);
	simulation.initScenario(scenario);

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

	upcxx::rput(simulation.getCopyLayer(BND_LEFT), leftInterfaceRegistry + myUpcxxRank).wait();
	upcxx::rput(simulation.getCopyLayer(BND_RIGHT), rightInterfaceRegistry + myUpcxxRank).wait();
	upcxx::rput(simulation.getCopyLayer(BND_TOP), topInterfaceRegistry + myUpcxxRank).wait();
	upcxx::rput(simulation.getCopyLayer(BND_BOTTOM), bottomInterfaceRegistry + myUpcxxRank).wait();
	upcxx::barrier();

	Interface interfaces[4];
	interfaces[BND_LEFT] = upcxx::rget(leftInterfaceRegistry + leftNeighborRank).wait();
	interfaces[BND_RIGHT] = upcxx::rget(rightInterfaceRegistry + rightNeighborRank).wait();
	interfaces[BND_BOTTOM] = upcxx::rget(bottomInterfaceRegistry + bottomNeighborRank).wait();
	interfaces[BND_TOP] = upcxx::rget(topInterfaceRegistry + topNeighborRank).wait();

	simulation.connectBoundaries(interfaces);


	/***************
	 * INIT OUTPUT *
	 ***************/


	// Initialize boundary size of the ghost layers
	io::BoundarySize boundarySize = {{1, 1, 1, 1}};
	outputFileName = generateBaseFileName(outputBaseName, localBlockPositionX, localBlockPositionY);
#ifdef WRITENETCDF
	// Construct a netCDF writer
	io::NetCdfWriter writer(
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
	io::VtkWriter writer(
			outputFileName,
			simulation.getBathymetry(),
			boundarySize,
			nxLocal,
			nyLocal,
			dxSimulation,
			dySimulation);
#endif // WRITENETCDF

	// Init fancy progressbar
	tools::ProgressBar progressBar(simulationDuration);

	// Write the output at t = 0
	tools::Logger::logger.printOutputTime((float) 0.);
	progressBar.update(0.);
	writer.writeTimeStep(
			simulation.getWaterHeight(),
			simulation.getMomentumHorizontal(),
			simulation.getMomentumVertical(),
			(float) 0.);


	/********************
	 * START SIMULATION *
	 ********************/


	// print the start message and reset the wall clock time
	progressBar.clear();
	if (upcxx::rank_me() == 0) {
		tools::Logger::logger.printStartMessage();
	}
	tools::Logger::logger.initWallClockTime(time(NULL));

	//! simulation time.
	t = 0.0;
	progressBar.update(t);

	float timestep;
	unsigned int iterations = 0;
	// loop over the count of requested checkpoints
	for(int i = 0; i < numberOfCheckPoints; i++) {
		// Simulate until the checkpoint is reached
		while(t < checkpointInstantOfTime[i]) {
			// reset CPU-Communication clock
			tools::Logger::logger.resetClockToCurrentTime("CpuCommunication");

			// set values in ghost cells.
			// we need to sync here since block boundaries get exchanged over ranks
			// TODO: what can we do if this becomes a bottleneck?
			upcxx::barrier();
			simulation.setGhostLayer();

			// reset the cpu clock
			tools::Logger::logger.resetClockToCurrentTime("Cpu");

			// compute numerical flux on each edge
			simulation.computeNumericalFluxes();

			// update the cpu time in the logger
			tools::Logger::logger.updateTime("Cpu");

			// max timestep has been reduced over all ranks in computeNumericalFluxes()
			timestep = simulation.getMaxTimestep();

			// reset the cpu time
			tools::Logger::logger.resetClockToCurrentTime("Cpu");

			// update the cell values
			simulation.updateUnknowns(timestep);

			// update the cpu and CPU-communication time in the logger
			tools::Logger::logger.updateTime("Cpu");
			tools::Logger::logger.updateTime("CpuCommunication");

			// update simulation time with time step width.
			t += timestep;
			iterations++;

			// print the current simulation time
			if (upcxx::rank_me() == 0) {
				progressBar.clear();
				tools::Logger::logger.printSimulationTime(t);
				progressBar.update(t);
			}
		}

		// print the time elapsed until this output
		if (upcxx::rank_me() == 0) {
			progressBar.clear();
			tools::Logger::logger.printOutputTime(t);
			progressBar.update(t);
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


	// write the statistics message
	progressBar.clear();
	tools::Logger::logger.printStatisticsMessage();

	// print the cpu time
	tools::Logger::logger.printTime("Cpu", "CPU time");

	// print CPU + Communication time
	tools::Logger::logger.printTime("CpuCommunication", "CPU + Communication time");

	// print the wall clock time (includes plotting)
	tools::Logger::logger.printWallClockTime(time(NULL));

	// printer iteration counter
	tools::Logger::logger.printIterationsDone(iterations);

	// print the finish message
	tools::Logger::logger.printFinishMessage();

	upcxx::finalize();

	return 0;
}

#include "main.def.h"
