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
	const min16float2 vf = { -1.0, 6.0 };

	// Unordered Gauss-Seidel iteration
	for (uint i = 0; i < 1024; ++i)
	{
		const min16float fPressPrev = g_rwUnknown[DTid];
		const min16float fPress = GaussSeidel(vf, DTid);
		
		if (abs(fPress - fPressPrev) < 0.0000001) return;
		g_rwUnknown[DTid] = fPress;
	}
}
