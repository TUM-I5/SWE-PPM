//
// Created by martin on 12/06/19.
//

#ifndef SWE_BENCHMARK_COMMUNICATOR_HPP
#define SWE_BENCHMARK_COMMUNICATOR_HPP
#include "blocks/SWE_DimensionalSplittingHpx.hh"
#include <hpx/include/lcos.hpp>
#include "types/Boundary.hh"
#include <array>

class SWE_DimensionalSplittingHpx;
template <typename T>
struct communicator
{


    typedef hpx::lcos::channel<T> channel_type;
    communicator(){}
    // rank: our rank in the system
    // num: number of participating partners
    communicator(std::size_t rank,std::size_t num,std::array<int, 4>  neighbours, std::array<SWE_DimensionalSplittingHpx*, 4> neighbourBlocks)
    {
        static const char* top_name = "top";
        static const char* bot_name = "bot";
        static const char* left_name = "left";
        static const char* right_name = "right";
        // Only set up channels if we have more than one partner
        neighbourInfo = neighbours;
        this->neighbourBlocks = neighbourBlocks;
        if (num > 1)
        {
            // We have an upper neighbor if our rank is greater than zero.
            if (neighbours[BND_TOP] >= 0 )
            {

                // Retrieve the channel from our upper neighbor from which we receive
                // the row we need to update the first row in our partition.
                recv[BND_TOP] = hpx::find_from_basename<channel_type>(bot_name, neighbours[BND_TOP]);

                // Create the channel we use to send our first row to our upper
                // neighbor
                send[BND_TOP] = channel_type(hpx::find_here());
                // Register the channel with a name such that our neighbor can find it.
                hpx::register_with_basename(top_name, send[BND_TOP], rank);
            }
            if (neighbours[BND_BOTTOM] >= 0)
            {
                // Retrieve the channel from our upper neighbor from which we receive
                // the row we need to update the first row in our partition.
                recv[BND_BOTTOM] = hpx::find_from_basename<channel_type>(top_name, neighbours[BND_BOTTOM]);

                // Create the channel we use to send our first row to our upper
                // neighbor
                send[BND_BOTTOM] = channel_type(hpx::find_here());
                // Register the channel with a name such that our neighbor can find it.
                hpx::register_with_basename(bot_name, send[BND_BOTTOM], rank);
            }
            if (neighbours[BND_LEFT] >= 0)
            {
                // Retrieve the channel from our upper neighbor from which we receive
                // the row we need to update the first row in our partition.
                recv[BND_LEFT] = hpx::find_from_basename<channel_type>(right_name, neighbours[BND_LEFT]);

                // Create the channel we use to send our first row to our upper
                // neighbor
                send[BND_LEFT] = channel_type(hpx::find_here());
                // Register the channel with a name such that our neighbor can find it.
                hpx::register_with_basename(left_name, send[BND_LEFT], rank);
            }
            if (neighbours[BND_RIGHT] >= 0)
            {
                // Retrieve the channel from our upper neighbor from which we receive
                // the row we need to update the first row in our partition.
                recv[BND_RIGHT] = hpx::find_from_basename<channel_type>(left_name, neighbours[BND_RIGHT]);

                // Create the channel we use to send our first row to our upper
                // neighbor
                send[BND_RIGHT] = channel_type(hpx::find_here());
                // Register the channel with a name such that our neighbor can find it.
                hpx::register_with_basename(right_name, send[BND_RIGHT], rank);
            }
        }
    }

    bool has_neighbor(Boundary n) const
    {
        return recv[n] && send[n];
    }

    void set(Boundary n, T&& t)
    {
        // Send our data to the neighbor n using fire and forget semantics
        // Synchronization happens when receiving values.
        if(neighbourInfo[n]>= 0){
            send[n].set(hpx::launch::apply, std::move(t));
        }

    }

    hpx::future<T> get(Boundary n, bool bat = false)
    {
        // Get our data from our neighbor, we return a future to allow the
        // algorithm to synchronize.
        if(neighbourInfo[n]>= 0){
            return recv[n].get(hpx::launch::async);
        }else if (neighbourInfo[n] == -2){
            //std::cout << "get Layer of neighbour" << std::endl;
            if(n == BND_LEFT)
            return hpx::make_ready_future(neighbourBlocks[n]->getGhostLayer(BND_RIGHT,bat));
            if(n == BND_RIGHT)
                return hpx::make_ready_future(neighbourBlocks[n]->getGhostLayer(BND_LEFT,bat));
            if(n == BND_TOP)
                return hpx::make_ready_future(neighbourBlocks[n]->getGhostLayer(BND_BOTTOM,bat));
            if(n == BND_BOTTOM)
                return hpx::make_ready_future(neighbourBlocks[n]->getGhostLayer(BND_TOP,bat));
        }

    }

    std::array<hpx::lcos::channel<T>, 4> recv;
    std::array<hpx::lcos::channel<T>, 4> send;
    std::array<int,4 > neighbourInfo;
    std::array<SWE_DimensionalSplittingHpx*, 4> neighbourBlocks;
  /*  template <typename Archive>
    void serialize(Archive & ar, unsigned)
    {
        ar & recv;
        ar & send;

    }*/
};
#endif //SWE_BENCHMARK_COMMUNICATOR_HPP
