//
//	パーティクル描画用
//
#ifndef _PARTICLE_RENDER_HLSL_
#define _PARTICLE_RENDER_HLSL_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

//
//	リソース定義
//
NAMESPACE(shader)
NAMESPACE(particleRender)
//右手系を採用
CONSTANT_BUFFER(CBParticle, b0, GS_SPACE)
{
	float3 cbPos;
	float cbRadius;
	float4 cbColor;
};

CONSTANT_BUFFER(CBCamera, b1, GS_SPACE)
{
	float4x4 cbViewMat;
	float4x4 cbProjMat;
};

END_NAMESPACE
END_NAMESPACE

#ifdef HLSL

//
//	ルートシグネチャー定義
//
#define RSForward \
	ROOT_FLAGS(DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, GS_SPACE_NO, SHADER_VISIBILITY_GEOMETRY, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_GEOMETRY)\
		CBV(b1, GS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

//
//	頂点シェーダ定義
//

void VSMain()
{
}

//
//	ジオメトリシェーダ定義
//

struct GSInput
{ };

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float4 color : COLOR0;
};

static float4 gBillboardOffsets[] = {
	float4(-0.5f, 0.5f, 0, 0),
	float4( 0.5f, 0.5f, 0, 0),
	float4(-0.5f,-0.5f, 0, 0),
	float4( 0.5f,-0.5f, 0, 0),
};

[maxvertexcount(4)]
void GSMain(point GSInput input[1], inout TriangleStream<PSInput> stream, uint id : SV_PrimitiveID)
{
	float4 posInView = mul(cbViewMat, float4(cbPos, 1));
	for(uint i=0u; i<4; ++i) {
		PSInput output;
		output.pos = posInView + gBillboardOffsets[i] * cbRadius;
		output.pos = mul(cbProjMat, output.pos);
		output.texcoord = gBillboardOffsets[i] + 0.5f;
		output.color = cbColor;
		stream.Append(output);
	}
	stream.RestartStrip();
}


//
//	ピクセルシェーダ定義
//

float4 PSMain(PSInput input) : SV_Target
{
	float alpha = 1;
	//円形にだすようにしている
	alpha = pow(1 - saturate(length(input.texcoord*2 - 1)), 0.5f);
	float4 color = input.color;
	color.a *= alpha;
	return color;
}

#endif

#endif