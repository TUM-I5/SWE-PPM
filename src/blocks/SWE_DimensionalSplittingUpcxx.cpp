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
#include "SWE_DimensionalSplittingUpcxx.hh"

#include <cassert>
#include <algorithm>
#include <omp.h>
#include <unistd.h>

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
SWE_DimensionalSplittingUpcxx::SWE_DimensionalSplittingUpcxx(int nx, int ny, float dx, float dy, float originX, float originY,bool localTimestepping) :
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
		hvNetUpdatesAbove(nx + 1, ny + 2) {

	computeTime = 0.;
	computeTimeWall = 0.;
	flopCounter = 0;
	communicationTime = 0;
	reductionTime = 0;

    upcxxLocalTimestep = upcxx::new_array<float>(4);
    upcxxBorderTimestep = upcxxLocalTimestep.local();
    upcxxDataReady= upcxx::new_array<std::atomic<bool>>(4);
    dataReady = upcxxDataReady.local();
    upcxxDataTransmitted= upcxx::new_array<std::atomic<bool>>(4);
    dataTransmitted = upcxxDataTransmitted.local();
    upcxxIteration = upcxx::new_<int>(0);

    for (int i = 0; i < 4; i++) {
        dataReady[i] = false;
        dataTransmitted[i] = false;
    }
}

void SWE_DimensionalSplittingUpcxx::connectBoundaries(BlockConnectInterface<upcxx::global_ptr<float>> p_neighbourCopyLayer[]) {
	for (int i = 0; i < 4; i++) {
		neighbourCopyLayer[i] = p_neighbourCopyLayer[i];
	}
}

/**
 * register the row or column layer next to a boundary as a "copy layer",
 * from which values will be copied into the ghost layer or a neighbour;
 * @return	a BlockConnectInterface object that contains row variables h, hu, and hv
 */
BlockConnectInterface<upcxx::global_ptr<float>> SWE_DimensionalSplittingUpcxx::getCopyLayer(Boundary boundary) {
	struct BlockConnectInterface<upcxx::global_ptr<float>> interface;
	interface.boundary = boundary;
	interface.pointerH = bufferH.getPointer();
	interface.pointerB = b.getPointer();
	interface.pointerHu = bufferHu.getPointer();
	interface.pointerHv = bufferHv.getPointer();
	interface.pointerTimestep = upcxxLocalTimestep+boundary;
    interface.dataReady = upcxxDataReady+boundary;
    interface.dataTransmitted = upcxxDataTransmitted+boundary;
    interface.rank =  upcxx::rank_me();
    interface.iteration = upcxxIteration;
 	switch (boundary) {

        case BND_LEFT:
            interface.size = ny;
            interface.stride = 1;
            interface.startIndex = (nx + 1) * (ny + 2) + 1;
            break;
        case BND_RIGHT:
            interface.size = ny;
            interface.stride = 1;
            interface.startIndex =  1;
            break;
        case BND_BOTTOM:
            interface.size = nx;
            interface.stride = ny + 2;
            interface.startIndex = ny + 2;
            break;
        case BND_TOP:
            interface.size = nx;
            interface.stride = ny + 2;
            interface.startIndex = ny + 2 + ny + 1;
            break;
	};

	return interface;
}

