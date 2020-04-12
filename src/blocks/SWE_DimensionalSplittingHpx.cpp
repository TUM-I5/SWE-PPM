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
SWE_DimensionalSplittingHpx::SWE_DimensionalSplittingHpx(int nx, int ny, float dx, float dy, float originX,
                                                         float originY, bool localTimestepping, std::string name, bool write) :
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
        SWE_Block(nx, ny, dx, dy, originX, originY, localTimestepping),

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
        hvNetUpdatesAbove(nx + 1, ny + 2)
       {
            if(write){
                writer = new NetCdfWriter(
                        name,
                        b,
                        {{1, 1, 1, 1}},
                        nx,
                        ny,
                        dx,
                        dy,
                        originX,
                        originY);

            }
       }


void SWE_DimensionalSplittingHpx::writeTimestep(float timestep) {
            if(write){
                writer->writeTimeStep(h, hu, hv, timestep);
            }

}

void SWE_DimensionalSplittingHpx::connectNeighbours(communicator_type
                                                    comm) {
    this->comm = comm;
}


typedef copyLayerStruct<std::vector<float>> communication_type;

//HPX_REGISTER_CHANNEL_DECLARATION(communication_type);
HPX_REGISTER_CHANNEL(communication_type);


void SWE_DimensionalSplittingHpx::exchangeBathymetry() {


    /*********
     * SEND *
     ********/


    if (boundaryType[BND_LEFT] == CONNECT && !comm.isLocal(BND_LEFT)) {

        int startIndex = ny + 2 + 1;

        comm.set(BND_LEFT, copyLayerStruct<std::vector<float>>{ny, std::vector<float>(b.getRawPointer() + startIndex,
                                                                                      b.getRawPointer() + startIndex +
                                                                                      ny)});

    }
    if (boundaryType[BND_RIGHT] == CONNECT && !comm.isLocal(BND_RIGHT)) {
        int startIndex = nx * (ny + 2) + 1;
        comm.set(BND_RIGHT, copyLayerStruct<std::vector<float>>{ny, std::vector<float>(b.getRawPointer() + startIndex,
                                                                                       b.getRawPointer() + startIndex +
                                                                                       ny)});

    }
    if (boundaryType[BND_BOTTOM] == CONNECT && !comm.isLocal(BND_BOTTOM)) {

        std::vector<float> send_bat;
        for (int i = 1; i < nx + 1; i++) {
            send_bat.push_back(b[i][1]);
        }

        comm.set(BND_BOTTOM, copyLayerStruct<std::vector<float>>{nx, send_bat});
    }
    if (boundaryType[BND_TOP] == CONNECT && !comm.isLocal(BND_TOP)) {

        std::vector<float> send_bat;
        for (int i = 1; i < nx + 1; i++) {
            send_bat.push_back(b[i][ny]);
        }
        comm.set(BND_TOP, copyLayerStruct<std::vector<float>>{nx, send_bat});

    }

    /***********
     * RECEIVE *
     **********/
    std::vector<hpx::future<void>> fut;

    if (boundaryType[BND_LEFT] == CONNECT) {
        fut.push_back(comm.get(BND_LEFT, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep, true));


    }

    if (boundaryType[BND_RIGHT] == CONNECT) {

        fut.push_back(comm.get(BND_RIGHT, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep, true));

    }

    if (boundaryType[BND_BOTTOM] == CONNECT) {
        fut.push_back(comm.get(BND_BOTTOM, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep, true));

    }

    if (boundaryType[BND_TOP] == CONNECT) {
        fut.push_back(comm.get(BND_TOP, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep, true));
    }


    hpx::wait_all(fut);

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
    collector.startCounter(Collector::CTR_EXCHANGE);
    if (boundaryType[BND_LEFT] == CONNECT && !comm.isLocal(BND_LEFT) && isSendable(BND_LEFT)) {

        int startIndex = ny + 2 + 1;

        comm.set(BND_LEFT, copyLayerStruct<std::vector<float>>{ny, {},
                                                               std::vector<float>(h.getRawPointer() + startIndex,
                                                                                  h.getRawPointer() + startIndex + ny),
                                                               std::vector<float>(hu.getRawPointer() + startIndex,
                                                                                  hu.getRawPointer() + startIndex + ny),
                                                               std::vector<float>(hv.getRawPointer() + startIndex,
                                                                                  hv.getRawPointer() + startIndex + ny),
                                                               getTotalLocalTimestep()});


    }
    if (boundaryType[BND_RIGHT] == CONNECT && !comm.isLocal(BND_RIGHT) && isSendable(BND_RIGHT)) {
        int startIndex = nx * (ny + 2) + 1;
        comm.set(BND_RIGHT, copyLayerStruct<std::vector<float>>{ny, {},
                                                                std::vector<float>(h.getRawPointer() + startIndex,
                                                                                   h.getRawPointer() + startIndex + ny),
                                                                std::vector<float>(hu.getRawPointer() + startIndex,
                                                                                   hu.getRawPointer() + startIndex +
                                                                                   ny),
                                                                std::vector<float>(hv.getRawPointer() + startIndex,
                                                                                   hv.getRawPointer() + startIndex +
                                                                                   ny),
                                                                getTotalLocalTimestep()});
    }
    if (boundaryType[BND_BOTTOM] == CONNECT && !comm.isLocal(BND_BOTTOM) && isSendable(BND_BOTTOM)) {

        std::vector<float> send_h;
        std::vector<float> send_hu;
        std::vector<float> send_hv;
        send_h.reserve(nx);
        send_hu.reserve(nx);
        send_hv.reserve(nx);
        for (int i = 1; i < nx + 1; i++) {
            send_h.push_back(h[i][1]);
            send_hu.push_back(hu[i][1]);
            send_hv.push_back(hv[i][1]);
        }

        comm.set(BND_BOTTOM,
                 copyLayerStruct<std::vector<float>>{nx, {}, send_h, send_hu, send_hv, getTotalLocalTimestep()});
    }
    if (boundaryType[BND_TOP] == CONNECT && !comm.isLocal(BND_TOP) && isSendable(BND_TOP)) {

        std::vector<float> send_h;
        std::vector<float> send_hu;
        std::vector<float> send_hv;
        send_h.reserve(nx);
        send_hu.reserve(nx);
        send_hv.reserve(nx);
        for (int i = 1; i < nx + 1; i++) {
            send_h.push_back(h[i][ny]);
            send_hu.push_back(hu[i][ny]);
            send_hv.push_back(hv[i][ny]);
        }

        comm.set(BND_TOP,
                 copyLayerStruct<std::vector<float>>{nx, {}, send_h, send_hu, send_hv, getTotalLocalTimestep()});

    }


    /***********
     * RECEIVE *
     **********/

    std::vector<hpx::future<void>> fut;
    if (boundaryType[BND_LEFT] == CONNECT && isReceivable(BND_LEFT)) {
        fut.push_back(comm.get(BND_LEFT, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep));
    }
    if (boundaryType[BND_RIGHT] == CONNECT && isReceivable(BND_RIGHT)) {
        fut.push_back(comm.get(BND_RIGHT, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep));
    }

    if (boundaryType[BND_BOTTOM] == CONNECT && isReceivable(BND_BOTTOM)) {
        fut.push_back(comm.get(BND_BOTTOM, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep));
    }

    if (boundaryType[BND_TOP] == CONNECT && isReceivable(BND_TOP)) {
        fut.push_back(comm.get(BND_TOP, nx, ny, &bufferH, &bufferHu, &bufferHv, &b, borderTimestep));
    }


    auto ret = hpx::dataflow(hpx::util::unwrapping([this]() -> void {
        checkAllGhostlayers();
        collector.stopCounter(Collector::CTR_EXCHANGE);
    }), std::move(fut));


    return ret;
}

