//
//	���ʂ̃}�N�����`
//

//
//	hlsl��cpp�t�@�C���ŋ��p�Ŏg����悤�ɂ��邽�߂̃}�N��

#ifdef __cplusplus

#include "../math/SimpleMath.h"
using namespace hinode::math;
#define CONSTANT_BUFFER(_name, _slot, _space) struct _name
#define NAMESPACE(_name) namespace _name {
#define END_NAMESPACE }

#else

#define CONSTANT_BUFFER(_name, _slot, _space) cbuffer _name : register(_slot, _space)
#define NAMESPACE(_name)
#define END_NAMESPACE

#endif
