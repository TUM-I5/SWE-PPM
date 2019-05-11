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

#ifndef SWEDIMENSIONALSPLITTINGMPI_HH_
#define SWEDIMENSIONALSPLITTINGMPI_HH_

#include <limits.h>
#include <ctime>
#include <time.h>
#include "blocks/SWE_Block.hh"
#include "scenarios/SWE_Scenario.hh"
#include "tools/Float2DNative.hh"

#include <mpi.h>
#include "solvers/HLLEFun.hpp"
#include "solvers/Hybrid.hpp"

class SWE_DimensionalSplittingMpi : public SWE_Block<Float2DNative> {
	public:
		// Constructor/Destructor
		SWE_DimensionalSplittingMpi(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal, float cellSizeVertical, float originX, float originY);
		~SWE_DimensionalSplittingMpi() {};

		// Interface methods
		void setGhostLayer();
		void connectBoundaries(Boundary boundary, SWE_Block &neighbour, Boundary neighbourBoundary);
		void computeNumericalFluxes();
		void updateUnknowns(float dt);

		// Mpi specific
		void freeMpiType();
		void connectNeighbours(int neighbourRankId[]);
		void exchangeBathymetry();

		float computeTime;
		float computeTimeWall;
        uint64_t getFlops();
	private:
        solver::HLLEFun<float> solver;

		// Max timestep reduced over all upcxx ranks
		float maxTimestepGlobal;

		// Temporary values after x-sweep and before y-sweep
		Float2DNative hStar;
		Float2DNative huStar;

		// net updates per cell
		Float2DNative hNetUpdatesLeft;
		Float2DNative hNetUpdatesRight;

		Float2DNative huNetUpdatesLeft;
		Float2DNative huNetUpdatesRight;

		Float2DNative hNetUpdatesBelow;
		Float2DNative hNetUpdatesAbove;

		Float2DNative hvNetUpdatesBelow;
		Float2DNative hvNetUpdatesAbove;

		/* Copy buffer:
		 * Since Float2D are stored column-wise in memory,
		 * it is expensive to read rows from a Float2D since it is necessary to stride an entire column after each read element.
		 * Therefore we duplicate elements belonging to the bottom/top copy layers into a copy buffer when updateUnkowns() runs.
		 */
		//float* topCopyBuffer[nx];
		//float* bottomCopyBuffer[nx];

		// Neighbouring block rank ids, indexed by Boundary
		int neighbourRankId[4];

		// Custom data types for bottom/top border which are requrired due to the stride
		MPI_Datatype HORIZONTAL_BOUNDARY;

		// timer
		std::clock_t computeClock;
		struct timespec startTime;
		struct timespec endTime;
};
#endif /* SWEDIMENSIONALSPLITTINGMPI_HH_ */
