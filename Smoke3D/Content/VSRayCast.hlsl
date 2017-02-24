//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Input/Output structures
//--------------------------------------------------------------------------------------
struct VSOut
{
	float4	Pos	: SV_Position;			// Position
	float3	Tex	: TEXCOORD;				// Texture coordinate
};

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
cbuffer cbMatrices	: register( b0 )
{
	matrix	g_mWorldViewProj;
	matrix	g_mWorld;
};

static const float4 g_vVertices[8] = {
	float4(-1.0, -1.0, 1.0, 1.0),
	float4(-1.0, 1.0, 1.0, 1.0),
	float4(1.0, -1.0, 1.0, 1.0),
	float4(1.0, 1.0, 1.0, 1.0),
	float4(1.0, -1.0, -1.0, 1.0),
	float4(1.0, 1.0, -1.0, 1.0),
	float4(-1.0, -1.0, -1.0, 1.0),
	float4(-1.0, 1.0, -1.0, 1.0)
};

static const uint g_uIndices[36] = {
	2, 3, 0, 1, 0, 3, 6, 7, 4, 5, 4, 7,
	0, 1, 6, 7, 6, 1, 4, 5, 2, 3, 2, 5,
	4, 2, 6, 0, 6, 2, 7, 1, 5, 3, 5, 1
};

/*static const float3 g_vNormals[6] = {
	float3(0, 0, 1.0), float3(0, 0, -1.0),
	float3(-1.0, 0, 0), float3(1.0, 0, 0),
	float3(0, -1.0, 0), float3(0, 1.0, 0)
};*/

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VSOut main(uint id : SV_VertexID)
{
	VSOut output;

	const uint vid = g_uIndices[id];
	const float4 Pos = g_vVertices[vid];
	
	output.Pos = mul(Pos, g_mWorldViewProj);
	output.Tex = Pos.xyz;
	
	return output;
}
