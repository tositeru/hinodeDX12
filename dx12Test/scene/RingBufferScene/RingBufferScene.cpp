#include "stdafx.h"

#include "RingBufferScene.h"

RingBufferScene::RingBufferScene()
{

}

RingBufferScene::~RingBufferScene()
{
	this->clear();
}

void RingBufferScene::clear()
{
	this->mCBPool.clear();
	this->mDHPool.clear();
	this->mMeshPTN.clear();
	this->mMeshPTNC.clear();
	this->mTexture.clear();
	this->mMesh.clear();
	this->mPipeline.clear();
}

void RingBufferScene::init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	utility::PreviewCamera::InitParam initParam;
	initParam.pos(0, 0, -5);
	initParam.aspect = graphics.calAspect();
	this->mCamera.init(initParam);

	this->mCBPool.create(graphics.device(), 64 * 1024);
	this->mDHPool.create(graphics.device(), 128, 1024);
	this->mPipeline = utility::ForwardPipeline::sCreatePCVertex(graphics.device());
	this->mPipelinePN = utility::ForwardPipeline::sCreatePNVertex(graphics.device());
	this->mPipelinePTN = utility::ForwardPipeline::sCreatePTNVertex(graphics.device());
	this->mPipelinePTNC = utility::ForwardPipeline::sCreatePTNCVertex(graphics.device());

	utility::ResourceUploader uploader;
	uploader.init(graphics.device());
	uploader.loadTextureFromDDS(graphics.device(), this->mTexture, "data/panda.dds");
	D3D12_SHADER_RESOURCE_VIEW_DESC srv = this->mTexture.makeSRV();
	this->mTexture.appendView(DX12Resource::View().setSRV(srv));

	{
		std::vector<utility::PCVertex> vertices;
		std::vector<uint16_t> indices;
		utility::TetrahedronCreator::sCreate(vertices, indices);
		utility::Mesh::InitParam initParam;
		initParam
			.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
			.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
		this->mMesh.create(graphics.device(), uploader, initParam);
	}
	{
		std::vector<utility::PNVertex> vertices;
		std::vector<uint16_t> indices;
		utility::PlaneCreator::sCreate(vertices, indices, utility::PlaneCreator::ePLANE_XZ);
		utility::Mesh::InitParam initParam;
		initParam
			.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
			.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
		this->mPlane.create(graphics.device(), uploader, initParam);
	}
	{
		std::vector<utility::PNVertex> vertices;
		std::vector<uint16_t> indices;
		utility::TetrahedronCreator::sCreate(vertices, indices);
		utility::Mesh::InitParam initParam;
		initParam
			.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
			.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
		this->mMeshPN.create(graphics.device(), uploader, initParam);
	}
	{
		std::vector<utility::PTNVertex> vertices;
		std::vector<uint16_t> indices;
		utility::TetrahedronCreator::sCreate(vertices, indices);
		utility::Mesh::InitParam initParam;
		initParam
			.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
			.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
		this->mMeshPTN.create(graphics.device(), uploader, initParam);
	}
	{
		std::vector<utility::PTNCVertex> vertices;
		std::vector<uint16_t> indices;
		utility::TetrahedronCreator::sCreate(vertices, indices);
		utility::Mesh::InitParam initParam;
		initParam
			.setVertex(vertices.data(), vertices.size(), sizeof(vertices[0]))
			.setIndex(indices.data(), indices.size(), DXGI_FORMAT_R16_UINT);
		this->mMeshPTNC.create(graphics.device(), uploader, initParam);
	}
}


