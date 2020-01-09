//
// Created by martin on 05/06/19.
//

/**
 * @file
 * This file is part of SWE.
 *
 * @author Jurek Olden (jurek.olden AT in.tum.de)
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 * This is a simple frontend for the SWE framework, which uses netCDF input files for bathymetry and displacement.
 * It then simulates this scenario according to the command line arguments it gets:
 *  - boundary condition
 *  - duration of the simulation
 *  - number of output checkpoints
 *  - resolution
 */

#include <cassert>
#include <string>
#include <ctime>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
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

//#include "blocks/SWE_DimensionalSplittingHpx.hh"

#include "blocks/SWE_Hpx_No_Component.hpp"

#include <hpx/hpx_init.hpp>
#include <hpx/include/compute.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/async.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/parallel_algorithm.hpp>
#include <hpx/lcos/broadcast.hpp>
#include <hpx/util/unwrap.hpp>


int hpx_main(boost::program_options::variables_map& vm)
{


    /**************
 * INIT INPUT *
 **************/
    float simulationDuration;
    int numberOfCheckPoints;
    int nxRequested;
    int nyRequested;
    int totalRanks;
    std::string outputBaseName;
    std::string batFile;
    std::string displFile;
    bool  localTimestepping = false;
    // Define command line arguments
    tools::Args args;

#ifdef ASAGI
    args.addOption("bathymetry-file", 'b', "File containing the bathymetry");
	args.addOption("displacement-file", 'd', "File containing the displacement");
#endif

    args.addOption("simulation-duration", 'e', "Time in seconds to simulate");
    args.addOption("checkpoint-count", 'n', "Number of simulation snapshots to be written");
    args.addOption("resolution-horizontal", 'x', "Number of simulation cells in horizontal direction");
    args.addOption("resolution-vertical", 'y', "Number of simulated cells in y-direction");
    args.addOption("output-basepath", 'o', "Output base file name");
    args.addOption("local-timestepping", 'l', "Activate local timestepping", tools::Args::Required, false);

    // Declare the variables needed to hold command line input

    // Declare variables for the output and the simulation time


/*
    // Parse command line arguments
    tools::Args::Result ret = args.parse(argc, argv);
    switch (ret)
    {
        case tools::Args::Error:
            return hpx::finalize();
        case tools::Args::Help:
            return hpx::finalize();
        case tools::Args::Success:
            break;
    }
    // Read in command line arguments
    simulationDuration = args.getArgument<float>("simulation-duration");
    numberOfCheckPoints = args.getArgument<int>("checkpoint-count");
    nxRequested = args.getArgument<int>("resolution-horizontal");
    nyRequested = args.getArgument<int>("resolution-vertical");
    outputBaseName = args.getArgument<std::string>("output-basepath");

#ifdef ASAGI
   batFile = args.getArgument<std::string>("bathymetry-file");
   displFile = args.getArgument<std::string>("displacement-file");
#endif
*/

    simulationDuration =  vm["simulation-duration"].as<float>();

       numberOfCheckPoints = vm["checkpoint-count"].as<int>();
    nxRequested = vm["resolution-horizontal"].as<int>();
    nyRequested = vm["resolution-vertical"].as<int>();
    totalRanks = vm["blocks"].as<int>();
    outputBaseName = vm["output-basepath"].as<std::string>();
    localTimestepping = vm["local-timestepping"].as<bool>();
#ifdef ASAGI
    batFile = vm["bathymetry-file"].as<std::string>();
   displFile = vm["displacement-file"].as<std::string>();
#endif



    int localityCount = hpx::get_num_localities().get();
    int localityNumber = hpx::get_locality_id();

    hpx::cout << "Locality " << localityNumber << "of " << localityCount << " Localities" << std::endl;

    SWE_Hpx_No_Component comp(totalRanks,localityNumber,localityCount,simulationDuration,numberOfCheckPoints,
                         nxRequested,nyRequested,outputBaseName,batFile,displFile,localTimestepping );

    comp.run();


    return hpx::finalize();
}
int main(int argc, char** argv) {


    /**************
     * INIT INPUT *
     **************/
    using namespace boost::program_options;

    options_description desc_commandline;



    desc_commandline.add_options()
     ("bathymetry-file,b", value<std::string>(),"File containing the bathymetry")
     ("displacement-file,d",value<std::string>(), "File containing the displacement")
     ("simulation-duration,e", value<float>()->default_value(100),"Time in seconds to simulate")
     ("checkpoint-count,n", value<int>()->default_value(100),"Number of simulation snapshots to be written")
     ("resolution-horizontal",value<int>()->default_value(100), "Number of simulation cells in horizontal direction")
     ("resolution-vertical", value<int>()->default_value(100),"Number of simulated cells in y-direction")
     ("output-basepath,o", value<std::string>()->default_value("lolo"),"Output base file name")
            ("blocks,", value<int>()->default_value(1),"Number of swe blocks")
    ;

    // Initialize and run HPX, this example requires to run hpx_main on all
    // localities
    std::vector<std::string> const cfg = {
            "hpx.run_hpx_main!=1"
    };

    return hpx::init(desc_commandline,argc, argv,cfg);
}
