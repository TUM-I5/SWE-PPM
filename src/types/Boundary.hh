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

#endif // __BOUNDARY_HH
