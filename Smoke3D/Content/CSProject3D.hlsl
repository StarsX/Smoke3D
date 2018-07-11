//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CSGradient3D.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const min16float2 g_vRestDen = { 0.8, 1.0 };

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWTexture3D<min16float3>	g_rwVelocity	: register (u0);
Texture3D<min16float3>		g_txVelocity	: register (t0);
Texture3D<min16float>		g_txPressure	: register (t1);

//--------------------------------------------------------------------------------------
// Projection
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Project the velocity onto its divergence-free component
	g_rwVelocity[DTid] = g_txVelocity[DTid] - Gradient3D(g_txPressure, DTid) / g_vRestDen[g_fMacCormack];
}
