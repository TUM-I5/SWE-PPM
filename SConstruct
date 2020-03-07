#! /usr/bin/python

# @file
# This file is part of SWE.
#
# @author Alexander Breuer
#  (breuera AT in.tum.de,
#   http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer)
# @author Sebastian Rettenberger
#  (rettenbs AT in.tum.de,
#   http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
#
# @section LICENSE
#
# SWE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SWE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SWE.  If not, see <http://www.gnu.org/licenses/>.
#
#
# @section DESCRIPTION
#
# Builds the SWE code with several options.
#

import os
import sys
from subprocess import check_output
# Explicitly import SCons symbols for proper linting
from SCons.Script import (Environment, Variables, PathVariable,
                          EnumVariable, BoolVariable, Help, Exit,
                          Export, Import, Builder, SConscript)
########
# INIT #
########

# Welcome message
print('****************************************')
print('** Welcome to the build script of SWE **')
print('****************************************')
print('SWE Copyright (C) 2012-2013')
print('')
print('  Technische Universitaet Muenchen')
print('  Department of Informatics')
print('  Chair of Scientific Computing')
print('  http://www5.in.tum.de/SWE')
print('')
print('SWE comes with ABSOLUTELY NO WARRANTY.')
print('SWE is free software, and you are welcome to redistribute it')
print('under certain conditions.')
print('Details can be found in the file \'gpl.txt\'.')
print('')
vars = Variables()

# read parameters from a file if given
vars.AddVariables(
        PathVariable('buildVariablesFile',
                     ('location of the python file, '
                      'which contains the build variables'),
                     None, PathVariable.PathIsFile)
                  )
env = Environment(variables=vars)
if 'buildVariablesFile' in env:
    vars = Variables(env['buildVariablesFile'])

# Compile time options
vars.AddVariables(
        # Build directory
        PathVariable('buildDir',
                     'where to build the code',
                     'build',
                     PathVariable.PathIsDirCreate),

        # Compiler
        EnumVariable('compiler',
                     'compiler to be used',
                     'gnu',
                     allowed_values=('gnu', 'intel', 'cray')
                     ),

        # Channel
        EnumVariable('compileMode',
                     'mode of the compilation',
                     'release',
                     allowed_values=('debug', 'release')
                     ),

        # SIMD settings
        EnumVariable('simdExtensions',

                     ('SIMD extensions used for vectorization '
                      '(for intrinsics code)'),

                     'NONE',
                     allowed_values=('NONE', 'SSE4', 'AVX')
                     ),

        # Parallelization
        EnumVariable('parallelization',
                     'level of parallelization',
                     'none',
                     allowed_values=('none', 'cuda', 'mpi_with_cuda',
                                     'mpi', 'ampi', 'charm', 'upcxx', 'hpx', 'chameleon')
                     ),
        BoolVariable('openmp',
                     'compile with OpenMP parallelization enabled',
                     False),

        # Compute capability
        EnumVariable('computeCapability',

                     ('optional architecture/compute capability '
                      'of the CUDA card'),

                     'sm_20',
                     allowed_values=('sm_10', 'sm_11', 'sm_12', 'sm_13',
                                     'sm_20', 'sm_21', 'sm_22', 'sm_23',
                                     'sm_30', 'sm_35')
                     ),

        # OpenGL
        BoolVariable('openGL', 'compile with OpenGL visualization', False),
        BoolVariable('openGL_instr',
                     'add instructions to openGL version (requires SDL_ttf)',
                     False),


        # NetCDF input/output
        BoolVariable('writeNetCDF',
                     'write output in the netCDF-format',
                     False),

        # ASAGI input
        BoolVariable('asagi',
                     'use ASAGI',
                     False),
        PathVariable('asagiInputDir',
                     'location of netcdf input files',
                     '',
                     PathVariable.PathAccept),

        # Solver
        EnumVariable('solver', 'Riemann solver', 'augrie',
                     allowed_values=('rusanov', 'fwave', 'augrie', 'hybrid',
                                     'fwavevec', 'augriefun', 'augrie_simd')
                     ),

        # Vectorization
        BoolVariable('vectorize',
                     'add pragmas to help vectorization (release only)',
                     False),
        BoolVariable('showVectorization',
                     'show loop vectorization (Intel compiler only)',
                     False),

        # Platform
        EnumVariable('platform',
                     'compile for a specific platform (Intel compiler only)',
                     'default',
                     allowed_values=('default', 'mic')
                     ),

        # Runtime parameters
        BoolVariable('xmlRuntime',
                     'use a xml-file for runtime parameters',
                     False),

        BoolVariable('copyenv',
                     'copy the whole environment',
                     False),

        # FLOP measuring
        BoolVariable('countflops',
                     'enable flop counting; defines the macro COUNTFLOPS',
                     False),

        BoolVariable('itt',
                     'enable itt api interface',
                     False)
)

