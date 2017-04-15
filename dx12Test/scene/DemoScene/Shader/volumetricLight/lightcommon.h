#ifndef _VOLUMETRIC_LIGHTING_LIGHT_COMMON_
#define _VOLUMETRIC_LIGHTING_LIGHT_COMMON_

#include <MacroDefine.hlsli>

#ifdef __cplusplus

#include "../math/SimpleMath.h"
using namespace hinode::math;

#endif

NAMESPACE(volumetricLightingCommon)

struct ScatteringLightParam
{
	float4x4 toShadowMapSpaceMat;	///< �r���[��ԏォ��V���h�E�}�b�v��֕ϊ�����
	float3 lightDir;				///< �r���[��ԏ�̃��C�g����
	float scatteringAmount;
	float3 lightColor;
	float G;						///< Henyey-Greenstein�ɂ��~�[�U���Ŏg���W���B
};

END_NAMESPACE

#ifdef __cplusplus
//�V�F�[�_���Ő錾�����\���̂�֐���C++���Ƃ̖��O�Փ˂�����邽�߂�shader���O��Ԃɓ����悤�ɂ��Ă��邪�A
// ���񏑂��̂��ʓ|�Ȃ̂ŁAshader�����͏ȗ��ł���悤�ɂ��Ă���
using namespace volumetricLightingCommon;
#endif

#ifndef __cplusplus

/// @brief Henyey-Greenstein�ɂ��~�[�U���֐����v�Z����
/// @param[in] cosTheta cos(���C�����ƃ��C�g�Ƃ̊p�x)
/// @retval �U����
float MieScattering(float cosTheta, float G)
{
#define PI 3.14159265f
	return pow((1-G),2) / (4*PI * pow(1 + pow(G, 2) - 2*G * cosTheta, 3.f/2.f));
}

/// @brief �r���[��Ԃ̈ʒu���V���h�E�}�b�v���W�n�ɕϊ�����
/// @param[in] pos
/// @param[in] toShadowMapSpace
/// @retval �V���h�E�}�b�v���W�n�̈ʒu(xy��0-1�͈̔͂ɂ��Ă���)
float4 toShadowMapFromView(float3 pos, float4x4 toShadowMapSpaceMat)
{
	float4 result = mul(toShadowMapSpaceMat, float4(pos,1));
	result /= result.w;
	result.xy *= float2(0.5f, -0.5f);
	result.xy += float2(0.5f, 0.5f);
	return result;
}

#endif

#endif