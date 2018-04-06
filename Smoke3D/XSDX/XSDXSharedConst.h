//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#ifndef TAA_SHOW_DIFF
#define TAA_CLIP_DIFF	1
#define TAA_FRAME_DIFF	2
#define TAA_MOTION		3
#define TAA_SHOW_DIFF	0
#endif

#ifndef	TEMPORAL_SSAA
#define TEMPORAL_SSAA	1
#endif
#ifndef	TEMPORAL_MSAA
#define TEMPORAL_MSAA	2
#endif

#ifndef	TEMPORAL_AA
#define TEMPORAL_AA		TEMPORAL_SSAA
#endif

#ifndef	TEMPORAL
#define TEMPORAL		TEMPORAL_AA
#endif

#ifndef NUM_CASCADE
#define	NUM_CASCADE		3
#endif

#define	PIDIV4		0.785398163f

static const float g_fFOVAngleY	= PIDIV4;
static const float g_fZNear		= 1.0f;
static const float g_fZFar		= 1000.0f;
