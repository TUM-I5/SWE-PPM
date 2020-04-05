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

#include "blocks/SWE_DimensionalSplitting.hh"

#ifdef WRITENETCDF

#include "writer/NetCdfWriter.hh"

#else
#include "writer/VtkWriter.hh"
#endif

#include "tools/args.hh"
#include "tools/Float2D.hh"
#include "tools/Logger.hh"
#include "tools/ProgressBar.hh"
//#include "scenarios/SWE_TsunamiScenario.hh"
#include "scenarios/SWE_simple_scenarios.hh"

int main(int argc, char **argv) {


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
    // Options
    simulationDuration = args.getArgument<float>("simulation-duration");
    numberOfCheckPoints = args.getArgument<int>("checkpoint-count");
    nxRequested = args.getArgument<int>("resolution-horizontal");
    nyRequested = args.getArgument<int>("resolution-vertical");
    outputBaseName = args.getArgument<std::string>("output-basepath");

    // Initialize netCDF scenario which reads the input files
    //SWE_TsunamiScenario scenario(bathymetryFileName.c_str(), displacementFileName.c_str());
    SWE_RadialDamBreakScenario scenario;

    /*
     * Initialize the simulation block according to the scenario
     * Calculate the cell widths of the grid used by the simulation:
     * Get the size of the actual domain and divide it by the requested resolution.
     */
    //float dxSimulation = scenario.getNx() * scenario.getDx() / nxRequested;
    //float dySimulation = scenario.getNy() * scenario.getDy() / nyRequested;
    float dxSimulation = 1;
    float dySimulation = 1;
    SWE_DimensionalSplitting simulation(nxRequested, nyRequested, dxSimulation, dySimulation);
    simulation.initScenarioImplicit(scenario);

    // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
    float *checkpointInstantOfTime = new float[numberOfCheckPoints];
    // Time delta is the time between any two checkpoints
    float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
    // The first checkpoint is reached after 0 + delta t
    checkpointInstantOfTime[0] = checkpointTimeDelta;
    for (int i = 1; i < numberOfCheckPoints; i++) {
        checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
    }


    /***************
     * INIT OUTPUT *
     ***************/


    // Initialize boundary size of the ghost layers
    io::BoundarySize boundarySize = {{1, 1, 1, 1}};
#ifdef WRITENETCDF
    // Construct a netCDF writer
    outputFileName = outputBaseName;
    io::NetCdfWriter writer(
            outputFileName,
            simulation.getBathymetry(),
            boundarySize,
            nxRequested,
            nyRequested,
            dxSimulation,
            dySimulation,
            0,
            0);
#else
    // Construct a vtk writer
    outputFileName = generateBaseFileName(outputBaseName, 0, 0);
    io::VtkWriter writer(
            outputFileName,
            simulation.getBathymetry(),
            boundarySize,
            nxRequested,
            nyRequested,
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
            simulation.getDischarge_hu(),
            simulation.getDischarge_hv(),
            (float) 0.);


    /********************
     * START SIMULATION *
     ********************/


    // print the start message and reset the wall clock time
    progressBar.clear();
    tools::Logger::logger.printStartMessage();
    tools::Logger::logger.initWallClockTime(time(NULL));

    //! simulation time.
    t = 0.0;
    progressBar.update(t);

    unsigned int iterations = 0;

    // loop over the count of requested checkpoints
    for (int i = 0; i < numberOfCheckPoints; i++) {
        // Simulate until a checkpoint is reached
        while (t < checkpointInstantOfTime[i]) {
            // set values in ghost cells:
            simulation.setGhostLayer();

            // reset the cpu clock
            tools::Logger::logger.resetClockToCurrentTime("Cpu");

            // compute numerical flux on each edge
            simulation.computeNumericalFluxes();

            // update the cell values
            simulation.updateUnknowns(simulation.getMaxTimestep());

            // update the cpu time in the logger
            tools::Logger::logger.updateTime("Cpu");

            // update simulation time with time step width.
            t += simulation.getMaxTimestep();
            iterations++;

            // print the current simulation time
            progressBar.clear();
            tools::Logger::logger.printSimulationTime(t);
            progressBar.update(t);
        }

        // print the time elapsed until this output
        progressBar.clear();
        tools::Logger::logger.printOutputTime(t);
        progressBar.update(t);

        // write output
        writer.writeTimeStep(
                simulation.getWaterHeight(),
                simulation.getDischarge_hu(),
                simulation.getDischarge_hv(),
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

    // print the wall clock time (includes plotting)
    tools::Logger::logger.printWallClockTime(time(NULL));

    // printer iteration counter
    tools::Logger::logger.printIterationsDone(iterations);

    return 0;
}
