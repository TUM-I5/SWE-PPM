//
// Created by martin on 16/06/19.
//

#ifndef SWE_BENCHMARK_SWE_HPX_COMPONENT_HPP
#define SWE_BENCHMARK_SWE_HPX_COMPONENT_HPP

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
        struct HPX_COMPONENT_EXPORT SWE_Hpx_Component
                : hpx::components::component_base<SWE_Hpx_Component>
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


            SWE_Hpx_Component(
                    int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
                    int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
                    float localOriginX, float localOriginY,std::array<BoundaryType,4> boundaries, std::array<int,4> neighbours);
            SWE_Hpx_Component(): simulation(0,0,0,0,0,0,communicator_type(0,0,std::array<int,4>())){}
            void invoke(std::vector<hpx::naming::id_type> const &test);
            HPX_DEFINE_COMPONENT_ACTION(SWE_Hpx_Component, invoke);
            float get_wave_speed();
            HPX_DEFINE_COMPONENT_ACTION(SWE_Hpx_Component, get_wave_speed);
            void initialize(std::vector<hpx::naming::id_type> const &ids);

            HPX_DEFINE_COMPONENT_ACTION(SWE_Hpx_Component, initialize);


        };
    }

HPX_REGISTER_ACTION_DECLARATION(
       remote::SWE_Hpx_Component::invoke_action, SWE_Hpx_Component_invoke_action);
HPX_REGISTER_ACTION_DECLARATION(
        remote::SWE_Hpx_Component::initialize_action, SWE_Hpx_Component_initialize_action);

HPX_REGISTER_ACTION_DECLARATION(
       remote::SWE_Hpx_Component::get_wave_speed_action, SWE_Hpx_Component_get_wave_speed_action);

    struct SWE_Hpx_Component
            : hpx::components::client_base<SWE_Hpx_Component,remote::SWE_Hpx_Component>
    {
        typedef hpx::components::client_base<SWE_Hpx_Component,remote::SWE_Hpx_Component>
                base_type;

        SWE_Hpx_Component(hpx::future<hpx::naming::id_type> && f)
                : base_type(std::move(f))
        {}

        SWE_Hpx_Component(hpx::naming::id_type && f)
                : base_type(std::move(f))
        {}
        SWE_Hpx_Component(hpx::naming::id_type && f,
                          int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
                          int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
                          float localOriginX, float localOriginY,std::array<BoundaryType,4> boundaries,std::array<int,4> neighbours)
                : base_type(std::move(f))
        {
            this->create(f,rank, totalRank, simulationDuration, numberOfCheckPoints, nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY,boundaries,neighbours);
        }
        void invoke(std::vector<hpx::naming::id_type> const &test)
        {
            hpx::async<remote::SWE_Hpx_Component::invoke_action>(this->get_id(),test).get();
        }
        hpx::future<void> initialize(std::vector<hpx::naming::id_type> const &ids)
        {
            return hpx::async<remote::SWE_Hpx_Component::initialize_action>(this->get_id(),ids);
        }
        float get_wave_speed()
        {
            return  hpx::async<remote::SWE_Hpx_Component::get_wave_speed_action>(this->get_id()).get();
        }

    };





#endif //SWE_BENCHMARK_SWE_HPX_COMPONENT_HPP
