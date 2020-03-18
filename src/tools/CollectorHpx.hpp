//
// Created by martin on 17/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTORHPX_HPP
#define SWE_BENCHMARK_COLLECTORHPX_HPP
class CollectorHpx : public Collector{
public:
    CollectorHpx(){};

    void collect ()  {
        //everything is collected by the main chare anyways, no need for reduction
    };
};
#endif //SWE_BENCHMARK_COLLECTORHPX_HPP
