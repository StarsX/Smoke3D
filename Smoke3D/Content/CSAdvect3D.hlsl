//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float g_fDissipation = 0.996;

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<half3>	g_rwVelocity	: register(u0);
RWTexture3D<half>	g_rwDensity		: register(u1);
Texture3D<half3>	g_roVelocity	: register(t0);
Texture3D<half>		g_roDensity		: register(t1);

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
	const half3 vU = g_roVelocity[DTid];
	const half3 vTex = (DTid + 0.5) * g_vTexel - vU;

	// Update velocity and density
	g_rwVelocity[DTid] = g_roVelocity.SampleLevel(g_smpLinear, vTex, 0);
	g_rwDensity[DTid] = g_roDensity.SampleLevel(g_smpLinear, vTex, 0) * g_fDissipation;
}
