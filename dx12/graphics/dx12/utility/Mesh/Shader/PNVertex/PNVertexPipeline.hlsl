//
//	�ʒu�Ɩ@�����������b�V���̕`��p�C�v���C��
//
#ifndef _HINODE_GRAPHICS_PN_VERTEX_PIPELINE_
#define _HINODE_GRAPHICS_PN_VERTEX_PIPELINE_

#include "../../../HLSL/RootSignatureCommon.hlsli"
#include "../../../HLSL/Utility.hlsli"
#include "../../../HLSL/MacroDefine.hlsli"
#include "../../../HLSL/StructureCommon.hlsli"

//
//	���\�[�X��`
//
NAMESPACE(shader)
NAMESPACE(PNVertexPipeline)
//�E��n���̗p
CONSTANT_BUFFER(CBModel, b0, VS_SPACE)
{
	matrix cbWorld;
};

CONSTANT_BUFFER(CBCamera, b0, COMMON_SPACE)
{
	CameraParam cbCamera;
};

CONSTANT_BUFFER(CBLight, b0, PS_SPACE)
{
	DirectionLightParam cbLight;
};

CONSTANT_BUFFER(CBMaterial, b1, PS_SPACE)
{
	MaterialParam cbMaterial;
};

END_NAMESPACE
END_NAMESPACE

#ifdef HLSL
//
//	���[�g�V�O�l�`���[��`
//
#define RSForward \
	ROOT_FLAGS(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, VS_SPACE_NO, SHADER_VISIBILITY_VERTEX, 0) CM\
	ROOT_CBV(b1, PS_SPACE_NO, SHADER_VISIBILITY_PIXEL, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		CBV(b0, COMMON_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_PIXEL)\
		CBV(b0, PS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

#define RSDeffered \
	ROOT_FLAGS(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, VS_SPACE_NO, SHADER_VISIBILITY_VERTEX, 0) CM\
	ROOT_CBV(b1, PS_SPACE_NO, SHADER_VISIBILITY_PIXEL, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_VERTEX)\
		CBV(b0, COMMON_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
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
	float3 normal : NORMAL;
	float3 worldPos : WORLD_POS;
};

PSInput VSMain(
	float4 pos : POSITION,
	float3 normal : NORMAL)
{
	PSInput o;
	o.worldPos = mul(cbWorld, pos).xyz;
	o.pos = mul(cbCamera.viewProj, float4(o.worldPos, 1));
	o.normal = mul((float3x3)cbWorld, normal);
	return o;
}

//
//	�s�N�Z���V�F�[�_��`
//

float4 PSForward(PSInput input) : SV_Target
{
	float3 eyeDir = cbCamera.eyePos-input.worldPos;
	float2 lit = calDirectionLightLit(input.normal, cbMaterial.specularPower, cbLight.direction, eyeDir);
	
	float4 color;
	color.rgb = cbMaterial.diffuseColor * cbLight.color * lit.x;
	color.rgb += cbMaterial.specularColor * cbLight.color * lit.y;
	color.a = cbMaterial.alpha;
	return color;
}

//-------------------------------------------------------------------------
//
//	�f�B�t�@�[�h�����_�����O�p
//
//-------------------------------------------------------------------------

DefferedOutput PSDeffered(PSInput input)
{
	return makeDefferedOutput(
		float4(cbMaterial.diffuseColor, cbMaterial.alpha),
		input.normal,
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
	float3 normal : NORMAL) :SV_POSITION
{
	pos = mul(cbWorld, pos);
	pos = mul(cbCamera.viewProj, pos);
	return pos;
}

#endif

#endif