//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#define THREAD_BLOCK_X		8
#define THREAD_BLOCK_Y		8
#define THREAD_BLOCK_Z		8

#define _MACCORMACK_

#ifdef _MACCORMACK_
#define REST_DENS			1.0
#else
#define REST_DENS			0.75
#endif
