//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CSGradient3D.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float2 g_vRestDen = { 0.75, 1.0 };

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWTexture3D<half3>	g_rwVelocity	: register (u0);
Texture3D<half3>	g_roVelocity	: register (t0);
Texture3D<half>		g_roPressure	: register (t1);

//--------------------------------------------------------------------------------------
// Projection
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Project the velocity onto its divergence-free component
	g_rwVelocity[DTid] = g_roVelocity[DTid] - Gradient3D(g_roPressure, DTid) / g_vRestDen[g_fMacCormack];
}
