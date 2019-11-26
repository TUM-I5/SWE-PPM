/**
 * @file
 * This file is part of SWE.
 *
 * @author Alexander Breuer (breuera AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer)
 *         Michael Bader (bader AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Univ.-Prof._Dr._Michael_Bader)
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
 *
 * Basic setting of SWE, which uses a wave propagation solver and an artificial or ASAGI scenario on a single block.
 */

#include <mpi.h>
#include "chameleon.h"
#include <cassert>
#include <string>
#include <ctime>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include <vector>
#include <numeric>

#ifdef ITT
#include <ittnotify.h>
#endif
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

#include "blocks/SWE_DimensionalSplittingChameleon.hh"

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
	args.addOption("x-blockcount", 'i', "Block Count in x-direction", tools::Args::Required, false);
	args.addOption("y-blockcount", 'j', "Block Count in y-direction", tools::Args::Required, false);
	args.addOption("x-imbalance", 'u', "Imbalance in x-direction", tools::Args::Required, false);
	args.addOption("y-imbalance", 'v', "Imbalance in y-direction", tools::Args::Required, false);
	args.addOption("write", 'w', "Write results", tools::Args::Required, false);
	args.addOption("iteration-count", 'i', "Iteration Count (Overrides t and n)", tools::Args::Required, false);

	// Parse command line arguments
	tools::Args::Result ret = args.parse(argc, argv);
	switch (ret) {
		case tools::Args::Error:
			return 1;
		case tools::Args::Help:
			return 0;
		case tools::Args::Success:
			break;
	}

	// Read in command line arguments
	float simulationDuration = args.getArgument<float>("simulation-duration");
	int numberOfCheckPoints = args.getArgument<int>("checkpoint-count");
	int nxRequested = args.getArgument<int>("resolution-horizontal");
	int nyRequested = args.getArgument<int>("resolution-vertical");
	std::string outputBaseName = args.getArgument<std::string>("output-basepath");
	bool write = false;
	if(args.isSet("write") && args.getArgument<int>("write") == 1)
		write = true;
	int iteration_count = 1000000;
	if(args.isSet("iteration-count")) {
		iteration_count = args.getArgument<int>("iteration-count");
		numberOfCheckPoints = 1;
		simulationDuration = 1000000.0;
	}

	// Initialize Scenario
#ifdef ASAGI
	SWE_AsagiScenario scenario(args.getArgument<std::string>("bathymetry-file"), args.getArgument<std::string>("displacement-file"));
