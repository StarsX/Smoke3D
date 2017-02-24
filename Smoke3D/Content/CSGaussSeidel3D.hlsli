//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
RWTexture3D<half>	g_rwUnknown	: register(u0);
Texture3D<half>		g_roKnown	: register(t0);

//--------------------------------------------------------------------------------------
// Gauss-Seidel iteration
//--------------------------------------------------------------------------------------
float GaussSeidel(const half2 vf, const uint3 vLoc)
{
	half fq = vf.x * g_roKnown[vLoc];
	fq += g_rwUnknown[vLoc - uint3(1, 0, 0)];
	fq += g_rwUnknown[vLoc + uint3(1, 0, 0)];
	fq += g_rwUnknown[vLoc - uint3(0, 1, 0)];
	fq += g_rwUnknown[vLoc + uint3(0, 1, 0)];
	fq += g_rwUnknown[vLoc - uint3(0, 0, 1)];
	fq += g_rwUnknown[vLoc + uint3(0, 0, 1)];

	return fq / vf.y;
}
