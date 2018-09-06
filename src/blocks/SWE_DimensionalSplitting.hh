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
#include "scenarios/SWE_TsunamiScenario.hh"
#include "tools/Float2D.hh"

#include <string>

#include "solvers/Hybrid.hpp"

class SWE_DimensionalSplitting: public SWE_Block {
	private:
		solver::Hybrid<float> solver;

		// Temporary values after x-sweep and before y-sweep
		Float2D hStar;
		Float2D huStar;

		// net updates per cell
		Float2D hNetUpdatesLeft;
		Float2D hNetUpdatesRight;

		Float2D huNetUpdatesLeft;
		Float2D huNetUpdatesRight;

		Float2D hNetUpdatesBelow;
		Float2D hNetUpdatesAbove;

		Float2D hvNetUpdatesBelow;
		Float2D hvNetUpdatesAbove;

	public:
		// Constructor: Initialize data needed for the simulation according to a preset scenario
		SWE_DimensionalSplitting(int l_nx, int l_ny, float dx, float dy);

		void initScenarioImplicit(SWE_Scenario &scenario);

		// Computes net updates on the entire simulation domain
		void computeNumericalFluxes();

		// Update the cells in the simulation domain w.r.t. to the net updates and some timestep
		void updateUnknowns(float dt);
		void updateUnknownsRow(float dt, int i);

		virtual ~SWE_DimensionalSplitting() {}
		};


#endif /* SWEDIMENSIONALSPLITTING_HH_ */
