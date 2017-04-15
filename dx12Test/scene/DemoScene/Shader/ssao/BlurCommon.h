//
//  ブラーの共通ヘッダー
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
//シェーダ側で宣言した構造体や関数はC++側との名前衝突を避けるためにshader名前空間に入れるようにしているが、
// 毎回書くのが面倒なので、shader部分は省略できるようにしている
using namespace blurCommon;
#endif

#ifndef __cplusplus
//HLSL側のみのコード

/// @brief グループIDとスレッドIDからサンプリング位置の片方の成分を計算する
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

/// @brief ブラーに必要なパラメータをサンプリングする
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

/// @brief ガウシアンブラーの係数を計算する
/// @param[in] offset 中心からの距離
/// @retval float
float calGaussianWeight(int offset, float sigmaSq)
{
	const float PI = 3.14159265f;
	const float A = 1 / (2.f*PI*sigmaSq);
	
	return A * exp(- offset*offset / (2*sigmaSq));
}

#endif

#endif