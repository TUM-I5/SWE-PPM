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
//#include "blocks/SWE_DimensionalSplittingComponent.hpp"
#include "blocks/SWE_Hpx_Component.hpp"

#include <hpx/hpx_init.hpp>
#include <hpx/include/compute.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/async.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/parallel_algorithm.hpp>



SWE_Hpx_Component initializeWorker (std::size_t rank,std::size_t totalRanks,hpx::id_type locality, float simulationDuration,
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


    auto myHpxRank = rank;
    auto totalHpxRanks = totalRanks;

    // Print status
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);

    printf("%i Spawned at %s\n", myHpxRank, hostname);

    /*
     * determine the layout of UPC++ ranks:
     * one block per process;
     * if the number of processes is a square number, l_blockCountX = l_blockCountY,
     * else l_blockCountX > l_blockCountY
     */
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

    return hpx::new_<SWE_Hpx_Component>(locality,myHpxRank,totalHpxRanks,simulationDuration,numberOfCheckPoints,
             nxLocal,nyLocal,dxSimulation,dySimulation,localOriginX,localOriginY,boundaries, myNeighbours);



   // communicator_type comm(myHpxRank,totalHpxRanks,std::move(myNeighbours));

   /*SWE_DimensionalSplittingHpx simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, comm);
    simulation.initScenario(scenario, boundaries);

    std::cout<< "Rank: " << myHpxRank << std::endl
            << "Left " << myNeighbours[BND_LEFT] << std::endl
            << "Right " << myNeighbours[BND_RIGHT] << std::endl
            << "Bottom " << myNeighbours[BND_BOTTOM] << std::endl
            << "TOP " << myNeighbours[BND_TOP] << std::endl;
    simulation.exchangeBathymetry();




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


   */

}

void cycle( float simulationDuration,
           int numberOfCheckPoints,
           int nxRequested,
           int nyRequested,
           std::string outputBaseName,
           std::string const &batFile,
           std::string const &displFile){

  /*  std::vector<SWE_DimensionalSplittingComponent> blocks;
    int totalRanks= hpx::get_num_worker_threads();
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

    for(auto block : blocks)fut.push_back(block.exchangeBathymetry());
    hpx::when_all(fut).wait();

    hpx::cout << "STARTED\n";


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
            fut = std::vector<hpx::future<void>>();
            for(auto block : blocks)fut.push_back(block.setGhostLayer());

            hpx::when_all(fut).wait();
            //hpx::cout << "Finished ghostlayer" << std::endl;
            // compute numerical flux on each edge
            // for(auto block : blocks)block.computeNumericalFluxes(ids);
            fut = std::vector<hpx::future<void>>();
            for(auto block : blocks)fut.push_back(block.computeXSweep());
            hpx::when_all(fut).wait();
           // hpx::cout << "Finished xSweep" << std::endl;
            std::vector<float> timesteps;
            for(auto block : blocks)timesteps.push_back(block.getMaxTimestep().get());

            timestep = *std::min_element(timesteps.begin(), timesteps.end());
            //hpx::cout << "Finished reduction" << std::endl;
            fut = std::vector<hpx::future<void>>();
            for(auto block : blocks)fut.push_back(block.setMaxTimestep(timestep));
            hpx::when_all(fut).wait();

            fut = std::vector<hpx::future<void>>();
            for(auto block : blocks)fut.push_back(block.computeYSweep());
            //hpx::cout << "Finished ySweep" << std::endl;
            // max timestep has been reduced over all ranks in computeNumericalFluxes()

            hpx::when_all(fut).wait();

            // update the cell values
            //simulation.updateUnknowns(timestep);

            // Accumulate wall time
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            wallTime += (endTime.tv_sec - startTime.tv_sec);
            wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

            // update simulation time with time step width.
            t += timestep;
            iterations++;


        }


        hpx::cout <<"Write timestep " << t<<"s" << std::endl;


        // write output
        /* writer.writeTimeStep(
                 simulation.getWaterHeight(),
                 simulation.getMomentumHorizontal(),
                 simulation.getMomentumVertical(),
                 t);
    }
*/
/*
    float totalCommTime = 0;
    float sumFlops = 0;
    for(auto block: blocks){
        block.printResult();
        totalCommTime += block.getCommunicationTime();
        sumFlops += block.getFlopCount();
    }



    //uint64_t  sumFlops = upcxx::reduce_all(simulation.getFlops(), upcxx::op_fast_add).wait();

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
    outputBaseName = vm["output-basepath"].as<std::string>();
#ifdef ASAGI
    batFile = vm["bathymetry-file"].as<std::string>();
   displFile = vm["displacement-file"].as<std::string>();
#endif

        int totalWorkers =  hpx::get_num_worker_threads();
        int nodes       =  hpx::get_num_localities(hpx::launch::sync);
        int workersPerNode = totalWorkers/nodes;
        // Create a single instance of the component on this locality.
        std::cout << "Application has access to:" << std::endl
                    << "Workers: " << totalWorkers << std::endl
                    << "Nodes: " << nodes << std::endl
                    << "Workers/Node: " << workersPerNode << std::endl;

        std::vector<SWE_Hpx_Component> blocks;

        int indiRank = 0;
        for(auto &locality : hpx::find_all_localities()){
            for(int i = 0; i < workersPerNode; i++){
                blocks.push_back(initializeWorker(indiRank,totalWorkers,locality,
                                                  simulationDuration,
                                                  numberOfCheckPoints,
                                                  nxRequested,
                                                  nyRequested,
                                                  outputBaseName,
                                                  batFile,
                                                  displFile));
                indiRank++;
            }
        }

        std::vector<hpx::naming::id_type> test;
        std::vector<hpx::future<void>> comps;
        for(auto comp : blocks ) test.push_back(comp.get_id());

        for(auto &block : blocks) {
            comps.push_back(block.initialize(test));
        }
        hpx::when_all(comps).get();

    float totalCommTime = 0;
    float sumFlops = 0;
    for(auto block: blocks){

        totalCommTime += block.getCommunicationTime();
        sumFlops += block.getFlopCount();
    }


    float wallTime = blocks[0].getWallTime();
    //uint64_t  sumFlops = upcxx::reduce_all(simulation.getFlops(), upcxx::op_fast_add).wait();

    hpx::cout   << "Flop count: " << sumFlops << std::endl
                << "Flops(Total): " << ((float)sumFlops)/(wallTime*1000000000) << "GFLOPS"<< std::endl;
    hpx::cout   << "Total Time (Wall): "<< wallTime <<"s"<<hpx::endl;
    hpx::cout   << "Communication Time(Total): "<< totalCommTime <<"s"<<hpx::endl;


/*    cycle(simulationDuration,
          numberOfCheckPoints,
          nxRequested,
          nyRequested,
          outputBaseName,
          batFile,
          displFile);*/
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

    ;

    // Initialize and run HPX, this example requires to run hpx_main on all
    // localities
    std::vector<std::string> const cfg = {
            //"hpx.run_hpx_main!=1"
    };

    return hpx::init(desc_commandline,argc, argv);//,cfg);
}
