//
//	シェーダで共通の構造体を宣言
//

//
//	hlslとcppファイルで共用で使えるようにするためのマクロ

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
	float4x4 toShadowMapUVSpace;	///< ワールド空間の位置をシャドウマップのUV座標に変換する行列
	float depthOffset;				///< Z Fightingを避けるためのパラメータ
	float darknessRate;				///< 影の濃さ, 0で真っ黒、1で影なし
	float2 pad;
};

END_NAMESPACE

#ifdef __cplusplus
//シェーダ側で宣言した構造体や関数はC++側との名前衝突を避けるためにshader名前空間に入れるようにしているが、
// 毎回書くのが面倒なので、shader部分は省略できるようにしている
using namespace shader;
#endif

#ifndef __cplusplus

//
//	HLSL専用構造体
//

struct DefferedOutput
{
	float4 albedo : SV_Target0;
	float3 normal : SV_Target1;
	float4 specular : SV_Target2;	// rgb: 色, a:強度
};

/// @brief ディファードレンダリング用の出力を生成する
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