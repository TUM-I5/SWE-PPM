//
// Created by martin on 05/06/19.
//

/**
 * @file
 * This file is part of an SWE fork created for the Tsunami-Simulation Bachelor Lab Course.
 *
 * @author Jurek Olden (jurek.olden AT in.tum.de)
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 * Implementation of SWE_DimensionalSplitting.hh
 *
 */
#include "SWE_DimensionalSplittingHpx.hh"

#include <cassert>
#include <algorithm>
#include <omp.h>
#include <typeinfo>
#include <hpx/include/parallel_transform_reduce.hpp>
#include <utility>
/*
 * Constructor of a SWE_DimensionalSplitting Block.
 * Computational domain is [1,...,nx]*[1,...,ny]
 * Ghost layer consists of two additional rows and columns
 *
 * State variables h, hu, hv and b are defined on the whole grid (including ghost layer)
 * Net updates coming from above/below/left/right are defined for each cell.
 *
 * Net updates are computed on all rows first, then on all columns, the total net updates are then composed
 * from the two 1D solutions.
 *
 * This strategy only works, if the timestep chosen w.r.t. to the maximum horizontal wave speeds
 * also satisfies the CFL-condition in y-direction.
 *
 * @param l_nx Size of the computational domain in x-direction
 * @param l_ny Size of the computational domain in y-direction
 * @param l_dx Cell width
 * @param l_dy Cell height
 */

SWE_DimensionalSplittingHpx::SWE_DimensionalSplittingHpx(int nx, int ny, float dx, float dy, float originX, float originY, communicator_type comm) :
/*
 * Important note concerning grid allocations:
 * Since index shifts all over the place are bug-prone and maintenance unfriendly,
 * an index of [x][y] is at the actual position x,y on the actual grid.
 * This implies that the allocation size in any direction might be larger than the number of values needed.
 * So if, for instance, array[x][y] needs to hold values in the domain [1,a][1,b],
 * it will be allocated with size (a+1, b+1) instead of (a, b).
 * array[0][0] is then unused.
 */
// Initialize grid metadata using the base class constructor
        SWE_Block(nx, ny, dx, dy, originX, originY),

        // intermediate state Q after x-sweep
        hStar(nx + 1, ny + 2),
        huStar(nx + 1, ny + 2),

        /*
         * Temporary storage for the net updates per grid cell during a sweep.
         * There are four update values per cell:
         * Left-going wave from the right edge, analogue for the left edge.
         * Down-going wave from the top edge, analogue for the bottom edge
         */

        // For the x-sweep
        hNetUpdatesLeft(nx + 2, ny + 2),
        hNetUpdatesRight(nx + 2, ny + 2),

        huNetUpdatesLeft(nx + 2, ny + 2),
        huNetUpdatesRight(nx + 2, ny + 2),

        // For the y-sweep
        hNetUpdatesBelow(nx + 1, ny + 2),
        hNetUpdatesAbove(nx + 1, ny + 2),

        hvNetUpdatesBelow(nx + 1, ny + 2),
        hvNetUpdatesAbove(nx + 1, ny + 2),
        comm(comm){



    computeTime = 0.;
    computeTimeWall = 0.;
    communicationTime = 0.;
    flopCounter = 0;
}



void SWE_DimensionalSplittingHpx::connectNeighbours(int p_neighbourRankId[]) {
     //comm(p_neighbourRankId[i])

}



typedef copyLayerStruct<std::vector<float>> communication_type;

HPX_REGISTER_CHANNEL_DECLARATION(communication_type);
HPX_REGISTER_CHANNEL(communication_type);




