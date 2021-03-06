//
// Created by martin on 01/07/19.
//

#include "SWE_Hpx_No_Component.hpp"
//
// Created by martin on 16/06/19.



#include <algorithm>
#include <iostream>
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

#include "tools/CollectorHpx.hpp"


typedef float timestep_type;

//HPX_REGISTER_CHANNEL_DECLARATION(timestep_type);
HPX_REGISTER_CHANNEL(timestep_type);


hpx::future<void> setGhostLayer(SWE_DimensionalSplittingHpx *simulation) {
    return simulation->setGhostLayer();
}

void computeNumericalFluxes(SWE_DimensionalSplittingHpx *simulation) {
    simulation->computeNumericalFluxes();
}

void computeMaxTimestep(SWE_DimensionalSplittingHpx *simulation, const float dryTol, const float cflNumber) {
    simulation->computeMaxTimestep(dryTol, cflNumber);
}



void exchangeBathymetry(SWE_DimensionalSplittingHpx *simulation) {
    simulation->exchangeBathymetry();
}

void updateUnknowns(SWE_DimensionalSplittingHpx *simulation) {
    simulation->updateUnknowns(simulation->maxTimestepGlobal);
}

std::array<BoundaryType, 4>
getBoundaries(int localBlockPositionX, int localBlockPositionY, int blockCountX, int blockCountY, SWE_Scenario *scen) {
#ifdef ASAGI
    SWE_AsagiScenario * scenario = (SWE_AsagiScenario *) scen;
#else
    //SWE_RadialDamBreakScenario *scenario = (SWE_RadialDamBreakScenario *) scen;
    // SWE_HalfDomainDry * scenario =  (SWE_HalfDomainDry *) scen  ;

    SWE_RadialBathymetryDamBreakScenario *scenario = (SWE_RadialBathymetryDamBreakScenario *) scen;;
#endif
    std::array<BoundaryType, 4> boundaries;
    boundaries[BND_LEFT] = (localBlockPositionX > 0) ? CONNECT : scenario->getBoundaryType(BND_LEFT);
    boundaries[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? CONNECT : scenario->getBoundaryType(BND_RIGHT);
    boundaries[BND_BOTTOM] = (localBlockPositionY > 0) ? CONNECT : scenario->getBoundaryType(BND_BOTTOM);
    boundaries[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? CONNECT : scenario->getBoundaryType(BND_TOP);
    return boundaries;
}

std::array<int, 4>
getNeighbours(int localBlockPositionX, int localBlockPositionY, int blockCountX, int blockCountY, int myHpxRank) {
    std::array<int, 4> myNeighbours;
    myNeighbours[BND_LEFT] = (localBlockPositionX > 0) ? myHpxRank - blockCountY : -1;
    myNeighbours[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? myHpxRank + blockCountY : -1;
    myNeighbours[BND_BOTTOM] = (localBlockPositionY > 0) ? myHpxRank - 1 : -1;
    myNeighbours[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? myHpxRank + 1 : -1;
    return myNeighbours;
}


SWE_Hpx_No_Component::SWE_Hpx_No_Component(int ranksPerLocality, int rank, int localityCount, float simulationDuration,
                                           int numberOfCheckPoints,
                                           int nxRequested,
                                           int nyRequested,
                                           std::string outputBaseName,
                                           std::string const &batFile,
                                           std::string const &displFile,
                                          float localTimestepping, bool write) {


    // Initialize Scenario
#ifdef ASAGI
    SWE_AsagiScenario scenario(batFile, displFile);
#else
    //SWE_HalfDomainDry scenario;
    //SWE_RadialDamBreakScenario scenario;
    SWE_RadialBathymetryDamBreakScenario scenario;

#endif

    int totalRanks = ranksPerLocality * localityCount;
    this->localityRank = rank;

    this->localityCount = localityCount;
    this->numberOfCheckPoints = numberOfCheckPoints;
    this->simulationDuration = simulationDuration;
    this->localTimestepping = localTimestepping;
    this->outputBaseName = outputBaseName;
    this->write = write;
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

    auto totalHpxRanks = totalRanks;

    localityChannel = localityChannel_type(localityRank, localityCount);

    // number of SWE-Blocks in x- and y-direction
    int blockCountY = std::sqrt(totalHpxRanks);
    while (totalHpxRanks % blockCountY != 0) blockCountY--;
    int blockCountX = totalHpxRanks / blockCountY;


    int startPoint = localityRank * ranksPerLocality;

    for (int i = startPoint; i < startPoint + ranksPerLocality; i++) {
        auto myHpxRank = i;
        int localBlockPositionX = myHpxRank / blockCountY;
        int localBlockPositionY = myHpxRank % blockCountY;



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


        // Determine the boundary types for the SWE_Block:
        // block boundaries bordering other blocks have a CONNECT boundary,
        // block boundaries bordering the entire scenario have the respective scenario boundary type
        std::array<BoundaryType, 4> boundaries = getBoundaries(localBlockPositionX, localBlockPositionY,
                                                               blockCountX, blockCountY, &scenario);

        std::array<int, 4> myNeighbours = getNeighbours(localBlockPositionX, localBlockPositionY, blockCountX,
                                                        blockCountY, myHpxRank);
        std::string outputFileName = outputBaseName+"_"+std::to_string(myHpxRank);

       // std::cout << myHpxRank<< "| " <<localOriginX<< " "<< localOriginY<< " " <<  nxLocal << " " << nyLocal << std::endl;
        simulationBlocks.push_back(std::shared_ptr<SWE_DimensionalSplittingHpx>(
                new SWE_DimensionalSplittingHpx(nxLocal, nyLocal, dxSimulation, dySimulation,
                                                localOriginX, localOriginY, localTimestepping, outputFileName,write)));

        simulationBlocks[i - startPoint]->initScenario(scenario, boundaries.data());
    }

    for (int i = startPoint; i < startPoint + ranksPerLocality; i++) {
        auto myHpxRank = i;
        int localBlockPositionX = myHpxRank / blockCountY;
        int localBlockPositionY = myHpxRank % blockCountY;
        std::array<int, 4> myNeighbours = getNeighbours(localBlockPositionX, localBlockPositionY, blockCountX,
                                                        blockCountY, myHpxRank);

        std::array<int, 4> refinedNeighbours;
        std::array<std::shared_ptr<SWE_DimensionalSplittingHpx>, 4> neighbourBlocks;


        for (int j = 0; j < 4; j++) {
            if (myNeighbours[j] >= startPoint && myNeighbours[j] < (startPoint + ranksPerLocality)) {
                refinedNeighbours[j] = -2;
                simulationBlocks[i - startPoint]->neighbourRankId[j] = myNeighbours[j];
                neighbourBlocks[j] = simulationBlocks[myNeighbours[j] - startPoint];
            } else {
                refinedNeighbours[j] = myNeighbours[j];
                simulationBlocks[i - startPoint]->neighbourRankId[j]  =  myNeighbours[j];
            }
        }

        simulationBlocks[i - startPoint]->connectNeighbours(
                SWE_DimensionalSplittingHpx::communicator_type(myHpxRank, totalHpxRanks, refinedNeighbours,
                                                               neighbourBlocks));
        simulationBlocks[i - startPoint]->setRank(i);
        simulationBlocks[i - startPoint]->setDuration(simulationDuration);

    }

}

void SWE_Hpx_No_Component::run() {

    CollectorHpx collector;
    std::vector<hpx::future<void>> fut;
    std::vector<hpx::future<void>> blockFuture;
    std::vector<float> timesteps;
    float maxLocalTimestep;


    collector.setMasterSettings(localityRank==0, outputBaseName + ".log",localityCount);
    collector.setRank(localityRank);

    for (auto &block: simulationBlocks)fut.push_back(hpx::async(exchangeBathymetry, block.get()));
    hpx::wait_all(fut);


    if (localTimestepping) {

        for (auto &block: simulationBlocks)
            blockFuture.push_back(hpx::async(computeMaxTimestep, block.get(), 0.01, 0.4));

        hpx::wait_all(blockFuture);

        blockFuture.clear();

        for (auto &block: simulationBlocks)timesteps.push_back(block->getMaxTimestep());

        float minTimestep = *std::max_element(timesteps.begin(), timesteps.end());

        if (localityRank == 0) {
            if (localityCount > 1) {
                maxLocalTimestep = hpx::dataflow(
                        hpx::util::unwrapping([](std::vector<float> globalTimesteps, float localTimestep) -> float {
                            return std::max(localTimestep,
                                            *std::max_element(globalTimesteps.begin(), globalTimesteps.end()));
                        }), std::move(localityChannel.get()), std::move(minTimestep)).get();
                float sendTs = maxLocalTimestep;
                localityChannel.set(std::move(sendTs));

            } else {
                maxLocalTimestep = minTimestep;
            }

        } else {
            localityChannel.set(std::move(minTimestep));
            maxLocalTimestep = localityChannel.get()[0].get();
        }
        maxLocalTimestep = localTimestepping;
        std::cout << "Max Local Timestep is " << maxLocalTimestep << std::endl;

        for (auto &block: simulationBlocks)block->setMaxLocalTimestep(maxLocalTimestep);
    }


    // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
    float *checkpointInstantOfTime = new float[numberOfCheckPoints];
    // Time delta is the time between any two checkpoints
    float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
    // The first checkpoint is reached after 0 + delta t
    checkpointInstantOfTime[0] = checkpointTimeDelta;
    for (int i = 1; i < numberOfCheckPoints; i++) {
        checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
    }


    float t = 0.;
    bool synchronizedTimestep = true;

    float timestep;
    unsigned int iterations = 0;

    blockFuture.reserve(simulationBlocks.size());

    timesteps.reserve(simulationBlocks.size());
    std::vector<hpx::future<void>> xsweepFuture(simulationBlocks.size());

    for (auto &block: simulationBlocks) {
        if (write) {
            block->writeTimestep(0.f);

        }
    }


    // loop over the count of requested checkpoints
    for (int i = 0; i < numberOfCheckPoints; i++) {
        // Simulate until the checkpoint is reached
        while (t < checkpointInstantOfTime[i]) {
            do {


                // Start measurement
                collector.startCounter(Collector::CTR_WALL);
                // set values in ghost cells.
                // this function blocks until everything has been received
                for (auto &block: simulationBlocks)block->currentTotalLocalTimestep = block->getTotalLocalTimestep();
                blockFuture.clear();
                for (auto &block: simulationBlocks)blockFuture.push_back(hpx::async(setGhostLayer, block.get()));
                hpx::wait_all(blockFuture);

                blockFuture.clear();


                for (auto &block: simulationBlocks)blockFuture.push_back(hpx::async(computeNumericalFluxes, block.get()));
                hpx::wait_all(blockFuture);


                //barrier
                if (!localTimestepping) {
                    timesteps.clear();
                    for (auto &block: simulationBlocks)timesteps.push_back(block->maxTimestep);


                    float minTimestep = *std::min_element(timesteps.begin(), timesteps.end());
                    collector.startCounter(Collector::CTR_REDUCE);
                    //timestep= hpx::all_reduce("timestep_reduce", minTimestep, min{}).get();
                    if (localityRank == 0) {
                        if (localityCount > 1) {

                            timestep = hpx::dataflow(hpx::util::unwrapping(
                                    [](std::vector<float> globalTimesteps, float localTimestep) -> float {
                                        return std::min(localTimestep, *std::min_element(globalTimesteps.begin(),
                                                                                         globalTimesteps.end()));
                                    }), std::move(localityChannel.get()), std::move(minTimestep)).get();
                            float sendTs = timestep;
                            localityChannel.set(std::move(sendTs));

                        } else {
                            timestep = minTimestep;
                        }

                    } else {

                        localityChannel.set(std::move(minTimestep));

                        timestep = localityChannel.get()[0].get();
                    }
                    collector.stopCounter(Collector::CTR_REDUCE);

                    for (auto &block: simulationBlocks)block->maxTimestep = timestep;

                }


                blockFuture.clear();

                for (auto &block: simulationBlocks)blockFuture.push_back(hpx::async(updateUnknowns, block.get()));
                hpx::wait_all(blockFuture);

                collector.stopCounter(Collector::CTR_WALL);

                if (localTimestepping) {
                    //if each block got the maxLocalTimestep the timestep is finished
                    synchronizedTimestep = true;
                    for (auto &block : simulationBlocks) {
                        if (!block->hasMaxLocalTimestep()) {
                            synchronizedTimestep = false;
                            // break;
                        }
                    }
                }

            } while (localTimestepping && !synchronizedTimestep);
            // update simulation time with time step width.
            t += localTimestepping ? maxLocalTimestep : timestep;
            if(localTimestepping){
                for (auto &block: simulationBlocks){

                    block->resetStepSizeCounter();
                }
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

        for (auto &block: simulationBlocks){

            blockFuture.push_back(hpx::async(setGhostLayer, block.get()));
        }
        hpx::wait_all(blockFuture);

    }
    for (auto &block: simulationBlocks) {
        collector += block->collector;
        if(write)
            delete block->writer;
    }

    collector.logResults();


}




