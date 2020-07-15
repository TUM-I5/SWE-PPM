#ifndef __CONSTANTS_HH
#define __CONSTANTS_HH

const float g = 9.81;
const float defaultDryTol = 0.1;
const float defaultCflNumber = 0.4;

// MPI Tags
const int MPI_TAG_TIMESTEP_LEFT = 1;
const int MPI_TAG_TIMESTEP_RIGHT = 2;
const int MPI_TAG_TIMESTEP_TOP = 3;
const int MPI_TAG_TIMESTEP_BOTTOM = 4;

const int MPI_TAG_OUT_H_LEFT = 5;
const int MPI_TAG_OUT_B_LEFT = 6;
const int MPI_TAG_OUT_HU_LEFT = 7;
const int MPI_TAG_OUT_HV_LEFT = 8;

const int MPI_TAG_OUT_H_RIGHT = 9;
const int MPI_TAG_OUT_B_RIGHT = 10;
const int MPI_TAG_OUT_HU_RIGHT = 11;
const int MPI_TAG_OUT_HV_RIGHT = 12;

const int MPI_TAG_OUT_H_BOTTOM = 13;
const int MPI_TAG_OUT_B_BOTTOM = 14;
const int MPI_TAG_OUT_HU_BOTTOM = 15;
const int MPI_TAG_OUT_HV_BOTTOM = 16;

const int MPI_TAG_OUT_H_TOP = 17;
const int MPI_TAG_OUT_B_TOP = 18;
const int MPI_TAG_OUT_HU_TOP = 19;
const int MPI_TAG_OUT_HV_TOP = 20;

#endif // __CONSTANTS_HH
