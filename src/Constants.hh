#ifndef __CONSTANTS_HH
#define __CONSTANTS_HH

const float g = 9.81;
const float defaultDryTol = 0.1;
const float defaultCflNumber = 0.4;

// MPI Tags
const int MPI_TAG_TIMESTEP = 1;

const int MPI_TAG_OUT_H_LEFT = 10;
const int MPI_TAG_OUT_B_LEFT = 11;
const int MPI_TAG_OUT_HU_LEFT = 12;
const int MPI_TAG_OUT_HV_LEFT = 13;

const int MPI_TAG_OUT_H_RIGHT = 20;
const int MPI_TAG_OUT_B_RIGHT = 21;
const int MPI_TAG_OUT_HU_RIGHT = 22;
const int MPI_TAG_OUT_HV_RIGHT = 23;

const int MPI_TAG_OUT_H_BOTTOM = 30;
const int MPI_TAG_OUT_B_BOTTOM = 31;
const int MPI_TAG_OUT_HU_BOTTOM = 32;
const int MPI_TAG_OUT_HV_BOTTOM = 33;

const int MPI_TAG_OUT_H_TOP = 40;
const int MPI_TAG_OUT_B_TOP = 41;
const int MPI_TAG_OUT_HU_TOP = 42;
const int MPI_TAG_OUT_HV_TOP = 43;

#endif // __CONSTANTS_HH
