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




typedef float timestep_type;

HPX_REGISTER_CHANNEL_DECLARATION(timestep_type);
HPX_REGISTER_CHANNEL(timestep_type);


    void setGhostLayer(SWE_DimensionalSplittingHpx *simulation){
        simulation->setGhostLayer();
    }
    void computeXSweep(SWE_DimensionalSplittingHpx *simulation){
        simulation->computeXSweep();
    }
    void computeYSweep(SWE_DimensionalSplittingHpx *simulation){
        simulation->computeYSweep();
    }

    void exchangeBathymetry(SWE_DimensionalSplittingHpx *simulation){
        simulation->exchangeBathymetry();
    }
    void updateUnknowns(SWE_DimensionalSplittingHpx *simulation){
        simulation->updateUnknowns(simulation->maxTimestepGlobal);
    }

    std::array<BoundaryType, 4> getBoundaries(int localBlockPositionX, int localBlockPositionY, int blockCountX, int blockCountY, SWE_Scenario *scen){
#ifdef ASAGI
        SWE_AsagiScenario * scenario = (SWE_AsagiScenario *) scen;
#else
       SWE_RadialDamBreakScenario * scenario =  (SWE_RadialDamBreakScenario*) scen;
       // SWE_HalfDomainDry * scenario =  (SWE_HalfDomainDry *) scen  ;
        //SWE_RadialDamBreakScenario scenario;
#endif
        std::array<BoundaryType, 4> boundaries;
        boundaries[BND_LEFT] = (localBlockPositionX > 0) ? CONNECT : scenario->getBoundaryType(BND_LEFT);
        boundaries[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? CONNECT : scenario->getBoundaryType(BND_RIGHT);
        boundaries[BND_BOTTOM] = (localBlockPositionY > 0) ? CONNECT : scenario->getBoundaryType(BND_BOTTOM);
        boundaries[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? CONNECT : scenario->getBoundaryType(BND_TOP);
        return boundaries;
    }
    std::array<int, 4> getNeighbours(int localBlockPositionX, int localBlockPositionY, int blockCountX, int blockCountY, int myHpxRank ){
        std::array<int, 4> myNeighbours;
        myNeighbours[BND_LEFT] = (localBlockPositionX > 0) ? myHpxRank - blockCountY : -1;
        myNeighbours[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? myHpxRank + blockCountY : -1;
        myNeighbours[BND_BOTTOM] = (localBlockPositionY > 0) ? myHpxRank - 1 : -1;
        myNeighbours[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? myHpxRank + 1 : -1;
        return myNeighbours;
    }


    SWE_Hpx_No_Component::SWE_Hpx_No_Component(int ranksPerLocality,int rank,int localityCount,float simulationDuration,
                                         int numberOfCheckPoints,
                                         int nxRequested,
                                         int nyRequested,
                                         std::string outputBaseName,
                                         std::string const &batFile,
                                         std::string const &displFile)
    {

        std::string outputFileName;
        // Initialize Scenario
#ifdef ASAGI
        SWE_AsagiScenario scenario(batFile, displFile);
#else
        //SWE_HalfDomainDry scenario;
        SWE_RadialDamBreakScenario scenario;
#endif

        int totalRanks = ranksPerLocality* localityCount;
        this->localityRank = rank;

        this->localityCount = localityCount;
        this->numberOfCheckPoints = numberOfCheckPoints;
        this->simulationDuration = simulationDuration;
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

        auto totalHpxRanks = totalRanks;

        localityChannel = localityChannel_type(localityRank,localityCount);

        // number of SWE-Blocks in x- and y-direction
        int blockCountY = std::sqrt(totalHpxRanks);
        while (totalHpxRanks % blockCountY != 0) blockCountY--;
        int blockCountX = totalHpxRanks / blockCountY;


        int startPoint = localityRank * ranksPerLocality;
        hpx::cout << "Locality Rank " << localityRank << std::endl;
        hpx::cout << "Total Ranks " << totalHpxRanks << std::endl;
        hpx::cout << "Start point " << startPoint << std::endl;
        for(int i = startPoint ; i < startPoint+ ranksPerLocality; i++) {
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

            simulationBlocks.push_back(SWE_DimensionalSplittingHpx(nxLocal, nyLocal, dxSimulation, dySimulation,
                                                                   localOriginX, localOriginY));

            simulationBlocks[i - startPoint].initScenario(scenario, boundaries.data());
        }
        for(int i = startPoint ; i < startPoint+ ranksPerLocality; i++) {
            auto myHpxRank = i;
            int localBlockPositionX = myHpxRank / blockCountY;
            int localBlockPositionY = myHpxRank % blockCountY;
            std::array<int, 4> myNeighbours = getNeighbours(localBlockPositionX, localBlockPositionY, blockCountX,
                                                            blockCountY, myHpxRank);

            std::array<int,4> refinedNeighours;
            std::array<SWE_DimensionalSplittingHpx * , 4> neighbourBlocks;
            for(int j = 0; j < 4 ; j++){
                if(myNeighbours[j] >= startPoint && myNeighbours[j] < (startPoint+ranksPerLocality)){
                    refinedNeighours[j] = -2;
                    neighbourBlocks[j] = &simulationBlocks[myNeighbours[j]-startPoint];
                }else {
                    refinedNeighours[j] = myNeighbours[i];
                }

            }
            simulationBlocks[i-startPoint].connectNeighbours(communicator_type(myHpxRank,totalHpxRanks,refinedNeighours,neighbourBlocks));
        }

    }
    void SWE_Hpx_No_Component::run()
    {
        std::vector<hpx::future<void>> fut;
        for(auto & block: simulationBlocks)fut.push_back(hpx::async(exchangeBathymetry, &block));
        hpx::wait_all(fut);


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
        struct timespec reductionTime;
        struct timespec endTime;

        float wallTime = 0.;
        float sumReductionTime = 0.;
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
                std::vector<hpx::future<void>> ghostlayer;
                ghostlayer.reserve(simulationBlocks.size());
                for(auto & block: simulationBlocks)ghostlayer.push_back(block.setGhostLayer());
                hpx::wait_all(ghostlayer);


                std::vector<hpx::future<void>> xsweep;
                xsweep.reserve(simulationBlocks.size());

                for(auto & block: simulationBlocks)xsweep.push_back(hpx::async(computeXSweep,&block));
                hpx::wait_all(xsweep);

                std::vector<float> timesteps;
                for(auto & block: simulationBlocks)timesteps.push_back(block.maxTimestepGlobal);


                float minTimestep= *std::min_element(timesteps.begin(), timesteps.end());

                float timestep;
                //barrier
                clock_gettime(CLOCK_MONOTONIC, &reductionTime);
                if(localityRank == 0){
                    if(localityCount > 1){
                        timestep =hpx::dataflow(hpx::util::unwrapping([](std::vector<float> globalTimesteps, float localTimestep) -> float {
                            return std::min(localTimestep,*std::min_element(globalTimesteps.begin(),globalTimesteps.end()));
                        }),std::move(localityChannel.get()),std::move(minTimestep)).get();
                        float sendTs = timestep;
                        localityChannel.set(std::move(sendTs));

                    }else {
                        timestep = minTimestep;
                    }

                } else {

                    localityChannel.set(std::move(minTimestep));

                    timestep = localityChannel.get()[0].get();
                }
                clock_gettime(CLOCK_MONOTONIC, &endTime);
               sumReductionTime += (endTime.tv_sec - reductionTime.tv_sec);
                sumReductionTime += (float) (endTime.tv_nsec -reductionTime.tv_nsec) / 1E9;
                for(auto & block: simulationBlocks)block.maxTimestepGlobal = timestep;
                //  hpx::lcos::broadcast<remote::SWE_DimensionalSplittingComponent::setMaxTimestep_action>(block_ids,timestep).get();
                std::vector<hpx::future<void>>ysweep;
                ysweep.reserve(simulationBlocks.size());

                for(auto & block: simulationBlocks)ysweep.push_back(hpx::async(computeYSweep,&block));
                hpx::wait_all(ysweep);

                std::vector<hpx::future<void>> unknown;
                unknown.reserve(simulationBlocks.size());

                for(auto & block: simulationBlocks)unknown.push_back(hpx::async(updateUnknowns,&block));
                hpx::wait_all(unknown);
                clock_gettime(CLOCK_MONOTONIC, &endTime);
                wallTime += (endTime.tv_sec - startTime.tv_sec);
                wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

                // update simulation time with time step width.
                t += timestep;
                iterations++;


            }

            if(localityRank == 0) {
                printf("Write timestep (%fs)\n", t);
            }
            //hpx::cout <<"Write timestep " << t<<"s" << std::endl;

        }


        float totalCommTime = 0;
        float sumFlops = 0;
        for(auto block: simulationBlocks){
            //block.printResult();
            totalCommTime += block.communicationTime;
            sumFlops += block.flopCounter;
        }

        hpx::cout   << "Flop count: " << sumFlops << std::endl
                    << "Flops(Total): " << ((float)sumFlops)/(wallTime*1000000000) << "GFLOPS"<< std::endl;
        hpx::cout   << "Total Time (Wall): "<< wallTime <<"s"<<hpx::endl;
        hpx::cout   << "Communication Time(Total): "<< totalCommTime <<"s"<<hpx::endl
                    << "Reduction Time(Total): " << sumReductionTime << "s" << std::endl;
    }




