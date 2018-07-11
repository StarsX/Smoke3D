//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture3D<halfV>	g_txKnown	: register (t0);
Texture3D<halfV>	g_txUnknown	: register (t1);

//--------------------------------------------------------------------------------------
// Jacobi iteration
//--------------------------------------------------------------------------------------
halfV Jacobi(const min16float2 vf, const uint3 vLoc)
{
	halfV fq = vf.x * g_txKnown[vLoc];
	fq += g_txUnknown[vLoc - uint3(1, 0, 0)];
	fq += g_txUnknown[vLoc + uint3(1, 0, 0)];
	fq += g_txUnknown[vLoc - uint3(0, 1, 0)];
	fq += g_txUnknown[vLoc + uint3(0, 1, 0)];
	fq += g_txUnknown[vLoc - uint3(0, 0, 1)];
	fq += g_txUnknown[vLoc + uint3(0, 0, 1)];

	return fq / vf.y;
}
