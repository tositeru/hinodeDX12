#include "stdafx.h"

#include "Pipeline.h"
#include <graphics/dx12/common/Log.h>
#include <graphics/dx12/utility/Mesh/MeshPipeline/MeshPipeline.h>
#include "../Enum/Enum.h"

using namespace hinode;
using namespace hinode::graphics;

namespace scene
{
	//-----------------------------------------------------------------------------------------------------------
	//
	//	scene::Pipeline::impl
	//
	//-----------------------------------------------------------------------------------------------------------
	class Pipeline::impl
	{
	public:
		static std::unique_ptr<impl> sNew(InitParam::TYPE type);

	public:
		virtual ~impl() {}

		virtual bool create(const InitParam& param)noexcept = 0;
		virtual bool create(const json11::Json& json)noexcept = 0;
		virtual bool validate(const json11::Json& json)noexcept = 0;
		virtual hinode::graphics::utility::PipelineSet createPipelineSet(ID3D12Device* pDevice, UINT nodeMask = 0u) = 0;

	public: //�ȉ��A�p����̐錾 �A�N�Z�T�̃X�R�[�v���������邽�߂ɕK�v
		class SystemPipeline;

	};

	//-----------------------------------------------------------------------------------------------------------
	//
	//	scene::Pipeline::impl
	//
	//-----------------------------------------------------------------------------------------------------------
	class Pipeline::impl::SystemPipeline : public Pipeline::impl
	{
	public:
		// json�̃L�[�l�ꗗ
		static const std::string KEY_TYPE;
		static const std::string KEY_VERTEX;

		/// ������Ɨ񋓌^�̊֘A�t���̓t�@�C���̖����ɏ����Ă܂�

		DEFINE_ENUM_CLASS(Type, SYSTEM_PIPELINE_TYPE);

	private:
		Type mType;
		Vertex mVertex;

	public:
		virtual ~SystemPipeline() {}

		virtual bool create(const InitParam& param)noexcept
		{
			this->mType = param.unique.system.type;
			this->mVertex = param.unique.system.vertex;
			return true;
		}

		virtual bool create(const json11::Json& json)noexcept
		{
			unless(this->validate(json)) {
				Log(Log::eERROR) << "Pipeline::impl::SystemPipeline::create : �n���ꂽjson�f�[�^�̓��e�ɑΉ����Ă��܂���B json=" << json.dump();
				return false;
			}

			this->mType = json[KEY_TYPE].string_value();
			this->mVertex = json[KEY_VERTEX].string_value();
			return true;
		}

		virtual bool validate(const json11::Json& json)noexcept
		{
			try {
				unless(json[KEY_TYPE].is_string()) {
					Log(Log::eERROR) << KEY_TYPE << "�̒l��������ł͂���܂���";
					return false;
				}
				unless(Type::sIsExist(json[KEY_TYPE].string_value())) {
					Log(Log::eERROR) << KEY_TYPE << "�̒l���Ή����Ă��Ȃ�������ɂȂ��Ă��܂��B " << KEY_TYPE << "=" << json[KEY_TYPE].string_value();
					return false;
				}
				unless(json[KEY_VERTEX].is_string()) {
					Log(Log::eERROR) << KEY_VERTEX << "�̒l��������ł͂���܂���";
					return false;
				}
				unless(Vertex::sIsExist(json[KEY_VERTEX].string_value())) {
					Log(Log::eERROR) << KEY_VERTEX << "�̒l���Ή����Ă��Ȃ�������ɂȂ��Ă��܂��B " << KEY_VERTEX << "=" << json[KEY_VERTEX].string_value();
					return false;
				}
				return true;
			} catch (...) {
				return false;
			}
		}

		virtual hinode::graphics::utility::PipelineSet createPipelineSet(ID3D12Device* pDevice, UINT nodeMask = 0u)
		{
			switch (this->mType) {
			case SYSTEM_PIPELINE_TYPE::eFORWARD:
				switch (this->mVertex) {
				case VERTEX::eP: return utility::ForwardPipeline::sCreatePVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePC: return utility::ForwardPipeline::sCreatePCVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePN: return utility::ForwardPipeline::sCreatePNVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePT: return utility::ForwardPipeline::sCreatePTVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePTN: return utility::ForwardPipeline::sCreatePTNVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePTNC: return utility::ForwardPipeline::sCreatePTNCVertex(pDevice, nullptr, nodeMask); break;
				default:
					assert(false && "������");
				}
			case SYSTEM_PIPELINE_TYPE::eDEFFERED:
				switch (this->mVertex) {
				case VERTEX::eP: return utility::DefferedPipeline::sCreatePVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePC: return utility::DefferedPipeline::sCreatePCVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePN: return utility::DefferedPipeline::sCreatePNVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePT: return utility::DefferedPipeline::sCreatePTVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePTN: return utility::DefferedPipeline::sCreatePTNVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePTNC: return utility::DefferedPipeline::sCreatePTNCVertex(pDevice, nullptr, nodeMask); break;
				default:
					assert(false && "������");
				}
			case SYSTEM_PIPELINE_TYPE::eZPASS:
				switch (this->mVertex) {
				case VERTEX::eP: return utility::ZPassPipeline::sCreatePVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePC: return utility::ZPassPipeline::sCreatePCVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePN: return utility::ZPassPipeline::sCreatePNVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePT: return utility::ZPassPipeline::sCreatePTVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePTN: return utility::ZPassPipeline::sCreatePTNVertex(pDevice, nullptr, nodeMask); break;
				case VERTEX::ePTNC: return utility::ZPassPipeline::sCreatePTNCVertex(pDevice, nullptr, nodeMask); break;
				default:
					assert(false && "������");
				}
			default:
				assert(false && "������");
			}
			return{};
		}

	};

