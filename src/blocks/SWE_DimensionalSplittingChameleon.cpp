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
SWE_DimensionalSplittingChameleon::SWE_DimensionalSplittingChameleon (int nx, int ny, float dx, float dy, float originX, float originY, bool localTimestepping,std::string name, bool write) :
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
	hvNetUpdatesAbove(nx + 1, ny + 2){


		MPI_Type_vector(nx, 1, ny + 2, MPI_FLOAT, &HORIZONTAL_BOUNDARY);
		MPI_Type_commit(&HORIZONTAL_BOUNDARY);
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
void SWE_DimensionalSplittingChameleon::writeTimestep(float timestep) {
    if(write){
        writer->writeTimeStep(h, hu, hv, timestep);
    }

}

void SWE_DimensionalSplittingChameleon::connectLocalNeighbours(std::array<std::shared_ptr<SWE_DimensionalSplittingChameleon>,4> neighbourBlocks){
    for (int i = 0; i < 4; i++) {
        if(boundaryType[i] == CONNECT_WITHIN_RANK){
            switch (i){
                case BND_LEFT:
                    left = neighbourBlocks[i].get();
                    break;
                case BND_RIGHT:
                    right = neighbourBlocks[i].get();
                    break;
                case BND_BOTTOM:
                    bottom = neighbourBlocks[i].get();
                    break;
                case BND_TOP:
                    top = neighbourBlocks[i].get();
                    break;
            }
        }

    }
}

void SWE_DimensionalSplittingChameleon::connectNeighbourLocalities(int p_neighbourRankId[]) {
    for (int i = 0; i < 4; i++) {
        neighbourLocality[i] = p_neighbourRankId[i];
    }
}



void SWE_DimensionalSplittingChameleon::freeMpiType() {
	MPI_Type_free(&HORIZONTAL_BOUNDARY);
}
int getTag(int rank, int tag){

    return (tag<<17)|rank;
}



void SWE_DimensionalSplittingChameleon::recvBathymetry() {

    /***********
     * RECEIVE *
     **********/

    MPI_Request recvReqs[4];
    MPI_Status stati[4];

    if (boundaryType[BND_LEFT] == CONNECT) {
        int startIndex = 1;
        MPI_Irecv(b.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT], getTag(myRank,MPI_TAG_OUT_B_RIGHT),
                  MPI_COMM_WORLD, &recvReqs[BND_LEFT]);
    } else {
        recvReqs[BND_LEFT] = MPI_REQUEST_NULL;
    }

    if (boundaryType[BND_RIGHT] == CONNECT) {
        int startIndex = (nx + 1) * (ny + 2) + 1;
        MPI_Irecv(b.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT], getTag(myRank,MPI_TAG_OUT_B_LEFT),
                  MPI_COMM_WORLD, &recvReqs[BND_RIGHT]);
    } else {
        recvReqs[BND_RIGHT] = MPI_REQUEST_NULL;
    }

    if (boundaryType[BND_BOTTOM] == CONNECT) {
        for (int i = 1; i < nx + 1; i++) {
            MPI_Irecv(&b[i][0], 1, MPI_FLOAT, neighbourLocality[BND_BOTTOM], getTag(myRank,MPI_TAG_OUT_B_TOP), MPI_COMM_WORLD,
                      &recvReqs[BND_BOTTOM]);
        }
    } else {
        recvReqs[BND_BOTTOM] = MPI_REQUEST_NULL;
    }

    if (boundaryType[BND_TOP] == CONNECT) {
        for (int i = 1; i < nx + 1; i++) {
            MPI_Irecv(&b[i][ny + 1], 1, MPI_FLOAT, neighbourLocality[BND_TOP], getTag(myRank,MPI_TAG_OUT_B_BOTTOM), MPI_COMM_WORLD,
                      &recvReqs[BND_TOP]);
        }
    } else {
        recvReqs[BND_TOP] = MPI_REQUEST_NULL;
    }

    MPI_Waitall(4, recvReqs, stati);

}

void SWE_DimensionalSplittingChameleon::sendBathymetry() {

    if (boundaryType[BND_RIGHT] == CONNECT_WITHIN_RANK) {
        for (int i = 1; i < ny + 1; i++) {
            b[nx + 1][i] = right->getBathymetry()[1][i];

        }
    }
    if (boundaryType[BND_LEFT] == CONNECT_WITHIN_RANK) {
        for (int i = 1; i < ny + 1; i++) {
            b[0][i] = left->getBathymetry()[nx][i];

        }
    }
    if (boundaryType[BND_TOP] == CONNECT_WITHIN_RANK) {


        for (int i = 1; i < nx + 1; i++) {
            b[i][ny + 1] = top->getBathymetry()[i][1];

        }
    }
    if (boundaryType[BND_BOTTOM] == CONNECT_WITHIN_RANK) {

        for (int i = 1; i < nx + 1; i++) {
            b[i][0] = bottom->getBathymetry()[i][ny];
        }
    }
    // The requests generated by the Isends are immediately freed, since we will wait on the requests generated by the corresponding receives
    MPI_Request req;
    /*********
     * SEND *
     ********/
    if (boundaryType[BND_LEFT] == CONNECT) {
        int startIndex = ny + 2 + 1;
        MPI_Isend(b.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT], getTag(neighbourRankId[BND_LEFT],MPI_TAG_OUT_B_LEFT),
                  MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);
    }
    if (boundaryType[BND_RIGHT] == CONNECT) {
        int startIndex = nx * (ny + 2) + 1;
        MPI_Isend(b.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT], getTag(neighbourRankId[BND_RIGHT],MPI_TAG_OUT_B_RIGHT),
                  MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);
    }
    if (boundaryType[BND_BOTTOM] == CONNECT) {
        for (int i = 1; i < nx + 1; i++) {
            MPI_Isend(&b[i][1], 1, MPI_FLOAT, neighbourLocality[BND_BOTTOM], getTag(neighbourRankId[BND_BOTTOM],MPI_TAG_OUT_B_BOTTOM), MPI_COMM_WORLD, &req);
            MPI_Request_free(&req);
        }
    }
    if (boundaryType[BND_TOP] == CONNECT) {
        for (int i = 1; i < nx + 1; i++) {
            MPI_Isend(&b[i][ny], 1, MPI_FLOAT, neighbourLocality[BND_TOP], getTag(neighbourRankId[BND_TOP],MPI_TAG_OUT_B_TOP), MPI_COMM_WORLD, &req);
            MPI_Request_free(&req);
        }
    }

}


void SWE_DimensionalSplittingChameleon::setGhostLayer() {
	// Apply appropriate conditions for OUTFLOW/WALL boundaries
	SWE_Block::applyBoundaryConditions();
    collector.startCounter(CollectorChameleon::CTR_EXCHANGE);
    if (boundaryType[BND_RIGHT] == CONNECT_WITHIN_RANK && isReceivable(BND_RIGHT)) {
        borderTimestep[BND_RIGHT] = right->getTotalLocalTimestep();
        for(int i = 1; i < ny+1; i++) {
            bufferH[nx+1][i] = right->getWaterHeight()[1][i];
            bufferHu[nx+1][i] = right->getMomentumHorizontal()[1][i];
            bufferHv[nx+1][i] = right->getMomentumVertical()[1][i];
        }
    }
    if (boundaryType[BND_LEFT] == CONNECT_WITHIN_RANK && isReceivable(BND_LEFT)) {
        borderTimestep[BND_LEFT] = left->getTotalLocalTimestep();
        for(int i = 1; i < ny+1; i++) {
            bufferH[0][i] = left->getWaterHeight()[left->nx][i];
            bufferHu[0][i] = left->getMomentumHorizontal()[left->nx][i];
            bufferHv[0][i] = left->getMomentumVertical()[left->nx][i];
        }
    }
    if (boundaryType[BND_TOP] == CONNECT_WITHIN_RANK && isReceivable(BND_TOP)) {
        borderTimestep[BND_TOP] = top->getTotalLocalTimestep();
        for(int i = 1; i < nx+1; i++) {
            bufferH[i][ny+1] = top->getWaterHeight()[i][1];
            bufferHu[i][ny+1] = top->getMomentumHorizontal()[i][1];
            bufferHv[i][ny+1] = top->getMomentumVertical()[i][1];
        }
    }
    if (boundaryType[BND_BOTTOM] == CONNECT_WITHIN_RANK && isReceivable(BND_BOTTOM)) {
        borderTimestep[BND_BOTTOM] = bottom->getTotalLocalTimestep();
        for(int i = 1; i < nx+1; i++) {
            bufferH[i][0] = bottom->getWaterHeight()[i][bottom->ny];
            bufferHu[i][0] = bottom->getMomentumHorizontal()[i][bottom->ny];
            bufferHv[i][0] = bottom->getMomentumVertical()[i][bottom->ny];
        }
    }
    MPI_Request req;
    float totalLocalTimestep = getTotalLocalTimestep();
    if (boundaryType[BND_LEFT] == CONNECT && isSendable(BND_LEFT)) {
        int startIndex = ny + 2 + 1;

        MPI_Isend(h.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT], getTag(neighbourRankId[BND_LEFT], MPI_TAG_OUT_H_LEFT), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(hu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT], getTag(neighbourRankId[BND_LEFT], MPI_TAG_OUT_HU_LEFT), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(hv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT], getTag(neighbourRankId[BND_LEFT], MPI_TAG_OUT_HV_LEFT), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(&totalLocalTimestep, 1, MPI_FLOAT, neighbourLocality[BND_LEFT], getTag(neighbourRankId[BND_LEFT], MPI_TAG_TIMESTEP_LEFT), MPI_COMM_WORLD,&req);
        MPI_Request_free(&req);

    }
    if (boundaryType[BND_RIGHT] == CONNECT && isSendable(BND_RIGHT)) {
        int startIndex = nx * (ny + 2) + 1;

        MPI_Isend(h.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT], getTag(neighbourRankId[BND_RIGHT], MPI_TAG_OUT_H_RIGHT), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(hu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT], getTag(neighbourRankId[BND_RIGHT], MPI_TAG_OUT_HU_RIGHT), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(hv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT], getTag(neighbourRankId[BND_RIGHT], MPI_TAG_OUT_HV_RIGHT), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(&totalLocalTimestep, 1, MPI_FLOAT, neighbourLocality[BND_RIGHT], getTag(neighbourRankId[BND_RIGHT], MPI_TAG_TIMESTEP_RIGHT),MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

    }
    if (boundaryType[BND_BOTTOM] == CONNECT && isSendable(BND_BOTTOM)) {

        //int code =
        MPI_Isend(&h[1][1], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_BOTTOM], getTag(neighbourRankId[BND_BOTTOM], MPI_TAG_OUT_H_BOTTOM), MPI_COMM_WORLD, &req);
        //if(code != MPI_SUCCESS)
        //	printf("%d: No success %d\n", myRank, code);
        MPI_Request_free(&req);

        MPI_Isend(&hu[1][1], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_BOTTOM], getTag(neighbourRankId[BND_BOTTOM], MPI_TAG_OUT_HU_BOTTOM), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(&hv[1][1], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_BOTTOM], getTag(neighbourRankId[BND_BOTTOM], MPI_TAG_OUT_HV_BOTTOM), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);
        //printf("%d: Sent to bottom %d, %f at %f\n", myRank, neighbourRankId[BND_BOTTOM], h[1][1], originX);

        MPI_Isend(&totalLocalTimestep, 1, MPI_FLOAT, neighbourLocality[BND_BOTTOM], getTag(neighbourRankId[BND_BOTTOM], MPI_TAG_TIMESTEP_BOTTOM),MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

    }
    if (boundaryType[BND_TOP] == CONNECT && isSendable(BND_TOP)) {

        MPI_Isend(&h[1][ny], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_TOP], getTag(neighbourRankId[BND_TOP], MPI_TAG_OUT_H_TOP), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(&hu[1][ny], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_TOP], getTag(neighbourRankId[BND_TOP], MPI_TAG_OUT_HU_TOP), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);

        MPI_Isend(&hv[1][ny], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_TOP], getTag(neighbourRankId[BND_TOP], MPI_TAG_OUT_HV_TOP), MPI_COMM_WORLD, &req);
        MPI_Request_free(&req);
        //printf("%d: Sent to top %d, %f at %f\n", myRank, neighbourRankId[BND_TOP], h[1][ny], originX);

        MPI_Isend(&totalLocalTimestep, 1, MPI_FLOAT, neighbourLocality[BND_TOP], getTag(neighbourRankId[BND_TOP], MPI_TAG_TIMESTEP_TOP), MPI_COMM_WORLD,&req);
        MPI_Request_free(&req);

    }
    printf("%d: send   %d:%d:%d:%d:\n", myRank,getTag(myRank, MPI_TAG_TIMESTEP_LEFT) ,getTag(myRank, MPI_TAG_TIMESTEP_RIGHT),
            , getTag(myRank, MPI_TAG_TIMESTEP_BOTTOM),getTag(myRank, MPI_TAG_TIMESTEP_TOP));

	assert(h.getRows() == ny + 2);
	assert(hu.getRows() == ny + 2);
	assert(hv.getRows() == ny + 2);
	assert(h.getCols() == nx + 2);
	assert(hu.getCols() == nx + 2);
	assert(hv.getCols() == nx + 2);

	/*********
	 * SEND *
	 ********/



}