void SWE_DimensionalSplittingUpcxx::exchangeBathymetry() {
	if (boundaryType[BND_LEFT] == CONNECT) {
		assert(neighbourCopyLayer[BND_LEFT].size == ny);
		BlockConnectInterface<upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_LEFT];

        int startIndex = ny + 2 + 1;
        upcxx::rput(b.getRawPointer()+startIndex,iface.pointerB + iface.startIndex ,ny).wait();
	}
	if (boundaryType[BND_RIGHT] == CONNECT) {
		assert(neighbourCopyLayer[BND_RIGHT].size == ny);
		BlockConnectInterface<upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_RIGHT];

        int startIndex = nx * (ny + 2) + 1;

        upcxx::rput(b.getRawPointer()+startIndex,iface.pointerB + iface.startIndex,ny).wait();
	}
	if (boundaryType[BND_BOTTOM] == CONNECT) {

		assert(neighbourCopyLayer[BND_BOTTOM].size == nx);
		BlockConnectInterface<upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_BOTTOM];

        uint thisStride = static_cast<uint>(sizeof(float) * (ny + 2));
        upcxx::rput_strided<2>(
                &b[1][1], {{sizeof(float), thisStride}},
                iface.pointerB + iface.startIndex,
                {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                {{1, (size_t) nx}}
        ).wait();

	}
	if (boundaryType[BND_TOP] == CONNECT) {

		assert(neighbourCopyLayer[BND_TOP].size == nx);
		BlockConnectInterface<upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_TOP];

        uint thisStride = static_cast<uint>(sizeof(float) * (ny + 2));

        upcxx::rput_strided<2>(
                &b[1][ny], {{sizeof(float), thisStride}},
                iface.pointerB + iface.startIndex,
                {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                {{1, (size_t) nx}}
        ).wait();
	}
}
void SWE_DimensionalSplittingUpcxx::notifyNeighbours(bool sync){
    for(int i = 0; i < 4 ; i++){

        if(boundaryType[i] == CONNECT ){
            rpc_ff(neighbourCopyLayer[i].rank,
                   [](upcxx::global_ptr<std::atomic<bool>> dataFlag){ dataFlag.local()[0] = true;},
                   sync?
                   neighbourCopyLayer[i].dataReady:
                   neighbourCopyLayer[i].dataTransmitted);
        }
    }


    std::atomic<bool>* flag;
    if(sync){
        flag = dataReady;

    } else {
        flag = dataTransmitted;
    }

        while(!flag[BND_LEFT] ||
               !flag[BND_RIGHT] ||
               !flag[BND_TOP] ||
               !flag[BND_BOTTOM]) {
            for(int i = 0; i < 4 ; i++){

                if(boundaryType[i] != CONNECT  || (!isSendable((Boundary)i))){

                    flag[i] = true; //only set true the ones who are either not sending anymore or not connected.
                }
            }
                upcxx::progress();

        }

}
/*
 * The UPCXX version of setGhostLayer() will use the BlockConnectInterfaces of its neighbours
 * to receive the ghost layers at CONNECT boundaries.
 * The function will use rget/rget_strided and will block until all values have been received
 * and processed.
 */
