//
//  AO�v�Z�̋��ʃw�b�_�[
//

#ifndef _AO_COMMON_
#define _AO_COMMON_

#include <MacroDefine.hlsli>

#ifdef __cplusplus

#include "../math/SimpleMath.h"
using namespace hinode::math;

#endif

NAMESPACE(ssaoCommon)

struct AOCommonParam
{
	float2 projXYToView;
	uint samplingCount;
	int samplingStep;
	
	uint samplingRange;
	float3 pad;
};

struct AlchemyAOParam
{
	float radius;
	float beta;
	float K;
	float sigma;
};

struct DebugShowParam
{
	uint2 samplingPos;
	int radius;
	uint samplingCount;
	
	float worldRadius;
	float2 projXYToView;
	int samplingStep;
};

END_NAMESPACE

#ifdef __cplusplus
//�V�F�[�_���Ő錾�����\���̂�֐���C++���Ƃ̖��O�Փ˂�����邽�߂�shader���O��Ԃɓ����悤�ɂ��Ă��邪�A
// ���񏑂��̂��ʓ|�Ȃ̂ŁAshader�����͏ȗ��ł���悤�ɂ��Ă���
using namespace ssaoCommon;
#endif

#ifndef __cplusplus
//HLSL���݂̂̃R�[�h


/// @brief ��������
///
/// �ȉ��̃T�C�g���玝���Ă���
/// http://www.reedbeta.com/blog/2013/01/12/quick-and-easy-gpu-random-numbers-in-d3d11/
/// @param[in] seed
/// @retval uint
uint wangHash(uint seed)
{
	seed = (seed^61) ^ (seed>>16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

/// @brief ���K�������T���v�����O�|�C���g�̃I�t�Z�b�g�𐶐�����
/// @param[inout] seed �����̂���
/// @retval float2 �l�͈̔͂�-1�`1�ɂȂ�
float2 makeSamplingPoint(inout uint seed)
{
	float2 pixel;
	seed = wangHash(seed);
	pixel.x = ((seed / (float)0xffffffff) * 2.f - 1);
	seed = wangHash(seed);
	pixel.y = ((seed / (float)0xffffffff) * 2.f - 1);
	pixel.y *= 1 - pow(abs(pixel.x), 3);//�ߎ������~�`�ɃT���v�����O���邽�߂̕␳
	return pixel;
	//�����Ɗp�x���狁�߂�
	seed = wangHash(seed);
	float length = (seed / (float)0xffffffff);
	length = pow(length, 2);
	seed = wangHash(seed);
	#define PI (3.141592f)
	float radian = 2 * PI * (seed / (float)0xffffffff);
	return length * float2(cos(radian), sin(radian));
}

/// @brief van der Corput��̌v�Z
///
/// @param[in]
/// @param[in] 
/// @retval float
float vdc(uint n, uint base)
{
	float h = 0;
	float f = 1.f / (float)base;
	float factor = f;
	
	[loop]while(0 < n) {
		h += (float)(n%base) * factor;
		n /= base;
		factor *= f;
	}
	return h;
}

/// @brief ���K�������T���v�����O�|�C���g�̃I�t�Z�b�g�𐶐�����
/// @param[inout] seed �����̂���
/// @retval float2 �l�͈̔͂�-1�`1�ɂȂ�
float2 makeSamplingPointByVDC(uint2 n)
{
	return float2(vdc(n.x, 2), vdc(n.y, 3)) * 2 - 1;
}

/// @brief �s�N�Z������r���[��Ԃ̈ʒu�֕ϊ�����
/// @param[in] pixel
/// @param[in] viewZ
/// @param[in] texSize
/// @param[in] projXYToView �v���W�F�N�V������Ԃ�XY�������r���[��Ԃ֕ϊ����邽�߂̃p�����[�^
/// @retval float3
float3 toViewPos(float2 pixel, float viewZ, float2 texSize, float2 projXYToView)
{
	float2 proj = pixel / texSize * float2(2.f,-2.f) - float2(1, -1);
	float3 viewPos;
	viewPos.xy = proj.xy * viewZ / projXYToView.xy;
	viewPos.z = viewZ;
	return viewPos;
}

/// @brief �r���[��Ԃ̖@�������߂�
/// @param[in] id
/// @param[in] texSize
/// @param[in] texViewZ
/// @param[in] float2 projXYToView
/// @retval float3
float3 calNormal(uint2 id, uint2 texSize, Texture2D<float> texViewZ, float2 projXYToView)
{
	if(texSize.x <= id.x+1) { id.x -= 1; }
	if(texSize.y <= id.y+1) { id.y -= 1; }
	
	float3 center = toViewPos((float2)id, texViewZ[id], (float2)texSize, projXYToView);
	uint2 pixel = id + uint2(0, 1);
	float3 vertical = toViewPos((float2)pixel, texViewZ[pixel], (float2)texSize, projXYToView);
	pixel = id + uint2(1, 0);
	float3 side = toViewPos((float2)pixel, texViewZ[pixel], (float2)texSize, projXYToView);
	
	float3 normal = cross((side-center), (vertical-center));
	return normalize(normal);
}

#define AO_EQUAL_TYPE 1
/// @brief Alchemy AO�̎����g����AO�v�Z
/// @param[in] center
/// @param[in] normal
/// @param[in] targetPos
/// @param[in] radiusSq
/// @param[in] beta
/// @param[inout] samplingCounter
/// @retval float
float calAlchemyAO(float3 center, float3 normal, float3 targetPos, float radiusSq, float beta, inout uint samplingCounter)
{
	float3 v = targetPos - center;
	float distanceSq = dot(v, v);
	[branch] if(radiusSq < distanceSq) {
		return 0;
	}
	++samplingCounter;
	
#if AO_EQUAL_TYPE == 1
	//Scalable AO�̎�
	const float EPSILON = 0.0001f;
	float A = max(pow(radiusSq - distanceSq, 3), 0);
	float B = max(0, (dot(v, normal) - beta) / (distanceSq + EPSILON));
	return A * B;
#else
	//Alchemy AO�̎�
	float numerator = max(0, dot(v, normal) - center.z*beta);
	const float EPSILON = 0.0001f;
	float denominator = dot(v, v) + EPSILON;
	return numerator / denominator;
#endif
}

float calAlchemyAOSum(float A, float radius, uint samplingCount, float sigma, float K)
{
#if AO_EQUAL_TYPE == 1
	return max(0, 1 - 5 / (pow(radius, 6) * samplingCount) * A);
#else
	return pow(max(0, 1 - 2 * A * sigma / (float)max(1, samplingCount)), K);
#endif
}

#endif

#endif
