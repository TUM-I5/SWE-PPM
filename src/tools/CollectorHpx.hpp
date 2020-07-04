//
// Created by martin on 17/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTORHPX_HPP
#define SWE_BENCHMARK_COLLECTORHPX_HPP

#include <hpx/collectives/all_reduce.hpp>
#include "Collector.hpp"

class max {
public:
    double operator()(double a, double b) const {
        return a < b ? b : a;
    }
};
class min {
public:
    float operator()(float a, float b) const {
        return a < b ? a : b;
    }
};

class CollectorHpx : public Collector {
public:
    CollectorHpx():Collector() {};

    void collect() {

        group_flop_ctr = hpx::all_reduce("flop_reduce", flop_ctr, std::plus<double>{}).get();
        result_ctrs[CTR_EXCHANGE] = hpx::all_reduce("exchange_reduce", total_ctrs[CTR_EXCHANGE].count(),
                                                    std::plus<double>{}).get();
        result_ctrs[CTR_REDUCE] = hpx::all_reduce("reduction_reduce", total_ctrs[CTR_REDUCE].count(),
                                                  std::plus<double>{}).get();
        result_ctrs[CTR_WALL] = hpx::all_reduce("wall_reduce", total_ctrs[CTR_WALL].count(), max{}).get();

    };
};

#endif //SWE_BENCHMARK_COLLECTORHPX_HPP
