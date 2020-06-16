//
// Created by martin on 07/04/2020.
//

//
// Created by martin on 01/07/19.
//


//
// Created by martin on 16/06/19.


#include <mpi.h>
#include <algorithm>
#include <iostream>
#include "tools/args.hh"
#include <limits.h>
#include "chameleon.h"
#include "tools/CollectorChameleon.hpp"
#include <unistd.h>
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




std::array<int, 4>
getNeighbours(int localBlockPositionX, int localBlockPositionY, int blockCountX, int blockCountY, int myRank) {
    std::array<int, 4> myNeighbours;
    myNeighbours[BND_LEFT] = (localBlockPositionX > 0) ? myRank - blockCountY : -1;
    myNeighbours[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? myRank + blockCountY : -1;
    myNeighbours[BND_BOTTOM] = (localBlockPositionY > 0) ? myRank - 1 : -1;
    myNeighbours[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? myRank + 1 : -1;
    return myNeighbours;
}


int main(int argc, char** argv) {



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
    args.addOption("blocks", 'b', "Blocks per rank", tools::Args::Required, false);

    args.addOption("write", 'w', "Write results", tools::Args::Required, false);
    //args.addOption("iteration-count", 'i', "Iteration Count (Overrides t and n)", tools::Args::Required, false);
    args.addOption("local-timestepping", 'l', "Activate local timestepping", tools::Args::Required, false);
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
    float localTimestepping = 0.f;
    std::vector<std::shared_ptr<SWE_DimensionalSplittingChameleon>> simulationBlocks;
    int ranksPerLocality = 1;
    if (args.isSet("local-timestepping") && args.getArgument<float>("local-timestepping") > 0 ) {
        localTimestepping =  args.getArgument<float>("local-timestepping");

    }
    if (args.isSet("blocks")) {
        ranksPerLocality = args.getArgument<int>("blocks");
    }
    if(args.isSet("write") && args.getArgument<int>("write") == 1)
        write = true;


    // Initialize Scenario
#ifdef ASAGI
    SWE_AsagiScenario scenario(args.getArgument<std::string>("bathymetry-file"), args.getArgument<std::string>("displacement-file"));
#else
    //SWE_RadialDamBreakScenario scenario;
    SWE_RadialBathymetryDamBreakScenario scenario;
#endif

    // Init MPI
    int localityRank, localityCount;
    int provided;
    int requested = MPI_THREAD_MULTIPLE;
    MPI_Init_thread(&argc, &argv, requested, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &localityCount);
    MPI_Comm_rank(MPI_COMM_WORLD, &localityRank);

    // Print status
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);

    printf("%i Spawned at %s\n", localityRank, hostname);
    int totalRanks = ranksPerLocality * localityCount;

    // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
    float *checkpointInstantOfTime = new float[numberOfCheckPoints];
    // Time delta is the time between any two checkpoints
    float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
    // The first checkpoint is reached after 0 + delta t
    checkpointInstantOfTime[0] = checkpointTimeDelta;
    for (int i = 1; i < numberOfCheckPoints; i++) {
        checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
    }


    int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
    int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);

    float dxSimulation = (float) widthScenario / nxRequested;
    float dySimulation = (float) heightScenario / nyRequested;



    // number of SWE-Blocks in x- and y-direction
    int blockCountY = std::sqrt(totalRanks);
    while (totalRanks % blockCountY != 0) blockCountY--;
    int blockCountX = totalRanks / blockCountY;


    int startPoint = localityRank * ranksPerLocality;

    for (int i = startPoint; i < startPoint + ranksPerLocality; i++) {
        auto myRank = i;
        int localBlockPositionX = myRank / blockCountY;
        int localBlockPositionY = myRank % blockCountY;

        // compute local number of cells for each SWE_Block w.r.t. the simulation domain
        // (particularly not the original scenario domain, which might be finer in resolution)
        // (blocks at the domain boundary are assigned the "remainder" of cells)

        int nxBlockSimulation = (nxRequested) / blockCountX;
        int nxRemainderSimulation = nxRequested - (blockCountX - 1) * nxBlockSimulation;
        int nyBlockSimulation = nyRequested / blockCountY;
        int nyRemainderSimulation = nyRequested - (blockCountY - 1) * nyBlockSimulation;

        int nxLocal = (localBlockPositionX < blockCountX - 1) ? nxBlockSimulation : nxRemainderSimulation;
        int nyLocal = (localBlockPositionY < blockCountY - 1) ? nyBlockSimulation : nyRemainderSimulation;



        // Compute the origin of the local simulation block w.r.t. the original scenario domain.
        float localOriginX =
                scenario.getBoundaryPos(BND_LEFT) + localBlockPositionX * dxSimulation * nxBlockSimulation;
        float localOriginY =
                scenario.getBoundaryPos(BND_BOTTOM) + localBlockPositionY * dySimulation * nyBlockSimulation;
        std::string outputFileName = generateBaseFileName(outputBaseName, localBlockPositionX, localBlockPositionY);


        simulationBlocks.push_back(std::shared_ptr<SWE_DimensionalSplittingChameleon>(
                new SWE_DimensionalSplittingChameleon(nxLocal, nyLocal, dxSimulation, dySimulation,
                                                localOriginX, localOriginY, localTimestepping, outputFileName,write)));

        //simulationBlocks[i - startPoint]->initScenario(scenario, boundaries.data());
    }

    for (int i = startPoint; i < startPoint + ranksPerLocality; i++) {
        auto myRank = i;
        int localBlockPositionX = myRank / blockCountY;
        int localBlockPositionY = myRank % blockCountY;
        std::array<int, 4> myNeighbours = getNeighbours(localBlockPositionX, localBlockPositionY, blockCountX,
                                                        blockCountY, myRank);

        int refinedNeighbours[4];
        int realNeighbours[4];
        std::array<std::shared_ptr<SWE_DimensionalSplittingChameleon>, 4> neighbourBlocks;
        std::array<BoundaryType, 4> boundaries;

        for (int j = 0; j < 4; j++) {
            if (myNeighbours[j] >= startPoint && myNeighbours[j] < (startPoint + ranksPerLocality)) {
                refinedNeighbours[j] = -2;
                realNeighbours[j] = myNeighbours[j];
                neighbourBlocks[j] = simulationBlocks[myNeighbours[j] - startPoint];
                boundaries[j] = CONNECT_WITHIN_RANK;
            }else if(myNeighbours[j] == -1){
                boundaries[j] = scenario.getBoundaryType((Boundary)j);
                refinedNeighbours[j] = -1;
                realNeighbours[j] = -1;
            } else {
                realNeighbours[j] = myNeighbours[j];
                refinedNeighbours[j] = myNeighbours[j] / ranksPerLocality;
                boundaries[j] = CONNECT;
            }
        }
        simulationBlocks[i - startPoint]->initScenario(scenario, boundaries.data());
        simulationBlocks[i - startPoint]->connectNeighbourLocalities(refinedNeighbours);
        simulationBlocks[i - startPoint]->connectNeighbours(realNeighbours);
        simulationBlocks[i - startPoint]->connectLocalNeighbours(neighbourBlocks);
        simulationBlocks[i - startPoint]->setRank(myRank);
        simulationBlocks[i - startPoint]->setDuration(simulationDuration);
       //std::cout << myRank <<"| " << realNeighbours[0] << " " << realNeighbours[1] << " " << realNeighbours[2] << " " << realNeighbours[3] << std::endl;

    }

#pragma omp parallel
    {
        chameleon_thread_init();
    }
    chameleon_determine_base_addresses((void *)&main);


    for (auto &block: simulationBlocks)block->sendBathymetry();
    for (auto &block: simulationBlocks)block->recvBathymetry();


    
    std::vector<float> timesteps;
    float maxLocalTimestep;

    if (localTimestepping) {
        float localTimestep = 0;
        for (auto &block: simulationBlocks){
            block->computeMaxTimestep(0.01, 0.4);
            localTimestep = std::max(localTimestep, block->getMaxTimestep());
        }

        MPI_Allreduce(&localTimestep, &maxLocalTimestep, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
        maxLocalTimestep = localTimestepping;
        for (auto &block: simulationBlocks)block->setMaxLocalTimestep(maxLocalTimestep);
    }



    float t = 0.;
    bool synchronizedTimestep = true;

    float timestep;

    if (write) {
        for (auto &block: simulationBlocks) {
            block->writeTimestep(0);
        }
    }

    CollectorChameleon collector;
    // loop over the count of requested

    for (int i = 0; i < numberOfCheckPoints; i++) {
        // Simulate until the checkpoint is reached
        while (t < checkpointInstantOfTime[i]) {
            do {

                collector.startCounter(CollectorChameleon::CTR_WALL);
#pragma omp parallel
                {
#pragma omp for
                for (int i = 0; i < simulationBlocks.size(); i++){
                        simulationBlocks[i]->setGhostLayer();
                    }

                }
#pragma omp parallel
                {
#pragma omp for
                    for (int i = 0; i < simulationBlocks.size(); i++){
                        simulationBlocks[i]->receiveGhostLayer();
                    }

                }

#pragma omp parallel
                {
#pragma omp for
                    for (int i = 0; i < simulationBlocks.size(); i++){
                        simulationBlocks[i]->computeNumericalFluxes();
                    }
                    chameleon_distributed_taskwait(0);
                }


                if (!localTimestepping) {
                    collector.startCounter(CollectorChameleon::CTR_REDUCE);
                    timesteps.clear();
                    for (auto &block: simulationBlocks)timesteps.push_back(block->maxTimestep);

                    float minTimestep = *std::min_element(timesteps.begin(), timesteps.end());
                    MPI_Allreduce(&minTimestep, &timestep, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);

                    for (auto &block: simulationBlocks)block->maxTimestep = timestep;
                    collector.stopCounter(CollectorChameleon::CTR_REDUCE);
                }else {
                    for (auto &block: simulationBlocks){
                        if(block->allGhostlayersInSync()){
                            block->maxTimestep = block->getRoundTimestep(block->maxTimestep);
                        }
                    }

                }

#pragma omp parallel
                {
#pragma omp for
                    for (int i = 0; i < simulationBlocks.size(); i++){
                        simulationBlocks[i]->updateUnknowns(timestep);
                    }
                    chameleon_distributed_taskwait(0);
                }

                collector.stopCounter(CollectorChameleon::CTR_WALL);

                if (localTimestepping) {
                    //if each block got the maxLocalTimestep the timestep is finished
                    synchronizedTimestep = true;
                    for (auto &block : simulationBlocks) {
                        if (!block->hasMaxLocalTimestep()) {
                            synchronizedTimestep = false;
                        }
                    }
                }

            } while (localTimestepping && !synchronizedTimestep);
            // update simulation time with time step width.
            t += localTimestepping ? maxLocalTimestep : timestep;
            if(localTimestepping){
                for (auto &block: simulationBlocks)block->resetStepSizeCounter();
            }
        }

        if (localityRank == 0) {
            printf("Write timestep (%fs)\n", t);
        }
        if (write) {
        for (auto &block: simulationBlocks) {
                block->writeTimestep(t);
            }
        }
    }

    if(localTimestepping){
    std::cout << "done sending ghostlayer\n";

        for (int i = 0; i < simulationBlocks.size(); i++){
            simulationBlocks[i]->setGhostLayer();
        }

        for (int i = 0; i < simulationBlocks.size(); i++){
            simulationBlocks[i]->receiveGhostLayer();
        }

    }

    if (localityRank == 0) {
       collector.setMasterSettings(true, outputBaseName + ".log");
    }
    for (auto &block: simulationBlocks) {

        collector += block->collector;
        if(write)
            delete block->writer;
    }

    for (auto &block: simulationBlocks) {
            block->freeMpiType();
        }

    collector.logResults();
#pragma omp parallel
    {
        chameleon_thread_finalize();
    }
     chameleon_finalize();


    MPI_Finalize();

}