void SWE_DimensionalSplittingHpx::exchangeBathymetry() {


    /*********
     * SEND *
     ********/


     if (boundaryType[BND_LEFT] == CONNECT) {

        int startIndex = ny + 2 + 1;

        comm.set(BND_LEFT,  copyLayerStruct<std::vector<float>> {ny,std::vector<float>(b.getRawPointer() + startIndex, b.getRawPointer() + startIndex + ny)});


    }
    if (boundaryType[BND_RIGHT] == CONNECT) {
        int startIndex = nx * (ny + 2) + 1;
        comm.set(BND_RIGHT,  copyLayerStruct<std::vector<float>> {ny,std::vector<float>(b.getRawPointer() + startIndex, b.getRawPointer() + startIndex + ny)});

    }
    if (boundaryType[BND_BOTTOM] == CONNECT) {

        std::vector<float> send_bat;
        for (int i = 1; i < nx+1 ; i++) {
            send_bat.push_back(b[i][1]);
        }

        comm.set(BND_BOTTOM,  copyLayerStruct<std::vector<float>> {nx,send_bat});
    }
    if (boundaryType[BND_TOP] == CONNECT) {

        std::vector<float> send_bat;
        for (int i = 1; i < nx+1 ; i++) {
            send_bat.push_back(b[i][ny]);
        }
        comm.set(BND_TOP,  copyLayerStruct<std::vector<float>> {nx,send_bat});

    }

    /***********
     * RECEIVE *
     **********/
   std::vector<hpx::future<copyLayerStruct<std::vector<float>>>>fut;

    if (boundaryType[BND_LEFT] == CONNECT) {
      fut.push_back(comm.get(BND_LEFT));


    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }

    if (boundaryType[BND_RIGHT] == CONNECT) {

        fut.push_back(comm.get(BND_RIGHT));

    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }

    if (boundaryType[BND_BOTTOM] == CONNECT) {
        fut.push_back(comm.get(BND_BOTTOM));

    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }

    if (boundaryType[BND_TOP] == CONNECT) {
        fut.push_back(comm.get(BND_TOP));
    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }

    auto borders = hpx::when_all(fut).get();


   if (boundaryType[BND_LEFT] == CONNECT) {

        int startIndex = 1;
        int i = 0;
        for(auto &elm: borders[BND_LEFT].get().B){
            b[0][i + 1] = elm;
            i++;
        }
    } else {

    }


    if (boundaryType[BND_RIGHT] == CONNECT) {
        int startIndex = (nx + 1) * (ny + 2) + 1;

        int i = 0;

        for(auto &elm:  borders[BND_RIGHT].get().B){

            b[nx+1][i + 1] = elm;
            i++;
        }

      } else {

    }

    if (boundaryType[BND_BOTTOM] == CONNECT) {
        int i = 0;
        for(auto &elm: borders[BND_BOTTOM].get().B){

            b[i+1][0] = elm;
            i++;
        }
    } else {

    }

    if (boundaryType[BND_TOP] == CONNECT) {
        int i = 0;
        for(auto &elm: borders[BND_TOP].get().B){

            b[i+1][ny+1] = elm;
            i++;
        }

    } else {

    }

}


