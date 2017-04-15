///
///	���ʂŎg����֐��Ƃ����܂Ƃ߂��w�b�_�[
///
#ifndef __cplusplus

/// @brief bool�^�̃x�N�g�����݂āA�S�v�f��true�Ȃ�true��Ԃ�
/// @param[in] b
/// @retval bool
bool isAnd(bool2 b)
{
	return b.x && b.y;
}

/// @brief bool�^�̃x�N�g�����݂āA�S�v�f����ȏ�true�Ȃ�
/// @param[in] b
/// @retval bool
bool isOr(bool2 b)
{
	return b.x || b.y;
}

/// @brief ���s���̃f�B�t���[�Y�ƃX�y�L���������v�Z����
/// @param[in] normal �֐������Ő��K������܂��B
/// @param[in] lihgtDir
/// @param[in] eyeDir �u���_�ւ́v�x�N�g���B�֐������Ő��K������܂��B
/// @retval float2 x:�f�B�t���[�Y����, y:�X�y�L�����[����
float2 calDirectionLightLit(float3 normal, float specularPower, float3 lightDir, float3 eyeDir)
{
	normal = normalize(normal);
	float2 lit;
	lit.x = -dot(normal, lightDir);
	float3 reflectDir = reflect(lightDir, normal);
	eyeDir = normalize(eyeDir);
	lit.y = max(0, dot(reflectDir, eyeDir));
	lit.y = pow(lit.y, specularPower);
	return lit;
}

/// @brief �[�x�l���烏�[���h��Ԃ̈ʒu�ɕϊ�����
/// @param[in] depth
/// @param[in] texel
/// @param[in] texSize
/// @param[in] invViewProj
/// @retval float4
float4 toWorldPosFromScreen(float depth, uint2 texel, uint2 texSize, float4x4 invViewProj)
{
	float4 result = float4((float2)texel / (float2)texSize * float2(2,-2) - float2(1,-1), depth, 1);
	result = mul(invViewProj, result);
	result /= result.w;
	return result;
}

/// @brief �@���e�N�X�`������@���𕜌�����
/// @param[in] txNormal
/// @param[in] texel
/// @retval float3
float3 sampleNormal(Texture2D<float3> txNormal, uint2 texel)
{
	return txNormal[texel] * 2 - 1;
}

#endif
