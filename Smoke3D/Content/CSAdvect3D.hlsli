//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<min16float>	g_rwScalar		: register (u0);
Texture3D<min16float>	g_roScalar		: register (t0);
Texture3D<min16float3>	g_roVelocity	: register (t1);

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
SamplerState		g_smpDefault	: register (s0);
SamplerState		g_smpLinear		: register (s1);

//--------------------------------------------------------------------------------------
// Advection
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Velocity tracing
	const float3 vU = g_roVelocity[DTid];
	const float3 vTex = (DTid + 0.5) * g_vTexel - vU * g_fDeltaTime;

	// Update
	g_rwScalar[DTid] = g_roScalar.SampleLevel(g_smpLinear, vTex, 0);
}
