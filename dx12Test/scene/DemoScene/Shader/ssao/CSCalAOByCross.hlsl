//
//	AO値を計算するシェーダ
//  縦横にサンプリングしていく
//

#ifndef _CS_CAL_AO_BY_CROSS_
#define _CS_CAL_AO_BY_CROSS_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

#include "AOCommon.h"

NAMESPACE(calAOByCross)

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

CONSTANT_BUFFER(AlchemyAOParam, b1, CS_SPACE)
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
		SRV(t0, CS_SPACE_NO, 2, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

Texture2D<float> tViewZ : register(t0, CS_SPACE);
Texture2D<float3> tNormal : register(t1, CS_SPACE);
RWTexture2D<float> uOutAO : register(u0, CS_SPACE);

/// @brief ライン上にサンプリングしていく関数
/// @param[in] center
/// @param[in] normal
/// @param[in] id
/// @param[in] samplingDirection
/// @param[inout] samplingCounter
/// @retval float
float samplingLine(float3 center, float3 normal, int2 id, int2 samplingDirection, uint2 texSize, inout uint samplingCounter)
{
	uint step = min(1, cbCommonParam.samplingRange / cbCommonParam.samplingCount);
	float A=0.f;
	[loop] for(uint offset=0; offset<cbCommonParam.samplingCount; ++offset) {
		int2 targetId = id + samplingDirection * step * offset;
		float3 targetPos = toViewPos((float2)targetId, tViewZ[targetId], (float2)texSize, cbCommonParam.projXYToView);
		A += calAlchemyAO(center, normal, targetPos,  cbAOParam.radius*cbAOParam.radius, cbAOParam.beta, samplingCounter);
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
		
		float A=0;
		uint samplingCounter = 0;
		A += samplingLine(center, normal, id, int2( 0, -1), texSize, samplingCounter);
		A += samplingLine(center, normal, id, int2( 0,  1), texSize, samplingCounter);
		A += samplingLine(center, normal, id, int2(-1,  0), texSize, samplingCounter);
		A += samplingLine(center, normal, id, int2( 1,  0), texSize, samplingCounter);
		samplingCounter = cbCommonParam.samplingCount;
		//float ao = pow(max(0, 1 - 2 * A * cbSigma / (float)max(1, samplingCounter)), cbK);
		float ao = calAlchemyAOSum(A, cbAOParam.radius, cbCommonParam.samplingCount, cbAOParam.sigma, cbAOParam.K);
		uOutAO[id] = ao;
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif
