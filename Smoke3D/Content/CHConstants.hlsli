//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "SharedMacros.h"

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
cbuffer cbImmutable	: register(b0)
{
	float3	g_vTexel;
};

cbuffer cbPerFrame	: register(b1)
{
	float3	g_vForce;
	float	g_fDens;
	float3	g_vImLoc;		// Impulse location
	float	g_fDeltaTime;
};
