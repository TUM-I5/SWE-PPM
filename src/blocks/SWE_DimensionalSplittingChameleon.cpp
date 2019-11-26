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
 * Implementation of SWE_DimensionalSplittingChameleon.hh
 *
 */
#include "SWE_DimensionalSplittingChameleon.hh"

#include <cassert>
#include <algorithm>
#include <omp.h>
#include <mpi.h>
#include <unistd.h>
#include "chameleon.h"

double getTime() {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return (double) time.tv_sec + ((double)time.tv_nsec)/1E9;
}

/*
 * Constructor of a SWE_DimensionalSplittingChameleon Block.
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
SWE_DimensionalSplittingChameleon::SWE_DimensionalSplittingChameleon (int nx, int ny, float dx, float dy, float originX, float originY) :
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
	hStar (nx + 1, ny + 2),
	huStar (nx + 1, ny + 2),

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
	left(NULL),
	right(NULL) {
		computeTime = 0.;
		computeTimeWall = 0.;

		MPI_Type_vector(nx, 1, ny + 2, MPI_FLOAT, &HORIZONTAL_BOUNDARY);
		MPI_Type_commit(&HORIZONTAL_BOUNDARY);
	}

void SWE_DimensionalSplittingChameleon::setLeft(SWE_DimensionalSplittingChameleon* argLeft) {
	left = argLeft;
}

void SWE_DimensionalSplittingChameleon::setRight(SWE_DimensionalSplittingChameleon* argRight) {
	right = argRight;
}

void SWE_DimensionalSplittingChameleon::setRank(int rank) {
	myRank = rank;
}

void SWE_DimensionalSplittingChameleon::freeMpiType() {
	MPI_Type_free(&HORIZONTAL_BOUNDARY);
}

void SWE_DimensionalSplittingChameleon::setGhostLayer() {
	// Apply appropriate conditions for OUTFLOW/WALL boundaries
	SWE_Block::applyBoundaryConditions();

	if (boundaryType[BND_RIGHT] == CONNECT_WITHIN_RANK) {
		for(int i = 1; i < ny+2; i++) {
			h[nx+1][i] = right->getWaterHeight()[1][i];
			hu[nx+1][i] = right->getMomentumHorizontal()[1][i];
			hv[nx+1][i] = right->getMomentumVertical()[1][i];
		}
	}
	if (boundaryType[BND_LEFT] == CONNECT_WITHIN_RANK) {
		for(int i = 1; i < ny+2; i++) {
			h[0][i] = left->getWaterHeight()[nx][i];
			hu[0][i] = left->getMomentumHorizontal()[nx][i];
			hv[0][i] = left->getMomentumVertical()[nx][i];
		}
	}
	if (boundaryType[BND_TOP] == CONNECT_WITHIN_RANK) {
		for(int i = 1; i < nx+2; i++) {
			h[i][ny+1] = top->getWaterHeight()[i][1];
			hu[i][ny+1] = top->getMomentumHorizontal()[i][1];
			hv[i][ny+1] = top->getMomentumVertical()[i][1];
		}
	}
	if (boundaryType[BND_BOTTOM] == CONNECT_WITHIN_RANK) {
		for(int i = 1; i < nx+2; i++) {
			h[i][0] = bottom->getWaterHeight()[i][ny];
			hu[i][0] = bottom->getMomentumHorizontal()[i][ny];
			hv[i][0] = bottom->getMomentumVertical()[i][ny];
		}
	}

	MPI_Status status;

	assert(h.getRows() == ny + 2);
	assert(hu.getRows() == ny + 2);
	assert(hv.getRows() == ny + 2);
	assert(h.getCols() == nx + 2);
	assert(hu.getCols() == nx + 2);
	assert(hv.getCols() == nx + 2);

	/*********
	 * SEND *
	 ********/

	int tagH = 1 << 30;
	int tagHU = 2 << 30;
	int tagHV = 3 << 30;

	// The requests generated by the Isends are immediately freed, since we will wait on the requests generated by the corresponding receives
	MPI_Request req;

	if (boundaryType[BND_LEFT] == CONNECT) {
		int startIndex = ny + 2 + 1;

		MPI_Isend(h.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_LEFT], ((int)originY)&tagH, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);

		MPI_Isend(hu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_LEFT], ((int)originY)&tagHU, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);

		MPI_Isend(hv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_LEFT], ((int)originY)&tagHV, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);
	}
	if (boundaryType[BND_RIGHT] == CONNECT) {
		int startIndex = nx * (ny + 2) + 1;

		MPI_Isend(h.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_RIGHT], ((int)originY)&tagH, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);

		MPI_Isend(hu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_RIGHT], ((int)originY)&tagHU, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);

		MPI_Isend(hv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_RIGHT], ((int)originY)&tagHV, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);
	}
	if (boundaryType[BND_BOTTOM] == CONNECT) {

		//int code = 
		MPI_Isend(&h[1][1], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_BOTTOM], ((int)originX)&tagH, MPI_COMM_WORLD, &req);
		//if(code != MPI_SUCCESS)
		//	printf("%d: No success %d\n", myRank, code);
		MPI_Request_free(&req);

		MPI_Isend(&hu[1][1], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_BOTTOM], ((int)originX)&tagHU, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);

		MPI_Isend(&hv[1][1], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_BOTTOM], ((int)originX)&tagHV, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);
		//printf("%d: Sent to bottom %d, %f at %f\n", myRank, neighbourRankId[BND_BOTTOM], h[1][1], originX);

	}
	if (boundaryType[BND_TOP] == CONNECT) {

		MPI_Isend(&h[1][ny], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_TOP], ((int)originX)&tagH, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);

		MPI_Isend(&hu[1][ny], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_TOP], ((int)originX)&tagHU, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);

		MPI_Isend(&hv[1][ny], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_TOP], ((int)originX)&tagHV, MPI_COMM_WORLD, &req);
		MPI_Request_free(&req);
		//printf("%d: Sent to top %d, %f at %f\n", myRank, neighbourRankId[BND_TOP], h[1][ny], originX);

	}
}

