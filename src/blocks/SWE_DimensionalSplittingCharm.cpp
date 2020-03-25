#include "SWE_DimensionalSplittingCharm.hh"

#include "examples/swe_charm.decl.h"

SWE_DimensionalSplittingCharm::SWE_DimensionalSplittingCharm(CkMigrateMessage *msg) {}

SWE_DimensionalSplittingCharm::SWE_DimensionalSplittingCharm(int nx, int ny, float dx, float dy, float originX, float originY, int posX, int posY,
							BoundaryType boundaries[], std::string outputFilename, std::string bathymetryFilename, std::string displacementFilename,bool localTimestepping) :
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
		SWE_Block(nx, ny, dx, dy, originX, originY,localTimestepping),

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
	for(int i = 1; i < checkpointCount; i++) {
		checkpointInstantOfTime[i] = checkpointInstantOfTime[i - 1] + checkpointTimeDelta;
	}

#ifdef ASAGI
	SWE_AsagiScenario scenario(bathymetryFilename, displacementFilename);
#else
	SWE_RadialDamBreakScenario scenario = SWE_RadialDamBreakScenario();
#endif
	initScenario(scenario, boundaries);

	// Initialize writer
	BoundarySize boundarySize = {{1, 1, 1, 1}};
	writer = new NetCdfWriter(outputFilename, b, boundarySize, nx, ny, dx, dy, originX, originY);
    collector = new CollectorCharm();
	// output at t=0
	writeTimestep();

	char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);

	CkPrintf("%i Spawned at %s\n", thisIndex, hostname);
	firstIteration = true;
}

SWE_DimensionalSplittingCharm::~SWE_DimensionalSplittingCharm() {}

void SWE_DimensionalSplittingCharm::xSweep() {
	
	// maximum (linearized) wave speed within one iteration
	float maxHorizontalWaveSpeed = (float) 0.;

	#pragma omp parallel private(solver)
	{
		// x-sweep, compute the actual domain plus ghost rows above and below
		// iterate over cells on the x-axis, leave out the last column (two cells per computation)
		#pragma omp for reduction(max : maxHorizontalWaveSpeed) collapse(2)
		for (int x = 0; x < nx + 1; x++) {
			// iterate over all rows, including ghost layer
          /*  // iterate over all rows, including ghost layer
#if defined(VECTORIZE)
#pragma omp simd reduction(max:maxHorizontalWaveSpeed)
#endif // VECTORIZE*/
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

    collector->addFlops(nx*ny*135);

    maxTimestep = (float) .4 * (dx / maxHorizontalWaveSpeed);
	// compute max timestep according to cautious CFL-condition
	if(localTimestepping){
        maxTimestep = getRoundTimestep(maxTimestep);
	}else {


        collector->startCounter(Collector::CTR_REDUCE);
        CkCallback cb(CkReductionTarget(SWE_DimensionalSplittingCharm, reduceWaveSpeed), thisProxy);
        contribute(sizeof(float), &maxTimestep, CkReduction::min_float, cb);
	}

}

void SWE_DimensionalSplittingCharm::reduceWaveSpeed(float maxWaveSpeed) {
	maxTimestep = maxWaveSpeed;
	reductionTrigger();

    collector->stopCounter(Collector::CTR_REDUCE);

}

void SWE_DimensionalSplittingCharm::ySweep() {

	float maxVerticalWaveSpeed = (float) 0.;

	#pragma omp parallel private(solver)
	{
		// set intermediary Q* states
		#pragma omp for collapse(2)
		for (int x = 1; x < nx + 1; x++) {
			for (int y = 0; y < ny+2; y++) {
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
            // iterate over all rows, including ghost layer
/*#if defined(VECTORIZE)
#pragma omp simd reduction(max:maxVerticalWaveSpeed)
#endif // VECTORIZE*/
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
			assert(maxTimestep < (float) .5 * (dy / maxVerticalWaveSpeed));
		}
		#endif // NDEBUG
	}

    collector->addFlops(nx*ny*135);

}

void SWE_DimensionalSplittingCharm::updateUnknowns(float dt) {

	// this assertion has to hold since the intermediary star states were calculated internally using a timestep width of maxTimestep
	assert(std::abs(dt - maxTimestep) < 0.00001);
	//update cell averages with the net-updates
	for (int x = 1; x < nx + 1; x++) {
		for (int y = 1; y < ny + 1; y++) {
			h[x][y] = hStar[x][y] - (dt / dx) * (hNetUpdatesBelow[x][y] + hNetUpdatesAbove[x][y]);
			hu[x][y] = huStar[x][y];
			hv[x][y] = hv[x][y] - (dt / dx) * (hvNetUpdatesBelow[x][y] + hvNetUpdatesAbove[x][y]);
		}
	}

}

void SWE_DimensionalSplittingCharm::processCopyLayer(copyLayer *msg) {

	// LEFT ghost layer consists of values from the left neighbours RIGHT copy layer etc.

	if(!msg->isDummy){
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

void SWE_DimensionalSplittingCharm::sendCopyLayers(bool sendBathymetry){


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
	if (boundaryType[BND_LEFT] == CONNECT  && isSendable(BND_LEFT)) {

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

	if (boundaryType[BND_RIGHT] == CONNECT  && isSendable(BND_RIGHT)) {

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

	if (boundaryType[BND_BOTTOM] == CONNECT  && isSendable(BND_BOTTOM)) {
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

	if (boundaryType[BND_TOP] == CONNECT  && isSendable(BND_TOP)) {
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
	writer->writeTimeStep(h, hu, hv, currentSimulationTime);

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
