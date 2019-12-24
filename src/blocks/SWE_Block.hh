/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema, Tobias Schnabel
 * @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
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
 * SWE_Block is the main data structure to compute our shallow water model
 * on a single Cartesian grid block:
 * SWE_Block is an abstract class (and interface) that should be extended
 * by respective implementation classes.
 *
 * <h3>Cartesian Grid for Discretization:</h3>
 *
 * SWE_Blocks uses a regular Cartesian grid of size #nx by #ny, where each
 * grid cell carries three unknowns:
 * - the water level #h
 * - the momentum components #hu and #hv (in x- and y- direction, resp.)
 * - the bathymetry #b
 *
 * Each of the components is stored as a 2D array, implemented as a Float2D object,
 * and are defined on grid indices [0,..,#nx+1]*[0,..,#ny+1].
 * The computational domain is indexed with [1,..,#nx]*[1,..,#ny].
 *
 * The mesh sizes of the grid in x- and y-direction are stored in static variables
 * #dx and #dy. The position of the Cartesian grid in space is stored via the
 * coordinates of the left-bottom corner of the grid, in the variables
 * #offsetX and #offsetY.
 *
 * <h3>Ghost layers:</h3>
 *
 * To implement the behaviour of the fluid at boundaries and for using
 * multiple block in serial and parallel settings, SWE_Block adds an
 * additional layer of so-called ghost cells to the Cartesian grid,
 * as illustrated in the following figure.
 * Cells in the ghost layer have indices 0 or #nx+1 / #ny+1.
 *
 * \image html ghost_cells.gif
 *
 * <h3>Memory Model:</h3>
 *
 * The variables #h, #hu, #hv for water height and momentum will typically be
 * updated by classes derived from SWE_Block. However, it is not assumed that
 * such and updated will be performed in every time step.
 * Instead, subclasses are welcome to update #h, #hu, and #hv in a lazy fashion,
 * and keep data in faster memory (incl. local memory of acceleration hardware,
 * such as GPGPUs), instead.
 *
 * It is assumed that the bathymetry data #b is not changed during the algorithm
 * (up to the exceptions mentioned in the following).
 *
 * To force a synchronization of the respective data structures, the following
 * methods are provided as part of SWE_Block:
 * - synchAfterWrite() to synchronize #h, #hu, #hv, and #b after an external update
 *   (reading a file, e.g.);
 * - synchWaterHeightAfterWrite(), synchDischargeAfterWrite(), synchBathymetryAfterWrite():
 *   to synchronize only #h or momentum (#hu and #hv) or bathymetry #b;
 * - synchGhostLayerAfterWrite() to synchronize only the ghost layers
 * - synchBeforeRead() to synchronize #h, #hu, #hv, and #b before an output of the
 *   variables (writing a visualization file, e.g.)
 * - synchWaterHeightBeforeRead(), synchDischargeBeforeRead(), synchBathymetryBeforeRead():
 *   as synchBeforeRead(), but only for the specified variables
 * - synchCopyLayerBeforeRead(): synchronizes the copy layer only (i.e., a layer that
 *   is to be replicated in a neighbouring SWE_Block.
 *
 * <h3>Derived Classes</h3>
 *
 * As SWE_Block just provides an abstract base class together with the most
 * important data structures, the implementation of concrete models is the
 * job of respective derived classes (see the class diagram at the top of this
 * page). Similar, parallel implementations that are based on a specific
 * parallel programming model (such as OpenMP) or parallel architecture
 * (such as GPU/CUDA) should form subclasses of their own.
 * Please refer to the documentation of these classes for more details on the
 * model and on the parallelisation approach.
 */

#ifndef __SWE_BLOCK_HH
#define __SWE_BLOCK_HH

#include "scenarios/SWE_Scenario.hh"
#include "types/Boundary.hh"
#include "Constants.hh"
#include <cassert>
#include <cstring>
#include <cmath>
#include <limits>
#include <algorithm>
#include "tools/Float2DBuffer.hh"
#include <iostream>
template <typename T, typename Buffer = Float2DBuffer>
class SWE_Block {
	public:
		// Default getter methods
		int getCellCountHorizontal();
		int getCellCountVertical();
		float getCellSizeHorizontal();
		float getCellSizeVertical();
		int getOriginX();
		int getOriginY();
		float getMaxTimestep();
		const T& getWaterHeight();
        T& getModifiableWaterHeight();
		const T& getMomentumHorizontal();
        T& getModifiableMomentumHorizontal();
		const T& getMomentumVertical();
        T& getModifiableMomentumVertical();
		const T& getBathymetry();
        T& getModifiableBathymetry();

        //Methods for local timestepping
        void checkAllGhostlayers();
        float getRoundTimestep(float timestep);
        void setMaxGlobalTimestep(float timestep);
        void interpolateGhostlayer(Boundary border, float remoteTimestep);
        float interpolateValue(float oldval, float newval, float remoteTimestep);
        float getTotalLocalTimestep();

        void setMaxLocalTimestep(float timestep);
        bool hasMaxLocalTimestep();
        bool isReceivable(Boundary border);
        bool isSendable(Boundary border);
        bool allGhostlayersInSync();

		// Default setter methods
		virtual void setBoundaryType(Boundary boundary, BoundaryType type);

		// Default methods
		virtual void initScenario(SWE_Scenario &scenario, BoundaryType boundaries[]);
		virtual void computeMaxTimestep(const float dryTol = defaultDryTol, const float cflNumber = defaultCflNumber);
		
	//protected:
		// Constructor/Destructor
		SWE_Block<T, Buffer >();
		SWE_Block<T , Buffer >(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal, float cellSizeVertical, float originX = 0, float originY = 0, bool localTimestepping = false);
		virtual ~SWE_Block() = 0;

		// Default methods
		// TODO: Who sets boundaries how? Init ghost layers?
		virtual void applyBoundaryBathymetry();
		virtual void applyBoundaryConditions();

		// Interface methods without a default implementation
		//virtual void setGhostLayer() = 0;
		// TODO is this needed (computeMaxTimeStep)
		virtual void computeNumericalFluxes() = 0;
		virtual void updateUnknowns(float dt) = 0;

		// Grid size (incl. ghost layer)
		int nx;
		int ny;

		// Grid cell width and height
		float dx;
		float dy;

		// Position of the block in the domain
		float originX;	///< x-coordinate of the origin (left-bottom corner) of the Cartesian grid
		float originY;	///< y-coordinate of the origin (left-bottom corner) of the Cartesian grid

		// maximum time step allowed to ensure stability of the method
		// it may be updated as part of the method computeNumericalFluxes()
		// or updateUnknowns() (depending on the numerical method)
		float maxTimestep;
        float maxTimestepLocal; // used for local timestepping
        int maxDivisor = 128; //smallest timestep possible maxTimestepLocal/maxDivisor
        bool localTimestepping; //true to activate localtimestepping
        int stepSize; //is used to determine the localstepsize;
        int stepSizeCounter; //used to count the steps;
        GhostlayerState receivedGhostlayer[4]; //determines if border received a valid timestep, thus is bigger or same then localtimestep.
        float borderTimestep[4]; //timesteps of each border are put in here
        int timestepCounter =0;
        // Unknowns
		T h;
		T hu;
		T hv;
		T b;

		Buffer bufferH; //buffer structure which are necessary for localtimestepping
		Buffer bufferHu; // if localTimestepping == false  buffers point to h,hu,hv
		Buffer bufferHv;
		// Boundary type at the block edges (uses Boundary as index)
		BoundaryType boundaryType[4];

};

/***************************
 * Default Implementations *
 ***************************/

/**
 * Constructor: allocate variables for simulation
 *
 * unknowns h (water height), hu,hv (discharge in x- and y-direction),
 * and b (bathymetry) are defined on grid indices [0,..,nx+1]*[0,..,ny+1]
 * -> computational domain is [1,..,nx]*[1,..,ny]
 * -> plus ghost cell layer
 */
template <typename T, typename Buffer>
SWE_Block <T, Buffer> ::SWE_Block() {
}

template <typename T, typename Buffer>
SWE_Block <T, Buffer>::SWE_Block(int nx, int ny, float dx, float dy, float originX, float originY,bool localTimestepping) :
		nx(nx),
		ny(ny),
		dx(dx),
		dy(dy),
		originX(originX),
		originY(originY),
		h(nx + 2, ny + 2),
		hu(nx + 2, ny + 2),
		hv(nx + 2, ny + 2),
		b(nx + 2, ny + 2),
		localTimestepping(localTimestepping),
		bufferH(nx+2, ny+2, localTimestepping, h),
        bufferHu(nx+2, ny+2, localTimestepping, hu),
        bufferHv(nx+2, ny+2, localTimestepping, hv){
	// initialise boundaries
	for (int i = 0; i < 4; i++) {
		boundaryType[i] = PASSIVE;
		receivedGhostlayer[i] = GL_NEXT;
	}


	stepSizeCounter = 0;
	stepSize = 1;
}

template <typename T, typename Buffer>
SWE_Block< T, Buffer>::~SWE_Block() {
}
template <typename T, typename Buffer>
bool SWE_Block <T, Buffer>::hasMaxLocalTimestep(){
    // If block is at the biggest Timestep and received all ghostlayers, then it is ready to write timestep
   if(stepSizeCounter == 1 && allGhostlayersInSync()){
        timestepCounter++;
        return true;
    }
    if(stepSizeCounter > stepSize  && allGhostlayersInSync()) {
        stepSizeCounter = 0;
    }
    return false;
    /*if(stepSizeCounter > stepSize  && allGhostlayersInSync()){
        stepSizeCounter = 0;

        return true;
    }else{
        return false;
    }*/
}

template <typename T, typename Buffer>
bool SWE_Block< T, Buffer>::allGhostlayersInSync() {
    return receivedGhostlayer[BND_LEFT] != GL_UNVALID && receivedGhostlayer[BND_RIGHT] != GL_UNVALID
        && receivedGhostlayer[BND_TOP] != GL_UNVALID && receivedGhostlayer[BND_BOTTOM] != GL_UNVALID;
}
template <typename T, typename Buffer>
bool SWE_Block<T, Buffer>::isSendable(Boundary border){
    return receivedGhostlayer[border] != GL_UNVALID;
}
template <typename T, typename Buffer>
bool SWE_Block< T, Buffer>::isReceivable(Boundary border){
    return receivedGhostlayer[border] == GL_NEXT || receivedGhostlayer[border] == GL_UNVALID;
}
template <typename T, typename Buffer>
float SWE_Block< T, Buffer>::interpolateValue(float oldval, float newval, float remoteTimestep){
    return oldval + (newval-oldval)*(getTotalLocalTimestep()/remoteTimestep);
}
template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::checkAllGhostlayers() {
    if(localTimestepping){

        //if neighbour stepped further than the local block, interpolation is possible, if not, block shall wait for fitting timestep
        for(int border = BND_LEFT; border <= BND_TOP; border++){
            if((boundaryType[border] == CONNECT )|| boundaryType[border] == CONNECT_WITHIN_RANK) {

                if (borderTimestep[border] == getTotalLocalTimestep()){
                    //This case the neighbor progressed as much as local block did and thus the received Ghostlayer is valid and we can expect
                    // a new timestep incoming in the next iteration.
                    interpolateGhostlayer(static_cast<Boundary>(border), borderTimestep[border]);
                    receivedGhostlayer[border] = GL_SYNC;
                }else if ((borderTimestep[border] > getTotalLocalTimestep()) && stepSizeCounter != 0 ){
                    //This case we need to interpolate and thus do not expect a new timestep in the next iteration.
                    interpolateGhostlayer(static_cast<Boundary>(border), borderTimestep[border]);
                    receivedGhostlayer[border] = GL_INTER;
                }else {
                    //in this case we need to wait for the next iteration to receive a valid timestep, block will not compute until received a valid timestep.
                    receivedGhostlayer[border] = GL_UNVALID;

                }

            }

        }

        if(allGhostlayersInSync()){
            //If it is interpolated we do need to receive next timestep
            receivedGhostlayer[BND_LEFT]    = (receivedGhostlayer[BND_LEFT]  == GL_INTER)?GL_INTER:GL_NEXT;
            receivedGhostlayer[BND_RIGHT]   = (receivedGhostlayer[BND_RIGHT]  == GL_INTER)?GL_INTER:GL_NEXT;
            receivedGhostlayer[BND_TOP]     = (receivedGhostlayer[BND_TOP]  == GL_INTER)?GL_INTER:GL_NEXT;
            receivedGhostlayer[BND_BOTTOM]  = (receivedGhostlayer[BND_BOTTOM]  == GL_INTER)?GL_INTER:GL_NEXT;
        }
    }
}
template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::interpolateGhostlayer(Boundary border,float remoteTimestep){

    switch(border){
        case BND_LEFT:
            for(int i = 1; i < ny+2; i++) {
                h[0][i] = interpolateValue(h[0][i],bufferH[0][i], remoteTimestep);
                hu[0][i] = interpolateValue(hu[0][i],bufferHu[0][i], remoteTimestep);
                hv[0][i] = interpolateValue(hv[0][i],bufferHv[0][i], remoteTimestep);
            }
            break;
        case BND_RIGHT:
            for(int i = 1; i < ny+2; i++) {
                h[nx+1][i] = interpolateValue(h[nx+1][i],bufferH[nx+1][i], remoteTimestep);
                hu[nx+1][i] = interpolateValue(hu[nx+1][i],bufferHu[nx+1][i], remoteTimestep);
                hv[nx+1][i] = interpolateValue(hv[nx+1][i],bufferHv[nx+1][i], remoteTimestep);
            }
            break;
        case BND_BOTTOM:
            for(int i = 1; i < nx+2; i++) {
                h[i][0] = interpolateValue(h[i][0],bufferH[i][0], remoteTimestep);
                hu[i][0] = interpolateValue(hu[i][0],bufferHu[i][0], remoteTimestep);
                hv[i][0] = interpolateValue(hv[i][0],bufferHv[i][0], remoteTimestep);
            }
            break;
        case BND_TOP:
            for(int i = 1; i < nx+2; i++) {
                h[i][ny+1] = interpolateValue(h[i][ny+1],bufferH[i][ny+1], remoteTimestep);
                hu[i][ny+1] = interpolateValue(hu[i][ny+1],bufferHu[i][ny+1], remoteTimestep);
                hv[i][ny+1] = interpolateValue(hv[i][ny+1],bufferHv[i][ny+1], remoteTimestep);
            }
            break;
    }

}
template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::setMaxLocalTimestep(float timestep){
    maxTimestepLocal = timestep;
}
template <typename T, typename Buffer>
float SWE_Block< T, Buffer>::getRoundTimestep(float timestep){
    //stepSizeCounter = stepSizeCounter%(stepSize);

    if(stepSizeCounter <= 0){
        //the timestep just started, determine stepsize
        //std::cout << "Calculated timestep :" << timestep << std::endl;
        float diff = maxTimestepLocal;
        for(int i = 1 ; i < maxDivisor ; i*=2){
            float newDiff = std::abs(maxTimestepLocal/i - timestep);
            if(diff > newDiff){
                diff = newDiff;
                stepSize = i;
            }
        }
    }

    stepSizeCounter ++;
    return (float)maxTimestepLocal/stepSize;

}
template <typename T, typename Buffer>
float SWE_Block< T, Buffer>::getTotalLocalTimestep(){
    //std::cout << "stepsize " << stepSize << std::endl;
    return (float)(maxTimestepLocal*stepSizeCounter)/stepSize;
}
template <typename T, typename Buffer>
int SWE_Block< T, Buffer>::getCellCountHorizontal() {
	return nx;
}

template <typename T, typename Buffer>
int SWE_Block< T, Buffer>::getCellCountVertical() {
	return ny;
}

template <typename T, typename Buffer>
float SWE_Block< T, Buffer>::getCellSizeHorizontal() {
	return dx;
}

template <typename T, typename Buffer>
float SWE_Block< T, Buffer>::getCellSizeVertical() {
	return dy;
}

template <typename T, typename Buffer>
int SWE_Block< T, Buffer>::getOriginX() {
	return originX;
}

template <typename T, typename Buffer>
int SWE_Block< T, Buffer>::getOriginY() {
	return originY;
}

template <typename T, typename Buffer>
float SWE_Block< T, Buffer>::getMaxTimestep() {
	return maxTimestep;
}

template <typename T, typename Buffer>
const T& SWE_Block< T, Buffer>::getWaterHeight() {
	return h;
}

template <typename T, typename Buffer>
const T& SWE_Block< T, Buffer>::getMomentumHorizontal() {
	return hu;
}
template <typename T, typename Buffer>
T& SWE_Block< T, Buffer>::getModifiableWaterHeight() {
    return h;
}
template <typename T, typename Buffer>
T& SWE_Block< T, Buffer>::getModifiableMomentumHorizontal() {
    return hu;
}
template <typename T, typename Buffer>
T& SWE_Block< T, Buffer>::getModifiableBathymetry() {
    return b;
}
template <typename T, typename Buffer>
T& SWE_Block< T, Buffer>::getModifiableMomentumVertical() {
    return hv;
}
template <typename T, typename Buffer>
const T& SWE_Block< T, Buffer>::getMomentumVertical() {
	return hv;
}

template <typename T, typename Buffer>
const T& SWE_Block< T, Buffer>::getBathymetry() {
	return b;
}

template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::setBoundaryType(Boundary boundary, BoundaryType type) {
	boundaryType[boundary] = type;
}

/**
 * Initializes the unknowns and bathymetry in all grid cells according to the given SWE_Scenario.
 *
 * @param scenario scenario to use during the setup.
 * @param boundaries array containing the boundary types surrounding the current block
 */
template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::initScenario(SWE_Scenario &scenario, BoundaryType boundaries[]) {
	float x = 0;
	float y = 0;
	for (int i = 1; i < ny + 1; i++) {
		for (int j = 1; j < nx + 1; j++) {
			/*
			 * Map the indices to actual points, shift by one because the ghost layer
			 * is inserted at indices [0][*], [*][0], [nx + 1][*], [*][ny + 1].
			 * Therefore, index [1][1], not [0][0], has to map to (originX, originY).
			 *
			 * Offset by 1/2 to query the value at the center of the current cell
			 *
			 * I.e.: If the origin is at 0,0 and the cell width is 1,
			 * array index [1][1] will map to the values at 0.5,0.5 ,
			 * array index [2][2] will map to 1.5,1.5 and so forth.
			 */
			x = (float) originX + (j - 0.5) * dx;
			y = (float) originY + (i - 0.5) * dy;
			b[j][i] = scenario.getBathymetry(x, y);
			h[j][i] = scenario.getWaterHeight(x, y);
			hu[j][i] = scenario.getVeloc_u(x, y) * h[j][i];
			hv[j][i] = scenario.getVeloc_v(x, y) * h[j][i];
		}
	}

	for (int i = 0; i < 4; i++) {
		boundaryType[i] = boundaries[i];
	}

	applyBoundaryConditions();
	applyBoundaryBathymetry();
}

/**
 * Compute the largest allowed time step for the current grid block
 * (reference implementation) depending on the current values of
 * variables h, hu, and hv, and store this time step size in member
 * variable maxTimestep.
 *
 * @param i_dryTol dry tolerance (dry cells do not affect the time step).
 * @param i_cflNumber CFL number of the used method.
 */
template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::computeMaxTimestep( const float dryTol, const float cflNumber) {
	// initialize the maximum wave speed
	float maximumWaveSpeed = (float) 0;

	// compute the maximum wave speed within the grid
	for(int i = 1; i < nx + 1; i++) {
		for(int j = 1; j < ny + 1; j++) {
			if(h[i][j] > dryTol) {
				float momentum = std::max(std::abs(hu[i][j]), std::abs(hv[i][j]));
				float particleVelocity = momentum / h[i][j];

				// approximate the wave speed
				float waveSpeed = particleVelocity + std::sqrt( g * h[i][j] );
				maximumWaveSpeed = std::max(maximumWaveSpeed, waveSpeed );
			}
		}
	}

	// set the maximum time step variable
	maxTimestep = std::min(dx, dy) / maximumWaveSpeed;

	// apply the CFL condition
	maxTimestep *= cflNumber;
}

/**
 * Sets the bathymetry on OUTFLOW or WALL boundaries.
 * Should be called every time a boundary is changed to a OUTFLOW or
 * WALL boundary <b>or</b> the bathymetry changes.
 */
template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::applyBoundaryBathymetry() {
	// set bathymetry values in the ghost layer if necessary
	if(boundaryType[BND_LEFT] == OUTFLOW || boundaryType[BND_LEFT] == WALL) {
		memcpy(b[0], b[1], sizeof(float) * (ny + 2));
	}
	if(boundaryType[BND_RIGHT] == OUTFLOW || boundaryType[BND_RIGHT] == WALL) {
		memcpy(b[nx+1], b[nx], sizeof(float) * (ny + 2));
	}
	if(boundaryType[BND_BOTTOM] == OUTFLOW || boundaryType[BND_BOTTOM] == WALL) {
		for(int i = 0; i <= nx + 1; i++) {
			b[i][0] = b[i][1];
		}
	}
	if(boundaryType[BND_TOP] == OUTFLOW || boundaryType[BND_TOP] == WALL) {
		for(int i = 0; i <= nx + 1; i++) {
			b[i][ny+1] = b[i][ny];
		}
	}

	// set corner values
	b[0][0] = b[1][1];
	b[0][ny+1] = b[1][ny];
	b[nx+1][0] = b[nx][1];
	b[nx+1][ny+1] = b[nx][ny];
}

/**
 * set the values of all ghost cells depending on the specifed
 * boundary conditions
 * - set boundary conditions for typs WALL and OUTFLOW
 * - derived classes need to transfer ghost layers
 */
template <typename T, typename Buffer>
void SWE_Block< T, Buffer>::applyBoundaryConditions() {
	// CONNECT boundary conditions are set in the calling function setGhostLayer
	// PASSIVE boundary conditions need to be set by the component using SWE_Block

	// left boundary
	switch(boundaryType[BND_LEFT]) {
		case WALL:
			{
				for(int j = 1; j <= ny; j++) {
					h[0][j] = h[1][j];
					hu[0][j] = -hu[1][j];
					hv[0][j] = hv[1][j];
				};
				break;
			}
		case OUTFLOW:
			{
				for(int j = 1; j <= ny; j++) {
					h[0][j] = h[1][j];
					hu[0][j] = hu[1][j];
					hv[0][j] = hv[1][j];
				};
				break;
			}
		case CONNECT:
        case CONNECT_WITHIN_RANK:
		case PASSIVE:
			break;
		default:
			assert(false);
			break;
	};

	// right boundary
	switch(boundaryType[BND_RIGHT]) {
		case WALL:
			{
				for(int j = 1; j <= ny; j++) {
					h[nx+1][j] = h[nx][j];
					hu[nx+1][j] = -hu[nx][j];
					hv[nx+1][j] = hv[nx][j];
				};
				break;
			}
		case OUTFLOW:
			{
				for(int j = 1; j <= ny; j++) {
					h[nx+1][j] = h[nx][j];
					hu[nx+1][j] = hu[nx][j];
					hv[nx+1][j] = hv[nx][j];
				};
				break;
			}
		case CONNECT:
        case CONNECT_WITHIN_RANK:
		case PASSIVE:
			break;
		default:
			assert(false);
			break;
	};

	// bottom boundary
	switch(boundaryType[BND_BOTTOM]) {
		case WALL:
			{
				for(int i = 1; i <= nx; i++) {
					h[i][0] = h[i][1];
					hu[i][0] = hu[i][1];
					hv[i][0] = -hv[i][1];
				};
				break;
			}
		case OUTFLOW:
			{
				for(int i = 1; i <= nx; i++) {
					h[i][0] = h[i][1];
					hu[i][0] = hu[i][1];
					hv[i][0] = hv[i][1];
				};
				break;
			}
		case CONNECT:
        case CONNECT_WITHIN_RANK:
		case PASSIVE:
			break;
		default:
			assert(false);
			break;
	};

	// top boundary
	switch(boundaryType[BND_TOP]) {
		case WALL:
			{
				for(int i = 1; i <= nx; i++) {
					h[i][ny+1] = h[i][ny];
					hu[i][ny+1] = hu[i][ny];
					hv[i][ny+1] = -hv[i][ny];
				};
				break;
			}
		case OUTFLOW:
			{
				for(int i = 1; i <= nx; i++) {
					h[i][ny+1] = h[i][ny];
					hu[i][ny+1] = hu[i][ny];
					hv[i][ny+1] = hv[i][ny];
				};
				break;
			}
		case CONNECT:
        case CONNECT_WITHIN_RANK:
		case PASSIVE:
			break;
		default:
			assert(false);
			break;
	};

	/*
	 * Set values in corner ghost cells. Required for dimensional splitting and visualization.
	 *   The quantities in the corner ghost cells are chosen to generate a zero Riemann solutions
	 *   (steady state) with the neighboring cells. For the lower left corner (0,0) using
	 *   the values of (1,1) generates a steady state (zero) Riemann problem for (0,0) - (0,1) and
	 *   (0,0) - (1,0) for both outflow and reflecting boundary conditions.
	 *
	 *   Remark: Unsplit methods don't need corner values.
	 *
	 * Sketch (reflecting boundary conditions, lower left corner):
	 * <pre>
	 *                  **************************
	 *                  *  _    _    *  _    _   *
	 *  Ghost           * |  h   |   * |  h   |  *
	 *  cell    ------> * | -hu  |   * |  hu  |  * <------ Cell (1,1) inside the domain
	 *  (0,1)           * |_ hv _|   * |_ hv _|  *
	 *                  *            *           *
	 *                  **************************
	 *                  *  _    _    *  _    _   *
	 *   Corner Ghost   * |  h   |   * |  h   |  *
	 *   cell   ------> * |  hu  |   * |  hu  |  * <----- Ghost cell (1,0)
	 *   (0,0)          * |_ hv _|   * |_-hv _|  *
	 *                  *            *           *
	 *                  **************************
	 * </pre>
	 */
	h [0][0] = h [1][1];
	hu[0][0] = hu[1][1];
	hv[0][0] = hv[1][1];

	h [0][ny+1] = h [1][ny];
	hu[0][ny+1] = hu[1][ny];
	hv[0][ny+1] = hv[1][ny];

	h [nx+1][0] = h [nx][1];
	hu[nx+1][0] = hu[nx][1];
	hv[nx+1][0] = hv[nx][1];

	h [nx+1][ny+1] = h [nx][ny];
	hu[nx+1][ny+1] = hu[nx][ny];
	hv[nx+1][ny+1] = hv[nx][ny];
}
#endif // __SWE_BLOCK_HH