bool RingBufferScene::paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)
{
	this->mCamera.update();
	graphics.waitForCurrentFrame();

	graphics.currentContext().record(nullptr, [&](DX12GraphicsCommandList& cmdList) {
		this->mDHPool.bind(cmdList);

		static float t = 0.f; t += 0.001f;
		const float clearColor[] = { 0.0f, 0.2f, t, 1.f };
		graphics.bindCurrentRenderTargetAndDepthStencil(cmdList);
		graphics.clearCurrentRenderTarget(cmdList, clearColor);
		graphics.clearCurrentDepthStencil(cmdList, 1.f, 0);

		utility::PipelineSet* pPipeline = &this->mPipeline;
		shader::CameraParam cameraParam;
		cameraParam.eyePos = this->mCamera.pos();
		cameraParam.viewProj = this->mCamera.viewProjMatrix();
		auto cbCameraParam = this->mCBPool.allocate(&cameraParam, sizeof(cameraParam));

		if(true)
		{
			pPipeline->updateCBV("CBCamera", this->mDHPool, cbCameraParam);

			shader::MaterialParam materialParam;
			materialParam.diffuseColor(1, 1, 1);
			materialParam.specularColor(1, 1, 1);
			materialParam.alpha = 1;
			materialParam.specularPower = 1;
			pPipeline->updateCBV("CBMaterial", this->mDHPool, &materialParam, sizeof(materialParam), this->mCBPool);

			PCVertexPipeline::CBModel modelParam;
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
		}

		PNVertexPipeline::CBLight lightParam;
		lightParam.cbLight.direction = math::normalize(math::float3{ 0, -1, 0 });
		lightParam.cbLight.color(1, 1, 1);
		auto cbLightParam = this->mCBPool.allocate(&lightParam, sizeof(lightParam));
		{//’n–Ê‚Ì•`‰æ
			this->mPipelinePN.updateCBV("CBCamera", this->mDHPool, cbCameraParam);
			this->mPipelinePN.updateCBV("CBLight", this->mDHPool, cbLightParam);

			shader::MaterialParam materialParam;
			materialParam.diffuseColor(1, 1, 1);
			materialParam.alpha = 1;
			materialParam.specularColor(1, 1, 1);
			materialParam.specularPower = 5.f;
			this->mPipelinePN.updateCBV("CBMaterial", this->mDHPool, &materialParam, sizeof(materialParam), this->mCBPool);

			PNVertexPipeline::CBModel modelParam;
			modelParam.cbWorld = math::makeScale<math::float4x4>({10, 10, 10});
			modelParam.cbWorld[3](0, -3, 0, 1);
			this->mPipelinePN.updateCBV("CBModel", this->mDHPool, &modelParam, sizeof(modelParam), this->mCBPool);
			this->mPipelinePN.bind(cmdList);

			this->mPlane.bind(cmdList);
			this->mPlane.draw(cmdList);
		}

		{
			shader::MaterialParam materialParam;
			materialParam.diffuseColor(1, 1, 1);
			materialParam.alpha = 1;
			materialParam.specularColor(1, 1, 1);
			materialParam.specularPower = 5.f;
			this->mPipelinePN.updateCBV("CBMaterial", this->mDHPool, &materialParam, sizeof(materialParam), this->mCBPool);

			PNVertexPipeline::CBModel modelParam;
			float rotaOffset = t * 10.f;
			modelParam.cbWorld = math::makeRotationX<math::float4x4>(rotaOffset);
			modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationX<float4x4>(rotaOffset));
			modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationZ<float4x4>(rotaOffset));
			modelParam.cbWorld[3](2, 0, 0, 1);
			this->mPipelinePN.updateCBV("CBModel", this->mDHPool, &modelParam, sizeof(modelParam), this->mCBPool);
			this->mPipelinePN.bind(cmdList);

			this->mMeshPN.bind(cmdList);
			this->mMeshPN.draw(cmdList);
		}
		{//PTNVertex
			this->mPipelinePTN.updateCBV("CBCamera", this->mDHPool, cbCameraParam);
			this->mPipelinePTN.updateCBV("CBLight", this->mDHPool, cbLightParam);

			shader::MaterialParam materialParam;
			materialParam.diffuseColor(1, 1, 1);
			materialParam.alpha = 1;
			materialParam.specularColor(1, 1, 1);
			materialParam.specularPower = 5.f;
			this->mPipelinePTN.updateCBV("CBMaterial", this->mDHPool, &materialParam, sizeof(materialParam), this->mCBPool);
			this->mPipelinePTN.updateSRV("txTexture", this->mDHPool, this->mTexture, &this->mTexture.view().srv());
			DX12SamplerDesc sampler;
			this->mPipelinePTN.updateSampler("smSampler", this->mDHPool, &sampler);

			PTNVertexPipeline::CBModel modelParam;
			float rotaOffset = t * 10.f;
			modelParam.cbWorld = math::makeRotationX<math::float4x4>(rotaOffset);
			modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationX<float4x4>(rotaOffset));
			modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationZ<float4x4>(rotaOffset));
			modelParam.cbWorld[3](0, 0, -2, 1);
			this->mPipelinePTN.updateCBV("CBModel", this->mDHPool, &modelParam, sizeof(modelParam), this->mCBPool);
			this->mPipelinePTN.bind(cmdList);

			this->mMeshPTN.bind(cmdList);
			this->mMeshPTN.draw(cmdList);
		}
		{//PTNCVertex
			this->mPipelinePTNC.updateCBV("CBCamera", this->mDHPool, cbCameraParam);
			this->mPipelinePTNC.updateCBV("CBLight", this->mDHPool, cbLightParam);

			shader::MaterialParam materialParam;
			materialParam.diffuseColor(1, 1, 1);
			materialParam.alpha = 1;
			materialParam.specularColor(1, 1, 1);
			materialParam.specularPower = 5.f;
			this->mPipelinePTNC.updateCBV("CBMaterial", this->mDHPool, &materialParam, sizeof(materialParam), this->mCBPool);
			this->mPipelinePTNC.updateSRV("txTexture", this->mDHPool, this->mTexture, &this->mTexture.view().srv());
			DX12SamplerDesc sampler;
			this->mPipelinePTNC.updateSampler("smSampler", this->mDHPool, &sampler);

			PTNVertexPipeline::CBModel modelParam;
			float rotaOffset = t * 10.f;
			modelParam.cbWorld = math::makeRotationX<math::float4x4>(rotaOffset);
			modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationX<float4x4>(rotaOffset));
			modelParam.cbWorld = math::mul(modelParam.cbWorld, math::makeRotationZ<float4x4>(rotaOffset));
			modelParam.cbWorld[3](0, 0, 2, 1);
			this->mPipelinePTNC.updateCBV("CBModel", this->mDHPool, &modelParam, sizeof(modelParam), this->mCBPool);
			this->mPipelinePTNC.bind(cmdList);

			this->mMeshPTNC.bind(cmdList);
			this->mMeshPTNC.draw(cmdList);
		}
		cmdList.setBarriers(graphics.currentRenderTarget().makeBarrier(D3D12_RESOURCE_STATE_PRESENT));
	});

	graphics.present(1, { graphics.currentContext().cmdList() });
	graphics.endAndGoNextFrame();
	this->mCBPool.endAndGoNextFrame(graphics.cmdQueue());
	this->mDHPool.endAndGoNextFrame(graphics.cmdQueue());

	return true;
}