void SWE_DimensionalSplittingHpx::computeNumericalFluxes() {
    computeXSweep();
    computeYSweep();
}

void SWE_DimensionalSplittingHpx::computeXSweep() {

    if (!allGhostlayersInSync()) return;


    //maximum (linearized) wave speed within one iteration
    float maxHorizontalWaveSpeed = (float) 0.;



    // x-sweep, compute the actual domain plus ghost rows above and below
    // iterate over cells on the x-axis, leave out the last column (two cells per computation)


/*std::vector<int> test;
test.reserve(nx+1);
for(int i = 0; i < nx+1 ; i++)test.push_back(i);
*/
/*
    hpx::parallel::for_loop(hpx::parallel::execution::par,
                                                      0,nx+1,

                                                       [this ,&maxHorizontalWaveSpeed](int x)
                                                       {
                                                            float localWaveSpeed;
                                                           for (int y = 0; y < ny+2; y++) {
                                                               solver.computeNetUpdates (
                                                                       h[x][y], h[x + 1][y],
                                                                       hu[x][y], hu[x + 1][y],
                                                                       b[x][y], b[x + 1][y],
                                                                       hNetUpdatesLeft[x][y], hNetUpdatesRight[x + 1][y],
                                                                       huNetUpdatesLeft[x][y], huNetUpdatesRight[x + 1][y],
                                                                       localWaveSpeed
                                                               );
                                                           }
                                                            if(x == nx) maxHorizontalWaveSpeed =localWaveSpeed;
                                                       }
                                                               );*/
    //   std::cout<< maxHorizontalWaveSpeed << std::endl;
    //  maxHorizontalWaveSpeed= *std::min_element(wave.begin(), wave.end());


    for (int x = 0; x < nx + 1; x++) {
        const int ny_end = ny + 2;
        // iterate over all rows, including ghost layer

        for (int y = 0; y < ny_end; y++) {
            solver.computeNetUpdates(
                    h[x][y], h[x + 1][y],
                    hu[x][y], hu[x + 1][y],
                    b[x][y], b[x + 1][y],
                    hNetUpdatesLeft[x][y], hNetUpdatesRight[x + 1][y],
                    huNetUpdatesLeft[x][y], huNetUpdatesRight[x + 1][y],
                    maxHorizontalWaveSpeed
            );
        }
    }


    collector.addFlops(nx * ny * 135);


    // compute max timestep according to cautious CFL-condition
    maxTimestep = (float) .4 * (dx / maxHorizontalWaveSpeed);
    if (localTimestepping) {
        maxTimestep = getRoundTimestep(maxTimestep);

    }
    maxTimestepGlobal = maxTimestep;


}

