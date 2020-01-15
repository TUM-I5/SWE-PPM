//
// Created by martin on 23/12/2019.
//

#ifndef SWE_BENCHMARK_FLOAT2DBUFFERUPCXX_HH
#define SWE_BENCHMARK_FLOAT2DBUFFERUPCXX_HH
//
// Created by martin on 10/12/2019.
//


#include <memory>

#include "tools/Float2D.hh"

#include "tools/Float2DUpcxx.hh"
class Float2DBufferUpcxx : public Float2D {
public:
    Float2DBufferUpcxx() :
            Float2D(0, 0) {};
    Float2DBufferUpcxx(int cols, int rows,bool localTimestepping, Float2DUpcxx &realData) :
            Float2D(cols, rows) {

        if(localTimestepping){

            data = upcxx::new_array<float>(rows * cols);
            rawData = data.local();

        }else {
            // If there is no local timestepping buffer points to h |hu | hv
            data = realData.getPointer();
            rawData = data.local();
        }

    }
    ~Float2DBufferUpcxx() {}
    upcxx::global_ptr<float> getPointer() const {
        return data;
    }
private:

    upcxx::global_ptr<float> data;
   
};


#endif //SWE_BENCHMARK_FLOAT2DBUFFERUPCXX_HH
