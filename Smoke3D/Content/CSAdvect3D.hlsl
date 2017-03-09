//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
#ifdef _MACCORMACK_
static const float g_fDecay = 1.0;
#else
static const float g_fDecay = 0.996;
#endif

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<half3>	g_rwPhiVel		: register(u0);
RWTexture3D<half>	g_rwPhiDen		: register(u1);
Texture3D<half3>	g_roPhiVel		: register(t0);
Texture3D<half>		g_roPhiDen		: register(t1);
Texture3D<half3>	g_roVelocity	: register(t2);

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
	const half3 vTex = (DTid + 0.5) * g_vTexel - vU * g_fDeltaTime;

	// Update velocity and density
	g_rwPhiVel[DTid] = g_roPhiVel.SampleLevel(g_smpLinear, vTex, 0);
	g_rwPhiDen[DTid] = g_roPhiDen.SampleLevel(g_smpLinear, vTex, 0) * g_fDecay;
}
