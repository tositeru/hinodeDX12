#pragma once

#include <graphics/dx12/utility/DX12Utility.h>

/// @brief エネルギーを運ぶ粒子
struct Particle
{
	hinode::math::float3 pos;
	float radius = 1.f;
	float energy;
	hinode::math::float3 velocity;
};

/// @brief Particleを管理するクラス
class ParticleManager
{
public:
	/// @brief 初期パラメータ
	struct InitParam
	{
		size_t particleLimit = 0u;
		hinode::math::float2 areaMin;
		hinode::math::float2 areaMax;

		hinode::graphics::utility::Graphics* pGraphics = nullptr;
	};

	/// @brief 描画パラメータ
	struct RenderParam
	{
		bool isUpdateCBCamera = true; ///< カメラパラメータの定数バッファを更新するか？ 更新する際に使われる値はworldCameraAllocateInfoになります。
		hinode::graphics::utility::ConstantBufferPool::AllocateInfo cameraAllocateInfo;
		hinode::graphics::utility::ConstantBufferPool* pCBPool;
		hinode::graphics::utility::DescriptorHeapPoolSet* pDHPool;
		hinode::math::float4 color;
	};

public:
	ParticleManager();
	~ParticleManager();

	void clear();

	void init(const InitParam& param);

	void update();

	/// @brief 粒子を追加する
	/// @retval bool 追加に失敗したらfalseを返す
	bool add(const Particle& particle);

	/// @brief 単一のMassを描画する
	///
	/// TODO まとめて描画するものを作成する
	void renderParticle(hinode::graphics::DX12GraphicsCommandList& cmdList, const Particle& target, RenderParam& param);

accessor_declaration:
	const std::vector<Particle>& particles()const noexcept;

private:
	std::vector<Particle> mParticles;
	hinode::graphics::utility::PipelineSet mPipeline;
	hinode::math::float2 mAreaMin;
	hinode::math::float2 mAreaMax;
};
