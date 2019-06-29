//
// Created by martin on 05/06/19.
//

#ifndef SWE_BENCHMARK_SWE_DIMENSIONALSPLITTINGHPX_HH
#define SWE_BENCHMARK_SWE_DIMENSIONALSPLITTINGHPX_HH



#include <limits.h>
#include <ctime>
#include <time.h>
#include "blocks/SWE_Block.hh"
#include "scenarios/SWE_Scenario.hh"
#include "tools/Float2DNative.hh"

#include "tools/Communicator.hpp"
#include "solvers/HLLEFun.hpp"
#include "solvers/Hybrid.hpp"
#include <hpx/include/compute.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/async.hpp>
#include <hpx/include/components.hpp>
template <typename T>
struct copyLayerStruct {

    int size;


    T B;
    T H;
    T Hu;
    T Hv;
    template <typename Archive>
    void serialize(Archive & ar, unsigned)
    {
        ar & size;
        ar & B;
        ar & H;
        ar & Hu;
        ar & Hv;
    }
};
typedef communicator<copyLayerStruct<std::vector<float>>> communicator_type;
class SWE_DimensionalSplittingHpx : public SWE_Block<Float2DNative> {

public:
    // Constructor/Destructor
    SWE_DimensionalSplittingHpx(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal,
                                float cellSizeVertical, float originX, float originY , communicator_type comm);
    ~SWE_DimensionalSplittingHpx() {};

    // Interface methods
    hpx::future<void> setGhostLayer();
    void connectBoundaries(Boundary boundary, SWE_Block &neighbour, Boundary neighbourBoundary);
    void computeXSweep();
    void computeYSweep();
    void computeNumericalFluxes();
    void updateUnknowns(float dt);

    // Hpx specific
    void freeHpxType();
    void connectNeighbours(int neighbourRankId[]);
    void exchangeBathymetry();

    float computeTime;
    float computeTimeWall;
    float communicationTime;
    float flopCounter;
    float maxTimestepGlobal;

   /* template <typename Archive>
    void serialize(Archive & ar, unsigned)
    {

        ar &  computeTime;
        ar &  computeTimeWall;
        ar & getFlops();
        ar &  maxTimestepGlobal;
        ar & solver;

        ar & comm;
        // Max timestep reduced over all upcxx ranks

        // Temporary values after x-sweep and before y-sweep
        ar & hStar;
        ar & huStar;

        // net updates per cell
        ar & hNetUpdatesLeft;
        ar & hNetUpdatesRight;

        ar & huNetUpdatesLeft;
        ar & huNetUpdatesRight;

        ar & hNetUpdatesBelow;
        ar & hNetUpdatesAbove;

        ar & hvNetUpdatesBelow;
        ar & hvNetUpdatesAbove;

        ar & computeClock;
        ar & startTime;
        ar & endTime;

    }*/
private:
    solver::HLLEFun<float> solver;

    communicator_type comm;
    // Max timestep reduced over all upcxx ranks

    // Temporary values after x-sweep and before y-sweep
    Float2DNative hStar;
    Float2DNative huStar;

    // net updates per cell
    Float2DNative hNetUpdatesLeft;
    Float2DNative hNetUpdatesRight;

    Float2DNative huNetUpdatesLeft;
    Float2DNative huNetUpdatesRight;

    Float2DNative hNetUpdatesBelow;
    Float2DNative hNetUpdatesAbove;

    Float2DNative hvNetUpdatesBelow;
    Float2DNative hvNetUpdatesAbove;

    /* Copy buffer:
     * Since Float2D are stored column-wise in memory,
     * it is expensive to read rows from a Float2D since it is necessary to stride an entire column after each read element.
     * Therefore we duplicate elements belonging to the bottom/top copy layers into a copy buffer when updateUnkowns() runs.
     */
    //float* topCopyBuffer[nx];
    //float* bottomCopyBuffer[nx];



    // timer
    std::clock_t computeClock;
    struct timespec startTime;
    struct timespec endTime;

};


#endif //SWE_BENCHMARK_SWE_DIMENSIONALSPLITTINGHPX_HH
