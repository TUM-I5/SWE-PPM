#ifndef SWE_DIMENSIONALSPLITTINGCHARM_HH
#define SWE_DIMENSIONALSPLITTINGCHARM_HH

#include "SWE_DimensionalSplittingCharm.decl.h"

#include <unistd.h>
#include <limits.h>
#include <ctime>
#include <time.h>
#include "blocks/SWE_Block.hh"
#include <pup_stl.h>
#ifdef ASAGI
#include "scenarios/SWE_AsagiScenario.hh"
#else
#include <functional>
#include "scenarios/SWE_simple_scenarios.hh"

#endif

#include "examples/swe_charm.decl.h"
#include "types/Boundary.hh"
#include "writer/NetCdfWriter.hh"
#include "tools/Float2DNative.hh"
#include "tools/CollectorCharm.hpp"

#if WAVE_PROPAGATION_SOLVER == 0
//#include "solvers/Hybrid.hpp"
#include "solvers/HLLEFun.hpp"

#elif WAVE_PROPAGATION_SOLVER == 1
#include "solvers/FWave.hpp"
#elif WAVE_PROPAGATION_SOLVER==2
#include "solvers/AugRie.hpp"
#endif

extern CProxy_swe_charm mainProxy;
extern int blockCountX;
extern int blockCountY;
extern float simulationDuration;
extern int checkpointCount;

class SWE_DimensionalSplittingCharm : public CBase_SWE_DimensionalSplittingCharm, public SWE_Block<Float2DNative> {

    SWE_DimensionalSplittingCharm_SDAG_CODE

public:
    // Charm++ specific constructor needed for object migration
    SWE_DimensionalSplittingCharm(CkMigrateMessage *msg);

    SWE_DimensionalSplittingCharm(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal,
                                  float cellSizeVertical,
                                  float originX, float originY, int posX, int posY, BoundaryType boundaries[],
                                  std::string outputFileName, std::string bathymetryFileName = "",
                                  std::string displacementFileName = "", bool localTimestepping = false,bool write =false);

    ~SWE_DimensionalSplittingCharm();

    // Charm++ entry methods
    void reduceWaveSpeed(float maxWaveSpeed);

    void printFlops(double flop);

    // Unused pure virtual interface methods
    void computeNumericalFluxes() {}

