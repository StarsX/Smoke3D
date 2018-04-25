//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Compute divergence
//--------------------------------------------------------------------------------------
min16float Divergence3D(Texture3D<min16float3> roVector, const uint3 vLoc)
{
	// Get values from neighboring cells
	const min16float fxL = roVector[vLoc - uint3(1, 0, 0)].x;
	const min16float fxR = roVector[vLoc + uint3(1, 0, 0)].x;
	const min16float fyU = roVector[vLoc - uint3(0, 1, 0)].y;
	const min16float fyD = roVector[vLoc + uint3(0, 1, 0)].y;
	const min16float fzF = roVector[vLoc - uint3(0, 0, 1)].z;
	const min16float fzB = roVector[vLoc + uint3(0, 0, 1)].z;

	// Take central differences of neighboring values
	return 0.5 * (fxR - fxL + fyD - fyU + fzB - fzF);
}
