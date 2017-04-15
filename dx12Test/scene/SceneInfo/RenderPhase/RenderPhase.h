#pragma once

#include <string>
#include <unordered_map>
#include <json11/json11.hpp>

#include <graphics/dx12/common/Common.h>
#include <graphics/dx12/utility/math/SimpleMath.h>
#include <graphics/dx12/graphicsCommandList/DX12GraphicsCommandList.h>
#include <graphics/dx12/utility/DX12Utility.h>

#include "../Element.h"

namespace scene
{
	/// @brief シーンでの描画単位
	class RenderPhase : public Element
	{
	public:
		enum class RENDERTARGET : size_t
		{
			eBACK_BUFFER,
			eGBUFFER,
			eSHADOW_MAP
		};
		static const int IGNORE_PRIORITY = -1; ///< この値が設定されていたら、描画時に無視される

	public:
		struct InitParam
		{
			std::string name = "";
			RENDERTARGET rendertarget = RENDERTARGET::eBACK_BUFFER;
			int priority = IGNORE_PRIORITY;

			union {
				struct {
					UINT width;
					UINT height;
				}gbuffer;
				struct {
					UINT width;
					UINT height;
					DXGI_FORMAT rtvFormat;
					DXGI_FORMAT srvFormat;
				}shadowmap;
			}unique;
		};

	public:
		RenderPhase();
		~RenderPhase();

		bool create(const json11::Json& json)noexcept;
		bool create(const InitParam& param)noexcept;
		json11::Json makeJson()const;

		void createResource(ID3D12Device* pDevice, UINT nodeMask = 0u);

		void begin(hinode::graphics::utility::Graphics& graphics, hinode::graphics::DX12GraphicsCommandList& cmdList);
		void end(hinode::graphics::utility::Graphics& graphics, hinode::graphics::DX12GraphicsCommandList& cmdList);

		bool operator <(const RenderPhase& right)noexcept;
		bool operator >(const RenderPhase& right)noexcept;
		bool operator <=(const RenderPhase& right)noexcept;
		bool operator >=(const RenderPhase& right)noexcept;
		bool operator ==(const RenderPhase& right)noexcept;
		bool operator !=(const RenderPhase& right)noexcept;

	accessor_declaration:
		const std::string& name()const noexcept override;
		RENDERTARGET rendertarget()const noexcept;
		int priority()const noexcept;

	private:
		class impl;///< パイプラインのタイプに応じて処理を切り替えるためのクラス
		std::unique_ptr<impl> mpImpl;

	};

}