    void pup(PUP::er &p) {
        //CkPrintf("%d:Start with PUP %d\n",CkMyPe(),p.isUnpacking());
       /*Base*/
        p|nx;
        p|ny;

        p| dx;
        p| dy;

        p| originX;
        p| originY;


        p| duration;
        p| maxTimestep;
        p| maxTimestepLocal; // used for local timestepping
        p| currentTimestep;
        p|maxDivisor; //smallest timestep possible maxTimestepLocal/maxDivisor
        p|localTimestepping; //true to activate localtimestepping
        p|notifiedLastTimestep;
        p|stepSize; //is used to determine the localstepsize;
        p|stepSizeCounter; //used to count the steps;
        p|iteration;
        p|timestepCounter;
        p|myRank;

        PUParray(p, receivedGhostlayer,4 );
        PUParray(p, borderTimestep,4 );
        PUParray(p, neighbourRankId,4 );
        PUParray(p, boundaryType,4 );
        PUParray(p, neighbourIndex,4 );

        p|write;
        p|currentSimulationTime;
        p|currentCheckpoint;
        p|receiveCounter;

        p|firstIteration;
        p|outputFilename;


        double *serial = p.isUnpacking()?collectorSerializer:collector->serialize(collectorSerializer);
        PUParray(p,serial,5);

        if (p.isUnpacking()){
            migrated = 1;
            CkPrintf("Unpacking\n");

            checkpointInstantOfTime = new float[checkpointCount];
            // For the x-sweep
            hNetUpdatesLeft = Float2DNative(nx + 2, ny + 2);
            hNetUpdatesRight = Float2DNative(nx + 2, ny + 2);

            huNetUpdatesLeft = Float2DNative(nx + 2, ny + 2);
            huNetUpdatesRight = Float2DNative(nx + 2, ny + 2);

            // For the y-sweep
            hNetUpdatesBelow = Float2DNative(nx + 1, ny + 2);
            hNetUpdatesAbove = Float2DNative(nx + 1, ny + 2);

            hvNetUpdatesBelow = Float2DNative(nx + 1, ny + 2);
            hvNetUpdatesAbove = Float2DNative (nx + 1, ny + 2);

            h  = Float2DNative(nx + 2, ny + 2);
            hu = Float2DNative(nx + 2, ny + 2);
            hv = Float2DNative(nx + 2, ny + 2);
            b  = Float2DNative(nx + 2, ny + 2);

            bufferH = Float2DBuffer(nx + 2, ny + 2, localTimestepping, h);
            bufferHu = Float2DBuffer(nx + 2, ny + 2, localTimestepping, h);
            bufferHv = Float2DBuffer(nx + 2, ny + 2, localTimestepping, h);

            //writer = (NetCdfWriter*) malloc(sizeof(NetCdfWriter));
            collector = new CollectorCharm();
            *collector += CollectorCharm::deserialize(collectorSerializer);
            float *checkpointInstantOfTime = new float[checkpointCount];
            if(write){

                // Initialize writer
                BoundarySize boundarySize = {{1, 1, 1, 1}};
                writer = new NetCdfWriter(outputFilename, b, boundarySize, nx, ny, dx, dy, originX, originY);

            }

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
        }

        PUParray(p, checkpointInstantOfTime,checkpointCount );

        int size = (nx+2)*(ny+2);
        PUParray(p, h.getRawPointer(),size );
        PUParray(p, hu.getRawPointer(),size );
        PUParray(p, hv.getRawPointer(),size );
        PUParray(p, b.getRawPointer(),size );

         std::string hString="";
        std::string huString="";
        std::string hvString="";
        std::string bString="";

        for (int i = 1; i < nx+2; i++) {

            for (int j=1; j < ny+2; ++j) {
                hString += std::to_string(bufferH[i][j])+ " ";
                huString += std::to_string(bufferHu[i][j])+ " ";
                hvString += std::to_string(bufferHv[i][j])+ " ";
                bString += std::to_string(b[i][j])+ " ";
            }
        }
        CkPrintf("%d: h %s\n\n\n",thisIndex, hString.c_str());
        CkPrintf("%d: hu %s\n\n\n",thisIndex, huString.c_str());
        CkPrintf("%d: hv %s\n\n\n",thisIndex, hvString.c_str());
        CkPrintf("%d: b %s\n\n\n",thisIndex, bString.c_str());
                if(localTimestepping){
            PUParray(p, bufferH.getRawPointer(),size );
            PUParray(p, bufferHu.getRawPointer(),size );
            PUParray(p, bufferHv.getRawPointer(),size );
        }
        //CkPrintf("%d:Done with PUP %d\n", thisIndex, p.isUnpacking());
    }

private:

    virtual void ResumeFromSync();
    void writeTimestep();

    void sendCopyLayers(bool sendBathymetry = false);

    void processCopyLayer(copyLayer *msg);

    void xSweep();

    void ySweep();

    void updateUnknowns(float dt);

    // Interface implementation
    void setGhostLayer();

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
    double collectorSerializer[5];
    float *checkpointInstantOfTime;
    bool write;
    NetCdfWriter *writer;
    float currentSimulationTime;
    int currentCheckpoint;
    int receiveCounter = 0;
    int migrated = 0;
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
    std::string outputFilename;
    // Interfaces to neighbouring block copy layers, indexed by Boundary
    int neighbourIndex[4];
    bool firstIteration;

    CollectorCharm *collector;


};

class copyLayer : public CMessage_copyLayer {
public:
    Boundary boundary;
    bool containsBathymetry;
    bool isDummy;
    float *b;
    float *h;
    float *hu;
    float *hv;
    float timestep;
};

class collectorMsg : public CMessage_collectorMsg {
public:

    double *deserialized;

};

#endif // SWE_DIMENSIONALSPLITTINGCHARM_HH