void SWE_DimensionalSplittingChameleon::receiveGhostLayer() {
	/***********
	 * RECEIVE *
	 **********/

	// 4 Boundaries times 3 arrays (h, hu, hv) means 12 requests
	MPI_Request recvReqs[12];
	MPI_Status stati[12];

	int tagH = 1 << 30;
	int tagHU = 2 << 30;
	int tagHV = 3 << 30;

	int leftReceive = 0;
	int rightReceive = 0;
	int bottomReceive = 0;
	int topReceive = 0;
	if (boundaryType[BND_LEFT] == CONNECT) {
		int startIndex = 1;
		MPI_Irecv(h.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_LEFT], ((int)originY)&tagH, MPI_COMM_WORLD, &recvReqs[0]);
		MPI_Irecv(hu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_LEFT], ((int)originY)&tagHU, MPI_COMM_WORLD, &recvReqs[1]);
		MPI_Irecv(hv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_LEFT], ((int)originY)&tagHV, MPI_COMM_WORLD, &recvReqs[2]);
		leftReceive = 1;
	} else {
		recvReqs[0] = MPI_REQUEST_NULL;
		recvReqs[1] = MPI_REQUEST_NULL;
		recvReqs[2] = MPI_REQUEST_NULL;
	}

	if (boundaryType[BND_RIGHT] == CONNECT) {
		int startIndex = (nx + 1) * (ny + 2) + 1;
		MPI_Irecv(h.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_RIGHT], ((int)originY)&tagH, MPI_COMM_WORLD, &recvReqs[3]);
		MPI_Irecv(hu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_RIGHT], ((int)originY)&tagHU, MPI_COMM_WORLD, &recvReqs[4]);
		MPI_Irecv(hv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourRankId[BND_RIGHT], ((int)originY)&tagHV, MPI_COMM_WORLD, &recvReqs[5]);
		rightReceive = 1;
	} else {
		recvReqs[3] = MPI_REQUEST_NULL;
		recvReqs[4] = MPI_REQUEST_NULL;
		recvReqs[5] = MPI_REQUEST_NULL;
	}

	if (boundaryType[BND_BOTTOM] == CONNECT) {

		MPI_Irecv(&h[1][0], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_BOTTOM], ((int)originX)&tagH, MPI_COMM_WORLD, &recvReqs[6]); 
		MPI_Irecv(&hu[1][0], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_BOTTOM], ((int)originX)&tagHU, MPI_COMM_WORLD, &recvReqs[7]); 
		MPI_Irecv(&hv[1][0], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_BOTTOM], ((int)originX)&tagHV, MPI_COMM_WORLD, &recvReqs[8]); 
		bottomReceive = 1;
	} else {
		recvReqs[6] = MPI_REQUEST_NULL;
		recvReqs[7] = MPI_REQUEST_NULL;
		recvReqs[8] = MPI_REQUEST_NULL;
	}
	
	if (boundaryType[BND_TOP] == CONNECT) {

		MPI_Irecv(&h[1][ny + 1], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_TOP], ((int)originX)&tagH, MPI_COMM_WORLD, &recvReqs[9]); 
		MPI_Irecv(&hu[1][ny + 1], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_TOP], ((int)originX)&tagHU, MPI_COMM_WORLD, &recvReqs[10]); 
		MPI_Irecv(&hv[1][ny + 1], 1, HORIZONTAL_BOUNDARY, neighbourRankId[BND_TOP], ((int)originX)&tagHV, MPI_COMM_WORLD, &recvReqs[11]); 
		topReceive = 1;
	} else {
		recvReqs[9] = MPI_REQUEST_NULL;
		recvReqs[10] = MPI_REQUEST_NULL;
		recvReqs[11] = MPI_REQUEST_NULL;
	}

	int code = MPI_Waitall(12, recvReqs, stati);
	if(code != MPI_SUCCESS)
		printf("%d: No success %d\n", myRank, code);
	//if(leftReceive)
	//	printf("%d: Received left from %d\n", myRank, neighbourRankId[BND_LEFT]);
	//if(rightReceive)
	//	printf("%d: Received right from %d\n", myRank, neighbourRankId[BND_RIGHT]);
	//if(bottomReceive)
	//	printf("%d: Received bottom from %d, %f at %f\n", myRank, neighbourRankId[BND_BOTTOM], h[1][0], originX);
	//if(topReceive)
	//	printf("%d: Received top from %d, %f at %f\n", myRank, neighbourRankId[BND_TOP], h[1][ny + 1], originX);
}

