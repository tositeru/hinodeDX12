#pragma once

#include <unordered_map>
#include <json11/json11.hpp>

#include <graphics/dx12/utility/Mesh/Mesh.h>
#include <graphics/dx12/utility/Mesh/MeshPipeline/MeshPipeline.h>

#include "../Element.h"
#include "../Enum/VertexEnum.h"

namespace scene
{
	class Model : public Element
	{
	public:
		enum class SYSTEM_MESH_PRIMITIVE : size_t {
			ePLANE,
			eTETRAHEDRON,
		};

		enum class SYSTEM_MESH_TEXTURE : size_t {
			eDIFFUSE,
			COUNT,
		};

		struct InitParam
		{
			enum TYPE {
				eTYPE_SYSTEM,
			};

			std::string name;
			TYPE type;

			struct {
				SYSTEM_MESH_PRIMITIVE primitive;
				VERTEX vertex;
				shader::MaterialParam material;
				std::array<std::string, static_cast<size_t>(SYSTEM_MESH_TEXTURE::COUNT)> textures;
			}system;
		};

	public:
		Model();
		Model(Model&& right);
		~Model();
		Model& operator=(Model&& right);

		bool create(const InitParam& param)noexcept;
		bool create(const json11::Json& json)noexcept;
		json11::Json makeJson()const;

		void createResource(ID3D12Device* pDevice, std::unordered_map<std::string, hinode::graphics::DX12Resource>* pOutTexHash, hinode::graphics::utility::ResourceUploader& uploader);

	accessor_declaration:
		const std::string& name()const noexcept override;
		hinode::graphics::utility::Mesh& mesh()noexcept;
		const hinode::graphics::utility::Mesh& mesh()const noexcept;

		const shader::MaterialParam& materialParam()const noexcept;
		const std::string& diffuseTexKey()const noexcept;

	private:
		std::string mName;
		hinode::graphics::utility::Mesh mMesh;
		shader::MaterialParam mMaterialParam;

		class impl;///< パイプラインのタイプに応じて処理を切り替えるためのクラス
		std::unique_ptr<impl> mpImpl;
	};
}