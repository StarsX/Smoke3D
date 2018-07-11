//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const min16float g_fDecay = 0.95;

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<min16float3>	g_rwVelocity	: register (u0);
RWTexture3D<min16float>		g_rwDensity		: register (u1);
Texture3D<min16float3>		g_txPhiVel		: register (t0);
Texture3D<min16float>		g_txPhiDen		: register (t1);
Texture3D<min16float3>		g_txPhiHatVel	: register (t2);
Texture3D<min16float>		g_txPhiHatDen	: register (t3);

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
	const float3 vU = g_txPhiVel[DTid];
	const float3 vDisp = vU * g_fDeltaTime;
	const float3 vTex = (DTid + 0.5) * g_vTexel - vDisp;
	const float3 vTexTx = floor(DTid + 1.0 - vDisp / g_vTexel) * g_vTexel;

	const float3 vHTexel = 0.5 * g_vTexel;
	const float3 vTexN[8] =
	{
		vTexTx + float3(-vHTexel.x, -vHTexel.y, -vHTexel.z),
		vTexTx + float3(-vHTexel.x, -vHTexel.y, vHTexel.z),
		vTexTx + float3(-vHTexel.x, vHTexel.y, -vHTexel.z),
		vTexTx + float3(-vHTexel.x, vHTexel.y, vHTexel.z),
		vTexTx + float3(vHTexel.x, -vHTexel.y, -vHTexel.z),
		vTexTx + float3(vHTexel.x, -vHTexel.y, vHTexel.z),
		vTexTx + float3(vHTexel.x, vHTexel.y, -vHTexel.z),
		vTexTx + float3(vHTexel.x, vHTexel.y, vHTexel.z)
	};

	min16float3 vPhiN[8];
	min16float fPhiN[8];
	for (uint i = 0; i < 8; ++i)
	{
		vPhiN[i] = g_txPhiVel.SampleLevel(g_smpPoint, vTexN[i], 0);
		fPhiN[i] = g_txPhiDen.SampleLevel(g_smpPoint, vTexN[i], 0);
	}

	min16float3 vPhiMin = min(min(min(vPhiN[0], vPhiN[1]), vPhiN[2]), vPhiN[3]);
	vPhiMin = min(min(min(min(vPhiMin, vPhiN[4]), vPhiN[5]), vPhiN[6]), vPhiN[7]);
	min16float3 vPhiMax = max(max(max(vPhiN[0], vPhiN[1]), vPhiN[2]), vPhiN[3]);
	vPhiMax = max(max(max(max(vPhiMax, vPhiN[4]), vPhiN[5]), vPhiN[6]), vPhiN[7]);

	min16float fPhiMin = min(min(min(fPhiN[0], fPhiN[1]), fPhiN[2]), fPhiN[3]);
	fPhiMin = min(min(min(min(fPhiMin, fPhiN[4]), fPhiN[5]), fPhiN[6]), fPhiN[7]);
	min16float fPhiMax = max(max(max(fPhiN[0], fPhiN[1]), fPhiN[2]), fPhiN[3]);
	fPhiMax = max(max(max(max(fPhiMax, fPhiN[4]), fPhiN[5]), fPhiN[6]), fPhiN[7]);

	const min16float3 vPhiAdv = g_txPhiVel.SampleLevel(g_smpLinear, vTex, 0);
	const min16float fPhiAdv = g_txPhiDen.SampleLevel(g_smpLinear, vTex, 0);

	const min16float3 vVelocity = vPhiAdv + 0.5 * (g_txPhiVel[DTid] - g_txPhiHatVel[DTid]);
	const min16float fDensity = fPhiAdv + 0.5 * (g_txPhiDen[DTid] - g_txPhiHatDen[DTid]);

	// Update velocity and density
	g_rwVelocity[DTid] = clamp(vVelocity, vPhiMin, vPhiMax);
	g_rwDensity[DTid] = clamp(fDensity, fPhiMin, fPhiMax) * g_fDecay;
}
