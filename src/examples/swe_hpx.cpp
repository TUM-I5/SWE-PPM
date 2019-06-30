//
// Created by martin on 05/06/19.
//

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
#include <algorithm>
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

#include "blocks/SWE_DimensionalSplittingHpx.hh"

#include "blocks/SWE_Hpx_Component.hpp"

#include <hpx/hpx_init.hpp>
#include <hpx/include/compute.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/async.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/parallel_algorithm.hpp>
#include <hpx/lcos/broadcast.hpp>
#include <hpx/util/unwrap.hpp>
/*SWE_DimensionalSplittingComponent initializeWorker (std::size_t rank,std::size_t totalRanks,hpx::id_type locality, float simulationDuration,
int numberOfCheckPoints,
int nxRequested,
int nyRequested,
 std::string outputBaseName,
std::string const &batFile,
std::string const &displFile){


    std::string outputFileName;
    // Initialize Scenario
#ifdef ASAGI
    SWE_AsagiScenario scenario(batFile, displFile);
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


    int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
    int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);
    float dxSimulation = (float) widthScenario / nxRequested;
    float dySimulation = (float) heightScenario / nyRequested;


    auto myHpxRank = rank;
    auto totalHpxRanks = totalRanks;

    // Print status
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);

    printf("%i Spawned at %s\n", myHpxRank, hostname);

    // number of SWE-Blocks in x- and y-direction
    int blockCountY = std::sqrt(totalHpxRanks);
    while (totalHpxRanks % blockCountY != 0) blockCountY--;
    int blockCountX = totalHpxRanks / blockCountY;

    // determine the local block position of each SWE_Block
    int localBlockPositionX = myHpxRank / blockCountY;
    int localBlockPositionY = myHpxRank % blockCountY;

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
    std::array<BoundaryType,4> boundaries;

    boundaries[BND_LEFT] = (localBlockPositionX > 0) ? CONNECT : scenario.getBoundaryType(BND_LEFT);
    boundaries[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? CONNECT : scenario.getBoundaryType(BND_RIGHT);
    boundaries[BND_BOTTOM] = (localBlockPositionY > 0) ? CONNECT : scenario.getBoundaryType(BND_BOTTOM);
    boundaries[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? CONNECT : scenario.getBoundaryType(BND_TOP);

    std::array<int, 4> myNeighbours;
    myNeighbours[BND_LEFT] = (localBlockPositionX > 0) ? myHpxRank - blockCountY : -1;
    myNeighbours[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? myHpxRank + blockCountY : -1;
    myNeighbours[BND_BOTTOM] = (localBlockPositionY > 0) ? myHpxRank - 1 : -1;
    myNeighbours[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? myHpxRank + 1 : -1;



#ifdef ASAGI
    return hpx::new_<SWE_DimensionalSplittingComponent>(locality,myHpxRank,totalHpxRanks,simulationDuration,numberOfCheckPoints,
             nxLocal,nyLocal,dxSimulation,dySimulation,localOriginX,localOriginY,boundaries, myNeighbours,batFile, displFile);
#else
    return hpx::new_<SWE_DimensionalSplittingComponent>(locality,myHpxRank,totalHpxRanks,simulationDuration,numberOfCheckPoints,
                                        nxLocal,nyLocal,dxSimulation,dySimulation,localOriginX,localOriginY,boundaries, myNeighbours);
#endif




} */

void cycle(int totalRanks,
           float simulationDuration,
           int numberOfCheckPoints,
           int nxRequested,
           int nyRequested,
           std::string outputBaseName,
           std::string const &batFile,
           std::string const &displFile){
/*
   std::vector<SWE_DimensionalSplittingComponent> blocks;
    for(int i = 0 ; i < totalRanks; i++){
        blocks.push_back(initializeWorker(i,totalRanks,hpx::find_here(),
                                          simulationDuration,
                                          numberOfCheckPoints,
                                          nxRequested,
                                          nyRequested,
                                          outputBaseName,
                                          batFile,
                                          displFile));

    }

    std::vector<hpx::future<void>> fut;
    std::vector<hpx::naming::id_type> block_ids;
    for(auto comp : blocks ) block_ids.push_back(comp.get_id());

    hpx::lcos::broadcast<remote::SWE_DimensionalSplittingComponent::exchangeBathymetry_action>(block_ids).wait();

    // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
    float* checkpointInstantOfTime = new float[numberOfCheckPoints];
    // Time delta is the time between any two checkpoints
    float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
    // The first checkpoint is reached after 0 + delta t
    checkpointInstantOfTime[0] = checkpointTimeDelta;
    for(int i = 1; i < numberOfCheckPoints; i++) {
        checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
    }


    // Initialize wall timer
    struct timespec startTime;
    struct timespec endTime;

    float wallTime = 0.;
    float t = 0.;


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

            hpx::future<void> ghostlayer = hpx::lcos::broadcast<remote::SWE_DimensionalSplittingComponent::setGhostLayer_action>(block_ids);
           // ghostlayer.get();
            hpx::future<std::vector<float>> timestepsFut= hpx::dataflow(hpx::util::unwrapping(
                    [&block_ids]()-> hpx::future<std::vector<float>> {
                    return hpx::lcos::broadcast<remote::SWE_DimensionalSplittingComponent::getMaxTimestep_action>(block_ids);
            })
                    ,std::move(ghostlayer));

            hpx::future<float> minTimestep = hpx::dataflow(hpx::util::unwrapping([](std::vector<float> timesteps)-> float {

                return *std::min_element(timesteps.begin(), timesteps.end());

            }),std::move(timestepsFut));


            float timestep = minTimestep.get();

            hpx::lcos::broadcast<remote::SWE_DimensionalSplittingComponent::setMaxTimestep_action>(block_ids,timestep).get();

            clock_gettime(CLOCK_MONOTONIC, &endTime);
            wallTime += (endTime.tv_sec - startTime.tv_sec);
            wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

            // update simulation time with time step width.
            t += timestep;
            iterations++;


        }

        printf("Write timestep (%fs)\n", t);
        //hpx::cout <<"Write timestep " << t<<"s" << std::endl;

    }


    float totalCommTime = 0;
    float sumFlops = 0;
    for(auto block: blocks){
        block.printResult();
        totalCommTime += block.getCommunicationTime();
        sumFlops += block.getFlopCount();
    }


    hpx::cout   << "Flop count: " << sumFlops << std::endl
                << "Flops(Total): " << ((float)sumFlops)/(wallTime*1000000000) << "GFLOPS"<< std::endl;
    hpx::cout   << "Total Time (Wall): "<< wallTime <<"s"<<hpx::endl;
    hpx::cout   << "Communication Time(Total): "<< totalCommTime <<"s"<<hpx::endl;
*/
}