void SWE_DimensionalSplittingUpcxx::setGhostLayer() {
	// Apply appropriate conditions for OUTFLOW/WALL boundaries
	SWE_Block::applyBoundaryConditions();

	upcxx::future<> leftFuture = upcxx::make_future<>();
	upcxx::future<> rightFuture = upcxx::make_future<>();
	upcxx::future<> bottomFuture = upcxx::make_future<>();
	upcxx::future<> topFuture = upcxx::make_future<>();
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    notifyNeighbours(true);

    for(int i = 0; i < 4; i++){
        dataReady[i] = false;
    }

    float totalLocalTimestep = getTotalLocalTimestep();


    if (boundaryType[BND_LEFT] == CONNECT && isSendable(BND_LEFT) ) {

            assert(neighbourCopyLayer[BND_LEFT].size == ny);
            assert(neighbourCopyLayer[BND_LEFT].stride == 1);

            BlockConnectInterface <upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_LEFT];

            upcxx::global_ptr<float> srcBaseH = iface.pointerH + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHu = iface.pointerHu + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHv = iface.pointerHv + iface.startIndex;


            int startIndex = ny + 2 + 1;

            auto leftFutH = upcxx::rput(h.getRawPointer()+startIndex,srcBaseH,ny);
            auto leftFutHu = upcxx::rput(hu.getRawPointer()+startIndex,srcBaseHu,ny);
            auto leftFutHv = upcxx::rput(hv.getRawPointer()+startIndex,srcBaseHv,ny);
            auto leftFutTs = upcxx::rput( &totalLocalTimestep,iface.pointerTimestep,1);
            leftFuture = upcxx::when_all(leftFutH, leftFutHu, leftFutHv, leftFutTs);

    }


    if (boundaryType[BND_RIGHT] == CONNECT && isSendable(BND_RIGHT) ) {

            assert(neighbourCopyLayer[BND_RIGHT].size == ny);
            assert(neighbourCopyLayer[BND_RIGHT].stride == 1);

            BlockConnectInterface <upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_RIGHT];

            upcxx::global_ptr<float> srcBaseH = iface.pointerH + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHu = iface.pointerHu + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHv = iface.pointerHv + iface.startIndex;

            int startIndex = nx * (ny + 2) + 1;

            auto rightFutH = upcxx::rput(h.getRawPointer()+startIndex,srcBaseH,ny);
            auto rightFutHu = upcxx::rput(hu.getRawPointer()+startIndex,srcBaseHu,ny);
            auto rightFutHv = upcxx::rput(hv.getRawPointer()+startIndex,srcBaseHv,ny);
            auto rightFutTs = upcxx::rput( &totalLocalTimestep,iface.pointerTimestep,1);

            rightFuture = upcxx::when_all(rightFutH, rightFutHu, rightFutHv, rightFutTs);

    }
    if (boundaryType[BND_BOTTOM] == CONNECT && isSendable(BND_BOTTOM)) {

            assert(neighbourCopyLayer[BND_BOTTOM].size == nx);

            BlockConnectInterface <upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_BOTTOM];

            upcxx::global_ptr<float> srcBaseH = iface.pointerH + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHu = iface.pointerHu + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHv = iface.pointerHv + iface.startIndex;
            uint thisStride = static_cast<uint>(sizeof(float) * (ny + 2));
            auto bottomFutH = upcxx::rput_strided<2>(
                    &h[1][1], {{sizeof(float), thisStride}},
                    srcBaseH,
                    {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                    {{1, (size_t) nx}}
                    );
            auto bottomFutHu = upcxx::rput_strided<2>(
                    &hu[1][1], {{sizeof(float), thisStride}},
                    srcBaseHu,
                    {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                    {{1, (size_t) nx}}
            );
            auto bottomFutHv = upcxx::rput_strided<2>(
                    &hv[1][1], {{sizeof(float), thisStride}},
                    srcBaseHv,
                    {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                    {{1, (size_t) nx}}
            );

            auto bottomFutTs = upcxx::rput( &totalLocalTimestep,iface.pointerTimestep,1);

            bottomFuture = upcxx::when_all(bottomFutH, bottomFutHu, bottomFutHv,  bottomFutTs);


    }
    if (boundaryType[BND_TOP] == CONNECT && isSendable(BND_TOP)) {

            assert(neighbourCopyLayer[BND_TOP].size == nx);
            BlockConnectInterface <upcxx::global_ptr<float>> iface = neighbourCopyLayer[BND_TOP];
            upcxx::global_ptr<float> srcBaseH = iface.pointerH + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHu = iface.pointerHu + iface.startIndex;
            upcxx::global_ptr<float> srcBaseHv = iface.pointerHv + iface.startIndex;
            uint thisStride = static_cast<uint>(sizeof(float) * (ny + 2));

            auto topFutH = upcxx::rput_strided<2>(
                    &h[1][ny], {{sizeof(float), thisStride}},
                    srcBaseH,
                    {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                    {{1, (size_t) nx}}
            );
            auto topFutHu = upcxx::rput_strided<2>(
                    &hu[1][ny], {{sizeof(float), thisStride}},
                    srcBaseHu,
                    {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                    {{1, (size_t) nx}}
            );
            auto topFutHv = upcxx::rput_strided<2>(
                    &hv[1][ny], {{sizeof(float), thisStride}},
                    srcBaseHv,
                    {{sizeof(float), static_cast<uint>(sizeof(float) *iface.stride)}},
                    {{1, (size_t) nx}}
            );

            auto topFutTs = upcxx::rput( &totalLocalTimestep,iface.pointerTimestep,1);
            topFuture = upcxx::when_all(topFutH, topFutHu, topFutHv, topFutTs);

    }

    upcxx::when_all(leftFuture, rightFuture, bottomFuture, topFuture).wait();
    notifyNeighbours(false);

    for(int i = 0; i < 4; i++){
        if(isReceivable((Boundary)i))
        borderTimestep[i]=upcxxBorderTimestep[i];
        dataTransmitted[i] = false;
    }
    checkAllGhostlayers();

    clock_gettime(CLOCK_MONOTONIC, &endTime);
    communicationTime += (endTime.tv_sec - startTime.tv_sec);
    communicationTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
    iteration++;
}
/**
 * Compute net updates for the block.
 * The member variable #maxTimestep will be updated with the
 * maximum allowed time step size
 */
void SWE_DimensionalSplittingUpcxx::computeNumericalFluxes () {
    if(!allGhostlayersInSync()) return;
	// Start compute clocks
	computeClock = clock();
	clock_gettime(CLOCK_MONOTONIC, &startTime);

	//maximum (linearized) wave speed within one iteration
	float maxHorizontalWaveSpeed = (float) 0.;
	float maxVerticalWaveSpeed = (float) 0.;

	#pragma omp parallel private(solver)
	{
		// x-sweep, compute the actual domain plus ghost rows above and below
		// iterate over cells on the x-axis, leave out the last column (two cells per computation)
		#pragma omp for reduction(max : maxHorizontalWaveSpeed) collapse(2)
		for (int x = 0; x < nx + 1; x++) {
            #if defined(VECTORIZE)
			const int ny_end = ny+2;
		    // iterate over all rows, including ghost layer
                #pragma omp simd reduction(max:maxHorizontalWaveSpeed)
            #endif // VECTORIZE
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
		}
	}

    flopCounter += nx*ny*135;
	// Accumulate compute time -> exclude the reduction
	computeClock = clock() - computeClock;
	computeTime += (float) computeClock / CLOCKS_PER_SEC;

	clock_gettime(CLOCK_MONOTONIC, &endTime);
	computeTimeWall += (endTime.tv_sec - startTime.tv_sec);
	computeTimeWall += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
    maxTimestep = (float) .4 * (dx / maxHorizontalWaveSpeed);
    if(localTimestepping){

        maxTimestep = getRoundTimestep(maxTimestep);

    }else {
        // compute max timestep according to cautious CFL-condition

        clock_gettime(CLOCK_MONOTONIC, &startTime);
        maxTimestepGlobal = upcxx::reduce_all(maxTimestep, [](float a, float b) { return std::min(a, b); }).wait();
        clock_gettime(CLOCK_MONOTONIC, &endTime);
        reductionTime += (endTime.tv_sec - startTime.tv_sec);
        reductionTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
        maxTimestep = maxTimestepGlobal;
    }
	//upcxx::barrier();

	// restart compute clocks
	computeClock = clock();
	clock_gettime(CLOCK_MONOTONIC, &startTime);

	#pragma omp parallel private(solver)
	{
		// set intermediary Q* states
		#pragma omp for collapse(2)
		for (int x = 1; x < nx + 1; x++) {
			for (int y = 0; y < ny + 2; y++) {
				hStar[x][y] = h[x][y] - (maxTimestep / dx) * (hNetUpdatesLeft[x][y] + hNetUpdatesRight[x][y]);
				huStar[x][y] = hu[x][y] - (maxTimestep / dx) * (huNetUpdatesLeft[x][y] + huNetUpdatesRight[x][y]);
			}
		}

		// y-sweep
		#ifndef NDEBUG
		#pragma omp for
		#else
		#pragma omp for reduction(max : maxVerticalWaveSpeed) collapse(2)
		#endif
		for (int x = 1; x < nx + 1; x++) {
#if defined(VECTORIZE)
            const int ny_end = ny+1;
            // iterate over all rows, including ghost layer
#pragma omp simd reduction(max:maxVerticalWaveSpeed)
#endif // VECTORIZE
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
		}

		#ifndef NDEBUG
		#pragma omp single
		{
			// check if the cfl condition holds in the y-direction
		//	assert(maxTimestep < (float) .5 * (dy / maxVerticalWaveSpeed));
		}
		#endif // NDEBUG
	}
    flopCounter += nx*ny*135;
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
void SWE_DimensionalSplittingUpcxx::updateUnknowns (float dt) {
    if(!allGhostlayersInSync()) return;
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
float SWE_DimensionalSplittingUpcxx::getFlops(){
    return flopCounter;
}
