//
//	空間の粒子の濃度をボクセルに設定するパイプライン
//
#ifndef _BUILD_SCATTERING_AMOUNT_GRID_
#define _BUILD_SCATTERING_AMOUNT_GRID_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

#include "GridCommon.h"

NAMESPACE(buildScatteringAmount)

CONSTANT_BUFFER(CBBuildScatteringAmountGridParam, b0, GS_SPACE)
{
	BuildScatteringAmountGridParam cbAmountGridParam;
};

/// @brief 粒子を表すパーティクル
struct ScatteringAmountParticle
{
	float3 pos;
	float size;
	float amount;
	float3 pad;
};

END_NAMESPACE

#ifndef __cplusplus

StructuredBuffer<ScatteringAmountParticle> bufParticles : register(t0, GS_SPACE);

//
//	ルートシグネチャー定義
//
#define RS \
	ROOT_FLAGS(DENY_VERTEX_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_PIXEL_SHADER_ROOT_ACCESS) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_GEOMETRY)\
		CBV(b0, GS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		SRV(t0, GS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

// 頂点シェーダでは何もしない
void VSMain()
{
	return;
}

struct GSInput{};

struct PSInput
{
	float4 pos 		: SV_POSITION;
	float amount 	: SCATTERING_AMOUNT;
	float2 localPos : TEXCOORD0;
	uint sliceIndex : SV_RenderTargetArrayIndex;
};

static float4 gBillboardOffsets[] = {
	float4(-0.5f, 0.5f, 0, 0),
	float4( 0.5f, 0.5f, 0, 0),
	float4(-0.5f,-0.5f, 0, 0),
	float4( 0.5f,-0.5f, 0, 0),
};

[maxvertexcount(8)]
void GSMain(point GSInput input[1], inout TriangleStream<PSInput> stream, uint id : SV_PrimitiveID)
{
	ScatteringAmountParticle particle = bufParticles[id];
	particle.pos.y += cbAmountGridParam.timer;
	if( 30.f < particle.pos.y) {
		int n = (particle.pos.y + 30.f) / 60.f;
		particle.pos.y -= n * 60.f;
	}
	
	float amountRate = particle.amount;
	amountRate *= clamp(1 - (particle.pos.y - 4.f) / 15.f, 0.f, 1.f);
	
	float4 viewPos = mul(cbAmountGridParam.viewMatrix, float4(particle.pos,1));
	float outputSlicePos = calGridZSlicePos(viewPos.z, cbAmountGridParam);
	float unitSlicePos[2] = {
		floor(outputSlicePos),//min
		ceil(outputSlicePos),//max
	};
	
	for(uint sliceIndex=0; sliceIndex<2; ++sliceIndex){
		uint targetSlice = unitSlicePos[sliceIndex];
		if(targetSlice < 0 || cbAmountGridParam.depthDivideCount <= targetSlice)
			continue;
		
		//float amount = calAmountForZSlice(particle.amount, outputSlicePos, targetSlice);
		float amount = calAmountForZSlice(amountRate, outputSlicePos, targetSlice);
		for(uint i=0; i<4; ++i){
			PSInput output;
			output.pos = mul(cbAmountGridParam.projMatrix, viewPos + particle.size * gBillboardOffsets[i]);
			output.amount = amount;
			output.localPos = gBillboardOffsets[i] + 0.5f;
			output.sliceIndex = targetSlice;
			stream.Append(output);
		}
		stream.RestartStrip();
	}
}

float PSMain(PSInput input) : SV_Target
{
	float dis = distance(input.localPos, float2(0.5f, 0.5f));
	float rate = 1 - saturate(dis / 0.5f);
	return input.amount * rate;//円形になるようにしている
}

#endif

#endif