//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<min16float3>	g_rwVelocity	: register (u0);
RWTexture3D<min16float>		g_rwDensity		: register (u1);
Texture3D<min16float3>		g_roVelocity	: register (t0);
Texture3D<min16float>		g_roDensity		: register (t1);

//--------------------------------------------------------------------------------------
// Gaussian distribution
//--------------------------------------------------------------------------------------
min16float Gaussian3D(const min16float3 vDisp, const min16float fRad)
{
	const min16float fRadSq = fRad * fRad;

	return exp(-4.0 * dot(vDisp, vDisp) / fRadSq);
}

//--------------------------------------------------------------------------------------
// Impulse
//--------------------------------------------------------------------------------------
min16float Impulse3D(const float3 vTex)
{
	const min16float3 vDisp = min16float3(vTex - g_vImLoc);
	return Gaussian3D(vDisp, 0.032);
}

//--------------------------------------------------------------------------------------
// Add force and density
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float3 vTex = DTid * g_vTexel;
	const min16float fBasis = Impulse3D(vTex);

	const min16float fDens = min16float(g_fDens * length(g_vForce));
	const min16float3 vForce = min16float3(g_vForce) * fBasis;

	g_rwVelocity[DTid] = g_roVelocity[DTid] + vForce * min16float(g_fDeltaTime);
	g_rwDensity[DTid] = g_roDensity[DTid] + fDens * fBasis;
}
