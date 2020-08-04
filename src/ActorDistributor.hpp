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

#include <upcxx/upcxx.hpp>

#include <cstddef>
#include <memory>


#pragma once

class ActorDistributor {
    public:
        const size_t xSize;
        const size_t ySize;

    public:
        // Due to the allocated memory, we don't allow assignment to avoid zombies/leaks
        ActorDistributor(ActorDistributor &) = delete;
        ActorDistributor & operator=(ActorDistributor &) = delete;

        ActorDistributor(size_t xSize, size_t ySize);
        virtual upcxx::intrank_t getRankFor(size_t x, size_t y) = 0;
        virtual std::vector<std::pair<size_t, size_t>> getLocalActorCoordinates() = 0;
        virtual ~ActorDistributor();
    
    protected:
        std::string toString(upcxx::intrank_t *placeDist);
};

std::unique_ptr<ActorDistributor> createActorDistributor(size_t xSize, size_t ySize);
