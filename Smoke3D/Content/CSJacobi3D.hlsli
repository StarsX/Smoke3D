//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture3D<halfV>	g_roKnown	: register(t0);
Texture3D<halfV>	g_roUnknown	: register(t1);

//--------------------------------------------------------------------------------------
// Jacobi iteration
//--------------------------------------------------------------------------------------
floatV Jacobi(const half2 vf, const uint3 vLoc)
{
	halfV fq = vf.x * g_roKnown[vLoc];
	fq += g_roUnknown[vLoc - uint3(1, 0, 0)];
	fq += g_roUnknown[vLoc + uint3(1, 0, 0)];
	fq += g_roUnknown[vLoc - uint3(0, 1, 0)];
	fq += g_roUnknown[vLoc + uint3(0, 1, 0)];
	fq += g_roUnknown[vLoc - uint3(0, 0, 1)];
	fq += g_roUnknown[vLoc + uint3(0, 0, 1)];

	return fq / vf.y;
}