hpx::future<void> SWE_DimensionalSplittingHpx::setGhostLayer() {
    // Apply appropriate conditions for OUTFLOW/WALL boundaries
    SWE_Block::applyBoundaryConditions();



    assert(h.getRows() == ny + 2);
    assert(hu.getRows() == ny + 2);
    assert(hv.getRows() == ny + 2);
    assert(h.getCols() == nx + 2);
    assert(hu.getCols() == nx + 2);
    assert(hv.getCols() == nx + 2);

    /*********
     * SEND *
     ********/
    struct timespec startTimeComm;
    clock_gettime(CLOCK_MONOTONIC, &startTimeComm);
    if (boundaryType[BND_LEFT] == CONNECT) {

        int startIndex = ny + 2 + 1;

        comm.set(BND_LEFT,  copyLayerStruct<std::vector<float>> {ny,{},
                                                                 std::vector<float>(h.getRawPointer() + startIndex, h.getRawPointer() + startIndex + ny),
                                                                 std::vector<float>(hu.getRawPointer() + startIndex, hu.getRawPointer() + startIndex + ny),
                                                                 std::vector<float>(hv.getRawPointer() + startIndex, hv.getRawPointer() + startIndex + ny)});


    }
    if (boundaryType[BND_RIGHT] == CONNECT) {
        int startIndex = nx * (ny + 2) + 1;
        comm.set(BND_RIGHT,  copyLayerStruct<std::vector<float>> {ny,{},
                                                                std::vector<float>(h.getRawPointer() + startIndex, h.getRawPointer() + startIndex + ny),
                                                                std::vector<float>(hu.getRawPointer() + startIndex, hu.getRawPointer() + startIndex + ny),
                                                                std::vector<float>(hv.getRawPointer() + startIndex, hv.getRawPointer() + startIndex + ny)});
    }
    if (boundaryType[BND_BOTTOM] == CONNECT) {

        std::vector<float> send_h;
        std::vector<float> send_hu;
        std::vector<float> send_hv;
        send_h.reserve(nx);
        send_hu.reserve(nx);
        send_hv.reserve(nx);
        for (int i = 1; i < nx+1 ; i++) {
            send_h.push_back(h[i][1]);
            send_hu.push_back(hu[i][1]);
            send_hv.push_back(hv[i][1]);
        }

        comm.set(BND_BOTTOM,  copyLayerStruct<std::vector<float>> {nx,{},send_h,send_hu,send_hv});
    }
    if (boundaryType[BND_TOP] == CONNECT) {

        std::vector<float> send_h;
        std::vector<float> send_hu;
        std::vector<float> send_hv;
        send_h.reserve(nx);
        send_hu.reserve(nx);
        send_hv.reserve(nx);
        for (int i = 1; i < nx+1 ; i++) {
            send_h.push_back(h[i][ny]);
            send_hu.push_back(hu[i][ny]);
            send_hv.push_back(hv[i][ny]);
        }

        comm.set(BND_TOP,  copyLayerStruct<std::vector<float>> {nx,{},send_h,send_hu,send_hv});

    }


    /***********
     * RECEIVE *
     **********/

    std::vector<hpx::future<copyLayerStruct<std::vector<float>>>>fut;
    if (boundaryType[BND_LEFT] == CONNECT) {
        fut.push_back(comm.get(BND_LEFT));
    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }
    if (boundaryType[BND_RIGHT] == CONNECT) {
        fut.push_back(comm.get(BND_RIGHT));
    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }

    if (boundaryType[BND_BOTTOM] == CONNECT) {
        fut.push_back(comm.get(BND_BOTTOM));

    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }

    if (boundaryType[BND_TOP] == CONNECT) {
        fut.push_back(comm.get(BND_TOP));
    } else {
        fut.push_back(hpx::future<copyLayerStruct<std::vector<float>>>());
    }


   auto ret = hpx::dataflow([this,startTimeComm] (std::vector<hpx::future<copyLayerStruct<std::vector<float>>>> borders )-> void {
            if (boundaryType[BND_LEFT] == CONNECT) {

                int startIndex = 1;
                auto border = borders[BND_LEFT].get();

                for(int i= 0; i < border.size; i++){

                    h[0][i + 1] = border.H[i];
                    hu[0][i + 1] = border.Hu[i];
                    hv[0][i + 1] = border.Hv[i];
                }
            }

            if (boundaryType[BND_RIGHT] == CONNECT) {
                int startIndex = (nx + 1) * (ny + 2) + 1;

                auto border = borders[BND_RIGHT].get();

                for(int i= 0; i < border.size; i++){

                    h[nx+1][i + 1] = border.H[i];
                    hu[nx+1][i + 1] = border.Hu[i];
                    hv[nx+1][i + 1] = border.Hv[i];
                }

            }

            if (boundaryType[BND_BOTTOM] == CONNECT) {

                auto border = borders[BND_BOTTOM].get();

                for(int i= 0; i < border.size; i++){

                    h[i + 1][0] = border.H[i];
                    hu[i + 1][0] = border.Hu[i];
                    hv[i + 1][0] = border.Hv[i];
                }

            }

            if (boundaryType[BND_TOP] == CONNECT) {
                auto border = borders[BND_TOP].get();

                for(int i= 0; i < border.size; i++){

                    h[i + 1][ny+1] = border.H[i];
                    hu[i + 1][ny+1] = border.Hu[i];
                    hv[i + 1][ny+1] = border.Hv[i];
                }

            }


            clock_gettime(CLOCK_MONOTONIC, &endTime);
            communicationTime += (endTime.tv_sec - startTimeComm.tv_sec);
            communicationTime += (float) (endTime.tv_nsec - startTimeComm.tv_nsec) / 1E9;


    },std::move(fut));


    return ret;
}