void SWE_DimensionalSplittingHpx::computeYSweep() {
    if (!allGhostlayersInSync()) return;

  if(!localTimestepping){
        maxTimestep = maxTimestepGlobal;
    }
    float maxVerticalWaveSpeed = (float) 0.;



    // set intermediary Q* states

    for (int x = 1; x < nx + 1; x++) {
        for (int y = 0; y < ny + 2; y++) {
            hStar[x][y] = h[x][y] - (maxTimestep / dx) * (hNetUpdatesLeft[x][y] + hNetUpdatesRight[x][y]);
            huStar[x][y] = hu[x][y] - (maxTimestep / dx) * (huNetUpdatesLeft[x][y] + huNetUpdatesRight[x][y]);
        }
    }
/*
    hpx::parallel::for_loop(hpx::parallel::execution::par,
                            0,nx+1,

                            [this ](int x)
                            {
                                float localWaveSpeed;
                                for (int y = 0; y < ny+1; y++) {
                                    solver.computeNetUpdates (
                                            h[x][y], h[x][y + 1],
                                            hv[x][y], hv[x][y + 1],
                                            b[x][y], b[x][y + 1],
                                            hNetUpdatesBelow[x][y], hNetUpdatesAbove[x][y + 1],
                                            hvNetUpdatesBelow[x][y], hvNetUpdatesAbove[x][y + 1],
                                           localWaveSpeed
                                    );
                                }

                            }
    );
  */
    for (int x = 1; x < nx + 1; x++) {
        const int ny_end = ny + 1;
        // iterate over all rows, including ghost layer
        for (int y = 0; y < ny_end; y++) {
            solver.computeNetUpdates(
                    h[x][y], h[x][y + 1],
                    hv[x][y], hv[x][y + 1],
                    b[x][y], b[x][y + 1],
                    hNetUpdatesBelow[x][y], hNetUpdatesAbove[x][y + 1],
                    hvNetUpdatesBelow[x][y], hvNetUpdatesAbove[x][y + 1],
                    maxVerticalWaveSpeed
            );
        }
    }
    collector.addFlops(nx * ny * 135);


}


/**
 * Updates the unknowns with the already computed net-updates.
 *
 * @param dt time step width used in the update. The timestep has to be equal to maxTimestep calculated by computeNumericalFluxes(),
 * since this is the step width used for the intermediary updates after the x-sweep.
 */
void SWE_DimensionalSplittingHpx::updateUnknowns(float dt) {
    if (!allGhostlayersInSync()) return;

    // this assertion has to hold since the intermediary star states were calculated internally using a timestep width of maxTimestep
    assert(std::abs(dt - maxTimestep) < 0.00001);
    //update cell averages with the net-updates
    for (int x = 1; x < nx + 1; x++) {
        for (int y = 1; y < ny + 1; y++) {
            h[x][y] = hStar[x][y] - (maxTimestep / dx) * (hNetUpdatesBelow[x][y] + hNetUpdatesAbove[x][y]);
            hu[x][y] = huStar[x][y];
            hv[x][y] = hv[x][y] - (maxTimestep / dx) * (hvNetUpdatesBelow[x][y] + hvNetUpdatesAbove[x][y]);
        }
    }

}
