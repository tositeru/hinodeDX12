//
//	�p�[�e�B�N���`��p
//
#ifndef _PARTICLE_RENDER_HLSL_
#define _PARTICLE_RENDER_HLSL_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

//
//	���\�[�X��`
//
NAMESPACE(shader)
NAMESPACE(particleRender)
//�E��n���̗p
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
//	���[�g�V�O�l�`���[��`
//
#define RSForward \
	ROOT_FLAGS(DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, GS_SPACE_NO, SHADER_VISIBILITY_GEOMETRY, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_GEOMETRY)\
		CBV(b1, GS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

//
//	���_�V�F�[�_��`
//

void VSMain()
{
}

//
//	�W�I���g���V�F�[�_��`
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
//	�s�N�Z���V�F�[�_��`
//

float4 PSMain(PSInput input) : SV_Target
{
	float alpha = 1;
	//�~�`�ɂ����悤�ɂ��Ă���
	alpha = pow(1 - saturate(length(input.texcoord*2 - 1)), 0.5f);
	float4 color = input.color;
	color.a *= alpha;
	return color;
}

#endif

#endif