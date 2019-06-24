#ifndef __FLOAT2DUPCXX_HH
#define __FLOAT2DUPCXX_HH

#include <upcxx/upcxx.hpp>

#include "tools/Float2D.hh"

class Float2DUpcxx : public Float2D {
	public:
		Float2DUpcxx(int cols, int rows) :
				Float2D(cols, rows) {
			data = upcxx::new_array<float>(rows * cols);
			rawData = data.local();
		}
		
		~Float2DUpcxx() {}

		upcxx::global_ptr<float> getPointer() const {
			return data;
		}

	private:
		upcxx::global_ptr<float> data;
};
#endif // FLOAT2DUPCXX_HH