void SWE_DimensionalSplittingHpx::computeXSweep (){
    computeClock = clock();
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    //maximum (linearized) wave speed within one iteration
   // float maxHorizontalWaveSpeed = (float) 0.;


    std::vector<float> wave;
    // x-sweep, compute the actual domain plus ghost rows above and below
    // iterate over cells on the x-axis, leave out the last column (two cells per computation)
    /*hpx::parallel::exclusive_scan(hpx::parallel::execution::par,                   // 3
                                  0,nx + 1, wave.begin(),
                                  [this](int x)
                                  {
                                      float maxHorizontalWaveSpeed = (float) 0.;
                                      for (int y = 0; y < ny+2; y++) {
                                          solver.computeNetUpdates (
                                                  h[x][y], h[x + 1][y],
                                                  hu[x][y], hu[x + 1][y],
                                                  b[x][y], b[x + 1][y],
                                                  hNetUpdatesLeft[x][y], hNetUpdatesRight[x + 1][y],
                                                  huNetUpdatesLeft[x][y], huNetUpdatesRight[x + 1][y],
                                                  maxHorizontalWaveSpeed
                                          );
                                      }
                                      return maxHorizontalWaveSpeed;
                                  });

*/

std::vector<int> test;
test.reserve(nx+1);
for(int i = 0; i < nx+1 ; i++)test.push_back(i);

    float maxHorizontalWaveSpeed = hpx::parallel::transform_reduce(hpx::parallel::execution::par,
                                                       std::begin(test), std::end(test),
                                                                   (float) 0.,   [](float a, float b){ return b; },
                                                       [this](int x) ->
                                                       float{

                                                           float maxHorizontalWaveSpeed = (float) 0.;
                                                           for (int y = 0; y < ny+2; y++) {
                                                               solver.computeNetUpdates (
                                                                       h[x][y], h[x + 1][y],
                                                                       hu[x][y], hu[x + 1][y],
                                                                       b[x][y], b[x + 1][y],
                                                                       hNetUpdatesLeft[x][y], hNetUpdatesRight[x + 1][y],
                                                                       huNetUpdatesLeft[x][y], huNetUpdatesRight[x + 1][y],
                                                                       maxHorizontalWaveSpeed
                                                               );
                                                           }
                                                           return maxHorizontalWaveSpeed;
                                                       }
                                                               );
 //   std::cout<< maxHorizontalWaveSpeed << std::endl;
  //  maxHorizontalWaveSpeed= *std::min_element(wave.begin(), wave.end());

    /*for (int x = 0; x < nx + 1; x++) {
        const int ny_end = ny+2;
        // iterate over all rows, including ghost layer

      for (int y = 0; y < ny_end; y++) {
            solver.computeNetUpdates (
                    h[x][y], h[x + 1][y],
                    hu[x][y], hu[x + 1][y],
                    b[x][y], b[x + 1][y],
                    hNetUpdatesLeft[x][y], hNetUpdatesRight[x + 1][y],
                    huNetUpdatesLeft[x][y], huNetUpdatesRight[x + 1][y],
                    maxHorizontalWaveSpeed
            );
        }
    }*/
    flopCounter += nx*ny*135;

    // Accumulate compute time -> exclude the reduction
    computeClock = clock() - computeClock;
    computeTime += (float) computeClock / CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    computeTimeWall += (endTime.tv_sec - startTime.tv_sec);
    computeTimeWall += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

    // compute max timestep according to cautious CFL-condition
    maxTimestep = (float) .4 * (dx / maxHorizontalWaveSpeed);

    maxTimestepGlobal = maxTimestep;
}
void SWE_DimensionalSplittingHpx::computeYSweep (){
    maxTimestep = maxTimestepGlobal;
    float maxVerticalWaveSpeed = (float) 0.;
    computeClock = clock();
    clock_gettime(CLOCK_MONOTONIC, &startTime);


    // set intermediary Q* states

    for (int x = 1; x < nx + 1; x++) {
        for (int y = 0; y < ny + 2; y++) {
            hStar[x][y] = h[x][y] - (maxTimestep / dx) * (hNetUpdatesLeft[x][y] + hNetUpdatesRight[x][y]);
            huStar[x][y] = hu[x][y] - (maxTimestep / dx) * (huNetUpdatesLeft[x][y] + huNetUpdatesRight[x][y]);
        }
    }

    std::vector<int> test;
    test.reserve(nx+1);
    for(int i = 1; i < nx+1 ; i++)test.push_back(i);
    float maxHorizontalWaveSpeed = hpx::parallel::transform_reduce(hpx::parallel::execution::par,
                                                                   std::begin(test), std::end(test),
                                                                   (float) 0.,   [](float a, float b){ return b; },
                                                                   [this](int x) ->
                                                                           float{

                                                                       float maxVerticalWaveSpeed = (float) 0.;
                                                                       for (int y = 0; y < ny+1; y++) {
                                                                           solver.computeNetUpdates (
                                                                                   h[x][y], h[x][y + 1],
                                                                                   hv[x][y], hv[x][y + 1],
                                                                                   b[x][y], b[x][y + 1],
                                                                                   hNetUpdatesBelow[x][y], hNetUpdatesAbove[x][y + 1],
                                                                                   hvNetUpdatesBelow[x][y], hvNetUpdatesAbove[x][y + 1],
                                                                                   maxVerticalWaveSpeed
                                                                           );
                                                                       }
                                                                       return maxVerticalWaveSpeed;
                                                                   }
    );
    /*for (int x = 1; x < nx + 1; x++) {
        const int ny_end = ny+1;
        // iterate over all rows, including ghost layer
        for (int y = 0; y < ny_end; y++) {
            solver.computeNetUpdates (
                    h[x][y], h[x][y + 1],
                    hv[x][y], hv[x][y + 1],
                    b[x][y], b[x][y + 1],
                    hNetUpdatesBelow[x][y], hNetUpdatesAbove[x][y + 1],
                    hvNetUpdatesBelow[x][y], hvNetUpdatesAbove[x][y + 1],
                    maxVerticalWaveSpeed
            );
        }
    }

*/

    flopCounter += nx*ny*135;
    // Accumulate compute time
    computeClock = clock() - computeClock;
    computeTime += (float) computeClock / CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    computeTimeWall += (endTime.tv_sec - startTime.tv_sec);
    computeTimeWall += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
}
/**
 * Compute net updates for the block.
 * The member variable #maxTimestep will be updated with the
 * maximum allowed time step size
 */
