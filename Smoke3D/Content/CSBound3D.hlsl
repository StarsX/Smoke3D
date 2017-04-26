//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "CHConstants.hlsli"

//--------------------------------------------------------------------------------------
// Structured Buffers
//--------------------------------------------------------------------------------------
RWTexture3D<half3>	g_rwVelocity	: register (u0);
Texture3D<half3>	g_roVelocity	: register (t0);

//--------------------------------------------------------------------------------------
// Boundary process
//--------------------------------------------------------------------------------------
[numthreads(THREAD_BLOCK_X, THREAD_BLOCK_Y, THREAD_BLOCK_Z)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint3 vDim;
	g_roVelocity.GetDimensions(vDim.x, vDim.y, vDim.z);
	
	// Current location
	int3 vLoc = DTid;

	const int3 vMax = vDim - 1;
	const int3 vOffset = {
		vLoc.x >= vMax.x ? -1 : (vLoc.x <= 0 ? 1 : 0),
		vLoc.y >= vMax.y ? -1 : (vLoc.y <= 0 ? 1 : 0),
		vLoc.z >= vMax.z ? -1 : (vLoc.z <= 0 ? 1 : 0)
	};
	vLoc += vOffset;

	if (vOffset.x || vOffset.y || vOffset.z)
		g_rwVelocity[DTid] = -g_roVelocity[vLoc];
	else g_rwVelocity[DTid] = g_roVelocity[DTid];
}
