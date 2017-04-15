//
//	平行光のディファードライティング
//
#ifndef _HINODE_GRAPHICS_DEFFERED_DIRECTION_LIGHTING_
#define _HINODE_GRAPHICS_DEFFERED_DIRECTION_LIGHTING_

#include "../../../HLSL/RootSignatureCommon.hlsli"
#include "../../../HLSL/Utility.hlsli"
#include "../../../HLSL/MacroDefine.hlsli"
#include "../../../HLSL/StructureCommon.hlsli"

//
//	リソース定義
//
NAMESPACE(shader)
NAMESPACE(CSDefferedDirectionLighting)

#define THREAD_X 8
#define THREAD_Y 8

#ifdef __cplusplus

/// @brief このシェーダのWaveサイズを返す
/// @retval uint3
inline uint3 getThreadCount(){
	return uint3(THREAD_X, THREAD_Y, 1);
}

#endif

//右手系を採用
CONSTANT_BUFFER(CBParam, b0, CS_SPACE)
{
	float4x4 cbInvViewProj;
	float3 cbEyePos;
	float pad;
};

CONSTANT_BUFFER(CBLight, b1, CS_SPACE)
{
	DirectionLightParam cbLight;
};

END_NAMESPACE
END_NAMESPACE

#ifdef HLSL

Texture2D<float4> txAlbedo : register(t0, CS_SPACE);
Texture2D<float3> txNormal : register(t1, CS_SPACE);
Texture2D<float4> txSpecular : register(t2, CS_SPACE);
Texture2D<float> txDepth : register(t3, CS_SPACE);

RWTexture2D<float4> outScreen : register(u0, CS_SPACE);

#define RS \
	ROOT_FLAGS(0) CM\
	ROOT_CBV(b1, CS_SPACE_NO, SHADER_VISIBILITY_ALL, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		CBV(b0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		SRV(t0, CS_SPACE_NO, 4, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE


[numthreads(THREAD_X, THREAD_Y, 1)]
void main( uint2 texel : SV_DispatchThreadID )
{
	uint2 screenSize;
	txAlbedo.GetDimensions(screenSize.x, screenSize.y);
	[branch] if(isAnd(texel < screenSize)){
		float depth = txDepth[texel];
		if(1.f <= depth){
			return;//何も書き込まれていなければ処理しない
		}
		
		float3 pos = toWorldPosFromScreen(depth, texel, screenSize, cbInvViewProj).xyz;
		float3 eyeVec = cbEyePos - pos;
		float3 normal = sampleNormal(txNormal, texel);
		float4 specular = txSpecular[texel];
		
		float2 lit = calDirectionLightLit(normal, specular.a, cbLight.direction, eyeVec);
		float4 result;
		result.rgb = lit.x * txAlbedo[texel].rgb * cbLight.color;
		result.rgb += lit.y * specular.rgb * cbLight.color;
		result.a = 0;
		outScreen[texel] += result;
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif
