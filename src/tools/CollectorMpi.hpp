//
// Created by martin on 07/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTORMPI_HPP
#define SWE_BENCHMARK_COLLECTORMPI_HPP

#include "Collector.hpp"
#include <mpi.h>

class CollectorMpi : public Collector {
public:
    static CollectorMpi &getInstance() {
        static CollectorMpi instance;

        return instance;
    }

protected:
    CollectorMpi() {};

    CollectorMpi(CollectorMpi const &);              // Don't Implement
    void operator=(CollectorMpi const &); // Don't implement


public:
    //  CollectorMpi(CollectorMpi const&) = delete;
    //void operator=(CollectorMpi const&) = delete;


    void collect() {
        double reduce_ctr = total_ctrs[CTR_REDUCE].count();
        double exchange_ctr = total_ctrs[CTR_EXCHANGE].count();
        double wall_ctr = total_ctrs[CTR_WALL].count();

        double final_flops = static_cast<double>(flop_ctr);
        
        MPI_Allreduce(&wall_ctr, &result_ctrs[CTR_WALL], 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
        MPI_Allreduce(&reduce_ctr, &result_ctrs[CTR_REDUCE], 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        MPI_Allreduce(&exchange_ctr, &result_ctrs[CTR_EXCHANGE], 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        MPI_Allreduce(&final_flops, &group_flop_ctr, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    };

};

#endif //SWE_BENCHMARK_COLLECTORMPI_HPP