#else
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

	/***************
	 * INIT BLOCKS *
	 ***************/

	// Init MPI
	int myRank, numRanks;
	int provided;
	int requested = MPI_THREAD_MULTIPLE;
	MPI_Init_thread(&argc, &argv, requested, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	/*
	 * Calculate the simulation grid layout.
	 * The cell count of the scenario as well as the scenario size is fixed, 
	 * Get the size of the actual domain and divide it by the requested resolution.
	 */
	int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
	int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);
	float dxSimulation = (float) widthScenario / nxRequested;
	float dySimulation = (float) heightScenario / nyRequested;

	// Determine number of ranks in x and y direction
	int xRankCount = std::sqrt(numRanks);
	while(numRanks % xRankCount != 0)
		xRankCount--;
	int yRankCount = numRanks/xRankCount;
	printf("xRankCount=%d, yRankCount=%d\n", xRankCount, yRankCount);

	int xBlockCount = 32;
	if(args.isSet("x-blockcount"))
		xBlockCount = args.getArgument<int>("x-blockcount");
	int yBlockCount = 32;
	if(args.isSet("y-blockcount"))
		yBlockCount = args.getArgument<int>("y-blockcount");
	assert(nxRequested % xBlockCount == 0);
	assert(nyRequested % yBlockCount == 0);
	int myXRank = myRank%xRankCount;
	int myYRank = myRank/xRankCount;

	// Imbalance vectors that determine the weights of the ranks
	std::vector<std::vector<float>> imbalanceVectors{
	{ 1, 1, 1, 1, 1, 1, 1, 1 },      //no imbalance
	{ 2, 1, 1, 1, 1, 1, 1, 1 },      //slight imbalance
	{ 128, 64, 32, 16, 8, 4, 2, 1 }, //imbalance
	{ 1, 0, 0, 0, 0, 0, 0, 0 }};     //extreme imbalance

	// read chosen imbalance vectors (optional)
	std::vector<float> xWeights = imbalanceVectors[0];
	if(args.isSet("x-imbalance"))
		xWeights = imbalanceVectors[args.getArgument<int>("x-imbalance")];
	std::vector<float> yWeights = imbalanceVectors[0];
	if(args.isSet("y-imbalance"))
		yWeights = imbalanceVectors[args.getArgument<int>("y-imbalance")];
	float xSum = std::accumulate(xWeights.begin(), xWeights.begin()+xRankCount, 0);
	float ySum = std::accumulate(yWeights.begin(), yWeights.begin()+yRankCount, 0);

	for(int i = 0; i < xRankCount; i++)
		xWeights[i] = (xWeights[i] / xSum) * xBlockCount;
	for(int i = 0; i < yRankCount; i++)
		yWeights[i] = (yWeights[i] / ySum) * yBlockCount;

	std::vector<int> xBounds(xRankCount+1);
	xBounds[0] = 0;
	std::vector<int> yBounds(yRankCount+1);
	yBounds[0] = 0;
	for(int i = 1; i < xRankCount; i++)
		xBounds[i] = xBounds[i-1] + xWeights[i];
	xBounds[xRankCount] = xBlockCount;
	for(int i = 1; i < yRankCount; i++)
		yBounds[i] = yBounds[i-1] + yWeights[i];
	yBounds[yRankCount] = yBlockCount;
	printf("%d: xBlockCount=%d, xBounds:", myRank, xBlockCount);
	for(int i = 0; i < xRankCount+1; i++)
		printf(" %d", xBounds[i]);
	printf("\n");
	printf("%d: yBlockCount=%d, yBounds:", myRank, yBlockCount);
	for(int i = 0; i < yRankCount+1; i++)
		printf(" %d", yBounds[i]);
	printf("\n");
	//printf("%d: xBlockCount:%d\n", myRank, xBlockCount);
	//printf("%d: xBounds:%d, %d, %d\n", myRank, xBounds[0], xBounds[1], xBounds[2]);
	//printf("%d: yBounds:%d, %d, %d\n", myRank, yBounds[0], yBounds[1], yBounds[2]);
	//printf("%d: xDim:%d, yDim:%d\n", myRank, xBounds[myXRank+1]-xBounds[myXRank], yBounds[myYRank+1]-yBounds[myYRank]);

	SWE_DimensionalSplittingChameleon* blocks[xBlockCount][yBlockCount];

	int x_blocksize = nxRequested / xBlockCount;
	int y_blocksize = nyRequested / yBlockCount;

	for(int x = xBounds[myXRank]; x < xBounds[myXRank+1]; x++) {
		for(int y = yBounds[myYRank]; y < yBounds[myYRank+1]; y++) {
			//printf("%d: x=%d, y=%d\n", myRank, x, y);
			int x_pos = x*x_blocksize;
			float originX = x_pos * dxSimulation;
			int y_pos = y*y_blocksize;
			float originY = y_pos * dySimulation;

			BoundaryType boundaries[4];			

			if(x == 0)
				boundaries[BND_LEFT] = scenario.getBoundaryType(BND_LEFT);
			else if(x == xBounds[myXRank])
				boundaries[BND_LEFT] = CONNECT;
			else
				boundaries[BND_LEFT] = CONNECT_WITHIN_RANK;
			
			if(x_pos+x_blocksize == nxRequested)
				boundaries[BND_RIGHT] = scenario.getBoundaryType(BND_RIGHT);
			else if(x == xBounds[myXRank+1]-1)
				boundaries[BND_RIGHT] = CONNECT;
			else
				boundaries[BND_RIGHT] = CONNECT_WITHIN_RANK;

			if(y == 0)
				boundaries[BND_BOTTOM] = scenario.getBoundaryType(BND_BOTTOM);
			else if(y == yBounds[myYRank])
				boundaries[BND_BOTTOM] = CONNECT;
			else
				boundaries[BND_BOTTOM] = CONNECT_WITHIN_RANK;

			if(y_pos + y_blocksize == nyRequested)
				boundaries[BND_TOP] = scenario.getBoundaryType(BND_TOP);
			else if(y == yBounds[myYRank+1]-1)
				boundaries[BND_TOP] = CONNECT;
			else
				boundaries[BND_TOP] = CONNECT_WITHIN_RANK;

			blocks[x][y] = new SWE_DimensionalSplittingChameleon(x_blocksize, y_blocksize, dxSimulation, dySimulation, originX, originY);
			blocks[x][y]->initScenario(scenario, boundaries);

			blocks[x][y]->myRank = myRank;
			if(myRank != 0)
				blocks[x][y]->neighbourRankId[BND_LEFT] = myRank-1;
			if(myRank != numRanks - 1)
				blocks[x][y]->neighbourRankId[BND_RIGHT] = myRank+1;
			if(myRank >= xRankCount)
				blocks[x][y]->neighbourRankId[BND_BOTTOM] = myRank-xRankCount;
			if(myRank < numRanks - xRankCount)
				blocks[x][y]->neighbourRankId[BND_TOP] = myRank+xRankCount;
		}
	}
	for(int x = xBounds[myXRank]; x < xBounds[myXRank+1]; x++) {
		for(int y = yBounds[myYRank]; y < yBounds[myYRank+1]; y++) {
			if(x != 0)
				blocks[x][y]->left = blocks[x-1][y];
			if(x != xBounds[myXRank+1]-1)
				blocks[x][y]->right = blocks[x+1][y];
			if(y != 0)
				blocks[x][y]->bottom = blocks[x][y-1];
			if(y != yBounds[myYRank+1]-1)
				blocks[x][y]->top = blocks[x][y+1];
		}
	}

	/***************
	 * INIT OUTPUT *
	 ***************/

	// block used for writing (only used on rank 0)
	// all ranks write their blocks to this write block on rank 0 (using one-sided communication)
	// This block is then written to get a single output file
	//printf("%d: Init write block with nxReq:%d, nyReq:%d, dxSim:%f, dySim:%f\n", myRank, nxRequested, nyRequested, dxSimulation, dySimulation);
	SWE_DimensionalSplittingChameleon writeBlock(nxRequested, nyRequested, dxSimulation, dySimulation, 0, 0);
	BoundaryType boundaries[4];
	boundaries[BND_LEFT] = scenario.getBoundaryType(BND_LEFT);
	boundaries[BND_RIGHT] = scenario.getBoundaryType(BND_RIGHT);
	boundaries[BND_TOP] = scenario.getBoundaryType(BND_TOP);
	boundaries[BND_BOTTOM] = scenario.getBoundaryType(BND_BOTTOM);
	if(write && myRank == 0)
		writeBlock.initScenario(scenario, boundaries);

	// Prepare writeBlock for usage with One-Sided Communication
  	MPI_Win writeBlockWin_h;
	MPI_Win writeBlockWin_hu;
  	MPI_Win writeBlockWin_hv;

	MPI_Win_create(writeBlock.h.getRawPointer(), (nxRequested+2)*(nyRequested*2), sizeof(float), MPI_INFO_NULL, MPI_COMM_WORLD, &writeBlockWin_h);
	MPI_Win_create(writeBlock.hu.getRawPointer(), (nxRequested+2)*(nyRequested*2), sizeof(float), MPI_INFO_NULL, MPI_COMM_WORLD, &writeBlockWin_hu);
	MPI_Win_create(writeBlock.hv.getRawPointer(), (nxRequested+2)*(nyRequested*2), sizeof(float), MPI_INFO_NULL, MPI_COMM_WORLD, &writeBlockWin_hv);

	// Initialize boundary size of the ghost layers
	BoundarySize boundarySize = {{1, 1, 1, 1}};
