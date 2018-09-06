/**
 * SWE_Block1D is a simple struct that can represent a single line or row of
 * SWE_Block unknowns (using the Float1D proxy class).
 * It is intended to unify the implementation of inflow and periodic boundary
 * conditions, as well as the ghost/copy-layer connection between several SWE_Block
 * grids.
 */

#ifndef __SWE_BLOCK1DUPCXX_HH
#define __SWE_BLOCK1DUPCXX_HH
#include "tools/Float1DUpcxx.hh"

struct SWE_Block1D {
	SWE_Block1DUpcxx(const Float1D& h, const Float1D& hu, const Float1D& hv) :
		h(h),
		hu(hu),
		hv(hv){
	};
	SWE_Block1DUpcxx(float* _h, float* _hu, float* _hv, int _size, int _stride=1) :
		h(_h, _size, _stride),
		hu(_hu, _size, _stride),
		hv(_hv, _size, _stride){};

	Float1DUpcxx h;
	Float1DUpcxx hu;
	Float1DUpcxx hv;
};
#endif // __SWE_BLOCK1DUPCXX_HH
