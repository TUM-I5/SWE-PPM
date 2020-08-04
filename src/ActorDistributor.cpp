/**
 * @file
 * This file is part of Pond.
 *
 * @author Michael Bader, Kaveh Rahnema, Tobias Schnabel
 * @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
 * @author Alexander Pöppl (poeppl AT in.tum.de, https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
 *
 * @section LICENSE
 *
 * Pond is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pond is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pond.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 * TODO
 */

#include "ActorDistributor.hpp"

//#include <upcxx/upcxx.hpp>

#include <iomanip>

#if defined(METIS_PARTITIONING)
#include "orchestration/MetisActorDistributor.hpp"
#else
#include "orchestration/SimpleActorDistributor.hpp"
#endif

ActorDistributor::ActorDistributor(size_t xSize, size_t ySize) 
    : xSize(xSize),
      ySize(ySize) {

}

ActorDistributor::~ActorDistributor() = default;
    
std::string ActorDistributor::toString(upcxx::intrank_t *actorDist) {
    std::stringstream ss;
    upcxx::intrank_t places = upcxx::rank_n();
    int digits = ceil(std::log10(places-1));
    ss << "\n";
    for (size_t y = 0; y < ySize; y++) {
        for (size_t x = 0; x < xSize; x++) {
            ss << std::setw(digits) << actorDist[y * xSize + x] << " ";
        }
        ss << std::endl;
    }
    return ss.str();
}

std::unique_ptr<ActorDistributor> createActorDistributor(size_t xSize, size_t ySize) {
#if defined(METIS_PARTITIONING)
    return std::make_unique<MetisActorDistributor>(xSize, ySize);
#else 
    return std::make_unique<SimpleActorDistributor>(xSize, ySize);
#endif
}
