//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Compute gradient
//--------------------------------------------------------------------------------------
min16float3 Gradient3D(Texture3D<min16float> roScalar, const uint3 vLoc)
{
	// Get values from neighboring cells
	const min16float fxL = roScalar[vLoc - uint3(1, 0, 0)];
	const min16float fxR = roScalar[vLoc + uint3(1, 0, 0)];
	const min16float fyU = roScalar[vLoc - uint3(0, 1, 0)];
	const min16float fyD = roScalar[vLoc + uint3(0, 1, 0)];
	const min16float fzF = roScalar[vLoc - uint3(0, 0, 1)];
	const min16float fzB = roScalar[vLoc + uint3(0, 0, 1)];

	// Compute the velocity's divergence using central differences
	return 0.5 * min16float3(fxR - fxL, fyD - fyU, fzB - fzF);
}
