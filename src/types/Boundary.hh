#ifndef __BOUNDARY_HH
#define __BOUNDARY_HH

// available boundary edges
typedef enum Boundary {
	BND_LEFT,
	BND_RIGHT,
	BND_BOTTOM,
	BND_TOP
} Boundary;

// available types of boundary conditions
typedef enum BoundaryType {
	OUTFLOW,
	WALL,
	INFLOW,
	CONNECT,
	PASSIVE
} BoundaryType;

/*
 * If Charm++ is used for parallelization, the framework needs to know how to serialize the enums
 * See http://charm.cs.illinois.edu/manuals/html/charm++/6_4.html
 * See https://lists.cs.illinois.edu/lists/arc/charm/2015-07/msg00017.html
 */
#ifdef CHARM
inline void operator|(PUP::er &p, Boundary &s) {
     pup_bytes(&p, (void *) &s, sizeof(Boundary));
}
inline void operator|(PUP::er &p, BoundaryType &s) {
     pup_bytes(&p, (void *) &s, sizeof(BoundaryType));
}
#endif

#endif // __BOUNDARY_HH
