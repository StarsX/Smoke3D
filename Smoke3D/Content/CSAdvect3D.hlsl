//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const min16float2 g_vDecay = { 0.996, 1.0 };

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<min16float3>	g_rwPhiVel		: register (u0);
RWTexture3D<min16float>		g_rwPhiDen		: register (u1);
Texture3D<min16float3>		g_roPhiVel		: register (t0);
Texture3D<min16float>		g_roPhiDen		: register (t1);
Texture3D<min16float3>		g_roVelocity	: register (t2);

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

	// Update velocity and density
	g_rwPhiVel[DTid] = g_roPhiVel.SampleLevel(g_smpLinear, vTex, 0);
	g_rwPhiDen[DTid] = g_roPhiDen.SampleLevel(g_smpLinear, vTex, 0) * g_vDecay[g_fMacCormack];
}
