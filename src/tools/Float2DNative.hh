#ifndef __FLOAT2DNATIVE_HH
#define __FLOAT2DNATIVE_HH

#include <memory>

#include "tools/Float2D.hh"

class Float2DNative : public Float2D {
public:
    Float2DNative() :
            Float2D(0, 0) {};

    Float2DNative(int cols, int rows) :
            Float2D(cols, rows) {
        std::shared_ptr<float> tmp(new float[rows * cols], std::default_delete<float[]>());
        data = tmp;
        rawData = data.get();
    }

    ~Float2DNative() {}

    std::shared_ptr<float> getPointer() {
        return data;
    }

private:
    std::shared_ptr<float> data;
};

#endif // FLOAT2DNATIVE_HH
