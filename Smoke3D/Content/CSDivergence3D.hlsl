//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CSDivergence3D.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<min16float>	g_rwDivergence	: register (u0);
Texture3D<min16float3>	g_roVector		: register (t0);

//--------------------------------------------------------------------------------------
// Compute divergence
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	g_rwDivergence[DTid] = Divergence3D(g_roVector, DTid);
}
