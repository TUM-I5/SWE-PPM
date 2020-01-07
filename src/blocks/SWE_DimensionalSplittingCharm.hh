#ifndef SWE_DIMENSIONALSPLITTINGCHARM_HH
#define SWE_DIMENSIONALSPLITTINGCHARM_HH

#include "SWE_DimensionalSplittingCharm.decl.h"

#include <unistd.h>
#include <limits.h>
#include <ctime>
#include <time.h>
#include "blocks/SWE_Block.hh"
#ifdef ASAGI
#include "scenarios/SWE_AsagiScenario.hh"
#else
#include "scenarios/SWE_simple_scenarios.hh"
#endif
#include "examples/swe_charm.decl.h"
#include "types/Boundary.hh"
#include "writer/NetCdfWriter.hh"
#include "tools/Float2DNative.hh"
#if WAVE_PROPAGATION_SOLVER==0
//#include "solvers/Hybrid.hpp"
#include "solvers/HLLEFun.hpp"
#elif WAVE_PROPAGATION_SOLVER==1
#include "solvers/FWave.hpp"
#elif WAVE_PROPAGATION_SOLVER==2
#include "solvers/AugRie.hpp"
#endif

extern CProxy_swe_charm mainProxy;
extern int blockCountX;
extern int blockCountY;
extern float simulationDuration;
extern int checkpointCount;

class SWE_DimensionalSplittingCharm : public CBase_SWE_DimensionalSplittingCharm, public SWE_Block<Float2DNative>  {

	SWE_DimensionalSplittingCharm_SDAG_CODE

	public:
		// Charm++ specific constructor needed for object migration
		SWE_DimensionalSplittingCharm(CkMigrateMessage *msg);
		SWE_DimensionalSplittingCharm(int cellCountHorizontal, int cellCountVertical, float cellSizeHorizontal, float cellSizeVertical,
						float originX, float originY, int posX, int posY, BoundaryType boundaries[],
						std::string outputFileName, std::string bathymetryFileName = "", std::string displacementFileName = "",bool localTimestepping = false);
		~SWE_DimensionalSplittingCharm();

		// Charm++ entry methods
		void reduceWaveSpeed(float maxWaveSpeed);
               void printFlops(double flop);
		// Unused pure virtual interface methods
		void computeNumericalFluxes() {}
	private:
		void writeTimestep();
		void sendCopyLayers(bool sendBathymetry = false);
		void processCopyLayer(copyLayer *msg);
		void xSweep();
		void ySweep();
		void updateUnknowns(float dt);
		// Interface implementation
		void setGhostLayer();
#if WAVE_PROPAGATION_SOLVER==0
    //! Hybrid solver (f-wave + augmented)
    //solver::Hybrid<float> solver;
		solver::HLLEFun<float> solver;
#elif WAVE_PROPAGATION_SOLVER==1
    //! F-wave Riemann solver
    solver::FWave<float> solver;
#elif WAVE_PROPAGATION_SOLVER==2
    //! Approximate Augmented Riemann solver
    solver::AugRie<float> solver;
#endif
		float *checkpointInstantOfTime;
		NetCdfWriter *writer;
		float currentSimulationTime;
		int currentCheckpoint;

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

		// Interfaces to neighbouring block copy layers, indexed by Boundary
		int neighbourIndex[4];

		// timer
		std::clock_t computeClock;

		struct timespec startTime;
		struct timespec commTime;
        struct timespec reducTime;
		struct timespec endTime;

		struct timespec startTimeCompute;
		struct timespec endTimeCompute;

		float computeTime;
		float computeTimeWall;
		float wallTime;
		float communicationTime;
        float reductionTime;
		float flopCounter;
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
};

#endif // SWE_DIMENSIONALSPLITTINGCHARM_HH
