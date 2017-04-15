//
//  �u���[�̋��ʃw�b�_�[
//

#ifndef _BLUR_COMMON_
#define _BLUR_COMMON_

#include <MacroDefine.hlsli>

#ifdef __cplusplus

#include "../math/SimpleMath.h"
using namespace hinode::math;

#endif

NAMESPACE(blurCommon)

struct BlurCommonParam
{
	int blurRadius;
	float spaceRange;
	float gaussianSigma;
	float pad;
};

END_NAMESPACE

#ifdef __cplusplus
//�V�F�[�_���Ő錾�����\���̂�֐���C++���Ƃ̖��O�Փ˂�����邽�߂�shader���O��Ԃɓ����悤�ɂ��Ă��邪�A
// ���񏑂��̂��ʓ|�Ȃ̂ŁAshader�����͏ȗ��ł���悤�ɂ��Ă���
using namespace blurCommon;
#endif

#ifndef __cplusplus
//HLSL���݂̂̃R�[�h

/// @brief �O���[�vID�ƃX���b�hID����T���v�����O�ʒu�̕Е��̐������v�Z����
/// @param[out] outLocalPos
/// @param[in] groupID
/// @param[in] threadID
/// @param[in] threadSum
/// @param[in] blurRadius
/// @retval int
int calSamplingPos(out int outLocalPos, int groupID, int threadID, int threadSum, int blurRadius)
{
	outLocalPos = threadID - blurRadius;
	return groupID * (threadSum - blurRadius*2) + outLocalPos;
}

/// @brief �u���[�ɕK�v�ȃp�����[�^���T���v�����O����
/// @param[in] pixel
/// @param[in] texSize
/// @param[in] texAO
/// @param[in] texViewZ
/// @retval float2
float2 samplingValues(int2 pixel, int2 texSize, Texture2D<float> texAO, Texture2D<float> texViewZ)
{
	pixel = clamp(pixel, 0, texSize-1);
	return float2(texAO[pixel], texViewZ[pixel]);
}

/// @brief �K�E�V�A���u���[�̌W�����v�Z����
/// @param[in] offset ���S����̋���
/// @retval float
float calGaussianWeight(int offset, float sigmaSq)
{
	const float PI = 3.14159265f;
	const float A = 1 / (2.f*PI*sigmaSq);
	
	return A * exp(- offset*offset / (2*sigmaSq));
}

#endif

#endif