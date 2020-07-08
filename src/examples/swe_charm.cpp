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

#include "swe_charm.hh"

#include <cassert>
#include <string>

#ifdef WRITENETCDF

#include "writer/NetCdfWriter.hh"

#else
#include "writer/VtkWriter.hh"
#endif

#include "blocks/SWE_DimensionalSplittingCharm.hh"
#include "tools/args.hh"
#include "tools/Float2D.hh"
#include "tools/Logger.hh"
#include "tools/ProgressBar.hh"

#ifdef ASAGI
#include "scenarios/SWE_AsagiScenario.hh"
#else

#include "scenarios/SWE_simple_scenarios.hh"

#endif

/* readonly */ CProxy_swe_charm mainProxy;
/* readonly */ int blockCountX;
/* readonly */ int blockCountY;
/* readonly */ float simulationDuration;
/* readonly */ int checkpointCount;

swe_charm::swe_charm(CkMigrateMessage *msg) {}

swe_charm::swe_charm(CkArgMsg *msg) {

   // usesAtSync = true; //enable AtSync()
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
    args.addOption("chares", 'c', "charecount", args.Required, false);
    args.addOption("write", 'w', "Write results", tools::Args::Required, false);

    args.addOption("local-timestepping", 'l', "Activate local timestepping", tools::Args::Required, false);
    // Declare the variables needed to hold command line input
    int nxRequested;
    int nyRequested;
    std::string bathymetryFilename;
    std::string displacementFilename;
    std::string outputBasename;
    float localTimestepping = 0.f;
    bool write = false;
    // Declare variables for the output and the simulation time
    std::string outputFilename;
    float t = 0.;

    // Parse command line arguments
    tools::Args::Result ret = args.parse(msg->argc, msg->argv);
    switch (ret) {
        case tools::Args::Error:
            exit();
        case tools::Args::Help:
            exit();
        case tools::Args::Success:
            break;
    }

    if (args.isSet("write") && args.getArgument<int>("write") == 1){
        CkPrintf("Write Output to file \n");
        write = true;
    }

    if (args.isSet("local-timestepping") && args.getArgument<float>("local-timestepping") > 0 ) {
        localTimestepping =  args.getArgument<float>("local-timestepping");

    }

    // Spawn one chare per CPU
    if (args.isSet("chares")) {
        chareCount = args.getArgument<int>("chares");
    } else {
        chareCount = CkNumPes();
    }
    // Read in command line arguments
    simulationDuration = args.getArgument<float>("simulation-duration");
    checkpointCount = args.getArgument<int>("checkpoint-count");
    nxRequested = args.getArgument<int>("resolution-horizontal");
    nyRequested = args.getArgument<int>("resolution-vertical");
#ifdef ASAGI
    bathymetryFilename = args.getArgument<std::string>("bathymetry-file");
    displacementFilename = args.getArgument<std::string>("displacement-file");
#endif
    outputBasename = args.getArgument<std::string>("output-basepath");


    mainCollector.setMasterSettings(true, outputBasename + ".log",(int)chareCount);
    mainCollector.setRank(0);
    // Initialize Scenario
#ifdef ASAGI
    SWE_AsagiScenario scenario(bathymetryFilename, displacementFilename);
#else
    //SWE_HalfDomainDry scenario;
    //SWE_RadialDamBreakScenario scenario;
    SWE_RadialBathymetryDamBreakScenario scenario;
#endif


    /****************************************
     * INIT WORK CHARES / SIMULATION BLOCKS *
     ****************************************/


    mainProxy = thisProxy;

    /*
     * Calculate the simulation grid layout.
     * The cell count of the scenario as well as the scenario size is fixed,
     * Get the size of the actual domain and divide it by the requested resolution.
     */
    int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
    int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);
    float dxSimulation = (float) widthScenario / nxRequested;
    float dySimulation = (float) heightScenario / nyRequested;

    // Declare empty proxy array (dynamic insertion after parameters have been determined)
    CProxy_SWE_DimensionalSplittingCharm blocks = CProxy_SWE_DimensionalSplittingCharm::ckNew();

    // number of SWE-Blocks in x- and y-direction
    blockCountY = std::sqrt(chareCount);
    while (chareCount % blockCountY != 0) blockCountY--;
    blockCountX = chareCount / blockCountY;

    int localBlockPositionX[chareCount];
    int localBlockPositionY[chareCount];

    // Spawn chares
    for (int i = 0; i < chareCount; i++) {
        // determine the local block position of each SWE_Block
        localBlockPositionX[i] = i / blockCountY;
        localBlockPositionY[i] = i % blockCountY;

        // compute local number of cells for each SWE_Block w.r.t. the simulation domain
        // (particularly not the original scenario domain, which might be finer in resolution)
        // (blocks at the domain boundary are assigned the "remainder" of cells)
        int nxBlockSimulation = nxRequested / blockCountX;
        int nxRemainderSimulation = nxRequested - (blockCountX - 1) * (nxRequested / blockCountX);
        int nyBlockSimulation = nyRequested / blockCountY;
        int nyRemainderSimulation = nyRequested - (blockCountY - 1) * (nyRequested / blockCountY);

        int nxLocal = (localBlockPositionX[i] < blockCountX - 1) ? nxBlockSimulation : nxRemainderSimulation;
        int nyLocal = (localBlockPositionY[i] < blockCountY - 1) ? nyBlockSimulation : nyRemainderSimulation;

        // Compute the origin of the local simulation block w.r.t. the original scenario domain.
        float localOriginX =
                scenario.getBoundaryPos(BND_LEFT) + localBlockPositionX[i] * dxSimulation * nxBlockSimulation;
        float localOriginY =
                scenario.getBoundaryPos(BND_BOTTOM) + localBlockPositionY[i] * dySimulation * nyBlockSimulation;

        // Determine the boundary types for the SWE_Block:
        // block boundaries bordering other blocks have a CONNECT boundary,
        // block boundaries bordering the entire scenario have the respective scenario boundary type
        BoundaryType boundaries[4];

        boundaries[BND_LEFT] = (localBlockPositionX[i] > 0) ? CONNECT : scenario.getBoundaryType(BND_LEFT);
        boundaries[BND_RIGHT] = (localBlockPositionX[i] < blockCountX - 1) ? CONNECT : scenario.getBoundaryType(
                BND_RIGHT);
        boundaries[BND_BOTTOM] = (localBlockPositionY[i] > 0) ? CONNECT : scenario.getBoundaryType(BND_BOTTOM);
        boundaries[BND_TOP] = (localBlockPositionY[i] < blockCountY - 1) ? CONNECT : scenario.getBoundaryType(BND_TOP);

        outputFilename = generateBaseFileName(outputBasename, localBlockPositionX[i], localBlockPositionY[i]);

        // Spawn chare for the current block and insert it into the proxy array
#ifdef ASAGI
        blocks[i].insert(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, localBlockPositionX[i], localBlockPositionY[i],
                 boundaries, outputFilename, bathymetryFilename, displacementFilename, localTimestepping,write);
#else

        blocks[i].insert(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY,
                         localBlockPositionX[i], localBlockPositionY[i],
                         boundaries, outputFilename, "", "", localTimestepping,write);

#endif
    }
    blocks.doneInserting();
    blocks.compute();
}

void swe_charm::done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce,
                     double ctr_wall) {
    double serialized[5] = {ctr_flop, ctr_exchange, ctr_barrier, ctr_reduce, ctr_wall};
    mainCollector += CollectorCharm::deserialize(serialized);

    if(--chareCount == 0){
        exit();
    }

}
void swe_charm::allEnd() {
    mainCollector.logResults();
    CkExit();
}
void swe_charm::exit() {
    CkExit();
}

#include "swe_charm.def.h"
