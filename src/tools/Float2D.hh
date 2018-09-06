/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema
 * @author Sebastian Rettenberger
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
 * Class Float2D is a very basic helper structure to deal with 2D float arrays:
 * The storage in memory is analogous to standard arrays, this means, that sequential reads
 * incrementing the right-most index are faster than sequential reads incrementing the left-most index.
 * The difference to standard arrays is that columns are identified by first index, not the second.
 *
 * TAKE CARE: This class will free its internal memory upon destruction, regardless of any shallow copies/pointers still using it!
 *
 * EXAMPLE with symbolic addresses demonstrating access speed (cache performance)
 * Actual grid:
 *       x
 *    ------->
 *   | 0   1
 * y | 2   3
 *   | 4   5
 *   |
 *   v
 *
 * Float2D
 * Memory:
 * 0x01		0x02		0x03		0x04
 * [0][0]: 0	[0][1]: 2	[0][2]: 4	[1][0]: 1 ...
 *
 * Standard array:
 * Memory:
 * 0x01		0x02		0x03		0x04
 * [0][0]: 0	[0][1]: 1	[0][2]: 2	[1][0]: 3 ...
 */

#ifndef __FLOAT2D_HH
#define __FLOAT2D_HH

class Float2D {
	public:
		int getRows() const {
			return rows;
		}
		
		int getCols() const {
			return cols;
		}
		
		float* getRawPointer() const {
			return rawData;
		}
		
		inline float* operator[](int index) {
			return (rawData + (rows * index));
		}
		
		inline const float* operator[](int index) const {
			return (rawData + (rows * index));
		}

	protected:
		Float2D() {}
		
		Float2D(int cols, int rows) :
				cols(cols),
				rows(rows) {}
		
		~Float2D() {}

		int cols;
		int rows;

		float *rawData;
};
#endif // FLOAT2D_HH
