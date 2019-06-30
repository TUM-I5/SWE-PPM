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
#include <hpx/lcos/broadcast.hpp>
#include "SWE_DimensionalSplittingComponent.hpp"

#include <hpx/util/unwrap.hpp>


#include <utility>


namespace remote
    {

        struct HPX_COMPONENT_EXPORT SWE_Hpx_Component
                : hpx::components::component_base<SWE_Hpx_Component>
        {
             std::vector<client::SWE_DimensionalSplittingComponent> simulationBlocks;
            float simulationDuration;
            int numberOfCheckPoints;

            SWE_Hpx_Component(int totalRanks,
                              float simulationDuration,
                              int numberOfCheckPoints,
                              int nxRequested,
                              int nyRequested,
                              std::string outputBaseName,
                              std::string const &batFile,
                              std::string const &displFile);
            void run();
            HPX_DEFINE_COMPONENT_ACTION(SWE_Hpx_Component, run);
        };
    }

HPX_REGISTER_ACTION_DECLARATION(
       remote::SWE_Hpx_Component::run_action, SWE_Hpx_Component_run_action);

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
        SWE_Hpx_Component(hpx::naming::id_type && f,int totalRanks,float simulationDuration,
                          int numberOfCheckPoints,
                          int nxRequested,
                          int nyRequested,
                          std::string outputBaseName,
                          std::string const &batFile,
                          std::string const &displFile)

                : base_type(std::move(f))
        {
            this->create(f,totalRanks,simulationDuration,numberOfCheckPoints,nxRequested,nyRequested,outputBaseName,batFile,displFile);
        }
        void run()
        {
            return hpx::async<remote::SWE_Hpx_Component::run_action>(this->get_id()).get();
        }
    };





#endif //SWE_BENCHMARK_SWE_HPX_COMPONENT_HPP