# external variables
vars.AddVariables(
        PathVariable('cudaToolkitDir', 'location of the CUDA toolkit', None),
        PathVariable('libSDLDir', 'location of libSDL', None),
        PathVariable('netCDFDir', 'location of netCDF', None),
        PathVariable('asagiDir', 'location of ASAGI', None),
        PathVariable('libxmlDir', 'location of libxml2', None)
                  )

# set environment
env = Environment(ENV={'PATH': os.environ['PATH']},
                  variables=vars)

# generate help text
Help(vars.GenerateHelpText(env))

# handle unknown, maybe misspelled variables
unknownVariables = vars.UnknownVariables()

# remove the buildVariablesFile from the list
# of unknown variables (used before)
if 'buildVariablesFile' in unknownVariables:
    unknownVariables.pop('buildVariablesFile')

# exit in the case of unknown variables
if unknownVariables:
    print(sys.stderr,
          "*** The following build variables are unknown:",
          unknownVariables.keys())
    Exit(1)

# valid solver for CUDA?
if (env['parallelization'] in ['cuda', 'mpi_with_cuda'] and
        env['solver'] not in ['rusanov', 'fwave', 'augrie']):
    print(sys.stderr,
          '** The "' + env['solver'] + '" solver is not supported in CUDA.')
    Exit(3)

# CUDA parallelization for openGL
if env['parallelization'] != 'cuda' and env['openGL']:
    print(sys.stderr,
          ('** The parallelization \"' + env['parallelization'] + '\" '
           'does not support OpenGL visualization (CUDA only).'))
    Exit(3)

# Copy whole environment?
if env['copyenv']:
    env.AppendUnique(ENV=os.environ, delete_existing=1)

#######################
# UPCXX Specific Init #
#######################

if env['parallelization'] == 'upcxx':
    env.Append(CCFLAGS=['-std=c++14'])
    # get the upcxx folder
    upcxxInstall = os.environ['UPCXX_PATH']
    if upcxxInstall == '':
        print(sys.stderr,
              'No UPC++ installation found. Did you set $UPCXX_PATH?')
        Exit(3)
    else:
        print("Found UPC++ install at: " + upcxxInstall)

    # point to the upcxx native upcxx-meta tool
    upcxxMeta =upcxxInstall + '/bin/upcxx-meta'

    # let upcxx-meta tell us the necessary compiler flags
    upcxxPpFlags = check_output([upcxxMeta, 'PPFLAGS']).split()
    upcxxLdFlags = check_output([upcxxMeta, 'LDFLAGS']).split()
    upcxxLibFlags = check_output([upcxxMeta, 'LIBFLAGS']).split()

    # SCons likes special formats!
    # cut the -D/-I/-L/-l prefixes from the options
    upcxxDefines = [i[2:] for i in upcxxPpFlags if i.startswith("-D")]
    upcxxIncludes = [i[2:] for i in upcxxPpFlags if i.startswith("-I")]
    upcxxLibPaths = [i[2:] for i in upcxxLibFlags if i.startswith("-L")]
    upcxxLibs = [i[2:] for i in upcxxLibFlags if i.startswith("-l")]
#######################
# HPX Specific Init #
#######################

if env['parallelization'] == 'hpx':
    env.Append(CCFLAGS=['-std=c++17'])
    # get the upcxx folder
    hpxInstall = os.environ['HPX_PATH']
    if hpxInstall == '':
        print(sys.stderr,
              'No HPX installation found. Did you set $HPX_PATH?')
        Exit(3)
    else:
        print("Found HPX install at: " + hpxInstall)

    boostInstall = os.environ['BOOST_ROOT']
    if boostInstall == '':
        print(sys.stderr,
              'No BOOST installation found. Did you set $BOOST_ROOT?')
        Exit(3)
    else:
        print("Found BOOST install at: " + boostInstall)

    hwInstall = os.environ['HWLOC_ROOT']
    if hwInstall == '':
        print(sys.stderr,
              'No HWLOC installation found. Did you set $HWLOC_ROOT?')
        Exit(3)
    else:
        print("Found HWLOC install at: " + hwInstall)
################################
# Charm++ / AMPI specific Init #
################################

if env['parallelization'] in ['charm', 'ampi']:
    # Path to the compiler
    charmInstall = os.environ['CHARM_PATH']
    if charmInstall == '':
        print(sys.stderr,
              'No Charm++ installation found. Did you set $CHARM_PATH?')
        Exit(3)
    else:
        print("Found Charm++ install at: " + charmInstall)

    # Custom builder for *.def.h and *.decl.h / Charm++ specific files

    def modifyTargets(target, source, env):
        target.append('${SOURCE.basename}.def.h')
        return target, source
    buildString = "cd ${SOURCE.dir};" +charmInstall + '/bin/charmc ' + '${SOURCE.file}'
    charmBuilder = Builder(action=buildString,
                           suffix='.decl.h',
                           src_suffix='.ci')

    env.Append(BUILDERS={'charmBuilder': charmBuilder})
    env.charmBuilder("src/blocks/SWE_DimensionalSplittingCharm.ci")
    ################################
# Chameleon specific
################################

if env['parallelization'] in ['chameleon']:
    env.Append(CCFLAGS=['-std=c++11'])
    env.Append(CCFLAGS=['-lchameleon', '-lm', '-lstdc++'])
    # get the chameleon folder
    libfabricPath = os.environ['LIBFABRIC_PATH']
    chameleonPath = os.environ['CHAM_PATH']
    if chameleonPath == '':
        print(sys.stderr,
              'No chameleon installation found. Did you set $CHAM_PATH?')
        Exit(3)
    else:
        print("Trying to find chameleon install at: " + chameleonPath
	)
    env.Append(CCFLAGS=['-I'+chameleonPath+'/include/'])
    env.Append(CCFLAGS=['-L'+chameleonPath+'/lib/'])
    env.Append(LINKFLAGS=['-L'+chameleonPath+'/lib/'])
    env.Append(LINKFLAGS=['-L'+libfabricPath])
    env.Append(LINKFLAGS=['-lchameleon'])
    env.Append(LINKFLAGS=['-lfabric'])
    env.Append(LINKFLAGS=['-lifcore'])
    env.Append(LINKFLAGS=['-lirng'])
    env.Append(RPATH=[chameleonPath+'/lib/'])

################################
# StarPU specific
################################

if env['parallelization'] in ['starpu']:
    # get the starpu folder
    starpuPath = os.environ['STARPU_PATH']
    if starpuPath == '':
        print(sys.stderr,
              'No StarPU installation found. Did you set $STARPU_PATH?')
        Exit(3)
    else:
        print("Trying to find StarPU install at: " + starpuPath)
    env.Append(CCFLAGS=['-I'+starpuPath+'/include/'])
    env.Append(CCFLAGS=['-I'+starpuPath+'/mpi/include/'])
    env.Append(CCFLAGS=['-L'+starpuPath+'/lib/'])
    env.Append(LINKFLAGS=['-L'+starpuPath+'/lib/'])
    env.Append(LINKFLAGS=['-lfabric'])
    env.Append(LINKFLAGS=['-lifcore'])
    env.Append(LINKFLAGS=['-lirng'])
    env.Append(LINKFLAGS=['-lstarpu-1.3'])
    env.Append(LINKFLAGS=['-lstarpumpi-1.3'])

#####################################
# Precompiler/Compiler/Linker flags #
#####################################

