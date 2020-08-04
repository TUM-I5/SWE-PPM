/**
 * @file
 * This file is part of Pond.
 *
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

#include "orchestration/SimpleActorDistributor.hpp"

#include "util/Logger.hh"

#include <cmath>

static tools::Logger &l = tools::Logger::logger;

using CoordPair = std::pair<size_t, size_t>;

SimpleActorDistributor::SimpleActorDistributor(size_t xSize, size_t ySize)
    : ActorDistributor(xSize, ySize),
      actorDistribution(new upcxx::intrank_t[xSize * ySize]) {
    auto tmp1 = static_cast<double>(xSize) / std::sqrt(static_cast<double>(upcxx::rank_n()));
    l.cout() << xSize << "/ sqrt(" << upcxx::rank_n() << ") = " << tmp1 << std::endl;
    double xBlockSize = std::floor(tmp1);
    l.cout() << xBlockSize << std::endl;
    size_t xSplits = std::max(xSize / static_cast<size_t>(xBlockSize), 1ul);
    size_t ySplits = upcxx::rank_n() / xSplits;
//#ifndef NDEBUG
    l.cout() <<"xSplits: " << xSplits << " ySplits: " << ySplits << std::endl;
//#endif

    putInitial(xSplits, ySplits);
    l.printString(toString(actorDistribution));
}

SimpleActorDistributor::~SimpleActorDistributor() {
    delete[] actorDistribution;
}

void SimpleActorDistributor::putInitial(size_t xSplits, size_t ySplits) {
    for (size_t x = 0; x < xSize; x++) {
        for (size_t y = 0; y < ySize; y++) {
            auto tmpX = x / (xSize / xSplits);
            auto tmpY = y / (ySize / ySplits);
            auto res = static_cast<upcxx::intrank_t>(tmpX * ySplits + tmpY);
            actorDistribution[x * ySize + y] = std::min(res, upcxx::rank_n() - 1);
        }
    }
}

upcxx::intrank_t SimpleActorDistributor::getRankFor(size_t x, size_t y) {
    return actorDistribution[x * ySize + y];
}

std::vector<CoordPair> SimpleActorDistributor::getLocalActorCoordinates() {
        std::vector<CoordPair> res;
        for (size_t x = 0; x < xSize; x++) {
            for (size_t y = 0; y < ySize; y++) {
                if (actorDistribution[x * ySize + y] == upcxx::rank_me()) {
                    res.push_back(std::make_pair(x,y));
                }
            }
        }
        return res;
}
