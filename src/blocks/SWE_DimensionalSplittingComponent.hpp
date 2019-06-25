//
// Created by martin on 24/06/19.
//

#ifndef SWE_BENCHMARK_SWE_DIMENSIONALSPLITTINGCOMPONENT_HPP
#define SWE_BENCHMARK_SWE_DIMENSIONALSPLITTINGCOMPONENT_HPP


#include <hpx/hpx.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/serialization.hpp>
#include "SWE_DimensionalSplittingHpx.hh"
#include <utility>

namespace remote
{
    struct HPX_COMPONENT_EXPORT SWE_DimensionalSplittingComponent
            : hpx::components::component_base<SWE_DimensionalSplittingComponent>
    {
        SWE_DimensionalSplittingHpx simulation;
/*#ifdef WRITENETCDF
        // Construct a netCDF writer
            NetCdfWriter writer;
#else
        // Construct a vtk writer
            VtkWriter writer;
#endif // WRITENETCDF*/
        std::size_t myHpxRank;
        std::size_t totalHpxRanks;
        const std::vector<hpx::naming::id_type> ids;
        float simulationDuration;
        int numberOfCheckPoints;
        std::vector<float> checkpointInstantOfTime;


        SWE_DimensionalSplittingComponent(
                int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
                int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
                float localOriginX, float localOriginY,std::array<BoundaryType,4> boundaries, std::array<int,4> neighbours);
        SWE_DimensionalSplittingComponent(): simulation(0,0,0,0,0,0,communicator_type(0,0,std::array<int,4>())){}

        void exchangeBathymetry();
        void setGhostLayer();
        float getMaxTimestep();
        void setMaxTimestep(float maxTimestep);
        void computeXSweep();
        void computeYSweep();
        void printResult();
        float getCommunicationTime();
        float getFlopCount();
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, exchangeBathymetry);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, setGhostLayer);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, getMaxTimestep);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, setMaxTimestep);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, computeXSweep);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, computeYSweep);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, printResult);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, getCommunicationTime);
        HPX_DEFINE_COMPONENT_ACTION(SWE_DimensionalSplittingComponent, getFlopCount);
    };
}

HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::exchangeBathymetry_action, SWE_DimensionalSplittingComponent_exchangeBathymetry_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::setGhostLayer_action, SWE_DimensionalSplittingComponent_setGhostLayer_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::getMaxTimestep_action, SWE_DimensionalSplittingComponent_getMaxTimestep_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::setMaxTimestep_action, SWE_DimensionalSplittingComponent_setMaxTimestep_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::computeXSweep_action, SWE_DimensionalSplittingComponent_computeXSweep_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::computeYSweep_action, SWE_DimensionalSplittingComponent_computeYSweep_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::printResult_action, SWE_DimensionalSplittingComponent_printResult_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::getCommunicationTime_action, SWE_DimensionalSplittingComponent_getCommunicationTime_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_DimensionalSplittingComponent::getFlopCount_action, SWE_DimensionalSplittingComponent_getFlopCount_action);
struct SWE_DimensionalSplittingComponent
        : hpx::components::client_base<SWE_DimensionalSplittingComponent,remote::SWE_DimensionalSplittingComponent>
{
    typedef hpx::components::client_base<SWE_DimensionalSplittingComponent,remote::SWE_DimensionalSplittingComponent>
            base_type;

    SWE_DimensionalSplittingComponent(hpx::future<hpx::naming::id_type> && f)
            : base_type(std::move(f))
    {}

    SWE_DimensionalSplittingComponent(hpx::naming::id_type && f)
            : base_type(std::move(f))
    {}
    SWE_DimensionalSplittingComponent(hpx::naming::id_type && f,
                      int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
                      int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
                      float localOriginX, float localOriginY,std::array<BoundaryType,4> boundaries,std::array<int,4> neighbours)
            : base_type(std::move(f))
    {
        this->create(f,rank, totalRank, simulationDuration, numberOfCheckPoints, nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY,boundaries,neighbours);
    }

    hpx::future<void> exchangeBathymetry()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::exchangeBathymetry_action>(this->get_id());
    }
    hpx::future<void> setGhostLayer()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::setGhostLayer_action>(this->get_id());
    }
    hpx::future<float> getMaxTimestep()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::getMaxTimestep_action>(this->get_id());
    }
    hpx::future<void> setMaxTimestep(float maxTimestep)
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::setMaxTimestep_action>(this->get_id(),maxTimestep);
    }
    hpx::future<void> computeXSweep()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::computeXSweep_action>(this->get_id());
    }
    hpx::future<void> computeYSweep()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::computeYSweep_action>(this->get_id());
    }
    void printResult()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::printResult_action>(this->get_id()).get();
    }
    float getCommunicationTime()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::getCommunicationTime_action>(this->get_id()).get();
    }
    float getFlopCount()
    {
        return hpx::async<remote::SWE_DimensionalSplittingComponent::getFlopCount_action>(this->get_id()).get();
    }
};


#endif //SWE_BENCHMARK_SWE_DIMENSIONALSPLITTINGCOMPONENT_HPP
