//
//	共通のマクロを定義
//

//
//	hlslとcppファイルで共用で使えるようにするためのマクロ

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