# Select the compiler (MPI and/or Intel, GNU is default)
if env['parallelization'] in ['mpi', 'mpi_with_cuda']:
    env.Append(CCFLAGS=['-std=c++11'])
    if env['compiler'] == 'cray':
        env['CXX'] = 'CC'
    else:
        env['CXX'] = env['LINKERFORPROGRAMS'] = env.Detect(['mpiCC', 'mpicxx'])

    if not env['CXX']:
        print(sys.stderr,
              ('** MPI compiler not found, '
               'please update PATH environment variable'))
        Exit(1)

    if env['compiler'] == 'intel':
        # TODO: ???
        # We need to the the mpiCC wrapper which compiler it should use
        # Here are several environment variables that
        # do the job for different MPI libraries
	envVars = ['OMPI_CXX', 'MPICH_CXX']
        for var in envVars:
            env['ENV'][var] = 'icpc'
elif env['parallelization'] == 'ampi':
    env['CXX'] = charmInstall + '/bin/ampicc'
elif env['parallelization'] == 'charm':
    env['CXX'] = charmInstall + '/bin/charmc' # ' -balancer NeighborLB'
elif env['parallelization'] == 'upcxx':
    env['CXX'] = check_output([upcxxMeta, 'CXX']).replace("\n", "")
elif env['parallelization'] == 'chameleon':
    env['CXX'] = 'mpicxx'
    #env['CXX'] = env['LINKERFORPROGRAMS'] = env.Detect(['mpiCC', 'mpicxx'])
elif env['parallelization'] == 'starpu':
    env['CXX'] = 'mpiicpc'
else:
    if env['compiler'] == 'intel':
        env['CXX'] = 'icpc'
    elif env['compiler'] == 'cray':
        env['CXX'] = env['LINKERFORPROGRAMS'] = 'CC'

# eclipse specific flag
if env['compiler'] != 'cray':
    env.Append(CCFLAGS=['-fmessage-length=0'])

# xml parameters for the compiler TODO

# set (pre-)compiler flags for the compile modes
if env['compileMode'] == 'debug':
    env.Append(CPPDEFINES=['DEBUG'])
    if env['compiler'] == 'gnu':
        env.Append(CCFLAGS=['-O0', '-g3', '-Wall' ])
    elif env['compiler'] == 'intel':
        env.Append(CCFLAGS=['-O0', '-g'])
    elif env['compiler'] == 'cray':
        env.Append(CCFLAGS=['-O0'])
elif env['compileMode'] == 'release':
    env.Append(CPPDEFINES=['NDEBUG'])
    if env['compiler'] == 'gnu':
        env.Append(CCFLAGS=['-O3', '-mtune=native'])
    elif env['compiler'] == 'intel':
        env.Append(CCFLAGS=['-O2', '-std=c++0x'])
    # especially for env['compiler'] == 'cray'
    else:
        env.Append(CCFLAGS=['-O3'])

# Other compiler flags (for all compilers)
if env['compiler'] != 'cray':
    env.Append(CCFLAGS=['-fstrict-aliasing', '-fargument-noalias'])
    # env.Append(CCFLAGS=['-fno-strict-aliasing',
    # '-fargument-noalias', '-g', '-g3', '-ggdb',
    # '-Wall', '-Wextra', '-Wstrict-aliasing=2'])

# Vectorization via Intrinsics
if env['simdExtensions'] == 'SSE4':
    env.Append(CCFLAGS=['-msse4'])
elif env['simdExtensions'] == 'AVX':
    env.Append(CCFLAGS=['-mavx'])

if env['countflops']:
    env.Append(CCFLAGS=['-DCOUNTFLOPS'])

# Vectorization?
if env['compileMode'] == 'release' and env['vectorize']:
    env.Append(CPPDEFINES=['VECTORIZE'])
    if env['compiler'] == 'intel' and env['platform'] != 'mic':
        env.Append(CCFLAGS=['-xHost'])

if env['compiler'] == 'intel' and env['showVectorization']:
    #env.Append(CCFLAGS=['-vec-report2', '-opt-report']) 
    env.Append(CCFLAGS=['-qopt-report-phase=vec', '-qopt-report=2'])