void SWE_DimensionalSplittingChameleon::receiveGhostLayer() {
	/***********
	 * RECEIVE *
	 **********/

    // The requests generated by the Isends are immediately freed, since we will wait on the requests generated by the corresponding receives

   // std::cout << myRank << "| start recv\n";

    MPI_Status status;
	// 4 Boundaries times 3 arrays (h, hu, hv) means 12 requests
	MPI_Request recvReqs[16];
	MPI_Status stati[16];



    if (boundaryType[BND_LEFT] == CONNECT && isReceivable(BND_LEFT)) {
		int startIndex = 1;

        MPI_Irecv(bufferH.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT],getTag(myRank, MPI_TAG_OUT_H_RIGHT), MPI_COMM_WORLD, &recvReqs[0]);
        MPI_Irecv(bufferHu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT],getTag(myRank, MPI_TAG_OUT_HU_RIGHT), MPI_COMM_WORLD, &recvReqs[1]);
        MPI_Irecv(bufferHv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_LEFT],getTag(myRank, MPI_TAG_OUT_HV_RIGHT), MPI_COMM_WORLD, &recvReqs[2]);
        MPI_Irecv(&borderTimestep[BND_LEFT], 1, MPI_FLOAT, neighbourLocality[BND_LEFT], getTag(myRank, MPI_TAG_TIMESTEP_RIGHT), MPI_COMM_WORLD, &recvReqs[3]);

    } else {
        recvReqs[0] = MPI_REQUEST_NULL;
        recvReqs[1] = MPI_REQUEST_NULL;
        recvReqs[2] = MPI_REQUEST_NULL;
        recvReqs[3] = MPI_REQUEST_NULL;

    }

	if (boundaryType[BND_RIGHT] == CONNECT && isReceivable(BND_RIGHT)) {
		int startIndex = (nx + 1) * (ny + 2) + 1;
        MPI_Irecv(bufferH.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT],getTag(myRank, MPI_TAG_OUT_H_LEFT), MPI_COMM_WORLD, &recvReqs[4]);
        MPI_Irecv(bufferHu.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT],getTag(myRank, MPI_TAG_OUT_HU_LEFT), MPI_COMM_WORLD, &recvReqs[5]);
        MPI_Irecv(bufferHv.getRawPointer() + startIndex, ny, MPI_FLOAT, neighbourLocality[BND_RIGHT],getTag(myRank, MPI_TAG_OUT_HV_LEFT), MPI_COMM_WORLD, &recvReqs[6]);
        MPI_Irecv(&borderTimestep[BND_RIGHT], 1, MPI_FLOAT, neighbourLocality[BND_RIGHT], getTag(myRank, MPI_TAG_TIMESTEP_LEFT), MPI_COMM_WORLD, &recvReqs[7]);

    } else {
        recvReqs[4] = MPI_REQUEST_NULL;
        recvReqs[5] = MPI_REQUEST_NULL;
        recvReqs[6] = MPI_REQUEST_NULL;
        recvReqs[7] = MPI_REQUEST_NULL;

    }

	if (boundaryType[BND_BOTTOM] == CONNECT && isReceivable(BND_BOTTOM)) {

        MPI_Irecv(&bufferH[1][0], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_BOTTOM], getTag(myRank, MPI_TAG_OUT_H_TOP),MPI_COMM_WORLD, &recvReqs[8]);
        MPI_Irecv(&bufferHu[1][0], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_BOTTOM], getTag(myRank, MPI_TAG_OUT_HU_TOP),MPI_COMM_WORLD, &recvReqs[9]);
        MPI_Irecv(&bufferHv[1][0], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_BOTTOM], getTag(myRank, MPI_TAG_OUT_HV_TOP),MPI_COMM_WORLD, &recvReqs[10]);
        MPI_Irecv(&borderTimestep[BND_BOTTOM], 1, MPI_FLOAT, neighbourLocality[BND_BOTTOM], getTag(myRank, MPI_TAG_TIMESTEP_TOP),MPI_COMM_WORLD, &recvReqs[11]);

    } else {
        recvReqs[8] = MPI_REQUEST_NULL;
        recvReqs[9] = MPI_REQUEST_NULL;
        recvReqs[10] = MPI_REQUEST_NULL;
        recvReqs[11] = MPI_REQUEST_NULL;
    }

	if (boundaryType[BND_TOP] == CONNECT && isReceivable(BND_TOP)) {

        MPI_Irecv(&bufferH[1][ny + 1], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_TOP], getTag(myRank, MPI_TAG_OUT_H_BOTTOM),MPI_COMM_WORLD, &recvReqs[12]);
        MPI_Irecv(&bufferHu[1][ny + 1], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_TOP], getTag(myRank, MPI_TAG_OUT_HU_BOTTOM),MPI_COMM_WORLD, &recvReqs[13]);
        MPI_Irecv(&bufferHv[1][ny + 1], 1, HORIZONTAL_BOUNDARY, neighbourLocality[BND_TOP], getTag(myRank, MPI_TAG_OUT_HV_BOTTOM),MPI_COMM_WORLD, &recvReqs[14]);
        MPI_Irecv(&borderTimestep[BND_TOP], 1, MPI_FLOAT, neighbourLocality[BND_TOP], getTag(myRank, MPI_TAG_TIMESTEP_BOTTOM),MPI_COMM_WORLD, &recvReqs[15]);

    } else {
        recvReqs[12] = MPI_REQUEST_NULL;
        recvReqs[13] = MPI_REQUEST_NULL;
        recvReqs[14] = MPI_REQUEST_NULL;
        recvReqs[15] = MPI_REQUEST_NULL;
    }

	int code = MPI_Waitall(16, recvReqs, stati);
	if(code != MPI_SUCCESS){
        printf("%d: No success %d  %d:%d:%d:%d:\n", myRank, code,getTag(myRank, MPI_TAG_TIMESTEP_RIGHT),getTag(myRank, MPI_TAG_TIMESTEP_LEFT)
                                                            ,getTag(myRank, MPI_TAG_TIMESTEP_TOP), getTag(myRank, MPI_TAG_TIMESTEP_BOTTOM));

    }

    checkAllGhostlayers();
    collector.stopCounter(CollectorChameleon::CTR_EXCHANGE);
}
void computeNumericalFluxesKernel(SWE_DimensionalSplittingChameleon* block, float* maxTimestep, float* h_data,
                                            float* hu_data, float* hv_data, float* b_data,
                                            float* hNetUpdatesLeft_data, float* hNetUpdatesRight_data,
                                            float* hNetUpdatesBelow_data, float* hNetUpdatesAbove_data,
                                            float* huNetUpdatesLeft_data, float* huNetUpdatesRight_data,
                                            float* hvNetUpdatesBelow_data, float* hvNetUpdatesAbove_data) {
    // Set data pointers correctly
    block->getModifiableWaterHeight().setRawPointer(h_data);
    block->getModifiableMomentumHorizontal().setRawPointer(hu_data);
    block->getModifiableMomentumVertical().setRawPointer(hv_data);
    block->getModifiableBathymetry().setRawPointer(b_data);
    block->hNetUpdatesLeft.setRawPointer(hNetUpdatesLeft_data);
    block->hNetUpdatesRight.setRawPointer(hNetUpdatesRight_data);
    block->hNetUpdatesBelow.setRawPointer(hNetUpdatesBelow_data);
    block->hNetUpdatesAbove.setRawPointer(hNetUpdatesAbove_data);
    block->huNetUpdatesLeft.setRawPointer(huNetUpdatesLeft_data);
    block->huNetUpdatesRight.setRawPointer(huNetUpdatesRight_data);
    block->hvNetUpdatesBelow.setRawPointer(hvNetUpdatesBelow_data);
    block->hvNetUpdatesAbove.setRawPointer(hvNetUpdatesAbove_data);


#if WAVE_PROPAGATION_SOLVER == 0
    solver::HLLEFun<float> localSolver = block->solver;
#elif WAVE_PROPAGATION_SOLVER == 1
    //! F-wave Riemann solver
    solver::FWave<float> localSolver = block->solver;
#elif WAVE_PROPAGATION_SOLVER==2
    //! Approximate Augmented Riemann solver
    solver::AugRie<float> localSolver = block->solver;
#endif

    float maxWaveSpeed = (float) 0.;

    /***************************************************************************************
     * compute the net-updates for the vertical edges
     **************************************************************************************/

    for (int i = 1; i < block->nx+2; i++) {
        const int ny_end = block->ny+1;
#if defined(VECTORIZE)

        // iterate over all rows, including ghost layer
#pragma omp simd reduction(max:maxWaveSpeed)
#endif // VECTORIZE
        for (int j=1; j < ny_end; ++j) {
            float maxEdgeSpeed;

            localSolver.computeNetUpdates (
                    block->getWaterHeight()[i - 1][j],block->getWaterHeight()[i][j],
                    block->getMomentumHorizontal()[i - 1][j], block->getMomentumHorizontal()[i][j],
                    block->getBathymetry()[i - 1][j], block->getBathymetry()[i][j],
                    block->hNetUpdatesLeft[i - 1][j - 1], block->hNetUpdatesRight[i - 1][j - 1],
                    block->huNetUpdatesLeft[i - 1][j - 1], block->huNetUpdatesRight[i - 1][j - 1],
                    maxEdgeSpeed
            );

            //update the thread-local maximum wave speed
            maxWaveSpeed = std::max(maxWaveSpeed, maxEdgeSpeed);
        }
    }

    /***************************************************************************************
     * compute the net-updates for the horizontal edges
     **************************************************************************************/

    for (int i=1; i < block->nx + 1; i++) {
        const int ny_end = block->ny+2;
#if defined(VECTORIZE)

        // iterate over all rows, including ghost layer
#pragma omp simd reduction(max:maxWaveSpeed)
#endif // VECTORIZE
        for (int j=1; j < ny_end; j++) {
            float maxEdgeSpeed;

            localSolver.computeNetUpdates (
                    block->getWaterHeight()[i][j - 1],  block->getWaterHeight()[i][j],
                    block->getMomentumVertical()[i][j - 1], block->getMomentumVertical()[i][j],
                    block->getBathymetry()[i][j - 1], block->getBathymetry()[i][j],
                    block->hNetUpdatesBelow[i - 1][j - 1], block->hNetUpdatesAbove[i - 1][j - 1],
                    block->hvNetUpdatesBelow[i - 1][j - 1], block->hvNetUpdatesAbove[i - 1][j - 1],
                    maxEdgeSpeed
            );

            //update the maximum wave speed
            maxWaveSpeed = std::max (maxWaveSpeed, maxEdgeSpeed);
        }
    }

    if (maxWaveSpeed > 0.00001) {

        *maxTimestep = std::min ( block->dx / maxWaveSpeed, block->dy / maxWaveSpeed);

        *maxTimestep *= (float) .4; //CFL-number = .5
    } else {
        //might happen in dry cells
        *maxTimestep = std::numeric_limits<float>::max ();
    }
}


