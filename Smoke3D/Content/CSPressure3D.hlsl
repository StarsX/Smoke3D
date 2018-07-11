//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CSGaussSeidel3D.hlsli"

#define PRESS_ITERATION	48

//--------------------------------------------------------------------------------------
// Poisson pressure
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const min16float2 vf = { -1.0, 6.0 };

	// Unordered Gauss-Seidel iteration
	[unroll]
	for (uint i = 0; i < PRESS_ITERATION; ++i)
	{
		const min16float fPressPrev = g_rwUnknown[DTid];
		const min16float fPress = GaussSeidel(vf, DTid);

		g_rwUnknown[DTid] = fPress;
		DeviceMemoryBarrierWithGroupSync();
	}
}
