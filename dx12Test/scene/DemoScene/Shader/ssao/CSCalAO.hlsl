//
//	AO値を計算するシェーダ
//

#ifndef _CS_CAL_AO_
#define _CS_CAL_AO_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

#include "AOCommon.h"

NAMESPACE(calAO)

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
	AOCommonParam cbCommonParam;
};

CONSTANT_BUFFER(AOParam, b1, CS_SPACE)
{
	AlchemyAOParam cbAOParam;
};

END_NAMESPACE

#ifdef HLSL

//
//	ルートシグネチャー定義
//
#define RS \
	ROOT_FLAGS(DENY_GRAPHICS_SHADER_ROOT_ACCESS) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		CBV(b0, CS_SPACE_NO, 2, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		SRV(t0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE


Texture2D<float> tViewZ : register(t0, CS_SPACE);
//Texture2D<float> tNormal : register(t1, CS_SPACE);
RWTexture2D<float> uOutAO : register(u0, CS_SPACE);

/// @brief ランダムにサンプリングしていく関数
/// @param[in] center
/// @param[in] normal
/// @param[in] id
/// @param[inout] samplingCounter
/// @retval float
float samplingRandom(float3 center, float3 normal, int2 id, uint2 texSize, inout uint samplingCounter)
{
	float A = 0.f;
	uint seed = id.y * texSize.x + id.x;
	[loop] for(uint count=0; count < cbCommonParam.samplingCount; ++count) {
		int2 pixel = (float2)(makeSamplingPoint(seed) * cbCommonParam.samplingRange);
		pixel += id;
		float3 targetPos = toViewPos((float2)pixel, tViewZ[pixel], (float2)texSize, cbCommonParam.projXYToView);
		A += calAlchemyAO(center, normal, targetPos, cbAOParam.radius*cbAOParam.radius, cbAOParam.beta, samplingCounter);
	}
	return A;
}

/// @brief 低食い違い量列をつかった関数
/// @param[in] center
/// @param[in] normal
/// @param[in] id
/// @param[inout] samplingCounter
/// @retval float
float samplingLDS(float3 center, float3 normal, int2 id, uint2 texSize, inout uint samplingCounter)
{
	float A = 0.f;
	uint seed = id.y * texSize.x + id.x;
	uint VDCSeed = wangHash(seed) % 128;
	[loop] for(uint count=0; count < cbCommonParam.samplingCount; ++count) {
		int2 pixel = makeSamplingPointByVDC(count + VDCSeed) * cbCommonParam.samplingRange;
		pixel += id;
		float3 targetPos = toViewPos((float2)pixel, tViewZ[pixel], (float2)texSize, cbCommonParam.projXYToView);
		A += calAlchemyAO(center, normal, targetPos, cbAOParam.radius*cbAOParam.radius, cbAOParam.beta, samplingCounter);
	}
	return A;
}

[numthreads(THREAD_X, THREAD_Y, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
	uint2 texSize;
	uOutAO.GetDimensions(texSize.x, texSize.y);
	[branch] if(id.x < texSize.x && id.y < texSize.y) {
		float viewZ = tViewZ[id];
		float3 center = toViewPos((float2)id, viewZ, (float2)texSize, cbCommonParam.projXYToView);
		float3 normal = calNormal(id, texSize, tViewZ, cbCommonParam.projXYToView);

		//サンプリング方法の指定
		#define RANDOM_SAMPLING_MODE 1	//ランダムにサンプリングしていく
		#define LDS_SAMPLING_MODE 2		//LDSでサンプリングしていく
		#define SAMPLING_MODE LDS_SAMPLING_MODE
		float A=0;
		uint samplingCounter = 0;
#if SAMPLING_MODE == RANDOM_SAMPLING_MODE
		A = samplingRandom(center, normal, id, texSize, samplingCounter);
#elif SAMPLING_MODE == LDS_SAMPLING_MODE
		A = samplingLDS(center, normal, id, texSize, samplingCounter);
#else
#error "error"
#endif
		float ao = calAlchemyAOSum(A, cbAOParam.radius, cbCommonParam.samplingCount, cbAOParam.sigma, cbAOParam.K);
		uOutAO[id] = ao;
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif
