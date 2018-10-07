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

#ifndef SWEDIMENSIONALSPLITTING_HH_
#define SWEDIMENSIONALSPLITTING_HH_

#include "blocks/SWE_Block.hh"
#include "scenarios/SWE_Scenario.hh"
#include "tools/Float2DNative.hh"
#include <ctime>
#include <time.h>

#include "solvers/Hybrid.hpp"

class SWE_DimensionalSplitting : public SWE_Block<Float2DNative> {
	public:
		// Constructor/Destructor
		SWE_DimensionalSplitting(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal, float cellSizeVertical, float originX, float originY);
		~SWE_DimensionalSplitting() {};

		// Interface methods
		void setGhostLayer();
		void computeNumericalFluxes();
		void updateUnknowns(float dt);

		float computeTime;
		float computeTimeWall;

	private:
		solver::Hybrid<float> solver;

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

		// timer
		std::clock_t computeClock;
		struct timespec startTime;
		struct timespec endTime;
};
#endif /* SWEDIMENSIONALSPLITTING_HH_ */
