#include "stdafx.h"

#include "Model.h"

#include <graphics/dx12/utility/Mesh/Primitive/Tetrahedron/Tetrahedron.h>
#include <graphics/dx12/utility/Mesh/Primitive/Plane/Plane.h>
#include <graphics/dx12/utility/ResourceUploader/ResourceUploader.h>

#include "../JsonUtility.h"

using namespace hinode;
using namespace hinode::graphics;

namespace scene
{

	//----------------------------------------------------------------------------------------------------
	//
	//	scene::Model::impl
	//
	//----------------------------------------------------------------------------------------------------

	class Model::impl
	{
	public:
		static std::unique_ptr<impl> sNew(InitParam::TYPE type);

	public:
		virtual ~impl() {}

		virtual bool create(const InitParam& param)noexcept = 0;
		virtual bool create(const json11::Json& json)noexcept = 0;
		virtual bool validate(const json11::Json& json)noexcept = 0;
		virtual void createMesh(ID3D12Device* pDevice, hinode::graphics::utility::Mesh* pOut, utility::ResourceUploader& uploader) = 0;
		virtual void createTexture(ID3D12Device* pDevice, std::unordered_map<std::string, DX12Resource>* pOutTexHash, utility::ResourceUploader& uploader) = 0;
		virtual const shader::MaterialParam& getMaterialParam()const noexcept = 0;
		virtual const std::string& getTextureKey(SYSTEM_MESH_TEXTURE type)const noexcept = 0;

	public: //以下、継承先の宣言 アクセサのスコープを解決するために必要
		class SystemMesh;
	};

	//----------------------------------------------------------------------------------------------------
	//
	//	scene::Model::impl::SystemMesh
	//
	//----------------------------------------------------------------------------------------------------

	class Model::impl::SystemMesh : public Model::impl
	{
	public:
		// jsonのキー値一覧
		static const std::string KEY_PRIMITIVE;
		static const std::string KEY_VERTEX;
		static const std::string KEY_MATERIAL;

		enum MATERIAL_MEMBER {
			eMATERIAL_DIFFUSE_COLOR,
			eMATERIAL_ALPHA,
			eMATERIAL_SPECULER_COLOR,
			eMATERIAL_SPECULER_POWER,
			eMATERIAL_MEMBER_COUNT,
		};
		static const std::array<std::string, eMATERIAL_MEMBER_COUNT> materialMembers;

		/// 文字列と列挙型の関連付けはファイルの末尾に書いてます

		DEFINE_ENUM_CLASS(Primitive, SYSTEM_MESH_PRIMITIVE);
		DEFINE_ENUM_CLASS(Texture, SYSTEM_MESH_TEXTURE);

	private:
		Primitive mPrimitive;
		Vertex mVertex;
		shader::MaterialParam mMaterialParam;
		std::unordered_map<SYSTEM_MESH_TEXTURE, std::string> mTexHash;

	public:
		virtual ~SystemMesh() {}

		virtual bool create(const InitParam& param)noexcept override
		{
			this->mPrimitive = param.system.primitive;
			this->mVertex = param.system.vertex;
			this->mMaterialParam = param.system.material;
			for (auto i = 0u; i < param.system.textures.size(); ++i) {
				if (param.system.textures[i].empty())
					continue;

				this->mTexHash[static_cast<SYSTEM_MESH_TEXTURE>(i)] = param.system.textures[i];
			}
			return true;
		}

		virtual bool create(const json11::Json& json)noexcept override
		{
			unless(this->validate(json)) {
				Log(Log::eERROR) << "Pipeline::impl::SystemMesh::create : 渡されたjsonデータの内容に対応していません。 json=" << json.dump();
				return false;
			}

			this->mPrimitive = json[KEY_PRIMITIVE].string_value();
			this->mVertex = json[KEY_VERTEX].string_value();

			{// マテリアルの読み込み
				auto& material = json[KEY_MATERIAL];
				this->mMaterialParam.diffuseColor = toFloat3(material[materialMembers[eMATERIAL_DIFFUSE_COLOR]], {1, 1, 1});
				this->mMaterialParam.alpha = toNumber(material[materialMembers[eMATERIAL_ALPHA]], 1.f);
				this->mMaterialParam.specularColor = toFloat3(material[materialMembers[eMATERIAL_SPECULER_COLOR]], { 1, 1, 1 });
				this->mMaterialParam.specularPower = toNumber(material[materialMembers[eMATERIAL_SPECULER_POWER]], 10.f);

				for (auto it : Texture::sHash) {
					unless(material[it.first].is_string()) {
						continue;
					}
					this->mTexHash.insert({ it.second, material[it.first].string_value() });
				}
			}

			return true;
		}

