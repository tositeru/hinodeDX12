#include "stdafx.h"

#include "MipmapScene.h"

#include <DirectXTex\WICTextureLoader\WICTextureLoader12.h>
#include <graphics\dx12\utility\winapi\KeyObserver.h>

MipmapScene::MipmapScene()
{
}

MipmapScene::~MipmapScene()
{
	this->clear();
}

void MipmapScene::clear()
{
	this->mMesh.clear();
	this->mGroundMesh.clear();
	this->mVisiblableMipmapTexture.clear();
	this->mMeshTexture.clear();
	this->mPTVertexPipeline.clear();
	this->mCBPool.clear();
	this->mDHPool.clear();
}

void MipmapScene::init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	utility::PreviewCamera::InitParam initParam;
	initParam.pos(0, 0, -5);
	initParam.aspect = graphics.calAspect();
	this->mCamera.init(initParam);

	this->mCBPool.create(graphics.device(), 64 * 1024);
	this->mDHPool.create(graphics.device(), 1024, 1024);
	this->mPTVertexPipeline = utility::ForwardPipeline::sCreatePTVertex(graphics.device());

	{
		utility::Shader vs, ps;
		vs.load(L"data/shader/CalMiplevel.hlsl", utility::Shader::CompileParam().setEntryAndModel("VSMain", "vs_5_1"));
		ps.load(L"data/shader/CalMiplevel.hlsl", utility::Shader::CompileParam().setEntryAndModel("PSMain", "ps_5_1"));
		DX12_GRAPHICS_PIPELINE_STATE_DESC desc;
		desc.setVS(vs.blob())
			.setPS(ps.blob())
			.setInputLayout({
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, }
				)
			;
		utility::Shader rootSignature;
		rootSignature.load(L"data/shader/CalMiplevel.hlsl", utility::Shader::CompileParam().setEntryAndModel("RS", "rootsig_1_1"));
		this->mCalMiplevelPipeline.create(graphics.device(), &desc, rootSignature);
	}
	utility::ResourceUploader uploader;
	uploader.init(graphics.device());

	//uploader.loadTextureFromWICFile(graphics.device(), this->mMeshTexture, "data/panda.png");
	uploader.loadTextureFromDDS(graphics.device(), this->mMeshTexture, "data/panda.dds");
	D3D12_SHADER_RESOURCE_VIEW_DESC srv = this->mMeshTexture.makeSRV();
	this->mMeshTexture.appendView(DX12Resource::View().setSRV(srv));

	{
		DX12ResourceHeapDesc heapDesc;
		DX12ResourceDesc resourceDesc;
		resourceDesc.desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256);
		resourceDesc.desc.MipLevels = DX12Resource::sCalMipLevel(static_cast<UINT16>(resourceDesc.desc.Width), static_cast<UINT16>(resourceDesc.desc.Height));
		resourceDesc.desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resourceDesc.initialStates = D3D12_RESOURCE_STATE_COPY_DEST;
		this->mVisiblableMipmapTexture.create(graphics.device(), heapDesc, resourceDesc);
		srv = this->mVisiblableMipmapTexture.makeSRV();
		srv.Texture2D.MostDetailedMip = 0;
		this->mVisiblableMipmapTexture.appendView(DX12Resource::View().setSRV(srv));

		std::vector<std::vector<uint32_t>> rawDatas;
		std::vector<utility::ResourceUploader::TextureDesc> uploadDescs;

		rawDatas.resize(resourceDesc.desc.MipLevels);
		uploadDescs.resize(rawDatas.size());
		auto w = resourceDesc.desc.Width;
		auto h = resourceDesc.desc.Height;
		for (auto i = 0u; i < rawDatas.size(); ++i) {
			static const std::array<uint32_t, 8> colorTbl = { {
					0xff0000ff,
					0xff0066ff,
					0xff00b9ff,
					0xff00ff93,
					0xff93ff00,
					0xffffa600,
					0xffff0080,
					0xff8000ff,
				} };
			auto length = w * h;
			rawDatas[i].resize(length, colorTbl[std::min<size_t>(i, colorTbl.size()-1)]);

			uploadDescs[i].pData = rawDatas[i].data();
			uploadDescs[i].subresource = i;
			uploadDescs[i].rowPitch = w * sizeof(rawDatas[i][0]);

			w = std::max<decltype(w)>(1, w / 2);
			h = std::max<decltype(h)>(1, h / 2);
		}
		uploader.entryTexture(this->mVisiblableMipmapTexture, uploadDescs);
	}

	{
		std::vector<utility::PTVertex> vertices;
		std::vector<uint16_t> indices;
		utility::TetrahedronCreator::sCreate(vertices, indices);
		utility::Mesh::InitParam initParam;
		initParam
			.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
			.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
		this->mMesh.create(graphics.device(), uploader, initParam);

		utility::PlaneCreator::sCreate(vertices, indices, utility::PlaneCreator::ePLANE_XZ);
		initParam
			.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
			.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
		this->mGroundMesh.create(graphics.device(), uploader, initParam);
	}
}