void SWE_DimensionalSplittingHpx::computeNumericalFluxes () {
    // Start compute clocks
    computeClock = clock();
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    //maximum (linearized) wave speed within one iteration
    float maxHorizontalWaveSpeed = (float) 0.;
    float maxVerticalWaveSpeed = (float) 0.;


        // x-sweep, compute the actual domain plus ghost rows above and below
        // iterate over cells on the x-axis, leave out the last column (two cells per computation)


        for (int x = 0; x < nx + 1; x++) {
            const int ny_end = ny+2;
            // iterate over all rows, including ghost layer
            /*hpx::parallel::for_loop(
                   hpx::parallel::execution::par,
                   0,ny_end,
                    [&x,this,&maxHorizontalWaveSpeed](int y)
                    {
                        solver.computeNetUpdates (
                                h[x][y], h[x + 1][y],
                                hu[x][y], hu[x + 1][y],
                                b[x][y], b[x + 1][y],
                                hNetUpdatesLeft[x][y], hNetUpdatesRight[x + 1][y],
                                huNetUpdatesLeft[x][y], huNetUpdatesRight[x + 1][y],
                                maxHorizontalWaveSpeed
                        );
                    });
*/
            for (int y = 0; y < ny_end; y++) {
                solver.computeNetUpdates (
                        h[x][y], h[x + 1][y],
                        hu[x][y], hu[x + 1][y],
                        b[x][y], b[x + 1][y],
                        hNetUpdatesLeft[x][y], hNetUpdatesRight[x + 1][y],
                        huNetUpdatesLeft[x][y], huNetUpdatesRight[x + 1][y],
                        maxHorizontalWaveSpeed
                );
            }
        }


    // Accumulate compute time -> exclude the reduction
    computeClock = clock() - computeClock;
    computeTime += (float) computeClock / CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    computeTimeWall += (endTime.tv_sec - startTime.tv_sec);
    computeTimeWall += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

    // compute max timestep according to cautious CFL-condition
    maxTimestep = (float) .4 * (dx / maxHorizontalWaveSpeed);
    //MPI_Allreduce(&maxTimestep, &maxTimestepGlobal, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);
    //maxTimestep = maxTimestepGlobal;

  //  maxTimestep=hpx::lcos::reduce<rand_num_action>(ids,
    //                                   Max<float>(),maxTimestep).get();
    // restart compute clocks
    computeClock = clock();
    clock_gettime(CLOCK_MONOTONIC, &startTime);


        // set intermediary Q* states

        for (int x = 1; x < nx + 1; x++) {
            for (int y = 0; y < ny + 2; y++) {
                hStar[x][y] = h[x][y] - (maxTimestep / dx) * (hNetUpdatesLeft[x][y] + hNetUpdatesRight[x][y]);
                huStar[x][y] = hu[x][y] - (maxTimestep / dx) * (huNetUpdatesLeft[x][y] + huNetUpdatesRight[x][y]);
            }
        }


        for (int x = 1; x < nx + 1; x++) {
            const int ny_end = ny+1;
            // iterate over all rows, including ghost layer
            for (int y = 0; y < ny_end; y++) {
                solver.computeNetUpdates (
                        h[x][y], h[x][y + 1],
                        hv[x][y], hv[x][y + 1],
                        b[x][y], b[x][y + 1],
                        hNetUpdatesBelow[x][y], hNetUpdatesAbove[x][y + 1],
                        hvNetUpdatesBelow[x][y], hvNetUpdatesAbove[x][y + 1],
                        maxVerticalWaveSpeed
                );
            }
        }


    // Accumulate compute time
    computeClock = clock() - computeClock;
    computeTime += (float) computeClock / CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    computeTimeWall += (endTime.tv_sec - startTime.tv_sec);
    computeTimeWall += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
}

