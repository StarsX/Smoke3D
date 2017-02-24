//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Compute gradient
//--------------------------------------------------------------------------------------
half3 Gradient3D(Texture3D<half> roScalar, const uint3 vLoc)
{
	// Get values from neighboring cells
	const half fxL = roScalar[vLoc - uint3(1, 0, 0)];
	const half fxR = roScalar[vLoc + uint3(1, 0, 0)];
	const half fyU = roScalar[vLoc - uint3(0, 1, 0)];
	const half fyD = roScalar[vLoc + uint3(0, 1, 0)];
	const half fzF = roScalar[vLoc - uint3(0, 0, 1)];
	const half fzB = roScalar[vLoc + uint3(0, 0, 1)];

	// Compute the velocity's divergence using central differences
	return 0.5 * half3(fxR - fxL, fyD - fyU, fzB - fzF);
}
