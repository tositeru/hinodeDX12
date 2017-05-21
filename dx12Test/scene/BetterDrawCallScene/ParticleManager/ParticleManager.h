#pragma once

#include <graphics/dx12/utility/DX12Utility.h>

/// @brief �G�l���M�[���^�ԗ��q
struct Particle
{
	hinode::math::float3 pos;
	float radius = 1.f;
	float energy;
	hinode::math::float3 velocity;
};

/// @brief Particle���Ǘ�����N���X
class ParticleManager
{
public:
	/// @brief �����p�����[�^
	struct InitParam
	{
		size_t particleLimit = 0u;
		hinode::math::float2 areaMin;
		hinode::math::float2 areaMax;

		hinode::graphics::utility::Graphics* pGraphics = nullptr;
	};

	/// @brief �`��p�����[�^
	struct RenderParam
	{
		bool isUpdateCBCamera = true; ///< �J�����p�����[�^�̒萔�o�b�t�@���X�V���邩�H �X�V����ۂɎg����l��worldCameraAllocateInfo�ɂȂ�܂��B
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

	/// @brief ���q��ǉ�����
	/// @retval bool �ǉ��Ɏ��s������false��Ԃ�
	bool add(const Particle& particle);

	/// @brief �P���Mass��`�悷��
	///
	/// TODO �܂Ƃ߂ĕ`�悷����̂��쐬����
	void renderParticle(hinode::graphics::DX12GraphicsCommandList& cmdList, const Particle& target, RenderParam& param);

accessor_declaration:
	const std::vector<Particle>& particles()const noexcept;

private:
	std::vector<Particle> mParticles;
	hinode::graphics::utility::PipelineSet mPipeline;
	hinode::math::float2 mAreaMin;
	hinode::math::float2 mAreaMax;
};