void computeNumericalFluxesHorizontalKernel(SWE_DimensionalSplittingChameleon* block, float* maxTimestep, float* h_data, float* hu_data, float* b_data,
								float* hNetUpdatesLeft_data, float* hNetUpdatesRight_data, float* huNetUpdatesLeft_data, float* huNetUpdatesRight_data) {
	// Set data pointers correctly
	block->getModifiableWaterHeight().setRawPointer(h_data);
	block->getModifiableMomentumHorizontal().setRawPointer(hu_data);
	block->getModifiableBathymetry().setRawPointer(b_data);
	block->hNetUpdatesLeft.setRawPointer(hNetUpdatesLeft_data);
	block->hNetUpdatesRight.setRawPointer(hNetUpdatesRight_data);
	block->huNetUpdatesLeft.setRawPointer(huNetUpdatesLeft_data);
	block->huNetUpdatesRight.setRawPointer(huNetUpdatesRight_data);
	
	// Start compute clocks
	block->computeClock = getTime();

	//maximum (linearized) wave speed within one iteration
	float maxHorizontalWaveSpeed = (float) 0.;
	float maxVerticalWaveSpeed = (float) 0.;
	solver::AugRie<float> localSolver = block->solver;

	// x-sweep, compute the actual domain plus ghost rows above and below
	// iterate over cells on the x-axis, leave out the last column (two cells per computation)
	//#pragma omp for reduction(max : maxHorizontalWaveSpeed) collapse(2)
	for (int x = 0; x < block->nx + 1; x++) {
		// iterate over all rows, including ghost layer
		for (int y = 0; y < block->ny + 2; y++) {
			localSolver.computeNetUpdates (
					block->getWaterHeight()[x][y], block->getWaterHeight()[x + 1][y],
					block->getMomentumHorizontal()[x][y], block->getMomentumHorizontal()[x + 1][y],
					block->getBathymetry()[x][y], block->getBathymetry()[x + 1][y],
					block->hNetUpdatesLeft[x][y], block->hNetUpdatesRight[x + 1][y],
					block->huNetUpdatesLeft[x][y], block->huNetUpdatesRight[x + 1][y],
					maxHorizontalWaveSpeed
					);
		}
	}

	// compute max timestep according to cautious CFL-condition
	block->maxTimestep = (float) .4 * (block->dx / maxHorizontalWaveSpeed);

	// Accumulate compute time

	block->computeTimeWall += getTime() - block->computeClock;

	*maxTimestep = block->maxTimestep;
	//usleep(10000);
}

