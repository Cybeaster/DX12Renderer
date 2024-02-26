// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS

#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS

#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS

#define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtils.hlsl"

struct InstanceData
{
	float4x4 World;
	float4x4 TexTransform;
	uint MaterialIndex;
	float2 DisplacementMapTexelSize;
	float GridSpatialStep;
};

struct MaterialData
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Roughness;
	float4x4 MatTransform;
	uint DiffuseMapIndex;
	int NormalMapIndex;
	uint MatPad0;
	uint MatPad1;
};

Texture2D gTextureMaps[7] : register(t0);
Texture2D gDisplacementMap : register(t7);
TextureCube gCubeMap : register(t8);

StructuredBuffer<InstanceData> gInstanceData : register(t0, space1);
StructuredBuffer<MaterialData> gMaterialData : register(t1, space1);

cbuffer cbPass : register(b0)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	float3 gEyePosW;
	float cbPerPassPad1;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float4 gAmbientLight;

	float4 gFogColor;
	float gFogStart;
	float gFogRange;
	float2 cbPerPassPad2;

	// Indices [0, NUM_DIR_LIGHTS) are directional lights;

	// indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;

	// indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)

	// are spot lights for a maximum of MaxLights per object.

	Light gLights[MaxLights];
};

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

float3 NormalSampleToWorldSpace(float3 NormalMapSample, float3 UnitNormalW, float3 TangentW)
{
	float3 normalT = NormalMapSample * 2.0f - 1.0f;
	float3 N = UnitNormalW;
	/*
	*  This code makes sure T is orthonormal to N by subtracting off any
		component of T along the direction N
	*/
	float3 T = normalize(TangentW - dot(TangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	float3 bumpedNormal = mul(normalT, TBN);
	return bumpedNormal;
}

bool IsTangentValid(float3 TangentW)
{
	return TangentW != float3(0.0f, 0.0f, 0.0f);
}