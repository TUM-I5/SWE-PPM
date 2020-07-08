#include "SWE_DimensionalSplittingCharm.hh"

#include "examples/swe_charm.decl.h"

SWE_DimensionalSplittingCharm::SWE_DimensionalSplittingCharm(CkMigrateMessage *msg) {}

SWE_DimensionalSplittingCharm::SWE_DimensionalSplittingCharm(int nx, int ny, float dx, float dy, float originX,
                                                             float originY, int posX, int posY,
                                                             BoundaryType boundaries[], std::string outputFilename,
                                                             std::string bathymetryFilename,
                                                             std::string displacementFilename, bool localTimestepping,bool write) :
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
        write(write),
        outputFilename(outputFilename),
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
           // myRank= rank;
    usesAtSync = true;
    currentSimulationTime = 0.;
    currentCheckpoint = 0;

    neighbourIndex[BND_LEFT] = (posX > 0) ? thisIndex - blockCountY : -1;
    neighbourIndex[BND_RIGHT] = (posX < blockCountX - 1) ? thisIndex + blockCountY : -1;
    neighbourIndex[BND_BOTTOM] = (posY > 0) ? thisIndex - 1 : -1;
    neighbourIndex[BND_TOP] = (posY < blockCountY - 1) ? thisIndex + 1 : -1;

    // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
    checkpointInstantOfTime = new float[checkpointCount];
    // Time delta is the time between any two checkpoints
    float checkpointTimeDelta = simulationDuration / checkpointCount;
    // The first checkpoint is reached after 0 + delta t
    checkpointInstantOfTime[0] = checkpointTimeDelta;
    for (int i = 1; i < checkpointCount; i++) {
        checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
    }

#ifdef ASAGI
    SWE_AsagiScenario scenario(bathymetryFilename, displacementFilename);
#else
    //SWE_RadialDamBreakScenario scenario = SWE_RadialDamBreakScenario();
    SWE_RadialBathymetryDamBreakScenario scenario = SWE_RadialBathymetryDamBreakScenario();
#endif
    initScenario(scenario, boundaries);
    setDuration(simulationDuration);
    if(write){

        // Initialize writer
        BoundarySize boundarySize = {{1, 1, 1, 1}};
        writer = new NetCdfWriter(outputFilename, b, boundarySize, nx, ny, dx, dy, originX, originY);

    }
    collector = new CollectorCharm();
    // output at t=0
    writeTimestep();

    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);

    CkPrintf("%i Spawned at %s\n", thisIndex, hostname);
    firstIteration = true;
}

SWE_DimensionalSplittingCharm::~SWE_DimensionalSplittingCharm() {
        delete []checkpointInstantOfTime;
        delete collector;

        if(write)
        delete writer;
        }
void SWE_DimensionalSplittingCharm::ResumeFromSync() {
    CkPrintf("I'm chare %d, I moved to PE %d \n", thisIndex, CkMyPe());
    //compute();
    thisProxy[thisIndex].compute();
}

void SWE_DimensionalSplittingCharm::xSweep() {
    if (!allGhostlayersInSync()) return;
    if(migrated)CkPrintf("%d: entered xSweep()\n",thisIndex);
//maximum (linearized) wave speed within one iteration
    float maxWaveSpeed = (float) 0.;
    float maxEdgeSpeed = 0;
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

#if defined(VECTORIZE)

        // iterate over all rows, including ghost layer
#pragma omp simd reduction(max:maxWaveSpeed) //
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
            maxWaveSpeed = std::max (maxWaveSpeed, maxEdgeSpeed);
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
    if(migrated)CkPrintf("%d: left xSweep() %f %p\n",thisIndex, maxTimestep, &maxTimestep);
    collector->addFlops(2*nx * ny * 135);

    // compute max timestep according to cautious CFL-condition
    if (localTimestepping) {
        maxTimestep = getRoundTimestep(maxTimestep);
    } else {

        collector->startCounter(Collector::CTR_REDUCE);
        CkCallback
        cb(CkReductionTarget(SWE_DimensionalSplittingCharm, reduceWaveSpeed), thisProxy);
        contribute(sizeof(float), &maxTimestep, CkReduction::min_float, cb);
    }

}

void SWE_DimensionalSplittingCharm::reduceWaveSpeed(float maxWaveSpeed) {
    maxTimestep = maxWaveSpeed;

    reductionTrigger();
if(!localTimestepping)
    collector->stopCounter(Collector::CTR_REDUCE);
}

void SWE_DimensionalSplittingCharm::ySweep() {

}

