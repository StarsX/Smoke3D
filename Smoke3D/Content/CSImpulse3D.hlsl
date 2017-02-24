//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<half3>	g_rwVelocity	: register(u0);
RWTexture3D<half>	g_rwDensity		: register(u1);
Texture3D<half3>	g_roVelocity	: register(t0);
Texture3D<half>		g_roDensity		: register(t1);

//--------------------------------------------------------------------------------------
// Gaussian distribution
//--------------------------------------------------------------------------------------
half Gaussian3D(const half3 vDisp, const half fRad)
{
	const half fRadSq = fRad * fRad;

	return exp(-4.0 * dot(vDisp, vDisp) / fRadSq);
}

//--------------------------------------------------------------------------------------
// Impulse
//--------------------------------------------------------------------------------------
half Impulse3D(const half3 vTex)
{
	const half3 vDisp = vTex - g_vImLoc;
	return Gaussian3D(vDisp, 0.032);
}

//--------------------------------------------------------------------------------------
// Add force and density
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const half3 vTex = DTid * g_vTexel;
	const half fBasis = Impulse3D(vTex);

	const half fDens = g_fDens * length(g_vForce);
	const half3 vForce = g_vForce * fBasis;

	g_rwVelocity[DTid] = g_roVelocity[DTid] + vForce * g_fDeltaTime;
	g_rwDensity[DTid] = g_roDensity[DTid] + fDens * fBasis;
}
