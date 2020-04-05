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
        group_flop_ctr = flop_ctr;
        for (int i = 0; i < 4; i++) {
            result_ctrs[i] = total_ctrs[i].count();
        }
    };

    double *serialize(double *arr) {

        arr[0] = flop_ctr;
        for (int i = 0; i < 4; i++) {
            arr[i + 1] = total_ctrs[i].count();
        }
        return arr;
    }

    static CollectorCharm deserialize(double *arr) {

        CollectorCharm instance;
        instance.flop_ctr = arr[0];
        for (int i = 0; i < 4; i++) {
            instance.total_ctrs[i] = std::chrono::duration<double>(arr[i + 1]);
        }
        return instance;
    }
};

#endif //SWE_BENCHMARK_COLLECTORCHARM_HPP
