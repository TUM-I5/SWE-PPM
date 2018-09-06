/**
 * @file
 * This file is part of SWE.
 *
 * @author Jurek Olden (jurek.olden (AT) in.tum.de)
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
 * This file provides a possibility to use scenarios which are generated from arbitrary netCDF input files.
 * The scenario takes two file names, pointing to bathymetry and displacement of the scenario.
 * A caller may then query for bathymetry etc. at specific coordinates.
 * The core functionality is to map real-world coordinates to Float2D indices.
 * For instance, if a grid is defined for the domain [(1,1), (10,10)], with cell width and height of 1,
 * then a real-world coordinate of (2,2) would map to the indices [1][1].
 * Since we are looking at the distance from any point inside a rectangle to any corner of the rectangle,
 * we can simplify the problem to determining in which half of the rectangle the point is (vertically and horizontally)
 * Figuratively, if the point is in the top right quarter of the rectangle, the distance to the
 * top right corner will be shortest.
 *
 * Example (1D):
 *
 * |----|----|-x--|
 * 0    ↑    ↑ ↑
 *    origin | |
 *           | requested coordinate: 2.2
 *           |
 *         nearest index: 2
 *
 */

#ifndef __SWE_TSUNAMISCENARIO_HH
#define __SWE_TSUNAMISCENARIO_HH

#include <cmath>
#include <algorithm>
#include <assert.h>

#include "scenarios/SWE_Scenario.hh"
#include "blocks/SWE_Block.hh"
#include "reader/netCdfReader.hh"
#include "tools/Float2D.hh"

class SWE_TsunamiScenario : public SWE_Scenario {
	protected:
		InputGridSpecification bathymetryGrid;
		InputGridSpecification displacementGrid;

		/*
		 * The function mapping a 1D coordinate to an index w.r.t. to the original grid.
		 * In order to do this, the origin of the coordinate (which maps to the index 0)
		 * and the step size between two indices is needed (origin + i * width maps to index i).
		 *
		 * First, the offset of the point from the previous grid point is calculated
		 * and then used to determine whether the point is sitting closer to the grid point
		 * to its right or left (top or bottom).
		 *
		 * Then the grid point found to be closest is mapped to the corresponding index.
		 *
		 * @param position A 1D coordinate which is to be mapped to the index of the nearest data point w.r.t. to this dimension.
		 * @param origin The real-world origin w.r.t. to the dimension at hand.
		 * @param width The distance between two real-world data point along the dimension at hand.
		 */
		int getNearestIndex(float position, float origin, float width) {
			float offset = fmod(position, width);
			float nextGridPoint = position + width - offset;
			float previousGridPoint = position - offset;

			// Offset needed to shift the first grid point to index zero
			// The first grid point will sit in the middle of the first cell: origin + width / 2
			float firstPointOffset = (origin + width / 2) / width;

			int nextGridPointIndex = nextGridPoint / width - firstPointOffset;
			int previousGridPointIndex = previousGridPoint / width - firstPointOffset;
			int nearestIndex = (offset >= width / 2) ? nextGridPointIndex : previousGridPointIndex;
			return nearestIndex;
		}

		size_t nx;
		size_t ny;
		float dx;
		float dy;
		float originX;
		float originY;
		int simulatedTimesteps;
		float currentTime;

	public:
		SWE_TsunamiScenario() {}
		SWE_TsunamiScenario(const char* inputFileName, const char* displacementFileName) {
			bathymetryGrid = readNetCdf(inputFileName);
			displacementGrid = readNetCdf(displacementFileName);

			// Assert that the displacement grid is subset of the bathymetry grid,
			// other cases are undefined and do not make sense
			assert(bathymetryGrid.originX <= displacementGrid.originX + 0.0001);
			assert(bathymetryGrid.originY <= displacementGrid.originY + 0.0001);
			assert(bathymetryGrid.originX + bathymetryGrid.nx * bathymetryGrid.dx
					>= displacementGrid.originX + displacementGrid.nx * displacementGrid.dx - 0.0001);
			assert(bathymetryGrid.originY + bathymetryGrid.ny * bathymetryGrid.dy
					>= displacementGrid.originY + displacementGrid.ny * displacementGrid.dy - 0.0001);

			// Load metadata of the simulation domain to member variables
			originX = bathymetryGrid.originX;
			originY = bathymetryGrid.originY;
			nx = bathymetryGrid.nx;
			ny = bathymetryGrid.ny;
			dx = bathymetryGrid.dx;
			dy = bathymetryGrid.dy;
			simulatedTimesteps = 0;
			currentTime = 0.0;
		}

