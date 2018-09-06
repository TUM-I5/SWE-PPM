/**
 * SWE_Block1D is a simple struct that can represent a single line or row of
 * SWE_Block unknowns (using the Float1D proxy class).
 * It is intended to unify the implementation of inflow and periodic boundary
 * conditions, as well as the ghost/copy-layer connection between several SWE_Block
 * grids.
 */

#ifndef __SWE_BLOCK1D_HH
#define __SWE_BLOCK1D_HH
#include "tools/Float1D.hh"

struct SWE_Block1D {
	SWE_Block1D(const Float1D& _h, const Float1D& _hu, const Float1D& _hv) :
		h(_h),
		hu(_hu),
		hv(_hv){
	};
	SWE_Block1D(float* _h, float* _hu, float* _hv, int _size, int _stride=1) :
		h(_h, _size, _stride),
		hu(_hu, _size, _stride),
		hv(_hv, _size, _stride){};

	Float1D h;
	Float1D hu;
	Float1D hv;
};
#endif // __SWE_BLOCK1D_HH
