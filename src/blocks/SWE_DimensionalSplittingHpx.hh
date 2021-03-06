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

#if WAVE_PROPAGATION_SOLVER == 0
//#include "solvers/Hybrid.hpp"
#include "solvers/HLLEFun.hpp"

#elif WAVE_PROPAGATION_SOLVER == 1
#include "solvers/FWave.hpp"
#elif WAVE_PROPAGATION_SOLVER==2
#include "solvers/AugRie.hpp"
#endif

#include "writer/NetCdfWriter.hh"
#include <hpx/include/compute.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/async.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/parallel_algorithm.hpp>
#include <hpx/include/iostreams.hpp>
#include "tools/CollectorHpx.hpp"

template<typename T>
struct copyLayerStruct {

    int size;


    T B;
    T H;
    T Hu;
    T Hv;
    float timestep = 0;

    template<typename Archive>
    void serialize(Archive &ar, unsigned) {
        ar & size;
        ar & B;
        ar & H;
        ar & Hu;
        ar & Hv;
        ar & timestep;
    }
};

class SWE_DimensionalSplittingHpx : public SWE_Block<Float2DNative> {

public:


    typedef communicator<copyLayerStruct<std::vector<float>>, SWE_DimensionalSplittingHpx> communicator_type;
    friend communicator_type;

    // Constructor/Destructor
    SWE_DimensionalSplittingHpx(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal,
                                float cellSizeVertical, float originX, float originY, bool localTimestepping,
                                std::string name, bool write);

    ~SWE_DimensionalSplittingHpx() {};

    // Interface methods
    hpx::future<void> setGhostLayer();



    void computeNumericalFluxes();

    void updateUnknowns(float dt);

    void connectNeighbours(communicator_type comm);

    void exchangeBathymetry();

    CollectorHpx collector;
    NetCdfWriter *writer;
    float maxTimestepGlobal;
    float checkTimestep;
    float currentTotalLocalTimestep; //used to not have raceconditions in local copying
    void writeTimestep(float timestep);

private:
#if WAVE_PROPAGATION_SOLVER == 0
    //! Hybrid solver (f-wave + augmented)
    //solver::Hybrid<float> solver;
    solver::HLLEFun<float> solver;
#elif WAVE_PROPAGATION_SOLVER == 1
    //! F-wave Riemann solver
    solver::FWave<float> solver;
#elif WAVE_PROPAGATION_SOLVER==2
    //! Approximate Augmented Riemann solver
    solver::AugRie<float> solver;
#endif

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




};


#endif //SWE_BENCHMARK_SWE_DIMENSIONALSPLITTINGHPX_HH
