//
// Created by martin on 12/06/19.
//

#ifndef SWE_BENCHMARK_COMMUNICATOR_HPP
#define SWE_BENCHMARK_COMMUNICATOR_HPP

#include <hpx/include/lcos.hpp>
#include "types/Boundary.hh"
#include <array>

template <typename T>
struct communicator
{


    typedef hpx::lcos::channel<T> channel_type;

    // rank: our rank in the system
    // num: number of participating partners
    communicator(std::size_t rank, std::size_t num,std::array<int, 4>  const &neighbours)
    {
        static const char* top_name = "top";
        static const char* bot_name = "bot";
        static const char* left_name = "left";
        static const char* right_name = "right";
        // Only set up channels if we have more than one partner
        if (num > 1)
        {
            // We have an upper neighbor if our rank is greater than zero.
            if (neighbours[BND_TOP] != -1)
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
            if (neighbours[BND_BOTTOM] != -1)
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
            if (neighbours[BND_LEFT] != -1)
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
            if (neighbours[BND_RIGHT] != -1)
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
        send[n].set(hpx::launch::apply, std::move(t));
    }

    hpx::future<T> get(Boundary n)
    {
        // Get our data from our neighbor, we return a future to allow the
        // algorithm to synchronize.
        return recv[n].get(hpx::launch::async);
    }

    std::array<hpx::lcos::channel<T>, 4> recv;
    std::array<hpx::lcos::channel<T>, 4> send;

  /*  template <typename Archive>
    void serialize(Archive & ar, unsigned)
    {
        ar & recv;
        ar & send;

    }*/
};
#endif //SWE_BENCHMARK_COMMUNICATOR_HPP
