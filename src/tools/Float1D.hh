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
 * class Float1D is a proxy class that can represent, for example, 
 * a column or row vector of a Float2D array, where row (sub-)arrays 
 * are stored with a respective stride. 
 * Besides constructor/deconstructor, the class provides overloading of 
 * the []-operator, such that elements can be accessed as v[i] 
 * (independent of the stride).
 * The class will never allocate separate memory for the vectors, 
 * but point to the interior data structure of Float2D (or other "host" 
 * data structures).
 */

#ifndef __FLOAT1D_HH
#define __FLOAT1D_HH

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

class Float1D {
	public:
		int getSize() const;
		//T getPointer() const;
		float* getRawPointer() const;

		inline float& operator[](int index);

	protected:
		Float1D(); 
		Float1D(float* rawData, int size, int stride = 1);
		virtual ~Float1D() = 0;
		
		int size;
		int stride;

		//T data;
		float* rawData;
};

Float1D::Float1D() {}

Float1D::Float1D(float* rawData, int size, int stride) :
			rawData(rawData),
			size(size),
			stride(stride) {}

Float1D::~Float1D() {}

int Float1D::getSize() const {
	return size;
}

//T Float1D::getPointer() const {
//	return data;
//}

float* Float1D::getRawPointer() const {
	return rawData;
}

inline float& Float1D::operator[](int index) { 
	return rawData[index * stride]; 
}
#endif // FLOAT1D_HH
