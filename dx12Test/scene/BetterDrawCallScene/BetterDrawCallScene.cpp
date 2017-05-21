#include "stdafx.h"

#include "BetterDrawCallScene.h"

#include <graphics/dx12/utility/winapi/KeyObserver.h>

#include "ParticleManager/ParticleRender.hlsl"

BetterDrawCallScene::BetterDrawCallScene()
{ }

BetterDrawCallScene::~BetterDrawCallScene()
{

}

void BetterDrawCallScene::clear()
{

}

void BetterDrawCallScene::init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	this->clear();

	{
		utility::DrafterCamera::InitParam cameraParam;
		cameraParam.pos(0, 30, 0);
		cameraParam.up(0, 0, 1);
		cameraParam.right(-1, 0, 0);
		cameraParam.planeZ(0.1f, 100.f);
		cameraParam.aspect = graphics.calAspect();
		this->mCamera.init(cameraParam);
		this->mCBPool.create(graphics.device(), 1024 * 246);
		this->mDHPool.create(graphics.device(), 256, 128);
	}
	{//パーティクルの初期化
		ParticleManager::InitParam particleInitParam;
		particleInitParam.particleLimit = 1000;
		particleInitParam.areaMin(-30, -30);
		particleInitParam.areaMax(30, 30);
		particleInitParam.pGraphics = &graphics;
		this->mParticleManager.init(particleInitParam);
	}

	{//クエリの作成
		D3D12_QUERY_HEAP_DESC desc;
		desc.NodeMask = 0;
		desc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		desc.Count = 2 * 2;
		this->mQueryHeap.create(graphics.device(), &desc);

		DX12ResourceDesc srcDesc;
		srcDesc.desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT64)*2 * 2);
		srcDesc.initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
		DX12ResourceHeapDesc heapDesc(D3D12_HEAP_TYPE_READBACK, D3D12_HEAP_FLAG_NONE);
		this->mQueryResult.create(graphics.device(), heapDesc, srcDesc);
	}
}

bool BetterDrawCallScene::paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	//-----------------------------------------------------------------------
	//キー操作の設定
	static winapi::KeyDownObserverManager keyObserver;
	static int swapCount = 10;
	static auto& deincrementSwapCount = keyObserver.add(VK_LEFT, true, [&](const winapi::KeyDownObserver& This) {
		swapCount = std::max(swapCount - 10, 10);
		Log(Log::eMEMO) << "swap count = " << swapCount;
	});
	static auto& incrementSwapCount = keyObserver.add(VK_RIGHT, true, [&](const winapi::KeyDownObserver& This) {
		swapCount = std::min(swapCount + 10, 500);
		Log(Log::eMEMO) << "swap count = " << swapCount;
	});

	static auto& showCommand = keyObserver.add('0', true, [](const winapi::KeyDownObserver& This) {
		Log(Log::eMEMO)
			<< "操作方法" << Log::eBR
			<< "0キー : この文章の表示"
			;
	});

	//-----------------------------------------------------------------------
	//更新
	keyObserver.update();
	this->mCamera.update();

	{
		Particle addParticle;
		addParticle.pos(0, 0, 0);
		addParticle.radius = math::simpleRandom(0.5f, 1.5f);
		addParticle.energy = math::simpleRandom(1, 10);
		addParticle.velocity.x = math::simpleRandom(-1, 1);
		addParticle.velocity.y = 0;
		addParticle.velocity.z = math::simpleRandom(-1, 1);
		addParticle.velocity = math::normalize(addParticle.velocity) * 0.25f;
		this->mParticleManager.add(addParticle);

		this->mParticleManager.update();
	}

	//-----------------------------------------------------------------------
	//描画開始
	graphics.waitForCurrentFrame();

	{//クエリ結果を取り出す。
		UINT64* pTimestamps;
		D3D12_RANGE range;
		range.Begin = graphics.currentBackBufferIndex() * 2;
		range.End = range.Begin + sizeof(UINT64) * 2;
		auto hr = this->mQueryResult->Map(0, &range, (void**)(&pTimestamps));
		if(SUCCEEDED(hr)) {
			auto time = static_cast<double>(pTimestamps[1] - pTimestamps[0]) / static_cast<double>(graphics.cmdQueue().GPUTimestampFrequence());
			Log(Log::eMEMO) << "timestamp = " << time * 1000.0 << "[ms]";
			this->mQueryResult->Unmap(0, nullptr);
		} else {
			Log(Log::eMEMO) << "failed timestamp";
		}
	}

	auto& context = graphics.currentContext();
	context.beginRecord(nullptr);
	//コマンドを細切れに発行するための関数
	auto swapAndExecute = [&]() {
		context.executeAndSwap(graphics.cmdQueue(), nullptr);
		this->mCBPool.endAndGoNextFrame(graphics.cmdQueue());
		this->mDHPool.endAndGoNextFrame(graphics.cmdQueue());
		auto pCmdList = context.currentCmdListPointer();
		this->mDHPool.bind(*pCmdList);
		graphics.bindCurrentRenderTargetAndDepthStencil(*pCmdList);
		return pCmdList;
	};

	auto pCmdList = context.currentCmdListPointer();
	this->mDHPool.bind(*pCmdList);
	graphics.bindCurrentRenderTargetAndDepthStencil(*pCmdList);
	float clearColor[] = { 0, 0.05f, 0.1f, 0 };
	graphics.clearCurrentRenderTarget(*pCmdList, clearColor);
	graphics.clearCurrentDepthStencil(*pCmdList, 1.f, 0);


	{
		(*pCmdList)->EndQuery(this->mQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, graphics.currentBackBufferIndex() * 2 + 0);
		shader::particleRender::CBCamera cbCamera;
		cbCamera.cbViewMat = this->mCamera.viewMatrix();
		cbCamera.cbProjMat = this->mCamera.projectionMatrix();

		{
			ParticleManager::RenderParam renderParam;
			renderParam.isUpdateCBCamera = true;
			renderParam.cameraAllocateInfo = this->mCBPool.allocate(&cbCamera, sizeof(cbCamera));
			renderParam.pCBPool = &this->mCBPool;
			renderParam.pDHPool = &this->mDHPool;
			renderParam.color(1, 1, 0, 1);
			int count = 0;
			const int LIMIT = swapCount;
			for (auto& particle : this->mParticleManager.particles()) {
				this->mParticleManager.renderParticle(*pCmdList, particle, renderParam);
				renderParam.isUpdateCBCamera = false;
				++count;
				if (count == LIMIT) {
					pCmdList = swapAndExecute();
					count = 0;
					//数によっては先に確保したものが破棄されてしまうかもしれないので、再確保している。
					renderParam.cameraAllocateInfo = this->mCBPool.allocate(&cbCamera, sizeof(cbCamera));
					renderParam.isUpdateCBCamera = true;
				}
			}
		}
		pCmdList->setBarriers(graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_PRESENT));

		(*pCmdList)->EndQuery(this->mQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, graphics.currentBackBufferIndex() * 2 + 1);
		(*pCmdList)->ResolveQueryData(this->mQueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, graphics.currentBackBufferIndex() * 2, 2, this->mQueryResult, graphics.currentBackBufferIndex() * sizeof(UINT64) * 2);
	}
	context.endRecord(graphics.cmdQueue());
	this->mCBPool.endAndGoNextFrame(graphics.cmdQueue());
	this->mDHPool.endAndGoNextFrame(graphics.cmdQueue());
	graphics.present(0, {});
	graphics.endAndGoNextFrame();

	return true;
}
