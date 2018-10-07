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

#include "blocks/SWE_DimensionalSplitting.hh"

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


	/**************
	 * INIT BLOCK *
	 **************/


	/*
	 * Calculate the simulation grid layout.
	 * The cell count of the scenario as well as the scenario size is fixed, 
	 * Get the size of the actual domain and divide it by the requested resolution.
	 */
	int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
	int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);
	float dxSimulation = (float) widthScenario / nxRequested;
	float dySimulation = (float) heightScenario / nyRequested;
	float originX = scenario.getBoundaryPos(BND_LEFT);
	float originY = scenario.getBoundaryPos(BND_BOTTOM);

	BoundaryType boundaries[4];

	boundaries[BND_LEFT] = scenario.getBoundaryType(BND_LEFT);
	boundaries[BND_RIGHT] = scenario.getBoundaryType(BND_RIGHT);
	boundaries[BND_BOTTOM] = scenario.getBoundaryType(BND_BOTTOM);
	boundaries[BND_TOP] = scenario.getBoundaryType(BND_TOP);

	SWE_DimensionalSplitting simulation(nxRequested, nyRequested, dxSimulation, dySimulation, originX, originY);
	simulation.initScenario(scenario, boundaries);


	/***************
	 * INIT OUTPUT *
	 ***************/


	// Initialize boundary size of the ghost layers
	BoundarySize boundarySize = {{1, 1, 1, 1}};
	outputFileName = outputBaseName;
#ifdef WRITENETCDF
	// Construct a netCDF writer
	NetCdfWriter writer(
			outputFileName,
			simulation.getBathymetry(),
			boundarySize,
			nxRequested,
			nyRequested,
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
			nxRequested,
			nyRequested,
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


	// Initialize timers
	std::clock_t computeClock;
	std::clock_t commClock;

	struct timespec startTime;
	struct timespec endTime;

	float computeTime = 0.;
	float commTime = 0.;
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
			commClock = clock();

			// set values in ghost cells.
			// we need to sync here since block boundaries get exchanged over ranks
			// TODO: what can we do if this becomes a bottleneck?
			simulation.setGhostLayer();

			// Accumulate comm time and start compute clock
			commClock = clock() - commClock;
			commTime += (float) commClock / CLOCKS_PER_SEC;
			computeClock = clock();

			// compute numerical flux on each edge
			simulation.computeNumericalFluxes();

			// max timestep has been reduced over all ranks in computeNumericalFluxes()
			timestep = simulation.getMaxTimestep();

			// update the cell values
			simulation.updateUnknowns(timestep);

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
		}

		printf("Write timestep (%fs)\n", t);

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


	printf("SMP : Compute Time (CPU): %fs - (WALL): %fs | Total Time (Wall): %fs\n", simulation.computeTime, simulation.computeTimeWall, wallTime); 

	return 0;
}
