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
    if (!allGhostlayersInSync()) return;
//maximum (linearized) wave speed within one iteration
    float maxWaveSpeed = (float) 0.;

    /***************************************************************************************
     * compute the net-updates for the vertical edges
     **************************************************************************************/

    for (int i = 1; i < nx+2; i++) {
        const int ny_end = ny+1;

#if defined(VECTORIZE)

            // iterate over all rows, including ghost layer
#pragma omp simd reduction(max:maxWaveSpeed)
#endif // VECTORIZE
        for (int j=1; j < ny_end; ++j) {


            solver.computeNetUpdates (
                    h[i - 1][j], h[i][j],
                    hu[i - 1][j], hu[i][j],
                    b[i - 1][j], b[i][j],
                    hNetUpdatesLeft[i - 1][j - 1], hNetUpdatesRight[i - 1][j - 1],
                    huNetUpdatesLeft[i - 1][j - 1], huNetUpdatesRight[i - 1][j - 1],
                    maxEdgeSpeed
            );
            maxWaveSpeed = std::max(maxWaveSpeed, maxEdgeSpeed);
        }

    }


    /***************************************************************************************
     * compute the net-updates for the horizontal edges
     **************************************************************************************/

    for (int i=1; i < nx + 1; i++) {
        const int ny_end = ny+2;
        float maxEdgeSpeed = 0;
#if defined(VECTORIZE)

        // iterate over all rows, including ghost layer
#pragma omp simd //reduction(max:maxEdgeSpeed) //lastprivate(maxEdgeSpeed)
#endif // VECTORIZE
        for (int j=1; j < ny_end; ++j) {

            solver.computeNetUpdates (
                    h[i][j - 1], h[i][j],
                    hv[i][j - 1], hv[i][j],
                    b[i][j - 1], b[i][j],
                    hNetUpdatesBelow[i - 1][j - 1], hNetUpdatesAbove[i - 1][j - 1],
                    hvNetUpdatesBelow[i - 1][j - 1], hvNetUpdatesAbove[i - 1][j - 1],
                    maxEdgeSpeed
            );

            //update the maximum wave speed
           //maxWaveSpeed = std::max (maxWaveSpeed, maxEdgeSpeed);
           //maxTestSpeed = std::max (maxTestSpeed, maxEdgeSpeed);

        }
    }

    if (maxWaveSpeed > 0.00001) {

        maxTimestep = std::min (dx / maxWaveSpeed, dy / maxWaveSpeed);

        maxTimestep *= (float) .4; //CFL-number = .5
    } else {
        //might happen in dry cells
        maxTimestep = std::numeric_limits<float>::max ();
    }

    collector.addFlops(2*nx * ny * 135);



    if (localTimestepping) {
        maxTimestep = getRoundTimestep(maxTimestep);
    }

    collector.addTimestep(maxTimestep);


}



/**
 * Updates the unknowns with the already computed net-updates.
 *
 * @param dt time step width used in the update. The timestep has to be equal to maxTimestep calculated by computeNumericalFluxes(),
 * since this is the step width used for the intermediary updates after the x-sweep.
 */
void SWE_DimensionalSplittingHpx::updateUnknowns(float dt) {
    if (!allGhostlayersInSync()) return;
//update cell averages with the net-updates
    dt=maxTimestep;
    for (int i = 1; i < nx+1; i++) {
        const int ny_end = ny+1;

#if defined(VECTORIZE)

        // iterate over all rows, including ghost layer
#pragma omp simd
#endif // VECTORIZE

        for (int j = 1; j < ny_end; j++) {
            h[i][j] -= dt / dx * (hNetUpdatesRight[i - 1][j - 1] + hNetUpdatesLeft[i][j - 1]) + dt / dy * (hNetUpdatesAbove[i - 1][j - 1] + hNetUpdatesBelow[i - 1][j]);
            hu[i][j] -= dt / dx * (huNetUpdatesRight[i - 1][j - 1] + huNetUpdatesLeft[i][j - 1]);
            hv[i][j] -= dt / dy * (hvNetUpdatesAbove[i - 1][j - 1] + hvNetUpdatesBelow[i - 1][j]);

            if (h[i][j] < 0) {
                //TODO: dryTol
#ifndef NDEBUG
                // Only print this warning when debug is enabled
				// Otherwise we cannot vectorize this loop
				if (h[i][j] < -0.1) {
					std::cerr << "Warning, negative height: (i,j)=(" << i << "," << j << ")=" << h[i][j] << std::endl;
					std::cerr << "         b: " << b[i][j] << std::endl;
				}
#endif // NDEBUG
                //zero (small) negative depths
                h[i][j] = hu[i][j] = hv[i][j] = 0.;
            } else if (h[i][j] < 0.1)
                hu[i][j] = hv[i][j] = 0.; //no water, no speed!
        }
    }

}
