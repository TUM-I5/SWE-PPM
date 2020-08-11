//
// Created by martin on 17/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTORCHARM_HPP
#define SWE_BENCHMARK_COLLECTORCHARM_HPP

#include "Collector.hpp"

class CollectorCharm : public Collector {
public:
    CollectorCharm() {};

    void collect() {
        group_flop_ctr = static_cast<double>(flop_ctr);
        for (int i = 0; i < 4; i++) {
            result_ctrs[i] = total_ctrs[i].count();
        }
    };
    CollectorCharm &operator+=(const CollectorCharm &other) {
        flop_ctr += other.flop_ctr;
        for (int i = 0; i < 3; i++) {
            total_ctrs[i] += other.total_ctrs[i];

        }
        //for(int i=0; i< 4; i++)  measure_ctrs[i] = other.measure_ctrs[i];
        //  total_ctrs[CTR_WALL] = std::max(total_ctrs[CTR_WALL],
        //                                other.total_ctrs[CTR_WALL]); //so we dont add WALL time together
        timesteps.insert( timesteps.end(), other.timesteps.begin(), other.timesteps.end() );
        return *this;
    }
    double *serialize(double *arr, bool all = false) {

        arr[0] = static_cast<double>(flop_ctr);
        for (int i = 0; i < 4; i++) {
            arr[i + 1] = total_ctrs[i].count();
        }
        if(all){
            for (int i = 0; i < 4; i++) {

                arr[i + 5] =(double)(std::chrono::time_point_cast<std::chrono::microseconds>(measure_ctrs[i]).time_since_epoch().count());
            }
        }

        return arr;
    }

    static CollectorCharm deserialize(double *arr,bool all = false) {

        CollectorCharm instance;
        instance.flop_ctr = static_cast<uint64_t>(arr[0]);
        for (int i = 0; i < 4; i++) {
            instance.total_ctrs[i] = std::chrono::duration<double>(arr[i + 1]);
        }
        if(all){
            for (int i = 0; i < 4; i++) {

                instance.measure_ctrs[i] = std::chrono::time_point<std::chrono::steady_clock>( std::chrono::microseconds ((long)arr[i+5]));

            }
        }

        return instance;
    }
};

#endif //SWE_BENCHMARK_COLLECTORCHARM_HPP