#ifdef WRITENETCDF
	// Construct a netCDF writer
	std::string outputFileName = outputBaseName;
	NetCdfWriter* writer;
	if(write && myRank == 0) {
		writer = new NetCdfWriter(
			outputFileName,
			writeBlock.getBathymetry(),
			boundarySize,
			writeBlock.getCellCountHorizontal(),
			writeBlock.getCellCountVertical(),
			dxSimulation,
			dySimulation,
			writeBlock.getOriginX(),
			writeBlock.getOriginY());
		//printf("%d: Init writer with CellCountHorizontal:%d, CellCountVertical:%d, OriginX:%d, getOriginX:%d\n", myRank, writeBlock.getCellCountHorizontal(), writeBlock.getCellCountVertical(), writeBlock.getOriginX(), writeBlock.getOriginY());
	}
#else
	// Construct a vtk writer
	std::string outputFileName = outputBaseName;
	VtkWriter writer(
		outputFileName,
		writeBlock.getBathymetry(),
		boundarySize,
		writeBlock.getCellCountHorizontal(),
		writeBlock.getCellCountVertical(),
		dxSimulation,
		dySimulation);
#endif // WRITENETCDF

	// Write the output at t = 0
	if(write && myRank == 0) {
		writer->writeTimeStep(
			writeBlock.getWaterHeight(),
			writeBlock.getMomentumHorizontal(),
			writeBlock.getMomentumVertical(),
			(float) 0.);
	}

	/********************
	 * START SIMULATION *
	 ********************/

    #pragma omp parallel
    {
        chameleon_thread_init();
    }
	
    // necessary to be aware of binary base addresses to calculate offset for target functions
    chameleon_determine_base_addresses((void *)&main);

    MPI_Barrier(MPI_COMM_WORLD);

	// Initialize timers
	std::clock_t computeClock;
	std::clock_t commClock;

	struct timespec startTime;
	struct timespec endTime;

	float computeTime = 0.;
	float commTime = 0.;
	float wallTime = 0.;

	// performance debug
	double lastTime = 0;
	double setGhostLayerTime = 0;
	double receiveGhostLayerTime = 0;
	double taskCreateHorizontalTime = 0;
	double taskWaitHorizontalTime = 0;
	double reductionTime = 0;
	double taskCreateVerticalTime = 0;
	double taskWaitVerticalTime = 0;
	double updateUnknownsTime = 0;
	double writeOutputTime = 0;

	float t = 0.0;
	float timestep;
	int iterations = 0;

	double startTimeWhole = getTime();
