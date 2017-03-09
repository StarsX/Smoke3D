//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const float g_fDecay = 0.95;

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<half3>	g_rwVelocity	: register(u0);
RWTexture3D<half>	g_rwDensity		: register(u1);
Texture3D<half3>	g_roPhiVel		: register(t0);
Texture3D<half>		g_roPhiDen		: register(t1);
Texture3D<half3>	g_roPhiHatVel	: register(t2);
Texture3D<half>		g_roPhiHatDen	: register(t3);

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
SamplerState		g_smpDefault	: register (s0);
SamplerState		g_smpLinear		: register (s1);
SamplerState		g_smpPoint		: register (s2);

//--------------------------------------------------------------------------------------
// Advection
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Velocity tracing
	const half3 vU = g_roPhiVel[DTid];
	const half3 vDisp = vU * g_fDeltaTime;
	const half3 vTex = (DTid + 0.5) * g_vTexel - vDisp;
	const half3 vTexTx = floor(DTid + 1.0 - vDisp / g_vTexel) * g_vTexel;

	const half3 vHTexel = 0.5 * g_vTexel;
	const half3 vTexN[8] =
	{
		vTexTx + half3(-vHTexel.x, -vHTexel.y, -vHTexel.z),
		vTexTx + half3(-vHTexel.x, -vHTexel.y, vHTexel.z),
		vTexTx + half3(-vHTexel.x, vHTexel.y, -vHTexel.z),
		vTexTx + half3(-vHTexel.x, vHTexel.y, vHTexel.z),
		vTexTx + half3(vHTexel.x, -vHTexel.y, -vHTexel.z),
		vTexTx + half3(vHTexel.x, -vHTexel.y, vHTexel.z),
		vTexTx + half3(vHTexel.x, vHTexel.y, -vHTexel.z),
		vTexTx + half3(vHTexel.x, vHTexel.y, vHTexel.z)
	};

	half3 vPhiN[8];
	half fPhiN[8];
	for (uint i = 0; i < 8; ++i)
	{
		vPhiN[i] = g_roPhiVel.SampleLevel(g_smpPoint, vTexN[i], 0);
		fPhiN[i] = g_roPhiDen.SampleLevel(g_smpPoint, vTexN[i], 0);
	}

	half3 vPhiMin = min(min(min(vPhiN[0], vPhiN[1]), vPhiN[2]), vPhiN[3]);
	vPhiMin = min(min(min(min(vPhiMin, vPhiN[4]), vPhiN[5]), vPhiN[6]), vPhiN[7]);
	half3 vPhiMax = max(max(max(vPhiN[0], vPhiN[1]), vPhiN[2]), vPhiN[3]);
	vPhiMax = max(max(max(max(vPhiMax, vPhiN[4]), vPhiN[5]), vPhiN[6]), vPhiN[7]);

	half fPhiMin = min(min(min(fPhiN[0], fPhiN[1]), fPhiN[2]), fPhiN[3]);
	fPhiMin = min(min(min(min(fPhiMin, fPhiN[4]), fPhiN[5]), fPhiN[6]), fPhiN[7]);
	half fPhiMax = max(max(max(fPhiN[0], fPhiN[1]), fPhiN[2]), fPhiN[3]);
	fPhiMax = max(max(max(max(fPhiMax, fPhiN[4]), fPhiN[5]), fPhiN[6]), fPhiN[7]);

	const half3 vPhiAdv = g_roPhiVel.SampleLevel(g_smpLinear, vTex, 0);
	const half fPhiAdv = g_roPhiDen.SampleLevel(g_smpLinear, vTex, 0);

	const half3 vVelocity = vPhiAdv + 0.5 * (g_roPhiVel[DTid] - g_roPhiHatVel[DTid]);
	const half fDensity = fPhiAdv + 0.5 * (g_roPhiDen[DTid] - g_roPhiHatDen[DTid]);

	// Update velocity and density
	g_rwVelocity[DTid] = clamp(vVelocity, vPhiMin, vPhiMax);
	g_rwDensity[DTid] = clamp(fDensity, fPhiMin, fPhiMax) * g_fDecay;
}