		virtual bool validate(const json11::Json& json)noexcept override
		{
			try {
				unless(json[KEY_PRIMITIVE].is_string()) {
					Log(Log::eERROR) << KEY_PRIMITIVE << "の値が文字列ではありません";
					return false;
				}
				unless(Primitive::sIsExist(json[KEY_PRIMITIVE].string_value())) {
					Log(Log::eERROR) << KEY_PRIMITIVE << "の値が対応していない文字列になっています。 " << KEY_PRIMITIVE  << "=" << json[KEY_PRIMITIVE].string_value();
					return false;
				}
				unless(json[KEY_VERTEX].is_string()) {
					Log(Log::eERROR) << KEY_VERTEX << "の値が文字列ではありません";
					return false;
				}
				unless(Vertex::sIsExist(json[KEY_VERTEX].string_value())) {
					Log(Log::eERROR) << KEY_VERTEX << "の値が対応していない文字列になっています。 " << KEY_VERTEX << "=" << json[KEY_VERTEX].string_value();
					return false;
				}
				unless(json[KEY_MATERIAL].is_object()) {
					Log(Log::eERROR) << KEY_MATERIAL << "の値がオブジェクトではありません。";
					return false;
				}
				return true;
			} catch (...) {
				return false;
			}
		}

		virtual void createMesh(ID3D12Device* pDevice, hinode::graphics::utility::Mesh* pOut, utility::ResourceUploader& uploader)override
		{
			switch (this->mPrimitive) {
			case SYSTEM_MESH_PRIMITIVE::eTETRAHEDRON:
				switch (this->mVertex) {
				case VERTEX::eP:		return utility::TetrahedronCreator::sCreateMesh<utility::PVertex>(pOut, pDevice, uploader);
				case VERTEX::ePC:		return utility::TetrahedronCreator::sCreateMesh<utility::PCVertex>(pOut, pDevice, uploader);
				case VERTEX::ePN:		return utility::TetrahedronCreator::sCreateMesh<utility::PNVertex>(pOut, pDevice, uploader);
				case VERTEX::ePT:		return utility::TetrahedronCreator::sCreateMesh<utility::PTVertex>(pOut, pDevice, uploader);
				case VERTEX::ePTN:		return utility::TetrahedronCreator::sCreateMesh<utility::PTNVertex>(pOut, pDevice, uploader);
				case VERTEX::ePTNC:		return utility::TetrahedronCreator::sCreateMesh<utility::PTNCVertex>(pOut, pDevice, uploader);
				default:
					assert(false && "未実装");
				}
			case SYSTEM_MESH_PRIMITIVE::ePLANE:
				switch (this->mVertex) {
				case VERTEX::eP:		return utility::PlaneCreator::sCreateMesh<utility::PVertex>(pOut, pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);
				case VERTEX::ePC:		return utility::PlaneCreator::sCreateMesh<utility::PCVertex>(pOut, pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);
				case VERTEX::ePN:		return utility::PlaneCreator::sCreateMesh<utility::PNVertex>(pOut, pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);
				case VERTEX::ePT:		return utility::PlaneCreator::sCreateMesh<utility::PTVertex>(pOut, pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);
				case VERTEX::ePTN:		return utility::PlaneCreator::sCreateMesh<utility::PTNVertex>(pOut, pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);
				case VERTEX::ePTNC:		return utility::PlaneCreator::sCreateMesh<utility::PTNCVertex>(pOut, pDevice, uploader, utility::PlaneCreator::ePLANE_XZ);
				default:
					assert(false && "未実装");
				}
			}
		}

		virtual void createTexture(ID3D12Device* pDevice, std::unordered_map<std::string, DX12Resource>* pOutTexHash, utility::ResourceUploader& uploader)override
		{
			// TODO 気が向いたら、実行時のテクスチャ作成にも対応する

			for (auto it : this->mTexHash) {
				if (pOutTexHash->end() != pOutTexHash->find(it.second)) {
					//同名のキーがあるなら無視する
					continue;
				}

				try {
					auto& tex = (*pOutTexHash)[it.second];
					auto pos = it.second.find(".dds");
					if (std::string::npos != pos) {
						uploader.loadTextureFromDDS(pDevice, tex, it.second);
					} else {
						uploader.loadTextureFromWICFile(pDevice, tex, it.second);
					}
					//TODO ビュー作成周りをまとめる　JSONで指定する形になる？
					tex.appendView(DX12Resource::View().setSRV(tex.makeSRV()));
				} catch (Exception& e) {
					e.writeLog();
					Log(Log::eERROR) << "テクスチャの読み込みに失敗しました。 filepath=" << it.second;
				}
			}
		}

		virtual const shader::MaterialParam& getMaterialParam()const noexcept override
		{
			return this->mMaterialParam;
		}

		virtual const std::string& getTextureKey(SYSTEM_MESH_TEXTURE type)const noexcept override
		{
			auto it = this->mTexHash.find(type);
			if (it != this->mTexHash.end()) {
				return it->second;
			} else {
				static const std::string empty = "";
				return empty;
			}
		}
	};

	// jsonのキー値一覧
	const std::string Model::impl::SystemMesh::KEY_PRIMITIVE = "prm";
	const std::string Model::impl::SystemMesh::KEY_VERTEX = "vertex";
	const std::string Model::impl::SystemMesh::KEY_MATERIAL = "mtl";

	const std::array<std::string, Model::impl::SystemMesh::eMATERIAL_MEMBER_COUNT> Model::impl::SystemMesh::materialMembers = { {
			"dif",
			"alp",
			"spc",
			"pow",
		} };

	std::unique_ptr<Model::impl> Model::impl::sNew(InitParam::TYPE type)
	{
		switch (type) {
		case InitParam::eTYPE_SYSTEM: return std::make_unique<impl::SystemMesh>();
		default:
			assert(false && "未実装");
			return nullptr;
		}
	}

	//----------------------------------------------------------------------------------------------------
	//
	//	scene::Model
	//
	//----------------------------------------------------------------------------------------------------

	Model::Model()
	{ }

	Model::Model(Model&& right)
		: Element(std::move(right))
		, mName(right.mName)
		, mMesh(std::move(right.mMesh))
		, mMaterialParam(std::move(right.mMaterialParam))
		, mpImpl(std::move(right.mpImpl))
	{}

	Model::~Model()
	{ }

	Model& Model::operator=(Model&& right)
	{
		this->mName = right.mName;
		this->mMesh = std::move(right.mMesh);
		this->mMaterialParam = std::move(right.mMaterialParam);
		this->mpImpl = std::move(right.mpImpl);
		return *this;
	}

	bool Model::create(const InitParam& param)noexcept
	{
		this->reset();
		this->mpImpl = impl::sNew(param.type);
		return this->mpImpl->create(param);
	}

	static const std::string KEY_SYSTEM = "system";

	bool Model::create(const json11::Json& json)noexcept
	{
		this->reset();

		//jsonデータの内容を確認してmpImplの実態を決定する
		const json11::Json* pInitValue = nullptr;
		if (json[KEY_SYSTEM].is_object()) {
			pInitValue = &json[KEY_SYSTEM];
			this->mpImpl = std::make_unique<impl::SystemMesh>();
		} else {
			Log(Log::eERROR) << "対応していないJSONデータが渡されました。json=" << json.dump();
			return false;
		}

		assert(nullptr != pInitValue);
		this->mName = json["name"].string_value();
		return this->mpImpl->create(*pInitValue);
	}

	json11::Json Model::makeJson()const
	{
		assert(false && "未実装");
		return json11::Json();
	}

	void Model::createResource(ID3D12Device* pDevice, std::unordered_map<std::string, hinode::graphics::DX12Resource>* pOutTexHash, utility::ResourceUploader& uploader)
	{
		assert(nullptr != this->mpImpl);
		this->mpImpl->createMesh(pDevice, &this->mMesh, uploader);
		this->mpImpl->createTexture(pDevice, pOutTexHash, uploader);
		this->setCreatedResourceFlag(true);
	}

	const std::string& Model::name()const noexcept
	{
		return this->mName;
	}

	hinode::graphics::utility::Mesh& Model::mesh()noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mMesh;
	}

	const hinode::graphics::utility::Mesh& Model::mesh()const noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mMesh;
	}

	const shader::MaterialParam& Model::materialParam()const noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->getMaterialParam();
	}

	const std::string& Model::diffuseTexKey()const noexcept
	{
		assert(nullptr != this->mpImpl);
		return this->mpImpl->getTextureKey(SYSTEM_MESH_TEXTURE::eDIFFUSE);
	}
}

ENUM_HASH_TYPE(scene::Model::SYSTEM_MESH_PRIMITIVE) = {
	{"plane", scene::Model::SYSTEM_MESH_PRIMITIVE::ePLANE},
	{"tetra", scene::Model::SYSTEM_MESH_PRIMITIVE::eTETRAHEDRON},
};

ENUM_HASH_TYPE(scene::Model::SYSTEM_MESH_TEXTURE) = {
	{"dif_tex", scene::Model::SYSTEM_MESH_TEXTURE::eDIFFUSE },
};

