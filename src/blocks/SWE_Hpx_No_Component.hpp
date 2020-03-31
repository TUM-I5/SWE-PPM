//
// Created by martin on 01/07/19.
//

#ifndef SWE_BENCHMARK_SWE_HPX_NO_COMPONENT_HPP
#define SWE_BENCHMARK_SWE_HPX_NO_COMPONENT_HPP


#include <hpx/hpx.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/collectives/broadcast.hpp>
#include <hpx/util/unwrap.hpp>


#include "SWE_DimensionalSplittingHpx.hh"
#include "tools/LocalityChannel.hpp"


#include <utility>



    typedef LocalityChannel<float> localityChannel_type;
    class SWE_Hpx_No_Component

{
public:

    SWE_Hpx_No_Component(int totalRanks,int rank,int localityCount,
                      float simulationDuration,
                      int numberOfCheckPoints,
                      int nxRequested,
                      int nyRequested,
                      std::string outputBaseName,
                      std::string const &batFile,
                      std::string const &displFile,
                      bool localTimestepping);
    void run();


private:
        float simulationDuration;
        int numberOfCheckPoints;
        int localityRank;
        int localityCount;
        localityChannel_type localityChannel;
        std::vector<std::shared_ptr<SWE_DimensionalSplittingHpx>> simulationBlocks;
        std::string outputBaseName;
        bool localTimestepping;
};





#endif //SWE_BENCHMARK_SWE_HPX_NO_COMPONENT_HPP