/**
 * Compute net updates for the block.
 * The member variable #maxTimestep will be updated with the
 * maximum allowed time step size
 */
void SWE_DimensionalSplittingChameleon::computeNumericalFluxesHorizontal() {

	chameleon_map_data_entry_t* args = new chameleon_map_data_entry_t[9];
    args[0] = chameleon_map_data_entry_create(this, sizeof(SWE_DimensionalSplittingChameleon), CHAM_OMP_TGT_MAPTYPE_TO);
	args[1] = chameleon_map_data_entry_create(&(this->maxTimestep), sizeof(float), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[2] = chameleon_map_data_entry_create(this->getWaterHeight().getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[3] = chameleon_map_data_entry_create(this->hu.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[4] = chameleon_map_data_entry_create(this->b.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[5] = chameleon_map_data_entry_create(this->hNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[6] = chameleon_map_data_entry_create(this->hNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[7] = chameleon_map_data_entry_create(this->huNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[8] = chameleon_map_data_entry_create(this->huNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
	
	cham_migratable_task_t *cur_task = chameleon_create_task(
        (void *)&computeNumericalFluxesHorizontalKernel,
        9, // number of args
        args);
	int32_t res = chameleon_add_task(cur_task);
}

void computeNumericalFluxesVerticalKernel(SWE_DimensionalSplittingChameleon* block, float* h_data, float* hu_data, float* hv_data, float* b_data,
								float* hNetUpdatesLeft_data, float* hNetUpdatesRight_data, float* huNetUpdatesLeft_data, float* huNetUpdatesRight_data,
								float* hNetUpdatesBelow_data, float* hNetUpdatesAbove_data, float* hvNetUpdatesBelow_data, float* hvNetUpdatesAbove_data,
								float* hStar_data, float* huStar_data) {
	// Set data pointers correctly
	block->getModifiableWaterHeight().setRawPointer(h_data);
	block->getModifiableMomentumHorizontal().setRawPointer(hu_data);
	block->getModifiableMomentumVertical().setRawPointer(hv_data);
	block->getModifiableBathymetry().setRawPointer(b_data);
	block->hNetUpdatesLeft.setRawPointer(hNetUpdatesLeft_data);
	block->hNetUpdatesRight.setRawPointer(hNetUpdatesRight_data);
	block->huNetUpdatesLeft.setRawPointer(huNetUpdatesLeft_data);
	block->huNetUpdatesRight.setRawPointer(huNetUpdatesRight_data);
	block->hNetUpdatesBelow.setRawPointer(hNetUpdatesBelow_data);
	block->hNetUpdatesAbove.setRawPointer(hNetUpdatesAbove_data);
	block->hvNetUpdatesBelow.setRawPointer(hvNetUpdatesBelow_data);
	block->hvNetUpdatesAbove.setRawPointer(hvNetUpdatesAbove_data);
	block->hStar.setRawPointer(hStar_data);
	block->huStar.setRawPointer(huStar_data);
	
	// Start compute clocks
	block->computeClock = getTime();

	//maximum (linearized) wave speed within one iteration
	float maxVerticalWaveSpeed = (float) 0.;
	solver::AugRie<float> localSolver = block->solver;

	// set intermediary Q* states
	//#pragma omp for collapse(2)
	for (int x = 1; x < block->nx + 1; x++) {
		for (int y = 0; y < block->ny + 2; y++) {
			block->hStar[x][y] = block->getWaterHeight()[x][y] - (block->maxTimestep / block->dx) * (block->hNetUpdatesLeft[x][y] + block->hNetUpdatesRight[x][y]);
			block->huStar[x][y] = block->getMomentumHorizontal()[x][y] - (block->maxTimestep / block->dx) * (block->huNetUpdatesLeft[x][y] + block->huNetUpdatesRight[x][y]);
		}
	}

	// y-sweep
	//#pragma omp for reduction(max : maxVerticalWaveSpeed) collapse(2)
	for (int x = 1; x < block->nx + 1; x++) {
		for (int y = 0; y < block->ny + 1; y++) {
			localSolver.computeNetUpdates (
					block->getWaterHeight()[x][y], block->getWaterHeight()[x][y + 1],
					block->getMomentumVertical()[x][y], block->getMomentumVertical()[x][y + 1],
					block->getBathymetry()[x][y], block->getBathymetry()[x][y + 1],
					block->hNetUpdatesBelow[x][y], block->hNetUpdatesAbove[x][y + 1],
					block->hvNetUpdatesBelow[x][y], block->hvNetUpdatesAbove[x][y + 1],
					maxVerticalWaveSpeed
					);
		}
	}
	
	#ifndef NDEBUG
	if(block->maxTimestep >= (float) .7 * (block->dy / maxVerticalWaveSpeed)) {
		printf("%d: %f, %f, %f\n", block->myRank, block->maxTimestep, block->dy, maxVerticalWaveSpeed);
	}
	// check if the cfl condition holds in the y-direction
	assert(block->maxTimestep < (float) .7 * (block->dy / maxVerticalWaveSpeed));
	#endif // NDEBUG

	// Accumulate compute time
	block->computeTimeWall += getTime() - block->computeClock;
}

/**
 * Compute net updates for the block.
 * The member variable #maxTimestep will be updated with the
 * maximum allowed time step size
 */
void SWE_DimensionalSplittingChameleon::computeNumericalFluxesVertical() {

	chameleon_map_data_entry_t* args = new chameleon_map_data_entry_t[15];
    args[0] = chameleon_map_data_entry_create(this, sizeof(SWE_DimensionalSplittingChameleon), CHAM_OMP_TGT_MAPTYPE_TO);
    args[1] = chameleon_map_data_entry_create(this->getWaterHeight().getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[2] = chameleon_map_data_entry_create(this->hu.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[3] = chameleon_map_data_entry_create(this->hv.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[4] = chameleon_map_data_entry_create(this->b.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[5] = chameleon_map_data_entry_create(this->hNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[6] = chameleon_map_data_entry_create(this->hNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[7] = chameleon_map_data_entry_create(this->huNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[8] = chameleon_map_data_entry_create(this->huNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[9] = chameleon_map_data_entry_create(this->hNetUpdatesBelow.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[10] = chameleon_map_data_entry_create(this->hNetUpdatesAbove.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[11] = chameleon_map_data_entry_create(this->hvNetUpdatesBelow.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[12] = chameleon_map_data_entry_create(this->hvNetUpdatesAbove.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[13] = chameleon_map_data_entry_create(this->hStar.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[14] = chameleon_map_data_entry_create(this->huStar.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
	
	cham_migratable_task_t *cur_task = chameleon_create_task(
        (void *)&computeNumericalFluxesVerticalKernel,
        15, // number of args
        args);
	int32_t res = chameleon_add_task(cur_task);
}

/**
 * Updates the unknowns with the already computed net-updates.
 *
 * @param dt time step width used in the update. The timestep has to be equal to maxTimestep calculated by computeNumericalFluxes(),
 * since this is the step width used for the intermediary updates after the x-sweep.
 */
void SWE_DimensionalSplittingChameleon::updateUnknowns (float dt) {
	// Start compute clocks
	computeClock = getTime();

	//printf("%d: Update with %f and %f\n", myRank, dt, maxTimestep);

	// this assertion has to hold since the intermediary star states were calculated internally using a timestep width of maxTimestep
	assert(std::abs(dt - maxTimestep) < 0.00001);
	//update cell averages with the net-updates
	//printf("%d: %p, %p, %p, %p\n", myRank, h.getRawPointer(), hStar.getRawPointer(), hNetUpdatesBelow.getRawPointer(), hNetUpdatesAbove.getRawPointer());
	for (int x = 1; x < nx+1; x++) {
		for (int y = 1; y < ny + 1; y++) {
			h[x][y] = hStar[x][y] - (maxTimestep / dx) * (hNetUpdatesBelow[x][y] + hNetUpdatesAbove[x][y]);
			hu[x][y] = huStar[x][y];
			hv[x][y] = hv[x][y] - (maxTimestep / dx) * (hvNetUpdatesBelow[x][y] + hvNetUpdatesAbove[x][y]);
		}
	}

	// Accumulate compute time
	computeTimeWall += getTime() - computeClock;
}
