///
///	共通で使える関数とかをまとめたヘッダー
///
#ifndef __cplusplus

/// @brief bool型のベクトルをみて、全要素がtrueならtrueを返す
/// @param[in] b
/// @retval bool
bool isAnd(bool2 b)
{
	return b.x && b.y;
}

/// @brief bool型のベクトルをみて、全要素内一つ以上trueなら
/// @param[in] b
/// @retval bool
bool isOr(bool2 b)
{
	return b.x || b.y;
}

/// @brief 平行光のディフューズとスペキュラ項を計算する
/// @param[in] normal 関数内部で正規化されます。
/// @param[in] lihgtDir
/// @param[in] eyeDir 「視点への」ベクトル。関数内部で正規化されます。
/// @retval float2 x:ディフューズ成分, y:スペキュラー成分
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

/// @brief 深度値からワールド空間の位置に変換する
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

/// @brief 法線テクスチャから法線を復元する
/// @param[in] txNormal
/// @param[in] texel
/// @retval float3
float3 sampleNormal(Texture2D<float3> txNormal, uint2 texel)
{
	return txNormal[texel] * 2 - 1;
}

#endif
