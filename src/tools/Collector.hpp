//
// Created by martin on 07/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTOR_HPP
#define SWE_BENCHMARK_COLLECTOR_HPP

#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
class Collector {
protected:

    double flop_ctr;
    double group_flop_ctr;
    bool is_master;
    std::string log_name;
    std::array<std::chrono::duration<double> ,4> total_ctrs;
    std::array<double, 4> result_ctrs;
    std::array<std::chrono::steady_clock::time_point,4> measure_ctrs;

public:
    enum COUNTERS {CTR_EXCHANGE,CTR_BARRIER,CTR_REDUCE,CTR_WALL};
    Collector& operator+=(const Collector& other){
        flop_ctr += other.flop_ctr;
        for(int i = 0; i < 3; i++){
            total_ctrs[i] += other.total_ctrs[i];
        }
        total_ctrs[CTR_WALL] = std::max(total_ctrs[CTR_WALL], other.total_ctrs[CTR_WALL]); //so we dont add WALL time together
        return *this;
    }

    Collector():flop_ctr(0.0f),group_flop_ctr(0.0f),result_ctrs{}, total_ctrs{},is_master(false),log_name("swe_counter.log") {};

    void setMasterSettings(bool master,std::string log_name){
        is_master = master;
        this->log_name = log_name;
    }
    void startCounter(COUNTERS ctr){
        measure_ctrs[ctr] = std::chrono::steady_clock::now();
    }
    void stopCounter(COUNTERS ctr){
        total_ctrs[ctr] +=  std::chrono::steady_clock::now()- measure_ctrs[ctr] ;
    }
    void addFlops(float flops){
        flop_ctr+= flops;
    }
    void printCounter(){
        std::cout   << "Flop count: " << group_flop_ctr << std::endl
                    << "Flops: " << ((float)1e-9*group_flop_ctr/result_ctrs[CTR_WALL]) << "GFLOPS"<< std::endl
                    << "Wall Time: " << result_ctrs[CTR_WALL] << "s"<< std::endl
                    << "Communication Time: " << result_ctrs[CTR_EXCHANGE] << "s" << std::endl
                    << "Reduction Time: " << result_ctrs[CTR_REDUCE] << "s" << std::endl;
    }
    virtual void collect () =0;
    void writeLog(){
        std::ifstream tmp(log_name);
        bool exists = false;
        if(tmp.good()){ // logfile does exist
            exists = true;
        }
        tmp.close();

        std::ofstream logfile;

        logfile.open (log_name, std::ios_base::app);
        if(!exists){
            logfile   <<  "FLOP_COUNT"
                      << "," << "FLOPS"
                      << "," << "WALL_TIME"
                      << "," << "COMMUNICATION_TIME"
                      << "," << "REDUCTION_TIME"  << std::endl;

        }
        logfile   <<  group_flop_ctr
                  << "," << ((float)group_flop_ctr/result_ctrs[CTR_WALL])
                  << "," << result_ctrs[CTR_WALL]
                  << "," << result_ctrs[CTR_EXCHANGE]
                  << "," << result_ctrs[CTR_REDUCE]  << std::endl;
        logfile.close();
    }
    void logResults(){
        collect();
        if(is_master){
            printCounter();
            writeLog();
        }
    }

};


#endif //SWE_BENCHMARK_COLLECTOR_HPP
