//
// Created by martin on 17/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTORCHAMELEON_HPP
#define SWE_BENCHMARK_COLLECTORCHAMELEON_HPP

#include "CollectorMpi.hpp"
#include <mpi.h>

class CollectorChameleon : public CollectorMpi {
public:
    CollectorChameleon() {};
    CollectorChameleon &operator+=(const CollectorChameleon &other) {
        flop_ctr += other.flop_ctr;
        for (int i = 0; i < 3; i++) {
            total_ctrs[i] += other.total_ctrs[i];

        }
        for(int i=0; i< 4; i++)  measure_ctrs[i] = other.measure_ctrs[i];
      //  total_ctrs[CTR_WALL] = std::max(total_ctrs[CTR_WALL],
        //                                other.total_ctrs[CTR_WALL]); //so we dont add WALL time together
        timesteps.insert( timesteps.end(), other.timesteps.begin(), other.timesteps.end() );
        return *this;
    }
};

#endif //SWE_BENCHMARK_COLLECTORCHAMELEON_HPP
