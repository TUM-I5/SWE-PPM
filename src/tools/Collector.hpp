//
// Created by martin on 07/03/2020.
//

#ifndef SWE_BENCHMARK_COLLECTOR_HPP
#define SWE_BENCHMARK_COLLECTOR_HPP

#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
class Collector {
//protected:
public:
    int rank;
    int totalBlocks;
    double flop_ctr;
    double group_flop_ctr;
    bool is_master;
    std::string log_name;
    std::array<std::chrono::duration<double>, 4> total_ctrs;
    std::array<double, 4> result_ctrs;
    std::array<std::chrono::steady_clock::time_point, 4> measure_ctrs;
    std::vector<float> timesteps;
public:
    enum COUNTERS {
        CTR_EXCHANGE, CTR_BARRIER, CTR_REDUCE, CTR_WALL
    };

    Collector &operator+=(const Collector &other) {
        flop_ctr += other.flop_ctr;
        for (int i = 0; i < 3; i++) {
            total_ctrs[i] += other.total_ctrs[i];
            measure_ctrs[i] = other.measure_ctrs[i];
        }
        total_ctrs[CTR_WALL] = std::max(total_ctrs[CTR_WALL],
                                        other.total_ctrs[CTR_WALL]); //so we dont add WALL time together
        timesteps.insert( timesteps.end(), other.timesteps.begin(), other.timesteps.end() );
        return *this;
    }

    Collector() : flop_ctr(0.0f), group_flop_ctr(0.0f), result_ctrs{}, total_ctrs{}, is_master(false),
                  log_name("swe_counter.log") {};
    void setRank(int rank) {
        Collector::rank = rank;
    }
    void setMasterSettings(bool master, std::string log_name,int totalRanks) {
        is_master = master;
        this->log_name = log_name;
        this->totalBlocks = totalRanks;
    }
    void addTimestep(float timestep){
        timesteps.push_back(timestep);
    }
    void startCounter(COUNTERS ctr) {
        measure_ctrs[ctr] = std::chrono::steady_clock::now();
    }

    void stopCounter(COUNTERS ctr) {
        total_ctrs[ctr] += std::chrono::steady_clock::now() - measure_ctrs[ctr];
    }

    void addFlops(float flops) {
        flop_ctr += flops;
    }

    void printCounter() {

        auto timestepMinMax = std::minmax_element(timesteps.begin(),timesteps.end());
        float timestepAvg =  timesteps.size()>0?std::accumulate(timesteps.begin(), timesteps.end(), 0.0f)/(float)timesteps.size():0;

        std::cout << "Flop count: " << group_flop_ctr << std::endl
                  << "Flops: " << ((float) 1e-9 * group_flop_ctr / result_ctrs[CTR_WALL]) << "GFLOPS" << std::endl
                  << "Wall Time: " << result_ctrs[CTR_WALL] << "s" << std::endl
                  << "Communication Time: " << result_ctrs[CTR_EXCHANGE] << "s" << std::endl
                  << "Reduction Time: " << result_ctrs[CTR_REDUCE] << "s" << std::endl
                  << "Timesteps Min: " << (timesteps.size()>0?*timestepMinMax.first:0) << " Max: " << (timesteps.size()>0?*timestepMinMax.second:0) << " Average: "<< timestepAvg << std::endl;
    }

    virtual void collect() = 0;
    void writeTimestepData(){
        if(timesteps.size() == 0) return;
        std::ofstream logfile;
        std::size_t pos = log_name.find(".log");      // position of "live" in str

        std::string stripped_log_name = log_name.substr (0,pos);
        std::string filename = stripped_log_name+"_timesteps_"+std::to_string(rank)+".log";

        logfile.open(filename, std::ios_base::out);
        for(auto ts : timesteps){
            logfile << ts<< std::endl;
        }

        logfile.close();
    }
    void writeLog() {
        std::ifstream tmp(log_name);
        bool exists = false;
        if (tmp.good()) { // logfile does exist
            exists = true;
        }
        tmp.close();

        std::ofstream logfile;

        logfile.open(log_name, std::ios_base::app);
        if (!exists) {

            logfile << "PE_COUNT"
                    << "," << "FLOP_COUNT"
                    << "," << "FLOPS"
                    << "," << "WALL_TIME"
                    << "," << "COMMUNICATION_TIME"
                    << "," << "REDUCTION_TIME" << std::endl;

        }
        logfile << totalBlocks
                << "," <<  group_flop_ctr
                << "," << ((float) group_flop_ctr / result_ctrs[CTR_WALL])
                << "," << result_ctrs[CTR_WALL]
                << "," << result_ctrs[CTR_EXCHANGE]
                << "," << result_ctrs[CTR_REDUCE] << std::endl;
        logfile.close();
    }

    void logResults() {
        collect();
        writeTimestepData();
        if (is_master) {
            printCounter();
            writeLog();

        }
    }

};


#endif //SWE_BENCHMARK_COLLECTOR_HPP
