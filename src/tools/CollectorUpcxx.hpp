//
// Created by martin on 17/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTORUPCXX_HPP
#define SWE_BENCHMARK_COLLECTORUPCXX_HPP
#include "Collector.hpp"
#include <upcxx/upcxx.hpp>
class CollectorUpcxx : public Collector{
public:
    static CollectorUpcxx& getInstance()
    {
        static CollectorUpcxx instance;

        return instance;
    }
private:
    CollectorUpcxx () {};
    CollectorUpcxx(CollectorUpcxx const&);              // Don't Implement
    void operator=(CollectorUpcxx const&); // Don't implement


public:
    //  CollectorUpcxx(CollectorUpcxx const&) = delete;
    //void operator=(CollectorUpcxx const&) = delete;


    void collect ()  {
        group_flop_ctr = upcxx::reduce_all(flop_ctr, upcxx::op_fast_add).wait();
        result_ctrs[CTR_EXCHANGE] = upcxx::reduce_all(total_ctrs[CTR_EXCHANGE].count(), upcxx::op_fast_add).wait();
        result_ctrs[CTR_REDUCE] = upcxx::reduce_all(total_ctrs[CTR_REDUCE].count(), upcxx::op_fast_add).wait();
        result_ctrs[CTR_WALL] = upcxx::reduce_all(total_ctrs[CTR_WALL].count(), upcxx::op_fast_max).wait();

    };

};
#endif //SWE_BENCHMARK_COLLECTORUPCXX_HPP