# OpenMP parallelism
# TODO Refactor: This can probably be an else if
if env['openmp']:
    if env['compiler'] == 'intel':
        env.Append(CCFLAGS=['-qopenmp'])
        env.Append(LINKFLAGS=['-qopenmp'])

    # cray: OpenMP turned on by default
    if env['compiler'] == 'gnu':
        env.Append(CCFLAGS=['-fopenmp'])
        env.Append(LINKFLAGS=['-fopenmp'])

# Platform
if env['compiler'] == 'intel' and env['platform'] == 'mic':
    env.Append(CCFLAGS=['-mmic'])
    env.Append(LINKFLAGS=['-mmic'])

# Compiler
if env['compiler'] == 'intel':
    # Add Intel specific libraries
    env.Append(LIBS=['svml', 'imf', 'intlc'])

# Add source directory to include path (important for subdirectories)
env.Append(CPPPATH=['.'])
env.Append(CPPPATH=['include'])

# upcxx specific compiler options
if env['parallelization'] == 'upcxx':
    env.Append(CPPDEFINES=['UPCXX'])
    env.Append(CPPDEFINES=upcxxDefines)
    env.Append(CPPPATH=upcxxIncludes)
    env.Append(LIBS=upcxxLibs)
    env.Append(LIBPATH=upcxxLibPaths)
if env['parallelization'] == 'hpx':

    boostInstallLib = boostInstall+ '/lib/'
    hpxInstallLib = hpxInstall + '/lib/'
    #boostInstallLib = boostInstall
    hwInstallLib = hwInstall + '/lib/'
    hpxIncDir=[hpxInstall+'/include',boostInstall+'/include', hwInstall+'/include']
    init_lib = File(hpxInstallLib+'libhpx_init.a')
    hpxLinDir=[init_lib,'libhpx.so','libhpx_iostreams.so'
        ,'libboost_atomic.so'
        ,'libboost_filesystem.so'
        ,'libboost_program_options.so'
        ,'libboost_regex.so'
        ,'libboost_system.so','-lpthread'
               #$(TCMALLOC_ROOT)/libtcmalloc_minimal.so
        ,'libhwloc.so', '-ldl' ,'-lrt']


    env.Append(LIBS=hpxLinDir)
    env.Append(CPPPATH=hpxIncDir)
    env.Append(LIBS=[init_lib])
    #env.Append(LINKFLAGS=['-Wl,-wrap=main'])
    env.Append(LIBPATH=[hpxInstallLib,boostInstallLib,hwInstallLib])

    env.Append(RPATH=[hpxInstallLib, boostInstallLib, hwInstallLib])



if env['parallelization'] == 'charm':
    env.Append(CPPDEFINES=['CHARM'])

# set the precompiler variables for the solver
if env['solver'] == 'fwave':
    env.Append(CPPDEFINES=['WAVE_PROPAGATION_SOLVER=1'])
elif env['solver'] == 'augrie' or env['solver'] == 'augriefun':
    env.Append(CPPDEFINES=['WAVE_PROPAGATION_SOLVER=2'])
elif env['solver'] == 'hybrid':
    env.Append(CPPDEFINES=['WAVE_PROPAGATION_SOLVER=0'])
elif env['solver'] == 'fwavevec':
    env.Append(CPPDEFINES=['WAVE_PROPAGATION_SOLVER=4'])
elif env['solver'] == 'augrie_simd':
    env.Append(CPPDEFINES=['WAVE_PROPAGATION_SOLVER=5'])

# set the precompiler flags for CUDA
if env['parallelization'] in ['cuda', 'mpi_with_cuda']:
    env.Append(CPPDEFINES=['CUDA'])

    # set the directories for the CudaTool
    if 'cudaToolkitDir' in env:
        env['CUDA_TOOLKIT_PATH'] = env['cudaToolkitDir']

    env.Tool('CudaTool', toolpath=['.'])

    # set precompiler flag for nvcc
    env.Append(NVCCFLAGS=['-DCUDA'])

    if env['solver'] == 'augrie':
        env.Append(NVCCFLAGS=['-DCUDA_AUGRIE'])

    # set the compute capability of the cuda compiler
    # (needs to be set after the CudaTool
    env.Append(NVCCFLAGS=['--gpu-architecture='+env['computeCapability']])

    # Append the source directory to the include path
    env.Append(NVCCFLAGS=['-Isrc'])

    # compile explicitly with 64-bit on Mac OS X
    if env['PLATFORM'] == 'darwin':
        env.Append(NVCCFLAGS=' -m64')