#ifdef ITT
	__itt_resume();	
#endif
	// loop over the count of requested checkpoints
	for(int i = 0; i < numberOfCheckPoints; i++) {
		// Simulate until the checkpoint is reached
		while(t < checkpointInstantOfTime[i] && iterations < iteration_count) {

			// Start measurement
			clock_gettime(CLOCK_MONOTONIC, &startTime);
			commClock = clock();
			lastTime = getTime();

			timestep = std::numeric_limits<float>::max();

			//TODO: exchange bathymetry

			int xLower = xBounds[myXRank]; int xUpper = xBounds[myXRank+1];
			int yLower = yBounds[myYRank]; int yUpper = yBounds[myYRank+1];
			//#pragma omp parallel for collapse(2)
			for(int x = xLower; x < xUpper; x++) {
				for(int y = yLower; y < yUpper; y++) {
					// set values in ghost cells.
					// we need to sync here since block boundaries get exchanged over ranks
					blocks[x][y]->setGhostLayer();
				}
			}

			setGhostLayerTime += getTime()-lastTime; lastTime = getTime();

			//if(myRank == 0) printf("After setGhostLayer() %f\n", (float)(clock() - commClock) / CLOCKS_PER_SEC);
			//#pragma omp parallel for collapse(2)
			for(int x = xLower; x < xUpper; x++) {
				for(int y = yLower; y < yUpper; y++) {
					blocks[x][y]->receiveGhostLayer();
				}
			}

			receiveGhostLayerTime += getTime()-lastTime; lastTime = getTime();
			//if(myRank == 0) printf("After receiveGhostLayer() %f\n", (float)(clock() - commClock) / CLOCKS_PER_SEC);

			// Accumulate comm time and start compute clock
			//commClock = clock() - commClock;
			//commTime += (float) commClock / CLOCKS_PER_SEC;
			computeClock = clock();

			#pragma omp parallel 
			{
				int xLower = xBounds[myXRank]; int xUpper = xBounds[myXRank+1];
				int yLower = yBounds[myYRank]; int yUpper = yBounds[myYRank+1];
				#pragma omp for
				for(int x = xLower; x < xUpper; x++) {
					for(int y = yLower; y < yUpper; y++) {
						// compute numerical flux on each edge
						blocks[x][y]->computeNumericalFluxesHorizontal();
					}
				}
				#pragma omp master
					taskCreateHorizontalTime += getTime()-lastTime; lastTime = getTime();
				//if(myRank == 0) printf("After computeNumericalFluxesHorizontal() Task Spawning %f\n", (float)(clock() - commClock) / CLOCKS_PER_SEC);
				chameleon_distributed_taskwait(0);
			}
			taskWaitHorizontalTime += getTime()-lastTime; lastTime = getTime();
			//if(myRank == 0) printf("After computeNumericalFluxesHorizontal() Task Wait %f\n", (float)(clock() - commClock) / CLOCKS_PER_SEC);

			for(int x = xBounds[myXRank]; x < xBounds[myXRank+1]; x++) {
				for(int y = yBounds[myYRank]; y < yBounds[myYRank+1]; y++) {
					if(blocks[x][y]->getMaxTimestep() < timestep)
						timestep = blocks[x][y]->getMaxTimestep();
				}
			}

			// reduce over all ranks
			float maxTimestepGlobal;
			MPI_Allreduce(&timestep, &maxTimestepGlobal, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
			timestep = maxTimestepGlobal;

			reductionTime += getTime()-lastTime; lastTime = getTime();
			#pragma omp parallel
			{
				int xLower = xBounds[myXRank]; int xUpper = xBounds[myXRank+1];
				int yLower = yBounds[myYRank]; int yUpper = yBounds[myYRank+1];
				#pragma omp for
				for(int x = xLower; x < xUpper; x++) {
					for(int y = yLower; y < yUpper; y++) {
						// compute numerical flux on each edge
						blocks[x][y]->maxTimestep = timestep;
						blocks[x][y]->computeNumericalFluxesVertical();
					}
				}
				#pragma omp master
					taskCreateVerticalTime += getTime()-lastTime; lastTime = getTime();
				//if(myRank == 0) printf("After computeNumericalFluxesVertical() Task Spawning %f\n", (float)(clock() - commClock) / CLOCKS_PER_SEC);
				chameleon_distributed_taskwait(0);
			}
			taskWaitVerticalTime += getTime()-lastTime; lastTime = getTime();
			//if(myRank == 0) printf("After computeNumericalFluxesVertical() Task Wait %f\n", (float)(clock() - commClock) / CLOCKS_PER_SEC);
			
			#pragma omp parallel for
			for(int x = xLower; x < xUpper; x++) {
				for(int y = yLower; y < yUpper; y++) {
					// update the cell values
					blocks[x][y]->updateUnknowns(timestep);
				}
			}
			updateUnknownsTime += getTime()-lastTime; lastTime = getTime();
			//if(myRank == 0) printf("After updateUnknowns() %f\n", (float)(clock() - commClock) / CLOCKS_PER_SEC);

			// Accumulate compute time
			computeClock = clock() - computeClock;
			computeTime += (float) computeClock / CLOCKS_PER_SEC;

			// Accumulate wall time
			clock_gettime(CLOCK_MONOTONIC, &endTime);
			wallTime += (endTime.tv_sec - startTime.tv_sec);
			wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
			// update simulation time with time step width.
			t += timestep;
			iterations++;

			if(myRank == 0)
				printf("Step, current time:%f\n", t);
		}

		// write output
		if(write) {
			MPI_Win_fence(0, writeBlockWin_h);
			MPI_Win_fence(0, writeBlockWin_hu);
			MPI_Win_fence(0, writeBlockWin_hv);
			for(int x = xBounds[myXRank]; x < xBounds[myXRank+1]; x++) {
				for(int y = yBounds[myYRank]; y < yBounds[myYRank+1]; y++) {
					// Send all data to rank 0, which will write it to a single file
					// send each column separately
					for(int j=1; j<blocks[x][y]->nx+1; j++) {
						int x_pos = x*x_blocksize;
						int y_pos = y*y_blocksize;
						MPI_Put(blocks[x][y]->h.getRawPointer()+1+(blocks[x][y]->ny+2)*j, blocks[x][y]->ny, MPI_FLOAT,
							0, 1+(nyRequested+2)*(1+j+x_pos)+y_pos, blocks[x][y]->ny, MPI_FLOAT, writeBlockWin_h);
						MPI_Put(blocks[x][y]->hu.getRawPointer()+1+(blocks[x][y]->ny+2)*j, blocks[x][y]->ny, MPI_FLOAT,
							0, 1+(nyRequested+2)*(1+j+x_pos)+y_pos, blocks[x][y]->ny, MPI_FLOAT, writeBlockWin_hu);
						MPI_Put(blocks[x][y]->hv.getRawPointer()+1+(blocks[x][y]->ny+2)*j, blocks[x][y]->ny, MPI_FLOAT,
							0, 1+(nyRequested+2)*(1+j+x_pos)+y_pos, blocks[x][y]->ny, MPI_FLOAT, writeBlockWin_hv);
					}
				}
			}
			MPI_Win_fence(0, writeBlockWin_h);
			MPI_Win_fence(0, writeBlockWin_hu);
			MPI_Win_fence(0, writeBlockWin_hv);
		}

		if(write && myRank == 0) {
			writer->writeTimeStep(
				writeBlock.getWaterHeight(),
				writeBlock.getMomentumHorizontal(),
				writeBlock.getMomentumVertical(),
				t);
		}
		if(write)
			MPI_Barrier(MPI_COMM_WORLD);
	}
#ifdef ITT
	__itt_detach();	
#endif

	// temporary stats
	float minTime=1000000.; float maxTime=0.;float avgTime=0.; int blockCount = 0;
	for(int x = xBounds[myXRank]; x < xBounds[myXRank+1]; x++) {
		for(int y = yBounds[myYRank]; y < yBounds[myYRank+1]; y++) {
			if(blocks[x][y]->computeTimeWall < minTime)
				minTime = blocks[x][y]->computeTimeWall;
			if(blocks[x][y]->computeTimeWall > maxTime)
				maxTime = blocks[x][y]->computeTimeWall;
			avgTime += blocks[x][y]->computeTimeWall;
			blockCount++;
		}
	}
	avgTime /= blockCount;
	printf("%d: min:%F, max:%f, avg:%f\n", myRank, minTime, maxTime, avgTime); 
	/************
	 * FINALIZE *
	 ************/

	//TODO: Free all allocated memory
	//TODO: Get times

	//printf("SMP : Compute Time (CPU): %fs - (WALL): %fs | Total Time (Wall): %fs\n", blocks[xBounds[myXRank]][myYRank]->computeTime, blocks[xBounds[myXRank]][myYRank]->computeTimeWall, wallTime);
	double wallTimeWhole = getTime() - startTimeWhole;
	if(myRank == 0)
		printf("Iterations: %d\n", iterations);
	printf("RESULT: Chameleon: Computation ended, walltime:%f\n", wallTimeWhole);

	printf("%d: setGhostLayerTime=%f\n", myRank, (float)setGhostLayerTime);
	printf("%d: receiveGhostLayerTime=%f\n", myRank, (float)receiveGhostLayerTime);
	printf("%d: taskCreateHorizontalTime=%f\n", myRank, (float)taskCreateHorizontalTime);
	printf("%d: taskWaitHorizontalTime=%f\n", myRank, (float)taskWaitHorizontalTime);
	printf("%d: reductionTime=%f\n", myRank, (float)reductionTime);
	printf("%d: taskCreateVerticalTime=%f\n", myRank, (float)taskCreateVerticalTime);
	printf("%d: taskWaitVerticalTime=%f\n", myRank, (float)taskWaitVerticalTime);
	printf("%d: updateUnknownsTime=%f\n", myRank, (float)updateUnknownsTime);
	printf("%d: writeOutputTime=%f\n", myRank, (float)writeOutputTime);

	MPI_Barrier(MPI_COMM_WORLD);

	for(int x = xBounds[myXRank]; x < xBounds[myXRank+1]; x++) {
		for(int y = yBounds[myYRank]; y < yBounds[myYRank+1]; y++) {
			blocks[x][y]->freeMpiType();
		}
	}

    #pragma omp parallel
    {
        chameleon_thread_finalize();
    }
    chameleon_finalize();

	if(write && myRank == 0)
		delete writer;
	MPI_Finalize();

	return 0;
}
