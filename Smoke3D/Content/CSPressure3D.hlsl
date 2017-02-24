//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CSGaussSeidel3D.hlsli"

//--------------------------------------------------------------------------------------
// Poisson pressure
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const half2 vf = { -1.0, 6.0 };

	// Unordered Gauss-Seidel iteration
	for (uint i = 0; i < 1024; ++i)
	{
		const half fPressPRev = g_rwUnknown[DTid];
		const half fPress = GaussSeidel(vf, DTid);
		
		if (abs(fPress - fPressPRev) < 0.0000001) return;
		g_rwUnknown[DTid] = fPress;
	}
}