int hpx_main(boost::program_options::variables_map& vm)
{


    /**************
 * INIT INPUT *
 **************/
    float simulationDuration;
    int numberOfCheckPoints;
    int nxRequested;
    int nyRequested;
    int totalRanks;
    std::string outputBaseName;
    std::string batFile;
    std::string displFile;
    // Define command line arguments
    tools::Args args;

#ifdef ASAGI
    args.addOption("bathymetry-file", 'b', "File containing the bathymetry");
	args.addOption("displacement-file", 'd', "File containing the displacement");
#endif

    args.addOption("simulation-duration", 'e', "Time in seconds to simulate");
    args.addOption("checkpoint-count", 'n', "Number of simulation snapshots to be written");
    args.addOption("resolution-horizontal", 'x', "Number of simulation cells in horizontal direction");
    args.addOption("resolution-vertical", 'y', "Number of simulated cells in y-direction");
    args.addOption("output-basepath", 'o', "Output base file name");


    // Declare the variables needed to hold command line input

    // Declare variables for the output and the simulation time


/*
    // Parse command line arguments
    tools::Args::Result ret = args.parse(argc, argv);
    switch (ret)
    {
        case tools::Args::Error:
            return hpx::finalize();
        case tools::Args::Help:
            return hpx::finalize();
        case tools::Args::Success:
            break;
    }
    // Read in command line arguments
    simulationDuration = args.getArgument<float>("simulation-duration");
    numberOfCheckPoints = args.getArgument<int>("checkpoint-count");
    nxRequested = args.getArgument<int>("resolution-horizontal");
    nyRequested = args.getArgument<int>("resolution-vertical");
    outputBaseName = args.getArgument<std::string>("output-basepath");

#ifdef ASAGI
   batFile = args.getArgument<std::string>("bathymetry-file");
   displFile = args.getArgument<std::string>("displacement-file");
#endif
*/

    simulationDuration =  vm["simulation-duration"].as<float>();

       numberOfCheckPoints = vm["checkpoint-count"].as<int>();
    nxRequested = vm["resolution-horizontal"].as<int>();
    nyRequested = vm["resolution-vertical"].as<int>();
    totalRanks = vm["blocks"].as<int>();
    outputBaseName = vm["output-basepath"].as<std::string>();
#ifdef ASAGI
    batFile = vm["bathymetry-file"].as<std::string>();
   displFile = vm["displacement-file"].as<std::string>();
#endif

    auto test = hpx::new_<SWE_Hpx_Component>(hpx::find_here(),totalRanks,simulationDuration,numberOfCheckPoints,
                                                        nxRequested,nyRequested,outputBaseName,batFile,displFile);
    test.run();
    return hpx::finalize();
}
int main(int argc, char** argv) {


    /**************
     * INIT INPUT *
     **************/
    using namespace boost::program_options;

    options_description desc_commandline;



    desc_commandline.add_options()
     ("bathymetry-file,b", value<std::string>(),"File containing the bathymetry")
     ("displacement-file,d",value<std::string>(), "File containing the displacement")
     ("simulation-duration,e", value<float>()->default_value(100),"Time in seconds to simulate")
     ("checkpoint-count,n", value<int>()->default_value(100),"Number of simulation snapshots to be written")
     ("resolution-horizontal",value<int>()->default_value(100), "Number of simulation cells in horizontal direction")
     ("resolution-vertical", value<int>()->default_value(100),"Number of simulated cells in y-direction")
     ("output-basepath,o", value<std::string>()->default_value("lolo"),"Output base file name")
            ("blocks,", value<int>()->default_value(1),"Number of swe blocks")
    ;

    // Initialize and run HPX, this example requires to run hpx_main on all
    // localities
    std::vector<std::string> const cfg = {
            //"hpx.run_hpx_main!=1"
    };

    return hpx::init(desc_commandline,argc, argv);//,cfg);
}
