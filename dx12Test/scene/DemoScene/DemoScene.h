#pragma once

#include "../IScene.h"

#include <graphics/dx12/utility/GBuffer/GBufferManager.h>
#include <graphics/dx12/utility/GBuffer/DefferedLightingManager/DefferedLightingManager.h>
#include <graphics/dx12/utility/ShadowMap/ShadowMapManager.h>

#include "SSAOManager.h"
#include "VolumetricLightManager.h"

class DemoScene : public IScene
{
public:
	DemoScene();
	~DemoScene();

	void clear()override;
	void init(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;
	bool paint(utility::Graphics& graphics, scene::SceneInfo& sceneInfo)override;

private:
	void initObjects(ID3D12Device* pDevice, hinode::graphics::utility::ResourceUploader& uploader);

private:
	utility::ConstantBufferPool mCBPool;
	utility::DescriptorHeapPoolSet mDHPool;
	utility::PreviewCamera mCamera;

	utility::GBufferManager mGBuffer;
	utility::DefferedLightingManager mDefferedLighting;
	utility::ShadowMapManager mShadowMap;

	SSAOManager mSSAO;
	VolumetricLightingManager mVolumetricLight;

	enum VERTEX_TYPE {
		eVERTEX_PTN,
		eVERTEX_PTNC,
		eVERTEX_TYPE_COUNT,
	};
	std::array<utility::PipelineSet, eVERTEX_TYPE_COUNT> mPipelinesGBuffer;
	std::array<utility::PipelineSet, eVERTEX_TYPE_COUNT> mPipelinesZPass;

	enum MESH_TYPE {
		eMESH_TETRA,
		eMESH_PLANE,
		eMESH_TYPE_COUNT,
	};
	std::array<std::array<utility::Mesh, eVERTEX_TYPE_COUNT>, eMESH_TYPE_COUNT> mMeshes;

	enum TEXTURE_TYPE {
		eTEX_PANDA,
		eTEX_TILE,
		eTEX_TYPE_COUNT,
	};
	std::array<DX12Resource, eTEX_TYPE_COUNT> mTextures;

	struct Object {
		hinode::math::float3 pos = { 0 };
		hinode::math::float3 rot = { 0 };
		hinode::math::float3 scale = { 1 };
		hinode::math::float4x4 orientation;
		shader::MaterialParam cbMaterial;
		VERTEX_TYPE vertexType = eVERTEX_PTN;
		MESH_TYPE meshType = eMESH_TETRA;
		int textureType = -1;

		Object() {
			this->cbMaterial.diffuseColor(1, 1, 1);
			this->cbMaterial.alpha = 1;
			this->cbMaterial.specularColor = { 0.3f };
			this->cbMaterial.specularPower = 50;
			math::setIdentity(this->orientation);
		}

		void updateOrientation()
		{
			auto tmpRotMat = math::makeRotationX<math::float3x3>(this->rot.x);
			tmpRotMat = math::mul(tmpRotMat, math::makeRotationY<math::float3x3>(this->rot.y));
			math::float4x4 rotMat = math::mul(tmpRotMat, math::makeRotationZ<math::float3x3>(this->rot.z));
			rotMat[3](0, 0, 0, 1);
			this->orientation = math::makeScale<math::float4x4>(this->scale);
			this->orientation = math::mul(this->orientation, rotMat);
			this->orientation[3] = math::float4(this->pos, 1);
		}
	};

	std::vector<Object> mObjects;
	size_t mDynamicObjCount;
	size_t mMoveObjCount;
	size_t mFloatFloorIndex;
};
