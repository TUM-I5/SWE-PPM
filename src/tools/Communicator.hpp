//
// Created by martin on 12/06/19.
//

#ifndef SWE_BENCHMARK_COMMUNICATOR_HPP
#define SWE_BENCHMARK_COMMUNICATOR_HPP
#include "blocks/SWE_DimensionalSplittingHpx.hh"
#include <hpx/include/lcos.hpp>
#include "types/Boundary.hh"
#include <array>
#include "tools/Float2DNative.hh"

#include <hpx/include/iostreams.hpp>

class SWE_DimensionalSplittingHpx;
template <typename T>
struct communicator
{


    typedef hpx::lcos::channel<T> channel_type;
    communicator(){}
    // rank: our rank in the system
    // num: number of participating partners
    communicator(std::size_t rank,std::size_t num,std::array<int, 4>  neighbours, std::array<std::shared_ptr<SWE_DimensionalSplittingHpx>, 4> neighbourBlocks)
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

    void set(Boundary n, T&& t)
    {
        // Send our data to the neighbor n using fire and forget semantics
        // Synchronization happens when receiving values.
        if(neighbourInfo[n]>= 0){
            send[n].set(hpx::launch::apply, std::move(t));
        }

    }

    bool isLocal(Boundary n){
        return neighbourInfo[n] < 0;
    }
    hpx::future<void> get(Boundary n,int nx, int ny,Float2DNative * h,Float2DNative * hu,Float2DNative * hv,Float2DNative * b, bool bat = false)
    {
        // Get our data from our neighbor, we return a future to allow the
        // algorithm to synchronize.




        if(neighbourInfo[n]>= 0){

            return hpx::dataflow(
                    hpx::util::unwrapping([&] (T border) -> void{
                        if (n == BND_LEFT) {

                            for(int i= 0; i < border.size; i++){

                                (*h)[0][i + 1] = border.H[i];
                                (*hu)[0][i + 1] = border.Hu[i];
                                (*hv)[0][i + 1] = border.Hv[i];
                            }
                        }

                        if (n == BND_RIGHT) {

                            for(int i= 0; i < border.size; i++){

                                (*h)[nx+1][i + 1] = border.H[i];
                                (*hu)[nx+1][i + 1] = border.Hu[i];
                                (*hv)[nx+1][i + 1] = border.Hv[i];
                            }

                        }

                        if (n == BND_BOTTOM) {

                            for(int i= 0; i < border.size; i++){

                                (*h)[i + 1][0] = border.H[i];
                                (*hu)[i + 1][0] = border.Hu[i];
                                (*hv)[i + 1][0] = border.Hv[i];
                            }

                        }

                        if (n == BND_TOP) {

                            for(int i= 0; i < border.size; i++){

                                (*h)[i + 1][ny+1] = border.H[i];
                                (*hu)[i + 1][ny+1] = border.Hu[i];
                                (*hv)[i + 1][ny+1] = border.Hv[i];
                            }

                        }
                    }),recv[n].get(hpx::launch::async));

        }else if (neighbourInfo[n] == -2){

            if(n == BND_LEFT){
                if(ny !=  neighbourBlocks[n]->ny) hpx::cout << "length difference left border"<<std::endl;
                int startIndexSender =  (neighbourBlocks[n]->nx)* (ny + 2) + 1;
                int startIndexReceiver = 1;

                if(!bat){
                    std::copy_n(neighbourBlocks[n]->h.getRawPointer()+startIndexSender,ny, h->getRawPointer()+ startIndexReceiver);
                    std::copy_n(neighbourBlocks[n]->hu.getRawPointer()+startIndexSender,ny, hu->getRawPointer()+ startIndexReceiver);
                    std::copy_n(neighbourBlocks[n]->hv.getRawPointer()+startIndexSender,ny, hv->getRawPointer()+ startIndexReceiver);
                }else {
                    std::copy_n(neighbourBlocks[n]->b.getRawPointer()+startIndexSender,ny, b->getRawPointer()+ startIndexReceiver);
                }

                return hpx::make_ready_future();
            }

            if(n == BND_RIGHT){
                if(ny !=  neighbourBlocks[n]->ny) hpx::cout << "length difference right border"<<std::endl;
                int startIndexSender =  ny + 2 + 1;
                int startIndexReceiver = (nx + 1) * (ny + 2) + 1;

                if(!bat){
                    std::copy_n(neighbourBlocks[n]->h.getRawPointer()+startIndexSender,ny, h->getRawPointer()+ startIndexReceiver);
                    std::copy_n(neighbourBlocks[n]->hu.getRawPointer()+startIndexSender,ny, hu->getRawPointer()+ startIndexReceiver);
                    std::copy_n(neighbourBlocks[n]->hv.getRawPointer()+startIndexSender,ny, hv->getRawPointer()+ startIndexReceiver);
                }else {
                    std::copy_n(neighbourBlocks[n]->b.getRawPointer()+startIndexSender,ny, b->getRawPointer()+ startIndexReceiver);
                }

                return hpx::make_ready_future();
            }

            if(n == BND_TOP) {
                if (nx != neighbourBlocks[n]->nx) hpx::cout << "length difference top border" << std::endl;
                if(!bat){
                    for (int i = 0; i < nx; i++) {
                        (*h)[i + 1][ny + 1] = neighbourBlocks[n]->h[i + 1][1];
                        (*hu)[i + 1][ny + 1] = neighbourBlocks[n]->hu[i + 1][1];
                        (*hv)[i + 1][ny + 1] = neighbourBlocks[n]->hv[i + 1][1];
                    }
                }else {
                    for (int i = 0; i < nx; i++) {
                       (*b)[i + 1][ny + 1] = neighbourBlocks[n]->b[i + 1][1];
                    }

                 }
                return hpx::make_ready_future();
            }
            if(n == BND_BOTTOM){
                if(nx !=  neighbourBlocks[n]->nx) hpx::cout << "length difference bottom border"<<std::endl;
                if(!bat){
                    for(int i = 0; i < nx ; i++){
                        (*h)[i+1][0] = neighbourBlocks[n]->h[i+1][neighbourBlocks[n]->ny];
                        (*hu)[i+1][0] = neighbourBlocks[n]->hu[i+1][neighbourBlocks[n]->ny];
                        (*hv)[i+1][0] = neighbourBlocks[n]->hv[i+1][neighbourBlocks[n]->ny];
                    }
                }else {
                    for(int i = 0; i < nx ; i++){
                        (*b)[i+1][0] = neighbourBlocks[n]->b[i+1][neighbourBlocks[n]->ny];
                     }
                }
                return hpx::make_ready_future();
            }
        }

    }

    std::array<hpx::lcos::channel<T>, 4> recv;
    std::array<hpx::lcos::channel<T>, 4> send;
    std::array<int,4 > neighbourInfo;
    std::array<std::shared_ptr<SWE_DimensionalSplittingHpx>, 4> neighbourBlocks;

};
#endif //SWE_BENCHMARK_COMMUNICATOR_HPP