		~SWE_TsunamiScenario() {
		}

		/*
		 * This function returns the bathymetry corresponding to any point inside the simulation domain.
		 * In order to do this, it first determines the grid point nearest to the requested point and the
		 * Float2D index of this grid point.
		 * It then checks, if a displacement (e.g. earthquake induced) is defined for this point,
		 * if yes it also calculates the Float2D index corresponding to the array holding displacement data.
		 * Then, the actual bathymetry is calculated and returned.
		 *
		 * @param x X-coordinate of the requested point.
		 * @param y Y-coordinate of the requested point.
		 */
		virtual float getBathymetry(float x, float y) {
			// Make sure the requested coordinates are within the simulation domain.
			assert(x > originX - 0.0001);
			assert(x < nx * dx + originX + 0.0001);
			assert(y > originY - 0.0001);
			assert(y < ny * dy + originY + 0.0001);

			// Calculate the corresponding Float2D entry and get the corresponding bathymetry.
			int xIndex = getNearestIndex(x, bathymetryGrid.originX, bathymetryGrid.dx);
			int yIndex = getNearestIndex(y, bathymetryGrid.originY, bathymetryGrid.dy);
			float b = bathymetryGrid.var[xIndex][yIndex];
			// Check if displacement exists at the requested coordinate, and add the corresponding displacement if true.
			if (   x > displacementGrid.originX - 0.0001
			    && y > displacementGrid.originY - 0.0001
			    && x < displacementGrid.originX + displacementGrid.nx * displacementGrid.dx + 0.0001
			    && y < displacementGrid.originY + displacementGrid.ny * displacementGrid.dy + 0.0001) {
				int xIndex_displ = getNearestIndex(x, displacementGrid.originX, displacementGrid.dx);
				int yIndex_displ = getNearestIndex(y, displacementGrid.originY, displacementGrid.dy);
				b += displacementGrid.var[xIndex_displ][yIndex_displ];
			}
			return b;
		}

		/*
		 * This function returns the water height corresponding to any point inside the simulation domain.
		 * In order to do this, it first determines the grid point nearest to the requested point and the
		 * Float2D index of this grid point.
		 * It then returns either zero, if there is land mass at the requested point,
		 * or the negative bathymetry at this point, such that the effective water height is 0 (sea level by definition).
		 *
		 * @param x X-coordinate of the requested point.
		 * @param y Y-coordinate of the requested point.
		 *
		 */
		virtual float getWaterHeight(float x, float y) {
			// Make sure the requested coordinates are within the simulation domain
			assert(x > bathymetryGrid.originX - 0.0001);
			assert(x < bathymetryGrid.nx * bathymetryGrid.dx + bathymetryGrid.originX + 0.0001);
			assert(y > bathymetryGrid.originY - 0.0001);
			assert(y < bathymetryGrid.ny * bathymetryGrid.dy + bathymetryGrid.originY + 0.0001);

			int xIndex = getNearestIndex(x, bathymetryGrid.originX, bathymetryGrid.dx);
			int yIndex = getNearestIndex(y, bathymetryGrid.originY, bathymetryGrid.dy);
			float ret =  -std::fmin(0.0, bathymetryGrid.var[xIndex][yIndex]);
			return ret;
		}

		/*
		 * This function returns the x- or y-coordinate of a given boundary edge.
		 *
		 * @param edge One of BND_TOP, BND_BOTTOM, BND_LEFT, BND_RIGHT.
		 *
		 */
		float getBoundary(BoundaryEdge edge) {
			if (edge == BND_LEFT)
				return originX;
			else if (edge == BND_RIGHT)
				return originX + nx * dx;
			else if (edge == BND_BOTTOM)
				return originY;
			else
				return originY + ny + dy;
		};

		size_t getNx() { return nx; };
		size_t getNy() { return ny; };
		float getDx() { return dx; };
		float getDy() { return dy; } ;
		float getOriginX() { return originX; };
		float getOriginY() { return originY; } ;
		int getSimulatedTimesteps() { return simulatedTimesteps; }
		float getSimulationTime() { return currentTime; }
};
#endif // __SWE_TSUNAMISCENARIO_HH
