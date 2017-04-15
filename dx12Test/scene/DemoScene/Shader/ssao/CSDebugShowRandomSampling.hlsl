//
//	デバッグ用: ランダムサンプリング時のサンプリングしているピクセルを表示するためのもの
//

#ifndef _CS_DEBUG_SHOW_RANDOM_SAMPLING_
#define _CS_DEBUG_SHOW_RANDOM_SAMPLING_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

#include "AOCommon.h"

NAMESPACE(debugShowRandomSampling)

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
	ROOT_CBV(b0, CS_SPACE_NO, SHADER_VISIBILITY_ALL, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		SRV(t0, CS_SPACE_NO, 2, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

Texture2D<float> tViewZ : register(t0, CS_SPACE);
RWTexture2D<float4> uOut : register(u0, CS_SPACE);

[numthreads(1, 1, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
	uint2 texSize;
	uOut.GetDimensions(texSize.x, texSize.y);
	[branch] if(cbShowParam.samplingPos.x < texSize.x && cbShowParam.samplingPos.y < texSize.y) {
		float3 center = toViewPos((float2)cbShowParam.samplingPos, tViewZ[cbShowParam.samplingPos], (float2)texSize, cbShowParam.projXYToView);
		
		uint seed = cbShowParam.samplingPos.y * texSize.x + cbShowParam.samplingPos.x;
		uint VDCSeed = wangHash(seed) % 64;
		[loop] for(uint i=0;i<cbShowParam.samplingCount; ++i) {
			//float2 pixel = makeSamplingPoint(seed);
			float2 pixel = makeSamplingPointByVDC(i + VDCSeed);
			pixel *= cbShowParam.radius;
			pixel += (float2)cbShowParam.samplingPos;
			
			//画面範囲チェック
			float3 P = toViewPos(pixel, tViewZ[(uint2)pixel], (float2)texSize, cbShowParam.projXYToView);
			if(distance(center, P) < cbShowParam.worldRadius) {
				uOut[(uint2)pixel] = float4(1, 0, 0, 1);
			} else {
				uOut[(uint2)pixel] = float4(0, 1, 1, 1);
			}
		}
		uOut[cbShowParam.samplingPos] = float4(0, 0, 1, 1);
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif
