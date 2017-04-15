//
//	AO項とシーンの合成
//

#ifndef _CS_COMPOSITE_
#define _CS_COMPOSITE_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

NAMESPACE(composite)

#define THREAD_X 8
#define THREAD_Y 8

#ifdef __cplusplus

/// @brief このシェーダのWaveサイズを返す
/// @retval uint3
inline uint3 getThreadCount(){
	return uint3(THREAD_X, THREAD_Y, 1);
}

#endif

END_NAMESPACE

#ifdef HLSL

//
//	ルートシグネチャー定義
//
#define RS \
	ROOT_FLAGS(DENY_GRAPHICS_SHADER_ROOT_ACCESS) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		SRV(t0, CS_SPACE_NO, 2, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

Texture2D<float4> tScene : register(t0, CS_SPACE);
Texture2D<float> tAO : register(t1, CS_SPACE);
RWTexture2D<float4> uOut : register(u0, CS_SPACE);

[numthreads(THREAD_X, THREAD_Y, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
	uint2 texSize;
	uOut.GetDimensions(texSize.x, texSize.y);
	[branch] if(id.x < texSize.x && id.y < texSize.y) {
		float4 color = tScene[id];
		float ao = tAO[id];
		color.rgb *= ao;
		uOut[id] = color;
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif