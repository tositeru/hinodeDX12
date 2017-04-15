//
//	�V�F�[�_�ŋ��ʂ̍\���̂�錾
//

//
//	hlsl��cpp�t�@�C���ŋ��p�Ŏg����悤�ɂ��邽�߂̃}�N��

#ifndef _STRUCTURE_COMMON_HLSLI_
#define _STRUCTURE_COMMON_HLSLI_

#include "MacroDefine.hlsli"

#ifdef __cplusplus

#include "../math/SimpleMath.h"
using namespace hinode::math;

#endif

NAMESPACE(shader)

struct CameraParam
{
	matrix viewProj;
	float3 eyePos;
	float pad;
};

struct DirectionLightParam
{
	float3 direction;
	float pad1;
	float3 color;
	float pad2;
};

struct MaterialParam
{
	float3 diffuseColor;
	float alpha;
	float3 specularColor;
	float specularPower;
};

struct ShadowMapParam
{
	float4x4 toShadowMapUVSpace;	///< ���[���h��Ԃ̈ʒu���V���h�E�}�b�v��UV���W�ɕϊ�����s��
	float depthOffset;				///< Z Fighting������邽�߂̃p�����[�^
	float darknessRate;				///< �e�̔Z��, 0�Ő^�����A1�ŉe�Ȃ�
	float2 pad;
};

END_NAMESPACE

#ifdef __cplusplus
//�V�F�[�_���Ő錾�����\���̂�֐���C++���Ƃ̖��O�Փ˂�����邽�߂�shader���O��Ԃɓ����悤�ɂ��Ă��邪�A
// ���񏑂��̂��ʓ|�Ȃ̂ŁAshader�����͏ȗ��ł���悤�ɂ��Ă���
using namespace shader;
#endif

#ifndef __cplusplus

//
//	HLSL��p�\����
//

struct DefferedOutput
{
	float4 albedo : SV_Target0;
	float3 normal : SV_Target1;
	float4 specular : SV_Target2;	// rgb: �F, a:���x
};

/// @brief �f�B�t�@�[�h�����_�����O�p�̏o�͂𐶐�����
/// @param[in] albedo
/// @param[in] normal
/// @param[in] specularColor
/// @param[in] specularPower
/// @retval DefferedOutput
DefferedOutput makeDefferedOutput(float4 albedo, float3 normal, float3 specularColor, float specularPower)
{
	DefferedOutput result;
	result.albedo = albedo;
	result.normal = normalize(normal)*0.5f + 0.5f;
	result.specular = float4(specularColor, specularPower);
	return result;
}

#endif

#endif