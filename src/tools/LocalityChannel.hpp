//
// Created by martin on 30/06/19.
//

#ifndef SWE_BENCHMARK_LOCALITYCHANNEL_HPP
#define SWE_BENCHMARK_LOCALITYCHANNEL_HPP
#include <hpx/include/lcos.hpp>
#include "types/Boundary.hh"
#include <array>

template <typename T>
struct LocalityChannel
{


    typedef hpx::lcos::channel<T> channel_type;

    // rank: our rank in the system
    // num: number of participating partners
    LocalityChannel()
    {

    }

    LocalityChannel(std::size_t rank, int localityRanks)
    {
        static const char* master_name = "master";
        static const char* slave_name = "slave";

        // Only set up channels if we have more than one partner
        if (localityRanks > 1 ){
        if(rank == 0){
            for(int i = 1; i < localityRanks; i++){
                recv.push_back(hpx::find_from_basename<channel_type>(slave_name, i));
                send.push_back(channel_type(hpx::find_here()));
                hpx::register_with_basename(master_name, send[i-1], i);
            }

        }   else {
            recv.push_back(hpx::find_from_basename<channel_type>(master_name, rank));
            send.push_back(channel_type(hpx::find_here()));
            hpx::register_with_basename(slave_name, send[0], rank);
        }

        }

    }


    void set(T&& t)
    {
        for(auto &channel : send){
            channel.set(hpx::launch::apply, std::move(t));
        }
    }

    std::vector<hpx::future<T>> get()
    {
        // Get our data from our neighbor, we return a future to allow the
        // algorithm to synchronize.
        std::vector<hpx::future<T>> ret;
        ret.reserve(recv.size());
        for(auto &channel : recv){
            ret.push_back(channel.get(hpx::launch::async));
        }
        return ret;
    }

    std::vector<hpx::lcos::channel<T>> recv;
    std::vector<hpx::lcos::channel<T>> send;

};
#endif //SWE_BENCHMARK_LOCALITYCHANNEL_HPP
