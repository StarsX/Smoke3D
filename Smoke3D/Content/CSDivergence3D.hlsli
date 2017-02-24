//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Compute divergence
//--------------------------------------------------------------------------------------
half Divergence3D(Texture3D<half3> roVector, const uint3 vLoc)
{
	// Get values from neighboring cells
	const half fxL = roVector[vLoc - uint3(1, 0, 0)].x;
	const half fxR = roVector[vLoc + uint3(1, 0, 0)].x;
	const half fyU = roVector[vLoc - uint3(0, 1, 0)].y;
	const half fyD = roVector[vLoc + uint3(0, 1, 0)].y;
	const half fzF = roVector[vLoc - uint3(0, 0, 1)].z;
	const half fzB = roVector[vLoc + uint3(0, 0, 1)].z;

	// Take central differences of neighboring values
	return 0.5 * (fxR - fxL + fyD - fyU + fzB - fzF);
}
