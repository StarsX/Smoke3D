//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#define NUM_SAMPLES			128
#define NUM_LIGHT_SAMPLES	32
#define ZERO_THRESHOLD		0.01
#define ONE_THRESHOLD		0.999
#define ABSORPTION			1.0

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
cbuffer cbImmutable	: register (b0)
{
	float4  g_vViewport;
	float4  g_vDirectional;
	float4  g_vAmbient;
};

cbuffer cbPerObject	: register (b2)
{
	float3	g_vLocalSpaceLightPt;
	float3	g_vLocalSpaceEyePt;
	matrix	g_mScreenToLocal;
};

static const float3 g_vCornflowerBlue = { 0.392156899, 0.584313750, 0.929411829 };

static const float3 g_vLightRad = g_vDirectional.xyz * g_vDirectional.w;	// 4.0
static const float3 g_vAmbientRad = g_vAmbient.xyz * g_vAmbient.w;			// 1.0

static const float g_fMaxDist = 2.0 * sqrt(3.0);
static const float g_fStepScale = g_fMaxDist / NUM_SAMPLES;
static const float g_fLStepScale = g_fMaxDist / NUM_LIGHT_SAMPLES;

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture3D<half>		g_roDensity		: register (t0);

//--------------------------------------------------------------------------------------
// RW textures
//--------------------------------------------------------------------------------------
RWTexture2D<half4>	g_rwPresent		: register (u0);

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
SamplerState		g_smpDefault	: register (s0);
SamplerState		g_smpLinear		: register (s1);

//--------------------------------------------------------------------------------------
// Screen space to loacal space
//--------------------------------------------------------------------------------------
float3 ScreenToLocal(const float3 vLoc)
{
	float4 vPos = mul(float4(vLoc, 1.0), g_mScreenToLocal);
	
	return vPos.xyz / vPos.w;
}

//--------------------------------------------------------------------------------------
// Compute start point of the ray
//--------------------------------------------------------------------------------------
bool ComputeStartPoint(inout float3 vPos, const half3 vRayDir)
{
	if (abs(vPos.x) <= 1.0 && abs(vPos.y) <= 1.0 && abs(vPos.z) <= 1.0) return true;

	const half3 vPos0 = vPos;
	//half U = asfloat(0x7f800000);	// INF
	half U = 3.402823466e+38;		// FLT_MAX
	bool bHit = false;

	[unroll]
	for (uint i = 0; i < 3; ++i)
	{
		const half u = (-sign(vRayDir[i]) - vPos0[i]) / vRayDir[i];
		if (u < 0.0h) continue;
			
		const half3 vHit = vRayDir * u + vPos0;
		if (abs(vHit[(i + 1) % 3]) > 1.0h) continue;
		if (abs(vHit[(i + 2) % 3]) > 1.0h) continue;
		if (u < U)
		{
			U = u;
			vPos = vHit;
			bHit = true;
		}
	}

	vPos = clamp(vPos, -1.0, 1.0);

	return bHit;
}

//--------------------------------------------------------------------------------------
// Sample density field
//--------------------------------------------------------------------------------------
half GetSample(const half3 vTex)
{
#ifdef IMPULSE_TEST
	static const half fDensRad = 0.25;
	static const half fDensRadSq = fDensRad * fDensRad;
	const half3 vDisp = vTex - half3(0.5, 0.9, 0.5);

	return exp(-4.0 * dot(vDisp, vDisp) / fDensRadSq);
#elif defined(SPHERE_TEST)
	const half3 vDisp = vTex - 0.5;

	return dot(vDisp, vDisp) <= 0.25 ? 1.0 : 0.0;
#elif defined(CUBE_TEST)
	return 1.0;
#else
	return min(g_roDensity.SampleLevel(g_smpLinear, vTex, 0), 16.0);
#endif
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
[numthreads(32, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float3 vPos = ScreenToLocal(float3(DTid.xy, 0.0));	// The point on the near plane
	const half3 vRayDir = normalize(vPos - g_vLocalSpaceEyePt);
	if (!ComputeStartPoint(vPos, vRayDir)) return;

	const half3 vStep = vRayDir * g_fStepScale;

#ifndef _POINT_LIGHT_
	const half3 vLRStep = normalize(g_vLocalSpaceLightPt) * g_fLStepScale;
#endif

	// Transmittance
	half fTransmit = 1.0;
	// In-scattered radiance
	half fScatter = 0.0;

	for (uint i = 0; i < NUM_SAMPLES; ++i)
	{
		if (abs(vPos.x) > 1.0 || abs(vPos.y) > 1.0 || abs(vPos.z) > 1.0) break;
		half3 vTex = half3(0.5, -0.5, 0.5) * vPos + 0.5;

		// Get a sample
		half fDens = GetSample(vTex);

		// Skip empty space
		if (fDens > ZERO_THRESHOLD)
		{
			// Attenuate ray-throughput
			const half fScaledDens = fDens * g_fStepScale;
			fTransmit *= saturate(1.0 - fScaledDens * ABSORPTION);
			if (fTransmit < ZERO_THRESHOLD) break;

			// Point light direction in texture space
#ifdef _POINT_LIGHT_
			const half3 vLRStep = normalize(g_vLightPt - vPos) * g_fLStepScale;
#endif

			// Sample light
			half fLRTrans = 1.0;	// Transmittance along light ray
			half3 vLRPos = vPos + vLRStep;

			for (uint j = 0; j < NUM_LIGHT_SAMPLES; ++j)
			{
				if (abs(vLRPos.x) > 1.0 || abs(vLRPos.y) > 1.0 || abs(vLRPos.z) > 1.0) break;
				vTex = half3(0.5, -0.5, 0.5) * vLRPos + 0.5;

				// Get a sample along light ray
				const half fLRDens = GetSample(vTex);

				// Attenuate ray-throughput along light direction
				fLRTrans *= saturate(1.0 - ABSORPTION * g_fLStepScale * fLRDens);
				if (fLRTrans < ZERO_THRESHOLD) break;

				// Update position along light ray
				vLRPos += vLRStep;
			}

			fScatter += fLRTrans * fTransmit * fScaledDens;
		}

		vPos += vStep;
	}

	//clip(ONE_THRESHOLD - fTransmit);

	half3 vResult = fScatter * g_vLightRad + g_vAmbientRad;
	vResult = lerp(vResult, g_vCornflowerBlue * g_vCornflowerBlue, fTransmit);

	g_rwPresent[DTid.xy] = half4(sqrt(vResult), 1.0);
}