bool MipmapScene::paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	static winapi::KeyDownObserverManager keyObserver;
	static auto& isShowReference = keyObserver.add('1', false, [&](const winapi::KeyDownObserver& This) {
		Log(Log::eMEMO) << "ミップレベル計算" << (This ? "GPU自動" : "自前");
	});
	static auto& useImage = keyObserver.add('X', false, [&](const winapi::KeyDownObserver& This) {
	});

	keyObserver.update();

	this->mCamera.update();
	graphics.waitForCurrentFrame();
	graphics.currentContext().record(nullptr, [&](DX12GraphicsCommandList& cmdList) {
		this->mDHPool.bind(cmdList);

		static float t = 0.f; t += 0.001f;
		const float clearColor[] = { 0.0f, 0.2f, t, 1.f };
		graphics.bindCurrentRenderTargetAndDepthStencil(cmdList);
		graphics.clearCurrentRenderTarget(cmdList, clearColor);
		graphics.clearCurrentDepthStencil(cmdList, 1.f, 0);

		utility::PipelineSet* pPipeline;
		if (isShowReference) {
			pPipeline = &this->mPTVertexPipeline;
		} else {
			pPipeline = &this->mCalMiplevelPipeline;
		};

		shader::CameraParam cameraParam;
		cameraParam.eyePos = this->mCamera.pos();
		cameraParam.viewProj = this->mCamera.viewProjMatrix();
		pPipeline->updateCBV("CBCamera", this->mDHPool, &cameraParam, sizeof(cameraParam), this->mCBPool);

		DX12SamplerDesc sampler;
		pPipeline->updateSampler("smSampler", this->mDHPool, &sampler);
		DX12Resource* pTex = useImage ? &this->mMeshTexture : &this->mVisiblableMipmapTexture;
		pPipeline->updateSRV("txTexture", this->mDHPool, *pTex, &pTex->view().srv());
		
		PTVertexPipeline::CBModel modelParam;
		float rotaOffset = t * 10.f;
		modelParam.cbWorld = math::makeRotationX<math::float4x4>(rotaOffset);
		modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationX<float4x4>(rotaOffset));
		modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationZ<float4x4>(rotaOffset));
		math::float4 pos(0, 0, 0, 1);
		modelParam.cbWorld[3] = pos;
		pPipeline->updateCBV("CBModel", this->mDHPool, &modelParam, sizeof(modelParam), this->mCBPool);
		pPipeline->bind(cmdList);

		this->mMesh.bind(cmdList);
		this->mMesh.draw(cmdList);

		modelParam.cbWorld = math::makeScale<math::float4x4>({10});
		modelParam.cbWorld[3](0, -5, 0, 1);
		pPipeline->updateCBV("CBModel", this->mDHPool, &modelParam, sizeof(modelParam), this->mCBPool);
		pPipeline->bind(cmdList);

		this->mGroundMesh.bind(cmdList);
		this->mGroundMesh.draw(cmdList);

		cmdList.setBarriers(graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_PRESENT));
	});

	graphics.present(1, { graphics.currentContext().cmdList() });
	graphics.endAndGoNextFrame();
	this->mCBPool.endAndGoNextFrame(graphics.cmdQueue());
	this->mDHPool.endAndGoNextFrame(graphics.cmdQueue());
	return true;
}
