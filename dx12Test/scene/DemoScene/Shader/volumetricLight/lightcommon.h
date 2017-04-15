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
	float4x4 toShadowMapSpaceMat;	///< ビュー空間上からシャドウマップ上へ変換する
	float3 lightDir;				///< ビュー空間上のライト方向
	float scatteringAmount;
	float3 lightColor;
	float G;						///< Henyey-Greensteinによるミー散乱で使う係数。
};

END_NAMESPACE

#ifdef __cplusplus
//シェーダ側で宣言した構造体や関数はC++側との名前衝突を避けるためにshader名前空間に入れるようにしているが、
// 毎回書くのが面倒なので、shader部分は省略できるようにしている
using namespace volumetricLightingCommon;
#endif

#ifndef __cplusplus

/// @brief Henyey-Greensteinによるミー散乱関数を計算する
/// @param[in] cosTheta cos(レイ方向とライトとの角度)
/// @retval 散乱量
float MieScattering(float cosTheta, float G)
{
#define PI 3.14159265f
	return pow((1-G),2) / (4*PI * pow(1 + pow(G, 2) - 2*G * cosTheta, 3.f/2.f));
}

/// @brief ビュー空間の位置をシャドウマップ座標系に変換する
/// @param[in] pos
/// @param[in] toShadowMapSpace
/// @retval シャドウマップ座標系の位置(xyは0-1の範囲にしてある)
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