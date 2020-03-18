//
// Created by martin on 17/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTORCHARM_HPP
#define SWE_BENCHMARK_COLLECTORCHARM_HPP

#include "Collector.hpp"

class CollectorCharm : public Collector{
public:
    CollectorCharm(){};

    void collect ()  {
        //everything is collected by the main chare anyways, no need for reduction
    };

    double * serialize(double *arr){
        //return collectorMessage{flop_ctr, {total_ctrs[CTR_EXCHANGE].count(),total_ctrs[CTR_BARRIER].count(),total_ctrs[CTR_REDUCE].count(),total_ctrs[CTR_WALL].count()}};

        arr[0] = flop_ctr;
        for(int i = 0 ; i< 4 ; i++){
            arr[i+1] = total_ctrs[i].count();
        }
        return arr;
    }
    static CollectorCharm deserialize(double *arr){
        //return collectorMessage{flop_ctr, {total_ctrs[CTR_EXCHANGE].count(),total_ctrs[CTR_BARRIER].count(),total_ctrs[CTR_REDUCE].count(),total_ctrs[CTR_WALL].count()}};
        CollectorCharm instance;
        instance.flop_ctr = arr[0];
        for(int i = 0 ; i< 4 ; i++){
            instance.total_ctrs[i] = std::chrono::duration<double>(arr[i+1]);
        }
        return instance;
    }
};
#endif //SWE_BENCHMARK_COLLECTORCHARM_HPP
