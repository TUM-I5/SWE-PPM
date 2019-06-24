//
// Created by martin on 16/06/19.
//

#include "SWE_Hpx_Component.hpp"


#include <algorithm>
#include <iostream>
#include "tools/args.hh"

#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#else
#include "writer/VtkWriter.hh"
#endif

#ifdef ASAGI
#include "scenarios/SWE_AsagiScenario.hh"
#else
#include "scenarios/SWE_simple_scenarios.hh"
#endif

template <typename T> class Min {
public:

   T operator()(const T& a, const T& b) const {return a < b ? a : b; }
};
typedef Min<float> max_type;
HPX_REGISTER_REDUCE_ACTION_DECLARATION(remote::SWE_Hpx_Component::get_wave_speed_action, max_type)
HPX_REGISTER_REDUCE_ACTION(remote::SWE_Hpx_Component::get_wave_speed_action, max_type)
 namespace remote
    {
     SWE_Hpx_Component::SWE_Hpx_Component(
             int rank, int totalRank, float simulationDuration, int numberOfCheckPoints,
             int nxLocal, int nyLocal,float  dxSimulation, float  dySimulation,
             float localOriginX, float localOriginY,  std::array<BoundaryType,4> boundaries, std::array<int,4> neighbours)
 /*
#ifdef WRITENETCDF
     // Construct a netCDF writer

	    writer(
			outputFileName,
			simulation.getBathymetry(),
			boundarySize,
			nxLocal,
			nyLocal,
			dxSimulation,
			dySimulation,
			simulation.getOriginX(),
			simulation.getOriginY())

#else
             // Construct a vtk writer
             writer(
                     outputFileName,
                     simulation.getBathymetry(),
                     boundarySize,
                     nxLocal,
                     nyLocal,
                     dxSimulation,
                     dySimulation)
#endif // WRITENETCDF*/

     :
             simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, communicator_type(rank,totalRank,neighbours))
     {
         std::string outputFileName;
         // Initialize Scenario
#ifdef ASAGI
         SWE_AsagiScenario scenario(batFile, displFile);
#else
         SWE_RadialDamBreakScenario scenario;
#endif
        //this->ids(ids);
         this->simulationDuration = simulationDuration;
         this->numberOfCheckPoints = numberOfCheckPoints;
         // Compute when (w.r.t. to the simulation time in seconds) the checkpoints are reached
         //float* checkpointInstantOfTime = new float[numberOfCheckPoints];
         // Time delta is the time between any two checkpoints
         float checkpointTimeDelta = simulationDuration / numberOfCheckPoints;
         // The first checkpoint is reached after 0 + delta t
         checkpointInstantOfTime.push_back(checkpointTimeDelta);
         for(int i = 1; i < numberOfCheckPoints; i++) {
             checkpointInstantOfTime.push_back(checkpointInstantOfTime[i - 1] + checkpointTimeDelta);
         }

         myHpxRank = rank;
         totalHpxRanks = totalRank;


         simulation.initScenario(scenario, boundaries.data());

        /* std::cout<< "Rank: " << myHpxRank << std::endl
                  << "Left " << myNeighbours[BND_LEFT] << std::endl
                  << "Right " << myNeighbours[BND_RIGHT] << std::endl
                  << "Bottom " << myNeighbours[BND_BOTTOM] << std::endl
                  << "TOP " << myNeighbours[BND_TOP] << std::endl;*/


         /*
          * Calculate the simulation grid layout.
          * The cell count of the scenario as well as the scenario size is fixed,
          * Get the size of the actual domain and divide it by the requested resolution.
          */
        /* int widthScenario = scenario.getBoundaryPos(BND_RIGHT) - scenario.getBoundaryPos(BND_LEFT);
         int heightScenario = scenario.getBoundaryPos(BND_TOP) - scenario.getBoundaryPos(BND_BOTTOM);
         float dxSimulation = (float) widthScenario / nxRequested;
         float dySimulation = (float) heightScenario / nyRequested;



         // Print status
         char hostname[HOST_NAME_MAX];
         gethostname(hostname, HOST_NAME_MAX);

         printf("%i Spawned at %s\n", myHpxRank, hostname);

         /*
          * determine the layout of UPC++ ranks:
          * one block per process;
          * if the number of processes is a square number, l_blockCountX = l_blockCountY,
          * else l_blockCountX > l_blockCountY
          */
         // number of SWE-Blocks in x- and y-direction
       /*  int blockCountY = std::sqrt(totalHpxRanks);
         while (totalHpxRanks % blockCountY != 0) blockCountY--;
         int blockCountX = totalHpxRanks / blockCountY;

         // determine the local block position of each SWE_Block
         int localBlockPositionX = myHpxRank / blockCountY;
         int localBlockPositionY = myHpxRank % blockCountY;

         // compute local number of cells for each SWE_Block w.r.t. the simulation domain
         // (particularly not the original scenario domain, which might be finer in resolution)
         // (blocks at the domain boundary are assigned the "remainder" of cells)
         int nxBlockSimulation = nxRequested / blockCountX;
         int nxRemainderSimulation = nxRequested - (blockCountX - 1) * (nxRequested / blockCountX);
         int nyBlockSimulation = nyRequested / blockCountY;
         int nyRemainderSimulation = nyRequested - (blockCountY - 1) * (nyRequested / blockCountY);

         int nxLocal = (localBlockPositionX < blockCountX - 1) ? nxBlockSimulation : nxRemainderSimulation;
         int nyLocal = (localBlockPositionY < blockCountY - 1) ? nyBlockSimulation : nyRemainderSimulation;

         // Compute the origin of the local simulation block w.r.t. the original scenario domain.
         float localOriginX = scenario.getBoundaryPos(BND_LEFT) + localBlockPositionX * dxSimulation * nxBlockSimulation;
         float localOriginY = scenario.getBoundaryPos(BND_BOTTOM) + localBlockPositionY * dySimulation * nyBlockSimulation;



         // Determine the boundary types for the SWE_Block:
         // block boundaries bordering other blocks have a CONNECT boundary,
         // block boundaries bordering the entire scenario have the respective scenario boundary type
         BoundaryType boundaries[4];

         boundaries[BND_LEFT] = (localBlockPositionX > 0) ? CONNECT : scenario.getBoundaryType(BND_LEFT);
         boundaries[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? CONNECT : scenario.getBoundaryType(BND_RIGHT);
         boundaries[BND_BOTTOM] = (localBlockPositionY > 0) ? CONNECT : scenario.getBoundaryType(BND_BOTTOM);
         boundaries[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? CONNECT : scenario.getBoundaryType(BND_TOP);

         // Initialize the simulation block according to the scenario




         int myNeighbours[4];
         myNeighbours[BND_LEFT] = (localBlockPositionX > 0) ? myHpxRank - blockCountY : -1;
         myNeighbours[BND_RIGHT] = (localBlockPositionX < blockCountX - 1) ? myHpxRank + blockCountY : -1;
         myNeighbours[BND_BOTTOM] = (localBlockPositionY > 0) ? myHpxRank - 1 : -1;
         myNeighbours[BND_TOP] = (localBlockPositionY < blockCountY - 1) ? myHpxRank + 1 : -1;

         communicator_type comm(myHpxRank,totalHpxRanks,myNeighbours);
         simulation(nxLocal, nyLocal, dxSimulation, dySimulation, localOriginX, localOriginY, comm);
         simulation.initScenario(scenario, boundaries);

         std::cout<< "Rank: " << myHpxRank << std::endl
                  << "Left " << myNeighbours[BND_LEFT] << std::endl
                  << "Right " << myNeighbours[BND_RIGHT] << std::endl
                  << "Bottom " << myNeighbours[BND_BOTTOM] << std::endl
                  << "TOP " << myNeighbours[BND_TOP] << std::endl;
         simulation.exchangeBathymetry();

         // Initialize boundary size of the ghost layers
         BoundarySize boundarySize = {{1, 1, 1, 1}};
         outputFileName = generateBaseFileName(outputBaseName, localBlockPositionX, localBlockPositionY);*/
         /***************
          * INIT OUTPUT *
          ***************/


         // Write the output at t = 0
       /*  writer.writeTimeStep(
                 simulation.getWaterHeight(),
                 simulation.getMomentumHorizontal(),
                 simulation.getMomentumVertical(),
                 (float) 0.);

*/
        hpx::cout << myHpxRank << " finished init" << std::endl;
     }
        void SWE_Hpx_Component::invoke(std::vector<hpx::naming::id_type>const &test)
        {


        }
     void SWE_Hpx_Component::initialize(std::vector<hpx::naming::id_type> const &ids)
     {



         simulation.exchangeBathymetry();
         hpx::cout << "STARTED\n";
         /********************
          * START SIMULATION *
          ********************/


         // Initialize wall timer
         struct timespec startTime;
         struct timespec endTime;

         float wallTime = 0.;
         float t = 0.;


         float timestep;
         unsigned int iterations = 0;
         hpx::lcos::barrier(std::string("precalc"), totalHpxRanks, myHpxRank).wait();

         // loop over the count of requested checkpoints
         for(int i = 0; i < numberOfCheckPoints; i++) {
             // Simulate until the checkpoint is reached
             while(t < checkpointInstantOfTime[i]) {
                 // Start measurement
                 clock_gettime(CLOCK_MONOTONIC, &startTime);

                 // set values in ghost cells.
                 // this function blocks until everything has been received

                 simulation.setGhostLayer();

                 // compute numerical flux on each edge
                // simulation.computeNumericalFluxes(ids);
                 simulation.computeXSweep();

                 hpx::lcos::barrier(std::string("midcalc"), totalHpxRanks, myHpxRank).wait();

                 simulation.maxTimestepGlobal = hpx::lcos::reduce<get_wave_speed_action>(ids,Min<float>()).get();
                 hpx::cout << myHpxRank << " timestep " << simulation.maxTimestepGlobal << std::endl;
                 simulation.computeYSweep();
                 // max timestep has been reduced over all ranks in computeNumericalFluxes()
                 timestep = simulation.getMaxTimestep();

                 // update the cell values
                 simulation.updateUnknowns(timestep);

                 // Accumulate wall time
                 clock_gettime(CLOCK_MONOTONIC, &endTime);
                 wallTime += (endTime.tv_sec - startTime.tv_sec);
                 wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;

                 // update simulation time with time step width.
                 t += timestep;
                 iterations++;
                 // upcxx::barrier();
                 hpx::lcos::barrier(std::string("endcalc"), totalHpxRanks, myHpxRank).wait();
             }

             if(myHpxRank == 0) {
                hpx::cout <<"Write timestep " << t<<"s" << std::endl;
             }

             // write output
            /* writer.writeTimeStep(
                     simulation.getWaterHeight(),
                     simulation.getMomentumHorizontal(),
                     simulation.getMomentumVertical(),
                     t);*/
         }


         /************
          * FINALIZE *
          ************/

         hpx::cout << "Rank "<< myHpxRank <<  ": Compute Time (CPU):" << simulation.computeTime
                 <<"s"<< " - (WALL): "<<  simulation.computeTimeWall<<"s"<< " | Total Time (Wall):"<< wallTime <<"s"<<std::endl;
         uint64_t sumFlops = 0;
         //uint64_t  sumFlops = upcxx::reduce_all(simulation.getFlops(), upcxx::op_fast_add).wait();
         if(myHpxRank == 0){
             hpx::cout   << "Rank: " << myHpxRank << std::endl
                         << "Flop count: " << sumFlops << std::endl
                         << "Flops(Total): " << ((float)sumFlops)/(wallTime*1000000000) << "GFLOPS"<< std::endl
                         << "Flops(Single): "<< ((float)simulation.getFlops())/(wallTime*1000000000) << std::endl;
         }

     }
        float SWE_Hpx_Component::get_wave_speed()
        {
            return simulation.maxTimestepGlobal;
        }




    }

HPX_REGISTER_COMPONENT_MODULE();

typedef hpx::components::component<
        remote::SWE_Hpx_Component
> SWE_Hpx_Component_type;


HPX_REGISTER_COMPONENT(SWE_Hpx_Component_type, SWE_Hpx_Component);

HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::invoke_action, SWE_Hpx_Component_invoke_action);
HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::initialize_action, SWE_Hpx_Component_initialize_action);
HPX_REGISTER_ACTION(
        remote::SWE_Hpx_Component::get_wave_speed_action, SWE_Hpx_Component_get_wave_speed_action);
