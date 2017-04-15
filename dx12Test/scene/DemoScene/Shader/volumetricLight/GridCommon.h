#ifndef _VOLUMETRIC_LIGHTING_GRID_COMMON_
#define _VOLUMETRIC_LIGHTING_GRID_COMMON_

#include <MacroDefine.hlsli>

#ifdef __cplusplus

#include "../math/SimpleMath.h"
using namespace hinode::math;

#endif

NAMESPACE(volumetricLightingGridCommon)

struct BuildScatteringAmountGridParam
{
	float4x4 viewMatrix;		///< �O���b�h�̃r���[�s��. ��{�I�ɃV�[���̃r���[�s��
	float4x4 projMatrix;		///< �O���b�h�p�̓��e�s��
	float4x4 invProjMatrix;		///< �O���b�h�̋t���e�s��
	float depthRange;			///< �r���[��ԏ�̃O���b�h��Z�͈̔�
	float curveture;			///< Z�X���C�X�̔z�u� 1���Ɠ��Ԋu. ����ȏ�Ŏ�O���ɏW�܂�
	float depthDivideCount;		///< �O���b�h��Z������
	float timer;				///< �p�[�e�B�N���̈ʒu�X�V�̂��߂̂���
};

END_NAMESPACE

#ifdef __cplusplus
//�V�F�[�_���Ő錾�����\���̂�֐���C++���Ƃ̖��O�Փ˂�����邽�߂�shader���O��Ԃɓ����悤�ɂ��Ă��邪�A
// ���񏑂��̂��ʓ|�Ȃ̂ŁAshader�����͏ȗ��ł���悤�ɂ��Ă���
using namespace volumetricLightingGridCommon;
#endif

#ifndef __cplusplus

/// @brief �r���[��ԏ��Z�l����O���b�h��Z�X���C�X�ʒu���v�Z����
/// @param[in] viewPosZ �r���[��ԏ��Z�l
/// @param[in] param
/// @retval Z�X���C�X�l 0�`cbDepthDivideCount�̊Ԃ����B�͂�
float calGridZSlicePos(float viewPosZ, BuildScatteringAmountGridParam param)
{
	return pow(viewPosZ/param.depthRange, (1/param.curveture)) * param.depthDivideCount;
}

/// @brief �r���[��ԏ��Z�l����O���b�h�̐��K�����ꂽZ�X���C�X�ʒu���v�Z����
/// @param[in] viewPosZ �r���[��ԏ��Z�l
/// @param[in] param
/// @retval Z�X���C�X�l 0�`1�̊Ԃ����B�͂�
float calGridZSliceNormalizePos(float viewPosZ, BuildScatteringAmountGridParam param)
{
	return pow(viewPosZ/param.depthRange, (1/param.curveture));
}

///	@brief �O���b�h�Ɋi�[����X�L���b�^�����O�ʂ��v�Z����
///	@param[in] amount �x�[�X�ƂȂ�X�L���b�^�����O��
///	@param[in] slicePos �O���b�h��Z�X���C�X�ʒu
///	@param[in] baseSlicePos �i�[��ƂȂ�Z�X���C�X�ʒu
/// @retval �X�L���b�^�����O��
float calAmountForZSlice(float amount, float slicePos, float baseSlicePos)
{
	float influence = 1 - saturate(abs(slicePos - baseSlicePos));
	return amount * influence;
}

/// @brief �O���b�h��Z�X���C�X�l����r���[��ԏ��Z�l�����߂�
/// @param[in] zSlicePos �O���b�h��Z�X���C�X�l
/// @parma[in] param
/// @retval �r���[��ԏ��Z�l
float calViewZFromGridZSlice(float zSlicePos, BuildScatteringAmountGridParam param)
{
	return param.depthRange * pow(zSlicePos/(param.depthDivideCount-1), param.curveture);
}

#endif

#endif
