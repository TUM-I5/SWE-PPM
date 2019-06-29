//
// Created by martin on 24/06/19.
//

#include "SWE_DimensionalSplittingComponent.hpp"
//
// Created by martin on 16/06/19.
//


#include <algorithm>
#include <iostream>
#include "tools/args.hh"

#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#else
#include "writer/VtkWriter.hh"
#endif
#include "scenarios/SWE_simple_scenarios.hh"
#ifdef ASAGI
#include "scenarios/SWE_AsagiScenario.hh"

#endif



namespace remote
{
    SWE_DimensionalSplittingComponent::SWE_DimensionalSplittingComponent(
            int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
            int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
            float localOriginX, float localOriginY,  std::array<BoundaryType,4> boundaries, std::array<int,4> neighbours)
    /*
   #ifdef WRITENETCDF
        // Construct a netCDF writer

           writer(
               outputFileName,
               simulation.getBathymetry(),
               boundarySize,
               nxLocal,
               nyLocal,
               dxSimulation,
               dySimulation,
               simulation.getOriginX(),
               simulation.getOriginY())

   #else
                // Construct a vtk writer
                writer(
                        outputFileName,
                        simulation.getBathymetry(),
                        boundarySize,
                        nxLocal,
                        nyLocal,
                        dxSimulation,
                        dySimulation)
   #endif // WRITENETCDF*/

            :
            simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, communicator_type(rank,totalRank,neighbours))
    {
        std::string outputFileName;

        SWE_RadialDamBreakScenario scenario;

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


    }
    SWE_DimensionalSplittingComponent::SWE_DimensionalSplittingComponent(
            int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
            int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
            float localOriginX, float localOriginY,  std::array<BoundaryType,4> boundaries, std::array<int,4> neighbours,std::string batFile, std::string displFile )
    /*
   #ifdef WRITENETCDF
        // Construct a netCDF writer

           writer(
               outputFileName,
               simulation.getBathymetry(),
               boundarySize,
               nxLocal,
               nyLocal,
               dxSimulation,
               dySimulation,
               simulation.getOriginX(),
               simulation.getOriginY())

   #else
                // Construct a vtk writer
                writer(
                        outputFileName,
                        simulation.getBathymetry(),
                        boundarySize,
                        nxLocal,
                        nyLocal,
                        dxSimulation,
                        dySimulation)
   #endif // WRITENETCDF*/

            :
            simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, communicator_type(rank,totalRank,neighbours))
    {
        std::string outputFileName;
        // Initialize Scenario
#ifdef ASAGI
        SWE_AsagiScenario scenario(batFile, displFile);
#else
        SWE_RadialDamBreakScenario scenario;
#endif
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


    }

    void SWE_DimensionalSplittingComponent::exchangeBathymetry()
    {
        simulation.exchangeBathymetry();

    }

    float SWE_DimensionalSplittingComponent::getMaxTimestep()
    {

        return simulation.maxTimestepGlobal;

    }
    void SWE_DimensionalSplittingComponent::setMaxTimestep(float maxTimestep)
    {
        simulation.maxTimestepGlobal = maxTimestep;
        computeYSweep();

    }
    void SWE_DimensionalSplittingComponent::computeXSweep()
    {
        simulation.computeXSweep();

    }
    void SWE_DimensionalSplittingComponent::computeYSweep()
    {
        simulation.computeYSweep();
        simulation.updateUnknowns(simulation.maxTimestepGlobal);

    }
   void SWE_DimensionalSplittingComponent::setGhostLayer()
    {

        simulation.setGhostLayer().get();
       computeXSweep();
    }
    void SWE_DimensionalSplittingComponent::printResult()
    {
        hpx::cout << "Rank "<< myHpxRank <<  ": Compute Time (CPU): " << simulation.computeTime
                  <<"s"<< " - (WALL): "<<  simulation.computeTimeWall<<"s - Communication Time: "<< simulation.communicationTime << "s"<< hpx::endl;

    }
    float SWE_DimensionalSplittingComponent::getCommunicationTime(){
            return simulation.communicationTime;
            }
    float SWE_DimensionalSplittingComponent::getFlopCount(){
        return simulation.flopCounter;
    }
}

HPX_REGISTER_COMPONENT_MODULE();

typedef hpx::components::component<
        remote::SWE_DimensionalSplittingComponent
> SWE_DimensionalSplittingComponent_type;


HPX_REGISTER_COMPONENT(SWE_DimensionalSplittingComponent_type, SWE_DimensionalSplittingComponent);


HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::exchangeBathymetry_action, SWE_DimensionalSplittingComponent_exchangeBathymetry_action);
HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::setGhostLayer_action, SWE_DimensionalSplittingComponent_setGhostLayer_action);
HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::getMaxTimestep_action, SWE_DimensionalSplittingComponent_getMaxTimestep_action);
HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::setMaxTimestep_action, SWE_DimensionalSplittingComponent_setMaxTimestep_action);
HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::computeXSweep_action, SWE_DimensionalSplittingComponent_computeXSweep_action);
HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::computeYSweep_action, SWE_DimensionalSplittingComponent_computeYSweep_action);

HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::printResult_action, SWE_DimensionalSplittingComponent_printResult_action);

HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::getCommunicationTime_action, SWE_DimensionalSplittingComponent_getCommunicationTime_action);
HPX_REGISTER_ACTION(
        remote::SWE_DimensionalSplittingComponent::getFlopCount_action, SWE_DimensionalSplittingComponent_getFlopCount_action);