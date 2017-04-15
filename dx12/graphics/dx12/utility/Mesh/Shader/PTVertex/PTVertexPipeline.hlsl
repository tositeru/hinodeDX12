//
//	�ʒu�ƃe�N�X�`�����W���������b�V���̕`��p�C�v���C��
//
#ifndef _HINODE_GRAPHICS_PT_VERTEX_PIPELINE_
#define _HINODE_GRAPHICS_PT_VERTEX_PIPELINE_

#include "../../../HLSL/RootSignatureCommon.hlsli"
#include "../../../HLSL/Utility.hlsli"
#include "../../../HLSL/MacroDefine.hlsli"
#include "../../../HLSL/StructureCommon.hlsli"

//
//	���\�[�X��`
//
NAMESPACE(shader)
NAMESPACE(PTVertexPipeline)
//�E��n���̗p
CONSTANT_BUFFER(CBModel, b0, VS_SPACE)
{
	matrix cbWorld;
};

CONSTANT_BUFFER(CBCamera, b0, COMMON_SPACE)
{
	CameraParam cbCamera;
};

CONSTANT_BUFFER(CBMaterial, b0, PS_SPACE)
{
	MaterialParam cbMaterial;
};

END_NAMESPACE
END_NAMESPACE

#ifdef HLSL

Texture2D<float4> txTexture : register(t0, PS_SPACE);
SamplerState smSampler : register(s0, PS_SPACE);

//
//	���[�g�V�O�l�`���[��`
//
#define RSForward \
	ROOT_FLAGS(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, VS_SPACE_NO, SHADER_VISIBILITY_VERTEX, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_VERTEX)\
		CBV(b0, COMMON_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_PIXEL)\
		CBV(b0, PS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		SRV(t0, PS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_PIXEL)\
		SAMPLER(s0, PS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND) \
	END_DESCRIPTOR_TABLE

#define RSZPass \
	ROOT_FLAGS(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_PIXEL_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, VS_SPACE_NO, SHADER_VISIBILITY_VERTEX, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_VERTEX)\
		CBV(b0, COMMON_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

//
//	���_�V�F�[�_��`
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
//	�s�N�Z���V�F�[�_��`
//

float4 PSForward(PSInput input) : SV_Target
{
	float4 color = txTexture.Sample(smSampler, input.texcoord).rgba;
	color *= float4(cbMaterial.diffuseColor, cbMaterial.alpha);
	return color;
}

//
//	�f�B�t�@�[�h�����_�����O�p
//

//
//	���_�V�F�[�_��`
//

struct PSDefferedInput
{
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 worldPos : WORLD_POSITION;
};

PSDefferedInput VSDeffered(
	float4 pos : POSITION,
	float2 texcoord : TEXCOORD0)
{
	PSDefferedInput o;
	o.worldPos = mul(cbWorld, pos).xyz;
	o.pos = mul(cbCamera.viewProj, float4(o.worldPos, 1));
	o.texcoord = texcoord;
	return o;
}

//
//	�s�N�Z���V�F�[�_��`
//

DefferedOutput PSDeffered(PSDefferedInput input)
{
	float4 albedo = txTexture.Sample(smSampler, input.texcoord) * float4(cbMaterial.diffuseColor, cbMaterial.alpha);
	//TODO�@ddx��ddy�̌��ʂ������ɂȂ��Ă��܂��̂Ō����𒲂ׂ�
	float3 dx = ddx(input.worldPos);
	float3 dy = ddy(input.worldPos);
	float3 normal = cross(dy, dx);
	return makeDefferedOutput(
		albedo,
		normal,
		cbMaterial.specularColor,
		cbMaterial.specularPower
	);

}

//-------------------------------------------------------------------------
//
//	Z�p�X
//
//-------------------------------------------------------------------------

float4 VSZPass(
	float4 pos : POSITION,
	float2 texcoord : TEXCOORD0) : SV_POSITION
{
	pos = mul(cbWorld, pos);
	pos = mul(cbCamera.viewProj, pos);
	return pos;
}

#endif

#endif