//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#define NUM_SAMPLES			128
#define NUM_LIGHT_SAMPLES	32
#define ZERO_THRESHOLD		0.01
#define ONE_THRESHOLD		0.999
#define ABSORPTION			1.0

//--------------------------------------------------------------------------------------
// Input/Output structures
//--------------------------------------------------------------------------------------
struct PSIn
{
	float4	Pos	: SV_Position;			// Position
	float3	Tex	: TEXCOORD;				// Texture coordinate
};

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
cbuffer cbImmutable	: register (b0)
{
	float4  g_vViewport;
	float4  g_vDirectional;
	float4  g_vAmbient;
};

cbuffer cbPerObject	: register(b2)
{
	float3	g_vLightPt;
	float3	g_vEyePt;
};

static const float3 g_vLightRad		= g_vDirectional.xyz * g_vDirectional.w;	// 4.0
static const float3 g_vAmbientRad	= g_vAmbient.xyz * g_vAmbient.w;			// 1.0

static const float g_fMaxDist = 2.0 * sqrt(3.0);
static const float g_fStepScale = g_fMaxDist / NUM_SAMPLES;
static const float g_fLStepScale = g_fMaxDist / NUM_LIGHT_SAMPLES;

//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
Texture3D<half>		g_roDensity		: register(t0);

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
SamplerState		g_smpDefault	: register (s0);
SamplerState		g_smpLinear		: register (s1);

//--------------------------------------------------------------------------------------
// Pixel Shader
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
half4 main(PSIn input) : SV_TARGET
{
	half3 vPos = input.Tex;
	const half3 vStep = normalize(vPos - g_vEyePt) * g_fStepScale;

#ifndef _POINT_LIGHT_
	const half3 vLRStep = normalize(g_vLightPt) * g_fLStepScale;
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

	const half3 vResult = fScatter * g_vLightRad + g_vAmbientRad;
	
	return half4(vResult, 1.0 - fTransmit);
}
