/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema, Tobias Schnabel
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
 * TODO
 */

#ifndef __SWE_SCENARIO_HH
#define __SWE_SCENARIO_HH

#include "types/Boundary.hh"

/**
 * SWE_Scenario defines an interface to initialise the unknowns of a 
 * shallow water simulation - i.e. to initialise water height, velocities,
 * and bathymatry according to certain scenarios.
 * SWE_Scenario can act as stand-alone scenario class, providing a very
 * basic scenario (all functions are constant); however, the idea is 
 * to provide derived classes that implement the SWE_Scenario interface
 * for more interesting scenarios.
 */
class SWE_Scenario {
public :
    virtual ~SWE_Scenario() {}

    virtual float getWaterHeight(float x, float y) { return 0; }

    virtual float getBathymetry(float x, float y) { return 0; }

    virtual float getVeloc_u(float x, float y) { return 0; }

    virtual float getVeloc_v(float x, float y) { return 0; }

    virtual BoundaryType getBoundaryType(Boundary boundary) { return OUTFLOW; }

    virtual float getBoundaryPos(Boundary boundary) {
        if (boundary == BND_LEFT || boundary == BND_BOTTOM)
            return 0.0f;
        else
            return 1.0f;
    }

    virtual float waterHeightAtRest() { return 0; };

    virtual float endSimulation() { return 0; };
};

#endif // _SWE_SCENARIO_HH
