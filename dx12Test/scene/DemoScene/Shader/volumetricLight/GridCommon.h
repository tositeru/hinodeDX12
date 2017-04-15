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
	float4x4 viewMatrix;		///< グリッドのビュー行列. 基本的にシーンのビュー行列
	float4x4 projMatrix;		///< グリッド用の投影行列
	float4x4 invProjMatrix;		///< グリッドの逆投影行列
	float depthRange;			///< ビュー空間上のグリッドのZの範囲
	float curveture;			///< Zスライスの配置具合 1だと等間隔. それ以上で手前側に集まる
	float depthDivideCount;		///< グリッドのZ分割数
	float timer;				///< パーティクルの位置更新のためのもの
};

END_NAMESPACE

#ifdef __cplusplus
//シェーダ側で宣言した構造体や関数はC++側との名前衝突を避けるためにshader名前空間に入れるようにしているが、
// 毎回書くのが面倒なので、shader部分は省略できるようにしている
using namespace volumetricLightingGridCommon;
#endif

#ifndef __cplusplus

/// @brief ビュー空間上のZ値からグリッドのZスライス位置を計算する
/// @param[in] viewPosZ ビュー空間上のZ値
/// @param[in] param
/// @retval Zスライス値 0～cbDepthDivideCountの間を取る。はず
float calGridZSlicePos(float viewPosZ, BuildScatteringAmountGridParam param)
{
	return pow(viewPosZ/param.depthRange, (1/param.curveture)) * param.depthDivideCount;
}

/// @brief ビュー空間上のZ値からグリッドの正規化されたZスライス位置を計算する
/// @param[in] viewPosZ ビュー空間上のZ値
/// @param[in] param
/// @retval Zスライス値 0～1の間を取る。はず
float calGridZSliceNormalizePos(float viewPosZ, BuildScatteringAmountGridParam param)
{
	return pow(viewPosZ/param.depthRange, (1/param.curveture));
}

///	@brief グリッドに格納するスキャッタリング量を計算する
///	@param[in] amount ベースとなるスキャッタリング量
///	@param[in] slicePos グリッドのZスライス位置
///	@param[in] baseSlicePos 格納先となるZスライス位置
/// @retval スキャッタリング量
float calAmountForZSlice(float amount, float slicePos, float baseSlicePos)
{
	float influence = 1 - saturate(abs(slicePos - baseSlicePos));
	return amount * influence;
}

/// @brief グリッドのZスライス値からビュー空間上のZ値を求める
/// @param[in] zSlicePos グリッドのZスライス値
/// @parma[in] param
/// @retval ビュー空間上のZ値
float calViewZFromGridZSlice(float zSlicePos, BuildScatteringAmountGridParam param)
{
	return param.depthRange * pow(zSlicePos/(param.depthDivideCount-1), param.curveture);
}

#endif

#endif