	// json�̃L�[�l�ꗗ
	const std::string Pipeline::impl::SystemPipeline::KEY_TYPE = "type";
	const std::string Pipeline::impl::SystemPipeline::KEY_VERTEX = "vertex";

	std::unique_ptr<Pipeline::impl> Pipeline::impl::sNew(InitParam::TYPE type)
	{
		switch (type) {
		case InitParam::eTYPE_SYSTEM: return std::make_unique<impl::SystemPipeline>();
		default:
			assert(false && "������");
			return nullptr;
		}
	}

	//-----------------------------------------------------------------------------------------------------------
	//
	//	scene::Pipeline
	//
	//-----------------------------------------------------------------------------------------------------------

	Pipeline::Pipeline()
	{

	}

	//Pipeline::Pipeline(const Pipeline& right)
	//	: mName(right.mName)
	//	, mPipelineSet(right.mPipelineSet)
	//	, mpImpl(std::make_unique<>());
	//{ }

	Pipeline::Pipeline(Pipeline&& right)
		: Element(std::move(right))
		, mName(std::move(right.mName))
		, mPipelineSet(std::move(right.mPipelineSet))
		, mpImpl(std::move(right.mpImpl))
	{}

	Pipeline::~Pipeline()
	{
	}

	Pipeline& Pipeline::operator=(Pipeline&& right)
	{
		this->mName = std::move(right.mName);
		this->mPipelineSet = std::move(right.mPipelineSet);
		this->mpImpl = std::move(right.mpImpl);
		return *this;
	}

	static const std::string KEY_SYSTEM = "system";

	bool Pipeline::create(const InitParam& param)noexcept
	{
		this->reset();
		this->mName = param.name;
		this->mpImpl = impl::sNew(param.type);
		return this->mpImpl->create(param);
	}

	bool Pipeline::create(const json11::Json& json)noexcept
	{
		this->reset();
		//json�f�[�^�̓��e���m�F����mpImpl�̎��Ԃ����肷��
		const json11::Json* pInitValue = nullptr;
		if (json[KEY_SYSTEM].is_object()) {
			pInitValue = &json[KEY_SYSTEM];
			this->mpImpl = std::make_unique<impl::SystemPipeline>();
		} else {
			Log(Log::eERROR) << "�Ή����Ă��Ȃ�JSON�f�[�^���n����܂����Bjson=" << json.dump();
			return false;
		}

		assert(nullptr != pInitValue);
		this->mName = json["name"].string_value();
		return this->mpImpl->create(*pInitValue);
	}

	json11::Json Pipeline::makeJson()const
	{
		assert(false && "������");
		return json11::Json();
	}

	void Pipeline::createPipelineSet(ID3D12Device* pDevice, UINT nodeMask)
	{
		assert(nullptr != this->mpImpl);
		this->mPipelineSet = this->mpImpl->createPipelineSet(pDevice, nodeMask);
		this->setCreatedResourceFlag(true);
		this->setEnableFlag(true);
	}

	const std::string& Pipeline::name()const noexcept
	{
		return this->mName;
	}

	hinode::graphics::utility::PipelineSet& Pipeline::pipelineSet()noexcept
	{
		return this->mPipelineSet;
	}

	const hinode::graphics::utility::PipelineSet& Pipeline::pipelineSet()const noexcept
	{
		return this->mPipelineSet;
	}

}

// ����: �L�[�l�ɂ͏��������g������
ENUM_HASH_TYPE(scene::Pipeline::SYSTEM_PIPELINE_TYPE) = {
	{ "forward", scene::Pipeline::SYSTEM_PIPELINE_TYPE::eFORWARD },
	{ "deffered", scene::Pipeline::SYSTEM_PIPELINE_TYPE::eDEFFERED },
	{ "zpass", scene::Pipeline::SYSTEM_PIPELINE_TYPE::eZPASS },
};
