/**
 * @file
 * This file is part of SWE.
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
 * Implementation of the SWE_Block abstract class that uses dimensional splitting.
 * It extends the computational domain to two dimensions by decomposing 2D updates
 * to updates on the x- and y-axis.
 *
 */

#ifndef SWEDIMENSIONALSPLITTINGUPCXX_HH_
#define SWEDIMENSIONALSPLITTINGUPCXX_HH_

#include "blocks/SWE_Block.hh"
#include "scenarios/SWE_Scenario.hh"
#include "tools/Float2DUpcxx.hh"
#include "types/BlockConnectInterface.hh"
#include <ctime>
#include <time.h>
#include "tools/Float2DBufferUpcxx.hh"
#include "tools/CollectorUpcxx.hpp"
#include <upcxx/upcxx.hpp>

#if WAVE_PROPAGATION_SOLVER==0
//#include "solvers/Hybrid.hpp"
#include "solvers/HLLEFun.hpp"
#elif WAVE_PROPAGATION_SOLVER==1
#include "solvers/FWave.hpp"
#elif WAVE_PROPAGATION_SOLVER==2
#include "solvers/AugRie.hpp"
#endif

class SWE_DimensionalSplittingUpcxx : public SWE_Block<Float2DUpcxx,Float2DBufferUpcxx> {
	public:
		// Constructor/Destructor
        SWE_DimensionalSplittingUpcxx();
		SWE_DimensionalSplittingUpcxx(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal, float cellSizeVertical, float originX, float originY, bool localTimestepping=false);
		~SWE_DimensionalSplittingUpcxx() {};

		// Interface methods
		void setGhostLayer();
		void connectBoundaries(Boundary boundary, SWE_Block &neighbour, Boundary neighbourBoundary);
		void computeNumericalFluxes();
		void updateUnknowns(float dt);
        void notifyNeighbours(bool sync);
		// Upcxx specific
		void connectBoundaries(BlockConnectInterface<upcxx::global_ptr<float>> neighbourCopyLayer[]);
		BlockConnectInterface<upcxx::global_ptr<float>> getCopyLayer(Boundary boundary);
		void exchangeBathymetry();


        int iteration = 0;
	//private:
#if WAVE_PROPAGATION_SOLVER==0
    //! Hybrid solver (f-wave + augmented)
    //solver::Hybrid<float> solver;
    solver::HLLEFun<float> solver;
#elif WAVE_PROPAGATION_SOLVER==1
    //! F-wave Riemann solver
    solver::FWave<float> solver;
#elif WAVE_PROPAGATION_SOLVER==2
    //! Approximate Augmented Riemann solver
    solver::AugRie<float> solver;
#endif

		// Max timestep reduced over all upcxx ranks
		float maxTimestepGlobal;

		// Temporary values after x-sweep and before y-sweep
		Float2DUpcxx hStar;
		Float2DUpcxx huStar;

		// net updates per cell
		Float2DUpcxx hNetUpdatesLeft;
		Float2DUpcxx hNetUpdatesRight;

		Float2DUpcxx huNetUpdatesLeft;
		Float2DUpcxx huNetUpdatesRight;

		Float2DUpcxx hNetUpdatesBelow;
		Float2DUpcxx hNetUpdatesAbove;

		Float2DUpcxx hvNetUpdatesBelow;
		Float2DUpcxx hvNetUpdatesAbove;

		// Interfaces to neighbouring block copy layers, indexed by Boundary
		BlockConnectInterface<upcxx::global_ptr<float>> neighbourCopyLayer[4];
        //Used to transmit timestep in localtimestepping
        upcxx::global_ptr<float> upcxxLocalTimestep;
        upcxx::global_ptr<std::atomic<bool>> upcxxDataReady;
        upcxx::global_ptr<std::atomic<bool>> upcxxDataTransmitted;
        upcxx::global_ptr<int> upcxxIteration;
        std::atomic<bool> *dataReady;
        std::atomic<bool> *dataTransmitted;
        float * upcxxBorderTimestep;
		// timer

};
#endif /* SWEDIMENSIONALSPLITTINGUPCXX_HH_ */
