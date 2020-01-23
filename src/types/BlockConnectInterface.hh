#ifndef __BLOCKCONNECTINTERFACE_HH
#define __BLOCKCONNECTINTERFACE_HH

#include "types/Boundary.hh"

#ifdef UPCXX
#include <upcxx/upcxx.hpp>
#endif

template <typename T>
struct BlockConnectInterface {
	Boundary boundary;
	int size;
	int stride;
	int startIndex;
	T pointerH;
	T pointerB;
	T pointerHu;
	T pointerHv;
    T pointerTimestep;
#ifdef UPCXX
    upcxx::global_ptr<std::atomic<bool>> dataTransmitted;
    upcxx::global_ptr<std::atomic<bool>> dataReady;
    upcxx::global_ptr<int> iteration;
    int rank;
#endif
};

#endif // __BLOCKCONNECTINTERFACE_HH
