//
// Created by martin on 26/11/2019.
//
#include <array>
#include <chrono>
#ifndef SWE_BENCHMARK_COUNTERUTILITY_HPP
#define SWE_BENCHMARK_COUNTERUTILITY_HPP
<
class CounterUtility {
    enum COUNTERS {CTR_EXCHANGE,CTR_BARRIER,CTR_REDUCE,CTR_WALL};
    private double flop_ctr;
    private std::array<std::chrono::duration,4> total_ctrs;
    private std::array<std::chrono::steady_clock::time_point,4> measure_ctrs;
    CounterUtility():0 {};
    void start_ctr(int ctr){
        measure_ctrs[ctr] = std::chrono::steady_clock::now();
    }
    void stop_ctr(int ctr){
        total_ctrs[ctr] +=  measure_ctrs[ctr] - std::chrono::steady_clock::now();
    }
    void add_flops(float flops){
        flop_ctr+= flops;
    }
    static void printCounter(const CounterUtility &ut){
        std::cout   << "Flop count: " << ut.flop_ctr << std::endl
                    << "Flops(Total): " << ((float)ut.flop_ctr/ut.total_ctrs[CTR_WALL].count()) << "GFLOPS"<< std::endl
                    << "Communication Time(Total): " << ut.total_ctrs[CTR_EXCHANGE].count() << "s" << std::endl
                    << "Reduction Time(Total): " << ut.total_ctrs[CTR_REDUCE].count() << "s" << std::endl;
                    << "Barrier Time(Total): " << ut.total_ctrs[CTR_BARRIER].count() << "s" << std::endl;

    }
};

#endif //SWE_BENCHMARK_COUNTERUTILITY_HPP