void SWE_DimensionalSplittingChameleon::computeNumericalFluxes() {

    if (!allGhostlayersInSync()) return;
    collector.addFlops(2*135*nx*ny);

    chameleon_map_data_entry_t* args = new chameleon_map_data_entry_t[14];
    args[0] = chameleon_map_data_entry_create(this, sizeof(SWE_DimensionalSplittingChameleon), CHAM_OMP_TGT_MAPTYPE_TO);
    args[1] = chameleon_map_data_entry_create(&(this->maxTimestep), sizeof(float), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[2] = chameleon_map_data_entry_create(this->getWaterHeight().getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[3] = chameleon_map_data_entry_create(this->hu.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[4] = chameleon_map_data_entry_create(this->hv.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[5] = chameleon_map_data_entry_create(this->b.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);

    args[6] = chameleon_map_data_entry_create(this->hNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[7] = chameleon_map_data_entry_create(this->hNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[8] = chameleon_map_data_entry_create(this->hNetUpdatesBelow.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[9] = chameleon_map_data_entry_create(this->hNetUpdatesAbove.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);

    args[10] = chameleon_map_data_entry_create(this->huNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[11] = chameleon_map_data_entry_create(this->huNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);

    args[12] = chameleon_map_data_entry_create(this->hvNetUpdatesBelow.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[13] = chameleon_map_data_entry_create(this->hvNetUpdatesAbove.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);

    cham_migratable_task_t *cur_task = chameleon_create_task(
            (void *)&computeNumericalFluxesKernel,
            14, // number of args
            args);
    int32_t res = chameleon_add_task(cur_task);
}




/**
 * Updates the unknowns with the already computed net-updates.
 *
 * @param dt time step width used in the update. The timestep has to be equal to maxTimestep calculated by computeNumericalFluxes(),
 * since this is the step width used for the intermediary updates after the x-sweep.
 */
 void updateUnkownsKernel(SWE_DimensionalSplittingChameleon* block, float* maxTimestep, float* h_in,float* hu_in, float* hv_in,

                          float* hNetUpdatesLeft_data, float* hNetUpdatesRight_data,
                          float* hNetUpdatesBelow_data, float* hNetUpdatesAbove_data,
                          float* huNetUpdatesLeft_data, float* huNetUpdatesRight_data,
                          float* hvNetUpdatesBelow_data, float* hvNetUpdatesAbove_data,float* h_data, float* hu_data, float* hv_data
){
    block->h.setRawPointer(h_data);
    block->hu.setRawPointer(hu_data);
    block->hv.setRawPointer(hv_data);

    block->hNetUpdatesLeft.setRawPointer(hNetUpdatesLeft_data);
    block->hNetUpdatesRight.setRawPointer(hNetUpdatesRight_data);
    block->hNetUpdatesBelow.setRawPointer(hNetUpdatesBelow_data);
    block->hNetUpdatesAbove.setRawPointer(hNetUpdatesAbove_data);

    block->huNetUpdatesLeft.setRawPointer(huNetUpdatesLeft_data);
    block->huNetUpdatesRight.setRawPointer(huNetUpdatesRight_data);

    block->hvNetUpdatesBelow.setRawPointer(hvNetUpdatesBelow_data);
    block->hvNetUpdatesAbove.setRawPointer(hvNetUpdatesAbove_data);

    Float2DNative h = Float2DNative(block->nx+2,block->ny+2);
    h.setRawPointer(h_in);
    Float2DNative hu = Float2DNative(block->nx+2,block->ny+2);
    hu.setRawPointer(hu_in);
    Float2DNative hv = Float2DNative(block->nx+2,block->ny+2);
    hv.setRawPointer(hv_in);

    float dt=*maxTimestep;
    for (int i = 1; i < block->nx +1; i++) {
        const int ny_end = block->ny + 1;

#if defined(VECTORIZE)

        // iterate over all rows, including ghost layer
#pragma omp simd
#endif // VECTORIZE
        for (int j = 1; j < ny_end; j++) {
            block->h[i][j] =  h[i][j] - (dt / block->dx * (block->hNetUpdatesRight[i - 1][j - 1] + block->hNetUpdatesLeft[i][j - 1]) + dt / block->dy * (block->hNetUpdatesAbove[i - 1][j - 1] + block->hNetUpdatesBelow[i - 1][j]));
            block->hu[i][j] = hu[i][j] - (dt / block->dx * (block->huNetUpdatesRight[i - 1][j - 1] + block->huNetUpdatesLeft[i][j - 1]));
            block->hv[i][j]=  hv[i][j] - (dt / block->dy * (block->hvNetUpdatesAbove[i - 1][j - 1] + block->hvNetUpdatesBelow[i - 1][j]));

            if (block->h[i][j] < 0) {
                //TODO: dryTol
#ifndef NDEBUG
                // Only print this warning when debug is enabled
				// Otherwise we cannot vectorize this loop
				if (block->h[i][j] < -0.1) {
					std::cerr << "Warning, negative height: (i,j)=(" << i << "," << j << ")=" << block->h[i][j] << std::endl;
					//std::cerr << "         b: " << b[i][j] << std::endl;
				}
#endif // NDEBUG
                //zero (small) negative depths
                block->h[i][j] = block->hu[i][j] = block->hv[i][j] = 0.;
            } else if (block->h[i][j] < 0.1)
                block->hu[i][j] = block->hv[i][j] = 0.; //no water, no speed!
        }
    }

}
void SWE_DimensionalSplittingChameleon::updateUnknowns (float dt) {
  /*  if (!allGhostlayersInSync()) return;
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
    return;*/
     if (!allGhostlayersInSync()) return;
//update cell averages with the net-updates

    chameleon_map_data_entry_t* args = new chameleon_map_data_entry_t[16];
    args[0] = chameleon_map_data_entry_create(this, sizeof(SWE_DimensionalSplittingChameleon), CHAM_OMP_TGT_MAPTYPE_TO);
    args[1] = chameleon_map_data_entry_create(&(this->maxTimestep), sizeof(float), CHAM_OMP_TGT_MAPTYPE_TO);
    args[2] = chameleon_map_data_entry_create(this->getWaterHeight().getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2),CHAM_OMP_TGT_MAPTYPE_TO);
    args[3] = chameleon_map_data_entry_create(this->hu.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[4] = chameleon_map_data_entry_create(this->hv.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);


    args[5] = chameleon_map_data_entry_create(this->hNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[6] = chameleon_map_data_entry_create(this->hNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[7] = chameleon_map_data_entry_create(this->hNetUpdatesBelow.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[8] = chameleon_map_data_entry_create(this->hNetUpdatesAbove.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);

    args[9] = chameleon_map_data_entry_create(this->huNetUpdatesLeft.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[10] = chameleon_map_data_entry_create(this->huNetUpdatesRight.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);

    args[11] = chameleon_map_data_entry_create(this->hvNetUpdatesBelow.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[12] = chameleon_map_data_entry_create(this->hvNetUpdatesAbove.getRawPointer(), sizeof(float)*(nx + 1)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_TO);
    args[13] = chameleon_map_data_entry_create(this->getWaterHeight().getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[14] = chameleon_map_data_entry_create(this->hu.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);
    args[15] = chameleon_map_data_entry_create(this->hv.getRawPointer(), sizeof(float)*(nx + 2)*(ny + 2), CHAM_OMP_TGT_MAPTYPE_FROM);

    cham_migratable_task_t *cur_task = chameleon_create_task(
            (void *)&updateUnkownsKernel,
            16, // number of args
            args);
    int32_t res = chameleon_add_task(cur_task);
}