/**
 * Updates the unknowns with the already computed net-updates.
 *
 * @param dt time step width used in the update. The timestep has to be equal to maxTimestep calculated by computeNumericalFluxes(),
 * since this is the step width used for the intermediary updates after the x-sweep.
 */
void SWE_DimensionalSplittingHpx::updateUnknowns (float dt) {
    // Start compute clocks
    computeClock = clock();
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    // this assertion has to hold since the intermediary star states were calculated internally using a timestep width of maxTimestep
    assert(std::abs(dt - maxTimestep) < 0.00001);
    //update cell averages with the net-updates
    for (int x = 1; x < nx+1; x++) {
        for (int y = 1; y < ny + 1; y++) {
            h[x][y] = hStar[x][y] - (maxTimestep / dx) * (hNetUpdatesBelow[x][y] + hNetUpdatesAbove[x][y]);
            hu[x][y] = huStar[x][y];
            hv[x][y] = hv[x][y] - (maxTimestep / dx) * (hvNetUpdatesBelow[x][y] + hvNetUpdatesAbove[x][y]);
        }
    }

    // Accumulate compute time
    computeClock = clock() - computeClock;
    computeTime += (float) computeClock / CLOCKS_PER_SEC;

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    computeTimeWall += (endTime.tv_sec - startTime.tv_sec);
    computeTimeWall += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
}
