//
//	�ʒu���݂̂̒��_�������b�V���̕`��p�C�v���C��
//
#ifndef _HINODE_GRAPHICS_P_VERTEX_PIPELINE_
#define _HINODE_GRAPHICS_P_VERTEX_PIPELINE_

#include "../../../HLSL/RootSignatureCommon.hlsli"
#include "../../../HLSL/Utility.hlsli"
#include "../../../HLSL/MacroDefine.hlsli"
#include "../../../HLSL/StructureCommon.hlsli"

//
//	���\�[�X��`
//
NAMESPACE(shader)
NAMESPACE(PVertexPipeline)
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
		CBV(b0, PS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
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
};

PSInput VSMain( float4 pos : POSITION)
{
	PSInput o;
	float4 posInWorld = mul(cbWorld, pos);
	o.pos = mul(cbCamera.viewProj, posInWorld);
	return o;
}

//
//	�s�N�Z���V�F�[�_��`
//
float4 PSForward( PSInput input ) : SV_Target
{
	return float4(cbMaterial.diffuseColor, cbMaterial.alpha);
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
	float3 worldPos : WORLD_POSITION;
};

PSDefferedInput VSDeffered(float4 pos : POSITION)
{
	PSDefferedInput o;
	o.worldPos = mul(cbWorld, pos).xyz;
	o.pos = mul(cbCamera.viewProj, float4(o.worldPos, 1));
	return o;
}

//
//	�s�N�Z���V�F�[�_��`
//

DefferedOutput PSDeffered(PSDefferedInput input)
{
	//TODO�@ddx��ddy�̌��ʂ������ɂȂ��Ă��܂��̂Ō����𒲂ׂ�
	float3 dx = ddx(input.worldPos);
	float3 dy = ddy(input.worldPos);
	float3 normal = cross(dy, dx);
	return makeDefferedOutput(
		float4(cbMaterial.diffuseColor, cbMaterial.alpha),
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

float4 VSZPass(float4 pos : POSITION) : SV_POSITION
{
	pos = mul(cbWorld, pos);
	pos = mul(cbCamera.viewProj, pos);
	return pos;
}

#endif

#endif