void SWE_DimensionalSplittingCharm::updateUnknowns(float dt) {
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

void SWE_DimensionalSplittingCharm::processCopyLayer(copyLayer *msg) {

    // LEFT ghost layer consists of values from the left neighbours RIGHT copy layer etc.

    if (!msg->isDummy) {
        if (msg->boundary == BND_RIGHT && boundaryType[BND_LEFT] == CONNECT && isReceivable(BND_LEFT)) {

            for (int i = 0; i < ny; i++) {
                if (msg->containsBathymetry)
                    b[0][i + 1] = msg->b[i];
                bufferH[0][i + 1] = msg->h[i];
                bufferHu[0][i + 1] = msg->hu[i];
                bufferHv[0][i + 1] = msg->hv[i];
            }
            borderTimestep[BND_LEFT] = msg->timestep;
        } else if (msg->boundary == BND_LEFT && boundaryType[BND_RIGHT] == CONNECT && isReceivable(BND_RIGHT)) {
            for (int i = 0; i < ny; i++) {
                if (msg->containsBathymetry)
                    b[nx + 1][i + 1] = msg->b[i];
                bufferH[nx + 1][i + 1] = msg->h[i];
                bufferHu[nx + 1][i + 1] = msg->hu[i];
                bufferHv[nx + 1][i + 1] = msg->hv[i];
            }
            borderTimestep[BND_RIGHT] = msg->timestep;
        } else if (msg->boundary == BND_TOP && boundaryType[BND_BOTTOM] == CONNECT && isReceivable(BND_BOTTOM)) {
            for (int i = 0; i < nx; i++) {
                if (msg->containsBathymetry)
                    b[i + 1][0] = msg->b[i];
                bufferH[i + 1][0] = msg->h[i];
                bufferHu[i + 1][0] = msg->hu[i];
                bufferHv[i + 1][0] = msg->hv[i];
            }
            borderTimestep[BND_BOTTOM] = msg->timestep;
        } else if (msg->boundary == BND_BOTTOM && boundaryType[BND_TOP] == CONNECT && isReceivable(BND_TOP)) {
            for (int i = 0; i < nx; i++) {
                if (msg->containsBathymetry)
                    b[i + 1][ny + 1] = msg->b[i];
                bufferH[i + 1][ny + 1] = msg->h[i];
                bufferHu[i + 1][ny + 1] = msg->hu[i];
                bufferHv[i + 1][ny + 1] = msg->hv[i];
            }
            borderTimestep[BND_TOP] = msg->timestep;
        }
    }
    delete msg;
    // Deallocate the message buffer
}

void SWE_DimensionalSplittingCharm::sendCopyLayers(bool sendBathymetry) {


// The array sizes for copy layers of either orientation, set bathymetry array to length zero
    int sizesVertical[] = {0, ny, ny, ny};
    int sizesHorizontal[] = {0, nx, nx, nx};

    // If we are sending bathymetry too, change the bathymetry array sizes from 0 to the respective value
    if (sendBathymetry) {
        sizesVertical[0] = ny;
        sizesHorizontal[0] = nx;
    }

    int size, stride, startIndex, endIndex;
    float totalLocalTimestep = getTotalLocalTimestep();
    if (boundaryType[BND_LEFT] == CONNECT && isSendable(BND_LEFT)) {

        assert(neighbourIndex[BND_LEFT] > -1);

        copyLayer *left = new(sizesVertical, 0) copyLayer();
        left->containsBathymetry = sendBathymetry;
        left->boundary = BND_LEFT;
        left->isDummy = false;
        // Fill left (stride 1, contiguous storage due to Float2D being column-major)
        size = ny;
        startIndex = ny + 2 + 1;
        endIndex = startIndex + size;
        if (sendBathymetry)
            std::copy(b.getRawPointer() + startIndex, b.getRawPointer() + endIndex, left->b);
        std::copy(h.getRawPointer() + startIndex, h.getRawPointer() + endIndex, left->h);
        std::copy(hu.getRawPointer() + startIndex, hu.getRawPointer() + endIndex, left->hu);
        std::copy(hv.getRawPointer() + startIndex, hv.getRawPointer() + endIndex, left->hv);
        left->timestep = totalLocalTimestep;
        // Send
        thisProxy[neighbourIndex[BND_LEFT]].receiveGhostRight(left);
    }

    if (boundaryType[BND_RIGHT] == CONNECT && isSendable(BND_RIGHT)) {

        assert(neighbourIndex[BND_RIGHT] > -1);

        copyLayer *right = new(sizesVertical, 0) copyLayer();

        right->containsBathymetry = sendBathymetry;
        right->boundary = BND_RIGHT;
        right->isDummy = false;
        // Fill right (stride 1, contiguous storage due to Float2D being column-major)
        size = ny;
        startIndex = nx * (ny + 2) + 1;
        endIndex = startIndex + size;
        if (sendBathymetry)
            std::copy(b.getRawPointer() + startIndex, b.getRawPointer() + endIndex, right->b);
        std::copy(h.getRawPointer() + startIndex, h.getRawPointer() + endIndex, right->h);
        std::copy(hu.getRawPointer() + startIndex, hu.getRawPointer() + endIndex, right->hu);
        std::copy(hv.getRawPointer() + startIndex, hv.getRawPointer() + endIndex, right->hv);
        right->timestep = totalLocalTimestep;

        thisProxy[neighbourIndex[BND_RIGHT]].receiveGhostLeft(right);
    }

    if (boundaryType[BND_BOTTOM] == CONNECT && isSendable(BND_BOTTOM)) {
        assert(neighbourIndex[BND_BOTTOM] > -1);

        copyLayer *bottom = new(sizesHorizontal, 0) copyLayer();
        bottom->containsBathymetry = sendBathymetry;
        bottom->boundary = BND_BOTTOM;
        bottom->isDummy = false;

        // Fill bottom
        size = nx;
        stride = ny + 2;
        startIndex = ny + 2 + 1;
        endIndex = startIndex + size;
        for (int i = 0; i < size; i++) {
            if (sendBathymetry)
                *(bottom->b + i) = *(b.getRawPointer() + startIndex + i * stride);
            *(bottom->h + i) = *(h.getRawPointer() + startIndex + i * stride);
            *(bottom->hu + i) = *(hu.getRawPointer() + startIndex + i * stride);
            *(bottom->hv + i) = *(hv.getRawPointer() + startIndex + i * stride);
        }
        bottom->timestep = totalLocalTimestep;
        // Send
        thisProxy[neighbourIndex[BND_BOTTOM]].receiveGhostTop(bottom);
    }

    if (boundaryType[BND_TOP] == CONNECT && isSendable(BND_TOP)) {
        assert(neighbourIndex[BND_TOP] > -1);

        copyLayer *top = new(sizesHorizontal, 0) copyLayer();
        top->containsBathymetry = sendBathymetry;
        top->boundary = BND_TOP;
        top->isDummy = false;

        // Fill top
        size = nx;
        stride = ny + 2;
        startIndex = ny + 2 + ny;
        endIndex = startIndex + size;
        for (int i = 0; i < size; i++) {
            if (sendBathymetry)
                *(top->b + i) = *(b.getRawPointer() + startIndex + i * stride);
            *(top->h + i) = *(h.getRawPointer() + startIndex + i * stride);
            *(top->hu + i) = *(hu.getRawPointer() + startIndex + i * stride);
            *(top->hv + i) = *(hv.getRawPointer() + startIndex + i * stride);
        }
        top->timestep = totalLocalTimestep;
        // Send
        thisProxy[neighbourIndex[BND_TOP]].receiveGhostBottom(top);
    }
}

void SWE_DimensionalSplittingCharm::writeTimestep() {
    if(write){
        writer->writeTimeStep(h, hu, hv, currentSimulationTime);
    }
}

void SWE_DimensionalSplittingCharm::setGhostLayer() {
    applyBoundaryConditions();

    // Initialize dummy messages
    // Charm++ does not allow buffer reuse, so initialize four separate messages
    int sizesEmpty[] = {0, 0, 0, 0};
    copyLayer *empty0 = new(sizesEmpty, 0) copyLayer();
    copyLayer *empty1 = new(sizesEmpty, 0) copyLayer();
    copyLayer *empty2 = new(sizesEmpty, 0) copyLayer();
    copyLayer *empty3 = new(sizesEmpty, 0) copyLayer();
    empty0->isDummy = true;
    empty1->boundary = BND_LEFT;

    empty1->isDummy = true;
    empty0->boundary = BND_RIGHT;
    empty2->isDummy = true;
    empty3->boundary = BND_BOTTOM;
    empty3->isDummy = true;
    empty2->boundary = BND_TOP;
    // Send out dummy messages to trigger the next timestep
    if (boundaryType[BND_LEFT] != CONNECT || !isReceivable(BND_LEFT)) {
        thisProxy[thisIndex].receiveGhostLeft(empty0);
    }
    if (boundaryType[BND_RIGHT] != CONNECT || !isReceivable(BND_RIGHT)) {
        thisProxy[thisIndex].receiveGhostRight(empty1);
    }
    if (boundaryType[BND_BOTTOM] != CONNECT || !isReceivable(BND_BOTTOM)) {
        thisProxy[thisIndex].receiveGhostBottom(empty2);
    }
    if (boundaryType[BND_TOP] != CONNECT || !isReceivable(BND_TOP)) {
        thisProxy[thisIndex].receiveGhostTop(empty3);
    }
}

#include "SWE_DimensionalSplittingCharm.def.h"
