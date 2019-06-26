/**
 * @file
 * This file is part of SWE.
 *
 * @author Alexander Breuer (breuera AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer)
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
 
 * Access to bathymetry and displacement files with ASAGI.
 */

#ifndef __SWE_ASAGISCENARIO_HH
#define __SWE_ASAGISCENARIO_HH

#include <cassert>
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <asagi.h>
#include "SWE_Scenario.hh"

using namespace asagi;

class SWE_AsagiScenario: public SWE_Scenario {
	public:
		SWE_AsagiScenario(
				const std::string bathymetryFilename,
				const std::string displacementFilename) {

			bathymetryGrid = Grid::create();
			displacementGrid = Grid::create();

			if(bathymetryGrid->open(bathymetryFilename.c_str()) != Grid::SUCCESS) {
				std::cout << "Could not open bathymetry file: " << bathymetryFilename << std::endl;
				assert(false);
			}
			if(displacementGrid->open(displacementFilename.c_str()) != Grid::SUCCESS) {
				std::cout << "Could not open displacement file: " << bathymetryFilename << std::endl;
				assert(false);
			}

			bathymetryRange[0] = bathymetryGrid->getMin(0);
			bathymetryRange[1] = bathymetryGrid->getMax(0);
			bathymetryRange[2] = bathymetryGrid->getMin(1);
			bathymetryRange[3] = bathymetryGrid->getMax(1);

			displacementRange[0] = displacementGrid->getMin(0);
			displacementRange[1] = displacementGrid->getMax(0);
			displacementRange[2] = displacementGrid->getMin(1);
			displacementRange[3] = displacementGrid->getMax(1);

#ifndef NDEBUG
		//print information
		std::cout << "  *** scenarios::Asagi created" << std::endl
			<< "    bathymetryRange[0]=" << bathymetryRange[0] << std::endl
			<< "    bathymetryRange[1]=" << bathymetryRange[1] << std::endl
			<< "    bathymetryRange[2]=" << bathymetryRange[2] << std::endl
			<< "    bathymetryRange[3]=" << bathymetryRange[3] << std::endl
			<< "    displacementRange[0]=" << displacementRange[0] << std::endl
			<< "    displacementRange[1]=" << displacementRange[1] << std::endl
			<< "    displacementRange[2]=" << displacementRange[2] << std::endl
			<< "    displacementRange[3]=" << displacementRange[3] << std::endl;
#endif
		}

		virtual ~SWE_AsagiScenario() {
			delete bathymetryGrid;	
			delete displacementGrid;
		}

		float getWaterHeight(float x, float y) {
			assert(x > bathymetryRange[0]);
			assert(x < bathymetryRange[1]);
			assert(y > bathymetryRange[2]);
			assert(y < bathymetryRange[3]);

			double position[] = {x, y};
			float bathymetryValue = bathymetryGrid->getFloat(position);

			if(bathymetryValue > (float) 0.) {
				return 0.;
			}
			else {
				return -bathymetryValue;
			}
		}

		float getBathymetry(float x, float y) {
			assert(x > bathymetryRange[0]);
			assert(x < bathymetryRange[1]);
			assert(y > bathymetryRange[2]);
			assert(y < bathymetryRange[3]);

			double position[] = {x, y};

			float bathymetryValue = bathymetryGrid->getFloat(position);
			float displacementValue = 0;

			if (x > displacementRange[0] &&
					x < displacementRange[1] &&
					y > displacementRange[2] &&
					y < displacementRange[3]) {
				displacementValue = displacementGrid->getFloat(position);
			}

			return bathymetryValue + displacementValue;
		}

		BoundaryType getBoundaryType(Boundary boundary) {
			return OUTFLOW;
		}

		float getBoundaryPos(Boundary boundary) {
			if (boundary == BND_LEFT)
				return bathymetryRange[0];
			else if (boundary == BND_RIGHT)
				return bathymetryRange[1];
			else if (boundary == BND_BOTTOM)
				return bathymetryRange[2];
			else
				return bathymetryRange[3];
		}



	private:
		Grid* bathymetryGrid;
		Grid* displacementGrid;

		float bathymetryRange[4];
		float displacementRange[4];

};
#endif // __SWE_ASAGISCENARIO_HH
