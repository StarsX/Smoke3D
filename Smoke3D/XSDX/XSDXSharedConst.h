//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#ifndef	TEMPORAL_AA
#define TEMPORAL_AA
#endif

#ifdef TEMPORAL_AA
#define TEMPORAL
#endif

#ifndef NUM_CASCADE
#define	NUM_CASCADE	3
#endif

#define	PIDIV4		0.785398163f

static const float g_fFOVAngleY	= PIDIV4;
static const float g_fZNear		= 1.0f;
static const float g_fZFar		= 1000.0f;
