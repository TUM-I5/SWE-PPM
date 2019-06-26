//
// Created by martin on 16/06/19.
//

#include "SWE_Hpx_Component.hpp"


#include <algorithm>
#include <iostream>
#include "tools/args.hh"

#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#else
#include "writer/VtkWriter.hh"
#endif


#include "scenarios/SWE_AsagiScenario.hh"

#include "scenarios/SWE_simple_scenarios.hh"


template <typename T> class Min {
public:

   T operator()(const T& a, const T& b) const {return a < b ? a : b; }
};
typedef Min<float> max_type;
HPX_REGISTER_REDUCE_ACTION_DECLARATION(remote::SWE_Hpx_Component::get_wave_speed_action, max_type)
HPX_REGISTER_REDUCE_ACTION(remote::SWE_Hpx_Component::get_wave_speed_action, max_type)
 namespace remote
    {
    SWE_Hpx_Component::SWE_Hpx_Component(
             int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
             int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
             float localOriginX, float localOriginY,  std::array<BoundaryType,4> boundaries, std::array<int,4> neighbours, std::string batFile, std::string displFile)

             :
             simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, communicator_type(rank,totalRank,neighbours))
     {
         std::string outputFileName;
         // Initialize Scenario
         SWE_AsagiScenario scenario(batFile, displFile);
         //this->ids(ids);
         this->simulationDuration = simulationDuration;
         this->numberOfCheckPoints = numberOfCheckPoints;
         // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
         //float* checkpointInstantOfTime = new float[numberOfCheckPoints];
         // Time delta is the time between any two checkpoints
         float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
         // The first checkpoint is reached after 0 + delta t
         checkpointInstantOfTime.push_back(checkpointTimeDelta);
         for(int i = 1; i < numberOfCheckPoints; i++) {
             checkpointInstantOfTime.push_back(checkpointInstantOfTime[i - 1] + checkpointTimeDelta);
         }

         myHpxRank = rank;
         totalHpxRanks = totalRank;


         simulation.initScenario(scenario, boundaries.data());

         hpx::cout << myHpxRank << " finished init" << std::endl;
     }
     SWE_Hpx_Component::SWE_Hpx_Component(
             int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
             int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
             float localOriginX, float localOriginY,  std::array<BoundaryType,4> boundaries, std::array<int,4> neighbours)
             :
             simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, communicator_type(rank,totalRank,neighbours))
     {
         std::string outputFileName;

         SWE_RadialDamBreakScenario scenario;
         this->simulationDuration = simulationDuration;
         this->numberOfCheckPoints = numberOfCheckPoints;
         // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
         //float* checkpointInstantOfTime = new float[numberOfCheckPoints];
         // Time delta is the time between any two checkpoints
         float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
         // The first checkpoint is reached after 0 + delta t
         checkpointInstantOfTime.push_back(checkpointTimeDelta);
         for(int i = 1; i < numberOfCheckPoints; i++) {
             checkpointInstantOfTime.push_back(checkpointInstantOfTime[i - 1] + checkpointTimeDelta);
         }

         myHpxRank = rank;
         totalHpxRanks = totalRank;


         simulation.initScenario(scenario, boundaries.data());

         hpx::cout << myHpxRank << " finished init" << std::endl;
     }
        void SWE_Hpx_Component::invoke(std::vector<hpx::naming::id_type>const &test)
        {


        }
     void SWE_Hpx_Component::initialize(std::vector<hpx::naming::id_type> const &ids)
     {



         simulation.exchangeBathymetry();

         /********************
          * START SIMULATION *
          ********************/


         // Initialize wall timer
         struct timespec startTime;
         struct timespec endTime;

         wallTime = 0.;
         float t = 0.;


         float timestep;
         unsigned int iterations = 0;
         hpx::lcos::barrier(std::string("precalc"), totalHpxRanks, myHpxRank).wait();

         // loop over the count of requested checkpoints
         for(int i = 0; i < numberOfCheckPoints; i++) {
             // Simulate until the checkpoint is reached
             while(t < checkpointInstantOfTime[i]) {
                 // Start measurement
                 clock_gettime(CLOCK_MONOTONIC, &startTime);

                 // set values in ghost cells.
                 // this function blocks until everything has been received

                 simulation.setGhostLayer();

                 // compute numerical flux on each edge
                // simulation.computeNumericalFluxes(ids);
                 simulation.computeXSweep();

                 hpx::lcos::barrier(std::string("midcalc"), totalHpxRanks, myHpxRank).wait();

                 simulation.maxTimestepGlobal = hpx::lcos::reduce<get_wave_speed_action>(ids,Min<float>()).get();
                // hpx::cout << myHpxRank << " timestep " << simulation.maxTimestepGlobal << std::endl;
                 simulation.computeYSweep();
                 // max timestep has been reduced over all ranks in computeNumericalFluxes()
                 timestep = simulation.getMaxTimestep();

                 // update the cell values
                 simulation.updateUnknowns(timestep);

                 // Accumulate wall time
                 clock_gettime(CLOCK_MONOTONIC, &endTime);
                 wallTime += (endTime.tv_sec - startTime.tv_sec);
                 wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

                 // update simulation time with time step width.
                 t += timestep;
                 iterations++;
                 // upcxx::barrier();
                 hpx::lcos::barrier(std::string("endcalc"), totalHpxRanks, myHpxRank).wait();
             }

             if(myHpxRank == 0) {
                hpx::cout <<"Write timestep " << t<<"s" << std::endl;
             }

             // write output
            /* writer.writeTimeStep(
                     simulation.getWaterHeight(),
                     simulation.getMomentumHorizontal(),
                     simulation.getMomentumVertical(),
                     t);*/
         }


         /************
          * FINALIZE *
          ************/

         if(myHpxRank == 0){

             hpx::cout << "Rank "<< myHpxRank <<  ": Compute Time (CPU):" << simulation.computeTime
                       <<"s"<< " - (WALL): "<<  simulation.computeTimeWall<<"s"<< " | Total Time (Wall):"<< wallTime <<"s"<<std::endl;
             hpx::cout   << "Rank: " << myHpxRank << std::endl

                         << "Flops(Single): "<< ((float)simulation.flopCounter)/(wallTime*1000000000) << std::endl
                         <<"Communication Time: "<< simulation.communicationTime << "s"<< hpx::endl;
         }

     }
        float SWE_Hpx_Component::get_wave_speed()
        {
            return simulation.maxTimestepGlobal;
        }
         float SWE_Hpx_Component::getCommunicationTime(){
             return simulation.communicationTime;
         }
         float SWE_Hpx_Component::getFlopCount(){
             return simulation.flopCounter;
         }
         float SWE_Hpx_Component::getWallTime(){
             return wallTime;
         }

    }

HPX_REGISTER_COMPONENT_MODULE();

typedef hpx::components::component<
        remote::SWE_Hpx_Component
> SWE_Hpx_Component_type;


HPX_REGISTER_COMPONENT(SWE_Hpx_Component_type, SWE_Hpx_Component);

HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::invoke_action, SWE_Hpx_Component_invoke_action);
HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::initialize_action, SWE_Hpx_Component_initialize_action);
HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::get_wave_speed_action, SWE_Hpx_Component_get_wave_speed_action);
HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::getCommunicationTime_action, SWE_Hpx_Component_getCommunicationTime_action);
HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::getFlopCount_action, SWE_Hpx_Component_getFlopCount_action);
HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::getWallTime_action, SWE_Hpx_Component_getWallTime_action);