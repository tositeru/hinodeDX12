//
//	バイラテラルブラー(横)
//
#ifndef _CS_BLUR_H_
#define _CS_BLUR_H_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

#include "BlurCommon.h"

NAMESPACE(blurH)

#define THREAD_X 128
#define THREAD_Y 1

#ifdef __cplusplus

/// @brief このシェーダのWaveサイズを返す
/// @retval uint3
inline uint3 getThreadCount(){
	return uint3(THREAD_X, THREAD_Y, 1);
}

#endif

CONSTANT_BUFFER(Param, b0, CS_SPACE)
{
	BlurCommonParam cbParam;
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
		SRV(t0, CS_SPACE_NO, 2, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

Texture2D<float> tAO : register(t0, CS_SPACE);
Texture2D<float> tViewZ : register(t1, CS_SPACE);
RWTexture2D<float> uOut : register(u0, CS_SPACE);

groupshared float2 sharedAOAndViewZ[THREAD_X];

[numthreads(THREAD_X, THREAD_Y, 1)]
void main(int2 groupID : SV_GroupID, int2 threadID : SV_GroupThreadID)
{
	uint2 texSize;
	uOut.GetDimensions(texSize.x, texSize.y);
	
	const int thread = threadID.x;
	int localX;
	int2 pixel;
	pixel.x = calSamplingPos(localX, groupID.x, thread, THREAD_X, cbParam.blurRadius);
	pixel.y = groupID.y;
	
	sharedAOAndViewZ[thread] = samplingValues(pixel, texSize, tAO, tViewZ);

	GroupMemoryBarrierWithGroupSync();

	[branch] if(cbParam.blurRadius <= thread && thread < (THREAD_X - cbParam.blurRadius) && pixel.x < texSize.x && pixel.y < texSize.y) {
		float2 centerAOViewZ = sharedAOAndViewZ[thread];
		
		float SIGMA_SQ = cbParam.gaussianSigma*cbParam.gaussianSigma;
		float weightSum = 0.0000001f;
		float aoSum = 0.f;
		[loop] for(int i=-cbParam.blurRadius; i<cbParam.blurRadius*2 + 1; ++i) {
			float weight = calGaussianWeight(i, SIGMA_SQ);
			float2 aoViewZ = sharedAOAndViewZ[thread + i];
			if(abs(aoViewZ.y-centerAOViewZ.y) < cbParam.spaceRange) {
				weightSum += weight;
				aoSum += aoViewZ.x * weight;
			}
		}
		
		uOut[pixel] = aoSum / weightSum;
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif
