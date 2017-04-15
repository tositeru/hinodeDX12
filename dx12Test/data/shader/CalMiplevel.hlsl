//
//	位置とテクスチャ座標情報をもつメッシュの描画パイプライン
//
#ifndef _SUNRISE_GRAPHICS_PT_VERTEX_PIPELINE_
#define _SUNRISE_GRAPHICS_PT_VERTEX_PIPELINE_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

//
//	リソース定義
//
NAMESPACE(PTVertexPipeline)
//右手系を採用
CONSTANT_BUFFER(CBModel, b0, VS_SPACE)
{
	matrix cbWorld;
};

CONSTANT_BUFFER(CBCamera, b0, COMMON_SPACE)
{
	CameraParam cbCamera;
};

END_NAMESPACE

#ifdef HLSL
//
//	ルートシグネチャー定義
//
#define RS \
	ROOT_FLAGS(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, VS_SPACE_NO, SHADER_VISIBILITY_VERTEX, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_VERTEX)\
		CBV(b0, COMMON_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_PIXEL)\
		SRV(t0, PS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_PIXEL)\
		SAMPLER(s0, PS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND) \
	END_DESCRIPTOR_TABLE

//
//	頂点シェーダ定義
//

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

PSInput VSMain(
	float4 pos : POSITION,
	float2 texcoord : TEXCOORD0)
{
	PSInput o;
	float4 posInWorld = mul(cbWorld, pos);
	o.pos = mul(cbCamera.viewProj, posInWorld);
	o.texcoord = texcoord;
	return o;
}

//
//	ピクセルシェーダ定義
//

Texture2D<float4> txTexture : register(t0, PS_SPACE);
SamplerState smSampler : register(s0, PS_SPACE);

float calMiplevel(float2 uv, float2 texSize)
{
	uv *= texSize;
	float2 dx = ddx(uv);
	float2 dy = ddy(uv);
	float Px = pow(dot(1, dx*dx), 1.f/2.f);
	float Py = pow(dot(1, dy*dy), 1.f/2.f);
	return log2(max(Px, Py));
/*	
	float2 dx = ddx(uv);
	float2 dy = ddy(uv);
	float deltaLength = lerp(length(dx), length(dy), 0.5f);
	float texSizeUnitPixel = length(1/texSize);//改良する必要あり?
	
	float t = deltaLength / texSizeUnitPixel;
	return t;
*/
}

float4 PSMain(PSInput input) : SV_Target
{
	float2 texSize;
	txTexture.GetDimensions(texSize.x, texSize.y);
	float miplevel = calMiplevel(input.texcoord, texSize);
	float4 color = txTexture.SampleLevel(smSampler, input.texcoord, miplevel).rgba;
	return color;
}

#endif

#endif