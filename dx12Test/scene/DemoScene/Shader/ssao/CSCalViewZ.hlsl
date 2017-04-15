//
//	�v���W�F�N�V������Ԃ�Z�l����r���[��ԏ��Z�l�ɕϊ�����
//

#ifndef _CS_CAL_VIEW_Z_
#define _CS_CAL_VIEW_Z_

#include <RootSignatureCommon.hlsli>
#include <Utility.hlsli>
#include <MacroDefine.hlsli>
#include <StructureCommon.hlsli>

NAMESPACE(calViewZ)

#define THREAD_X 8
#define THREAD_Y 8

#ifdef __cplusplus

/// @brief ���̃V�F�[�_��Wave�T�C�Y��Ԃ�
/// @retval uint3
inline uint3 getThreadCount(){
	return uint3(THREAD_X, THREAD_Y, 1);
}

#endif

//�E��n���̗p
CONSTANT_BUFFER(Param, b0, CS_SPACE)
{
	float2 cbNearFarZ;
	float2 pad;
};

END_NAMESPACE

#ifdef HLSL

//
//	���[�g�V�O�l�`���[��`
//
#define RS \
	ROOT_FLAGS(DENY_GRAPHICS_SHADER_ROOT_ACCESS) CM\
	ROOT_CBV(b0, CS_SPACE_NO, SHADER_VISIBILITY_ALL, 0) CM\
	DESCRIPTOR_TABLE(SHADER_VISIBILITY_ALL)\
		SRV(t0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) CM\
		UAV(u0, CS_SPACE_NO, 1, DESCRIPTOR_RANGE_OFFSET_APPEND, DATA_STATIC_WHILE_SET_AT_EXECUTE) \
	END_DESCRIPTOR_TABLE

Texture2D<float> tDepth : register(t0, CS_SPACE);
RWTexture2D<float> uOutViewZ : register(u0, CS_SPACE);

/// @brief �r���[��Ԃ�Z�l��[�x�l�ɕϊ�����
/// @param[in] viewZ
/// @retval float
float toDepth(float viewZ)
{
	return ((viewZ - cbNearFarZ.x) * cbNearFarZ.y) / ((cbNearFarZ.y - cbNearFarZ.x) * viewZ);
}

/// @brief �[�x�l����r���[��ԏ��Z�l�ɕϊ�
/// @param[in] depth
/// @retval float
float toViewZ(float depth)
{
	return (-cbNearFarZ.x * cbNearFarZ.y) / (depth * (cbNearFarZ.y - cbNearFarZ.x) - cbNearFarZ.y);
}

[numthreads(THREAD_X, THREAD_Y, 1)]
void main(uint2 id : SV_DispatchThreadID)
{
	uint2 texSize;
	uOutViewZ.GetDimensions(texSize.x, texSize.y);
	[branch] if(id.x < texSize.x && id.y < texSize.y) {
		float depth = tDepth[id];
		uOutViewZ[id] = toViewZ(depth);
	}
}

#endif

#undef THREAD_X
#undef THREAD_Y

#endif
