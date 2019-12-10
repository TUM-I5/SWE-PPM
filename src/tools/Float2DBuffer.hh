//
// Created by martin on 10/12/2019.
//

#ifndef SWE_BENCHMARK_FLOAT2DBUFFER_HH
#define SWE_BENCHMARK_FLOAT2DBUFFER_HH
#include <memory>

#include "tools/Float2D.hh"
#include "tools/Float2DNative.hh"

class Float2DBuffer : public Float2D {
public:
    Float2DBuffer() :
            Float2D(0, 0) {};
    Float2DBuffer(int cols, int rows,bool localTimestepping, Float2DNative *realData) :
            Float2D(cols, rows) {

        if(localTimestepping){
            std::shared_ptr<float> tmp(new float[rows * cols], std::default_delete<float[]>());
            data = tmp;
            rawData = data.get();
        }else {
            // If there is no local timestepping buffer points to h |hu | hv
            rawData = (*realData).getPointer();
        }

    }

    ~Float2DBuffer() {}

    std::shared_ptr<float> getPointer() {
        return data;
    }

private:

    std::shared_ptr<float> data;
};
#endif //SWE_BENCHMARK_FLOAT2DBUFFER_HH
