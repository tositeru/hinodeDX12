//
//	デバッグ用: 縦横サンプリング時のサンプリングしているピクセルを表示するためのもの
//
#ifndef _CS_DEBUG_CROSS_SAMPLING_
#define _CS_DEBUG_CROSS_SAMPLING_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

#include "AOCommon.h"

NAMESPACE(debugShowCrossSampling)

#define THREAD_X 8
#define THREAD_Y 8

#ifdef __cplusplus

/// @brief このシェーダのWaveサイズを返す
/// @retval uint3
inline uint3 getThreadCount(){
	return uint3(THREAD_X, THREAD_Y, 1);
}

#endif

CONSTANT_BUFFER(CommonParam, b0, CS_SPACE)
{
	DebugShowParam cbShowParam;
};

END_NAMESPACE

#ifdef HLSL

//
//	ルートシグネチャー定義
//
#define RS \
	ROOT_FLAGS(DENY_GRAPHICS_SHADER_ROOT_ACCESS) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		CBV(b0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		SRV(t0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

Texture2D<float> tViewZ : register(t0, CS_SPACE);
RWTexture2D<float4> uOut : register(u0, CS_SPACE);

/// @brief ライン上にサンプリングしていく関数
/// @param[in] center
/// @param[in] id
/// @param[in] samplingDirection
/// @param[in] texSize
/// @retval float
void samplingLine(float3 center, int2 id, int2 samplingDirection, float2 texSize)
{
	uint step = min(2, cbShowParam.radius / cbShowParam.samplingCount);
	[loop] for(uint offset=0; offset<cbShowParam.samplingCount; ++offset) {
		int2 pixel = id + samplingDirection * step * offset;
		
		float3 P = toViewPos(pixel, tViewZ[(uint2)pixel], (float2)texSize, cbShowParam.projXYToView);
		if(distance(center, P) < cbShowParam.worldRadius) {
			uOut[(uint2)pixel] = float4(1, 0, 0, 1);
		} else {
			uOut[(uint2)pixel] = float4(0, 1, 1, 1);
		}
	}
}

[numthreads(1, 1, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
	uint2 texSize;
	uOut.GetDimensions(texSize.x, texSize.y);
	[branch] if(cbShowParam.samplingPos.x < texSize.x && cbShowParam.samplingPos.y < texSize.y) {
		float3 center = toViewPos((float2)cbShowParam.samplingPos, tViewZ[cbShowParam.samplingPos], (float2)texSize, cbShowParam.projXYToView);
		
		samplingLine(center, cbShowParam.samplingPos, int2( 0, -1), texSize);
		samplingLine(center, cbShowParam.samplingPos, int2( 0,  1), texSize);
		samplingLine(center, cbShowParam.samplingPos, int2(-1,  0), texSize);
		samplingLine(center, cbShowParam.samplingPos, int2( 1,  0), texSize);

		uOut[cbShowParam.samplingPos] = float4(0, 0, 1, 1);
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif
