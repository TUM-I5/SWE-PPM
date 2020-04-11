
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
#include "tools/CollectorChameleon.hpp"
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

int main(int argc, char **argv) {


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
    args.addOption("write", 'w', "Write results", tools::Args::Required, false);
    args.addOption("local-timestepping", 'l', "Activate local timestepping", tools::Args::Required, false);
    // Declare the variables needed to hold command line input
    float simulationDuration;
    int numberOfCheckPoints;
    int nxRequested;
    int nyRequested;
    bool localTimestepping = false;
    bool write = false;
    std::string outputBaseName;



    // Declare variables for the output and the simulation time
    std::string outputFileName;

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

    if (args.isSet("local-timestepping") && args.getArgument<int>("local-timestepping") == 1) {
        localTimestepping = true;

    }

    if (args.isSet("write") && args.getArgument<int>("write") == 1)
        write = true;
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
    float *checkpointInstantOfTime = new float[numberOfCheckPoints];
    // Time delta is the time between any two checkpoints
    float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
    // The first checkpoint is reached after 0 + delta t
    checkpointInstantOfTime[0] = checkpointTimeDelta;
    for (int i = 1; i < numberOfCheckPoints; i++) {
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
    float dySimulation = (float) heightScenario / nyRequested;

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
    SWE_DimensionalSplittingMpi simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY,
                                           localTimestepping);
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
    NetCdfWriter *writer;
if(write){
    BoundarySize boundarySize = {{1, 1, 1, 1}};
    outputFileName = generateBaseFileName(outputBaseName, localBlockPositionX, localBlockPositionY);
#ifdef WRITENETCDF
    // Construct a netCDF writer
    writer = new NetCdfWriter(
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

}


    // Write the output at t = 0
    if (write) {
        writer->writeTimeStep(simulation.getWaterHeight(),
                             simulation.getMomentumHorizontal(),
                             simulation.getMomentumVertical(),
                             (float) 0.);
    }


    /********************
     * START SIMULATION *
     ********************/
    float maxLocalTimestep;
    if (localTimestepping) {
        simulation.computeMaxTimestep(0.01, 0.4);
        float localTimestep = simulation.getMaxTimestep();
        // reduce over all ranks
        MPI_Allreduce(&localTimestep, &maxLocalTimestep, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);

        simulation.setMaxLocalTimestep(maxLocalTimestep);

    }

    // Initialize wall timer

    float t = 0.0;

    float timestep;

    // loop over the count of requested checkpoints
    for (int i = 0; i < numberOfCheckPoints; i++) {
        // Simulate until the checkpoint is reached
        while (t < checkpointInstantOfTime[i]) {
            do {
                // Start measurement
                CollectorMpi::getInstance().startCounter(CollectorMpi::CTR_WALL);
                // this is an implicit block (mpi recv in setGhostLayer()
                simulation.setGhostLayer();

                // compute numerical flux on each edge
                simulation.computeNumericalFluxes();

                // max timestep has been reduced over all ranks in computeNumericalFluxes()
                timestep = simulation.getMaxTimestep();

                // update the cell values
                simulation.updateUnknowns(timestep);

                // Accumulate wall time
                CollectorMpi::getInstance().stopCounter(CollectorMpi::CTR_WALL);

            } while (localTimestepping && !simulation.hasMaxLocalTimestep());
            // update simulation time with time step width.
            t += localTimestepping ? maxLocalTimestep : timestep;
            if(localTimestepping){
                simulation.resetStepSizeCounter();
            }
        }

        if (myMpiRank == 0) {
            printf("Write timestep (%fs)\n", t);
        }

        if (write) {
            // write output
            writer->writeTimeStep(
                    simulation.getWaterHeight(),
                    simulation.getMomentumHorizontal(),
                    simulation.getMomentumVertical(),
                    t);
        }

    }


    /************
     * FINALIZE *
     ************/

    if (myMpiRank == 0) {
        CollectorMpi::getInstance().setMasterSettings(true, outputBaseName + ".log");
    }

    CollectorMpi::getInstance().logResults();
    simulation.freeMpiType();
    if (write)
        delete writer;
    MPI_Finalize();
    return 0;
}
