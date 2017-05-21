#include "stdafx.h"

#include "ParticleManager.h"

#include "ParticleRender.hlsl"

using namespace hinode;
using namespace hinode::graphics;

ParticleManager::ParticleManager()
{

}

ParticleManager::~ParticleManager()
{

}

void ParticleManager::clear()
{
	this->mParticles.clear();
	this->mParticles.shrink_to_fit();
	this->mPipeline.clear();
}

void ParticleManager::init(const InitParam& param)
{
	assert(nullptr != param.pGraphics);
	this->clear();

	this->mParticles.reserve(param.particleLimit);

	this->mAreaMin = param.areaMin;
	this->mAreaMax = param.areaMax;

	{
		utility::Shader vs, gs, ps, rootSignature;
		utility::Shader::CompileParam compileParam;
		vs.load(L"scene/BetterDrawCallScene/ParticleManager/ParticleRender.hlsl", compileParam.setEntryAndModel("VSMain", "vs_5_1"));
		gs.load(L"scene/BetterDrawCallScene/ParticleManager/ParticleRender.hlsl", compileParam.setEntryAndModel("GSMain", "gs_5_1"));
		ps.load(L"scene/BetterDrawCallScene/ParticleManager/ParticleRender.hlsl", compileParam.setEntryAndModel("PSMain", "ps_5_1"));
		rootSignature.load(L"scene/BetterDrawCallScene/ParticleManager/ParticleRender.hlsl", compileParam.setEntryAndModel("RSForward", "rootsig_1_1"));
		CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		DX12DepthStencilDesc depthStencilDesc;
		depthStencilDesc
			.setDepth(false)
			.isEnableStencil(false);
		DX12_GRAPHICS_PIPELINE_STATE_DESC desc;
		desc
			.setVS(vs.blob())
			.setGS(gs.blob())
			.setPS(ps.blob())
			.setBlend(&blendDesc)
			.setDepthStencil(&depthStencilDesc)
			.setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT)
			.setRTV({ DXGI_FORMAT_R8G8B8A8_UNORM })
			;
		this->mPipeline.create(param.pGraphics->device(), &desc, rootSignature);
	}
}

void ParticleManager::update()
{
	for (auto& particle : this->mParticles) {
		particle.pos += particle.velocity;
		if (particle.pos.x < this->mAreaMin.x) {
			particle.velocity.x *= -1;
		} else if (this->mAreaMax.x < particle.pos.x) {
			particle.velocity.x *= -1;
		}

		if (particle.pos.z < this->mAreaMin.y) {
			particle.velocity.z *= -1;
		} else if (this->mAreaMax.y < particle.pos.z) {
			particle.velocity.z *= -1;
		}
	}
}

bool ParticleManager::add(const Particle& particle)
{
	if(this->mParticles.capacity() <= this->mParticles.size()){
		return false;
	}

	this->mParticles.push_back(particle);
	return true;
}

void ParticleManager::renderParticle(hinode::graphics::DX12GraphicsCommandList& cmdList, const Particle& target, RenderParam& param)
{
	auto& pipeline = this->mPipeline;
	if (param.isUpdateCBCamera) {
		pipeline.updateCBV("CBCamera", *param.pDHPool, param.cameraAllocateInfo);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		cmdList->IASetVertexBuffers(0, 0, nullptr);
		cmdList->IASetIndexBuffer(nullptr);
	}

	shader::particleRender::CBParticle cbParticle;
	cbParticle.cbPos = target.pos;
	cbParticle.cbRadius = target.radius;
	cbParticle.cbColor = param.color;
	pipeline.updateCBV("CBParticle", *param.pDHPool, &cbParticle, sizeof(cbParticle), *param.pCBPool);
	pipeline.bind(cmdList);
	cmdList->DrawInstanced(1, 1, 0, 0);
}

const std::vector<Particle>& ParticleManager::particles()const noexcept
{
	return this->mParticles;
}