# set the nvcc precompiler flags for MPI (CUDA)
if env['parallelization'] == 'mpi_with_cuda':
    env.Append(NVCCFLAGS=['-DUSEMPI'])

# set the precompiler flags for MPI (C++)
if env['parallelization'] in ['mpi_with_cuda', 'mpi', 'ampi']:
    env.Append(CPPDEFINES=['USEMPI'])

if env['openGL']:
    env.Append(LIBS=['SDL', 'GL', 'GLU'])
    if env['openGL_instr']:
        # We assume that SDL_ttf is in the same directory as SDL
        env.Append(LIBS=['SDL_ttf'])
        env.Append(CPPDEFINES=['USESDLTTF'])

# set the compiler flags for libSDL
if 'libSDLDir' in env:
    env.Append(CPPPATH=[env['libSDLDir']+'/include'])
    env.Append(LIBPATH=[env['libSDLDir']+'/lib'])
    env.Append(RPATH=[env['libSDLDir']+'/lib'])

# set the precompiler flags and includes for netCDF
if env['writeNetCDF']:
    env.Append(CPPDEFINES=['WRITENETCDF'])
    env.Append(LIBS=['netcdf'])
    path_netcdf=os.environ['NETCDF_BASE']
	# set netCDF location
    env.Append(CPPPATH=[path_netcdf+'/include'])
    env.Append(LIBPATH=[path_netcdf+ '/lib'])
    env.Append(RPATH=[path_netcdf+ '/lib'])

# set the precompiler flags, includes and libraries for ASAGI
if env['asagi']:
    env.Append(CPPDEFINES=['ASAGI'])
    if (env['parallelization'] == 'none' or
            env['parallelization'] not in ['mpi', 'mpi_with_cuda', 'ampi']):
        env.Append(CPPDEFINES=['ASAGI_NOMPI'])
        env.Append(LIBS=['asagi_nompi'])
        env.Append(LIBPATH=['lib'])
    else:
        env.Append(LIBS=['asagi'])

    if 'asagiDir' in env:
        print(env['asagiDir'])
	env.Append(CPPPATH=[env['asagiDir']+'/include'])
        env.Append(LIBPATH=[env['asagiDir']+'/build'])
        env.Append(RPATH=[os.path.join(env['asagiDir'], 'build')])

    if 'netCDFDir' in env:
        env.Append(LIBPATH=[env['netCDFDir']+'/lib'])
        env.Append(RPATH=[os.path.join(env['netCDFDir'], 'lib')])

    if 'asagiInputDir' in env:
        env.Append(CPPFLAGS=['\'-DASAGI_INPUT_DIR="'
                             + env['asagiInputDir'] + '"\''])

if env['itt']:
    env.Append(CPPDEFINES=['ITT'])
    env.Append(LIBS=['ittnotify'])

# xml runtime parameters
if env['xmlRuntime']:  # TODO
    print('xml runtime parameters are not implemented so far.')
    Exit(1)
    env.Append(CPPDEFINES=['READXML'])
    # set xmllib2 location
    if 'libxmlDir' in env:
        env.Append(CPPPATH=[env['libxmlDir']+'/include/libxml2'])
        env.Append(LIBPATH=[env['libxmlDir']+'/lib'])

#
# setup the program name and the build directory
#
program_name = 'SWE'

# compiler
if env['parallelization'] != 'ampi':
    program_name += '_' + env['compiler']
else:
    program_name += '_' + 'ampicc'

# compile mode
program_name += '_' + env['compileMode']

# parallelization
program_name += '_' + env['parallelization']

# using openmp?
if env['openmp']:
	program_name += '_omp'

# solver
program_name += '_' + env['solver']

# vectorization
if env['vectorize']:
    program_name += '_vec'

# build directory
build_dir = env['buildDir'] + '/build_' + program_name

# get the static object files
env.objects = []

Export('env')
SConscript('src/SConscript', variant_dir=build_dir, duplicate=0)
Import('env')

# build the program
env.Program('build/' + program_name, env.objects)
