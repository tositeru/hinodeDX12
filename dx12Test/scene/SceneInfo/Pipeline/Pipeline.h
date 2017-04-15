#pragma once

#include <json11/json11.hpp>

#include <graphics/dx12/utility/Shader/PipelineSet/PipelineSet.h>

#include "../Element.h"
#include "../Enum/VertexEnum.h"

namespace scene
{
	/// @brief シーン内で使用するパイプラインを表すクラス
	class Pipeline : public Element
	{
	public:
		enum class SYSTEM_PIPELINE_TYPE : size_t {
			eFORWARD,
			eDEFFERED,
			eZPASS,
		};

		struct InitParam
		{
			enum TYPE {
				eTYPE_SYSTEM,
			};

			std::string name = "";
			TYPE type = eTYPE_SYSTEM;
			union {
				struct {
					SYSTEM_PIPELINE_TYPE type;
					VERTEX vertex;
				}system;
			}unique;
		};

	public:
		Pipeline();
		//Pipeline(const Pipeline& right);
		Pipeline(Pipeline&& right);
		~Pipeline();
		//Pipeline& operator=(const Pipeline& right);
		Pipeline& operator=(Pipeline&& right);

		bool create(const InitParam& param)noexcept;
		bool create(const json11::Json& json)noexcept;
		json11::Json makeJson()const;

		void createPipelineSet(ID3D12Device* pDevice, UINT nodeMask = 0u);

	accessor_declaration:
		const std::string& name()const noexcept override;
		hinode::graphics::utility::PipelineSet& pipelineSet()noexcept;
		const hinode::graphics::utility::PipelineSet& pipelineSet()const noexcept;

	private:
		std::string mName;
		hinode::graphics::utility::PipelineSet mPipelineSet;

		class impl;///< パイプラインのタイプに応じて処理を切り替えるためのクラス
		std::unique_ptr<impl> mpImpl;
	};

}
