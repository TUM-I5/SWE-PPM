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

#include <upcxx/upcxx.hpp>

#include "solvers/Hybrid.hpp"
#include "solvers/HLLEFun.hpp"
class SWE_DimensionalSplittingUpcxx : public SWE_Block<Float2DUpcxx> {
	public:
		// Constructor/Destructor
		SWE_DimensionalSplittingUpcxx(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal, float cellSizeVertical, float originX, float originY);
		~SWE_DimensionalSplittingUpcxx() {};

		// Interface methods
		void setGhostLayer();
		void connectBoundaries(Boundary boundary, SWE_Block &neighbour, Boundary neighbourBoundary);
		void computeNumericalFluxes();
		void updateUnknowns(float dt);

		// Upcxx specific
		void connectBoundaries(BlockConnectInterface<upcxx::global_ptr<float>> neighbourCopyLayer[]);
		BlockConnectInterface<upcxx::global_ptr<float>> getCopyLayer(Boundary boundary);
		void exchangeBathymetry();

		float computeTime;
		float computeTimeWall;
        uint64_t getFlops();
	private:
		solver::HLLEFun<float> solver;

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

		// timer
		std::clock_t computeClock;
		struct timespec startTime;
		struct timespec endTime;
};
#endif /* SWEDIMENSIONALSPLITTINGUPCXX_HH_ */
