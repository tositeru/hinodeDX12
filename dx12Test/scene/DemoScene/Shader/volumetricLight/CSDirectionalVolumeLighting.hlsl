//
//	平行光のボリュームライトを計算するシェーダ
//
#ifndef _CS_DIRECTIONAL_VOLUME_LIGHTING_
#define _CS_DIRECTIONAL_VOLUME_LIGHTING_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

#include "LightCommon.h"
#include "GridCommon.h"

NAMESPACE(directionalVolumetricLighting)

#define THREAD_X 8
#define THREAD_Y 8

#ifdef __cplusplus

/// @brief このシェーダのWaveサイズを返す
/// @retval uint3
inline uint3 getThreadCount(){
	return uint3(THREAD_X, THREAD_Y, 1);
}

#endif

CONSTANT_BUFFER(ScatteringParam, b0, CS_SPACE)
{
	uint cbRayMarchCount;
	float3 _pad;
};

CONSTANT_BUFFER(CameraParam, b1, CS_SPACE)
{
	matrix cbInvProj;
	matrix cbCameraProj;	///< グリッド情報を取り出すために使用
};

CONSTANT_BUFFER(CBScatteringLightParam, b2, CS_SPACE)
{
	ScatteringLightParam cbScatteringLightParam;
};

CONSTANT_BUFFER(CBBuildScatteringAmountGridParam, b3, CS_SPACE)
{
	BuildScatteringAmountGridParam cbAmountGridParam;
};

END_NAMESPACE

#ifdef HLSL

//
//	ルートシグネチャー定義
//
#define RS \
	ROOT_FLAGS(DENY_GRAPHICS_SHADER_ROOT_ACCESS) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		CBV(b0, CS_SPACE_NO, 4, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		SRV(t0, CS_SPACE_NO, 5, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		SAMPLER(s0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND) \
	END_DESCRIPTOR_TABLE

SamplerState 		samLinerClamp : register(s0, CS_SPACE);
Texture2D<float>	texDepth : register(t0, CS_SPACE);
Texture2D<float>	texShadowMap : register(t1, CS_SPACE);
Texture2D<uint>		texDitheredPattern : register(t2, CS_SPACE);
Texture3D<float>	voxelScatteringAmountGrid : register(t3, CS_SPACE);
Texture2D<float4>	texSceneImage: register(t4, CS_SPACE);

RWTexture2D<float4>	outColor: register(u0, CS_SPACE);

float getGridAmount(float3 viewPos)
{
	float3 gridSize;
	voxelScatteringAmountGrid.GetDimensions(gridSize.x, gridSize.y, gridSize.z);
	
	float4 voxelUVW = mul(cbCameraProj, float4(viewPos, 1));
	voxelUVW /= voxelUVW.w;
	voxelUVW.xy = voxelUVW.xy*float2(0.5f, -0.5f) + float2(0.5f, 0.5f); 
	voxelUVW.z = calGridZSliceNormalizePos(viewPos.z, cbAmountGridParam);
	
	return voxelScatteringAmountGrid.SampleLevel(samLinerClamp, voxelUVW, 0);
}

float3 toViewFromScreen(uint2 texIndex, float screenZ, uint2 texSize)
{
	float4 result = float4((float2)texIndex / (float2)texSize * float2(2,-2) - float2(1,-1), screenZ, 1);
	result = mul(cbInvProj, result);
	result /= result.w;
	return result.xyz;
}

float calScattering(float radian)
{
	return MieScattering(radian, cbScatteringLightParam.G);
}

[numthreads(THREAD_X, THREAD_Y, 1)]
void main(uint2 pixel : SV_DispatchThreadID)
{
	uint2 texSize;
	outColor.GetDimensions(texSize.x, texSize.y);
	
	[branch] if(pixel.x < texSize.x && pixel.y < texSize.y){
		float rayStepRate = 0.9f + 0.2f * texDitheredPattern[uint2(pixel.x*7, (pixel.y+3)*7) % 4] / 16.f;
		
		//ビュー空間上の位置を求める
		//特に明記しない場合はビュー空間上のパラメータです。
		float screenDepth = texDepth[pixel];
		float3 startPos = toViewFromScreen(pixel, 0.f, texSize);
		float3 finishPos = toViewFromScreen(pixel, screenDepth, texSize);
		
		const float FEED_OUT_DISTANCE = 50.f;
		float3 rayVec = normalize(finishPos - startPos) * min(FEED_OUT_DISTANCE, finishPos.z);
		rayVec /= cbRayMarchCount * rayStepRate;
		
		//レイマーチ開始
		float radian = dot(normalize(rayVec), cbScatteringLightParam.lightDir);
		float intensity = 0.f;
		float3 rayPos = startPos;
		float end_z = min(FEED_OUT_DISTANCE, finishPos.z);
		uint raymarchCount;
		[loop] for(raymarchCount=0; raymarchCount < cbRayMarchCount && rayPos.z < end_z; ++raymarchCount) {
			rayPos += rayVec;
			
			float4 shadowMapPos = toShadowMapFromView(rayPos, cbScatteringLightParam.toShadowMapSpaceMat);
			float shadowDepth = texShadowMap.SampleLevel(samLinerClamp, shadowMapPos.xy, 0);
			[branch] if(shadowMapPos.z < shadowDepth){
				//ミー散乱計算
				float amount = getGridAmount(rayPos);
				intensity += calScattering(radian) * cbScatteringLightParam.scatteringAmount*(amount);
			}
		}
		intensity /= raymarchCount;
		
		float4 scatteringColor = float4(cbScatteringLightParam.lightColor*min(1.f, intensity),0);
		//結果格納
		outColor[pixel] = texSceneImage[pixel] + scatteringColor;
		//outColor[pixel] = scatteringColor;
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif