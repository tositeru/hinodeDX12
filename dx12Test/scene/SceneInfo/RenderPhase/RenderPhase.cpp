#include "stdafx.h"

#include "RenderPhase.h"

#include <graphics/dx12/utility/GBuffer/GBufferManager.h>
#include <graphics/dx12/utility/ShadowMap/ShadowMapManager.h>

using namespace hinode;
using namespace hinode::graphics;

namespace scene
{
	//-----------------------------------------------------------------------------------------------------------
	//
	// scene::RenderPhase::impl
	//
	//-----------------------------------------------------------------------------------------------------------

	class RenderPhase::impl
	{
		RENDERTARGET mRT = RENDERTARGET::eBACK_BUFFER;
		int mPriority = IGNORE_PRIORITY;
		std::string mName = "";

	public://à»â∫ÅAåpè≥êÊ
		class BackBuffer;
		class GBuffer;
		class ShadowMap;

	public:
		static std::unique_ptr<impl> sNew(RENDERTARGET rendertarger);

	public:
		virtual ~impl() {}

		virtual void begin(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList) = 0;
		virtual void end(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList) = 0;

		virtual bool create(const json11::Json& json)noexcept
		{
			assert(false && "ñ¢é¿ëï");
			return true;
		}

		virtual bool validate(const json11::Json& json)noexcept
		{
			assert(false && "ñ¢é¿ëï");
			return true;
		}

		virtual bool create(const InitParam& param)noexcept
		{
			this->mName = param.name;
			this->mRT = param.rendertarget;
			this->mPriority = param.priority;
			return true;
		}

		virtual json11::Json makeJson()const
		{
			assert(false && "ñ¢é¿ëï");
			return{};
		}

		virtual void createResource(ID3D12Device* pDevice, UINT nodeMask) { }

		bool operator<(const RenderPhase& right)noexcept
		{
			return this->priority() < right.priority();
		}

		bool operator >(const RenderPhase& right)noexcept
		{
			return ! (*this < right);
		}

		bool operator <=(const RenderPhase& right)noexcept
		{
			return this->priority() <= right.priority();
		}

		bool operator >=(const RenderPhase& right)noexcept
		{
			return !(*this <= right);
		}

		bool operator ==(const RenderPhase& right)noexcept
		{
			return this->priority() == right.priority();
		}

		bool operator !=(const RenderPhase& right)noexcept
		{
			return !(*this == right);
		}

	accessor_declaration:
		RENDERTARGET rendertarget()const noexcept
		{
			return this->mRT;
		}

		int priority()const noexcept
		{
			return this->mPriority;
		}

		const std::string& name()const noexcept
		{
			return this->mName;
		}
	};

	//-----------------------------------------------------------------------------------------------------------
	//
	// scene::RenderPhase::impl::BackBuffer
	//
	//-----------------------------------------------------------------------------------------------------------

	class RenderPhase::impl::BackBuffer : public RenderPhase::impl
	{
	public:
		virtual ~BackBuffer() {}

		virtual void begin(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList)override
		{
			const float clearColor[] = { 0.0f, 0.15f, 0.2f, 1.f };
			graphics.bindCurrentRenderTargetAndDepthStencil(cmdList);
			graphics.clearCurrentRenderTarget(cmdList, clearColor);
			graphics.clearCurrentDepthStencil(cmdList, 1.f, 0);
		}

		virtual void end(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList)override
		{
		}

		virtual bool create(const json11::Json& json)noexcept override
		{
			assert(false && "ñ¢é¿ëï");
			return true;
		}

		virtual bool validate(const json11::Json& json)noexcept override
		{
			assert(false && "ñ¢é¿ëï");
			return true;
		}
	};

	//-----------------------------------------------------------------------------------------------------------
	//
	// scene::RenderPhase::impl::GBuffer
	//
	//-----------------------------------------------------------------------------------------------------------

	class RenderPhase::impl::GBuffer : public RenderPhase::impl
	{
		UINT mWidth, mHeight;
		std::unique_ptr<utility::GBufferManager> mpGBuffer;
		
	public:
		virtual ~GBuffer() {}

		virtual void begin(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList)override
		{
			this->mpGBuffer->begin(cmdList);
		}

		virtual void end(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList)override
		{
			this->mpGBuffer->end(cmdList);
		}

		virtual bool create(const InitParam& param)noexcept override
		{
			unless(this->create(param)) return false;

			this->mWidth = param.unique.gbuffer.width;
			this->mHeight = param.unique.gbuffer.height;
			return true;
		}

		virtual bool create(const json11::Json& json)noexcept override
		{
			unless(impl::validate(json)) return false;
			return true;
		}

		virtual bool validate(const json11::Json& json)noexcept override
		{
			unless(impl::validate(json)) return false;
			assert(false && "ñ¢é¿ëï");
			return true;
		}

		virtual void createResource(ID3D12Device* pDevice, UINT nodeMask)override
		{
			this->mpGBuffer = std::make_unique<utility::GBufferManager>();
			this->mpGBuffer->create(pDevice, this->mWidth, this->mHeight);
		}
	};

	//-----------------------------------------------------------------------------------------------------------
	//
	// scene::RenderPhase::impl::ShadowMap
	//
	//-----------------------------------------------------------------------------------------------------------

	class RenderPhase::impl::ShadowMap : public RenderPhase::impl
	{
		UINT mWidth, mHeight;
		DXGI_FORMAT mRTVFormat, mSRVFormat;
		std::unique_ptr<utility::ShadowMapManager> mpShadowMap;

	public:
		virtual ~ShadowMap() {}

		virtual void begin(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList)override
		{
			this->mpShadowMap->begin(cmdList);
		}

		virtual void end(utility::Graphics& graphics, DX12GraphicsCommandList& cmdList)override
		{
			this->mpShadowMap->end(cmdList);
		}

		virtual bool create(const InitParam& param)noexcept override
		{
			unless(this->create(param)) return false;

			this->mWidth = param.unique.shadowmap.width;
			this->mHeight = param.unique.shadowmap.height;
			this->mRTVFormat = param.unique.shadowmap.rtvFormat;
			this->mSRVFormat = param.unique.shadowmap.srvFormat;
			return true;
		}

		virtual bool create(const json11::Json& json)noexcept override
		{
			unless(impl::validate(json)) return false;
			return true;
		}

		virtual bool validate(const json11::Json& json)noexcept override
		{
			unless(impl::validate(json)) return false;
			assert(false && "ñ¢é¿ëï");
			return true;
		}

		virtual void createResource(ID3D12Device* pDevice, UINT nodeMask)override
		{
			this->mpShadowMap = std::make_unique<utility::ShadowMapManager>();
			this->mpShadowMap->create(pDevice, this->mWidth, this->mHeight, this->mRTVFormat, this->mSRVFormat, nodeMask);
		}
	};

	//-----------------------------------------------------------------------------------------------------------
	//
	// scene::RenderPhase
	//
	//-----------------------------------------------------------------------------------------------------------

	std::unique_ptr<RenderPhase::impl> RenderPhase::impl::sNew(RENDERTARGET rendertarget)
	{
		switch (rendertarget) {
		case RENDERTARGET::eBACK_BUFFER: return std::make_unique<impl::BackBuffer>();
		case RENDERTARGET::eGBUFFER: return std::make_unique<impl::GBuffer>();
		case RENDERTARGET::eSHADOW_MAP: return std::make_unique<impl::ShadowMap>();
		default:						assert(false && "ñ¢é¿ëï");  return nullptr;
		}
	}

	RenderPhase::RenderPhase()
	{}

	RenderPhase::~RenderPhase()
	{}

	bool RenderPhase::create(const json11::Json& json)noexcept
	{
		assert(false && "ñ¢é¿ëï");
		this->reset();
//		this->mpImpl = impl::sNew();
		return this->mpImpl->create(json);
	}

	bool RenderPhase::create(const InitParam& param)noexcept
	{
		this->reset();
		this->mpImpl = impl::sNew(param.rendertarget);
		return this->mpImpl->create(param);
	}

	json11::Json RenderPhase::makeJson()const
	{
		return this->mpImpl->makeJson();
	}

	void RenderPhase::createResource(ID3D12Device* pDevice, UINT nodeMask)
	{
		this->mpImpl->createResource(pDevice, nodeMask);
		this->setCreatedResourceFlag(true);
	}

	void RenderPhase::begin(hinode::graphics::utility::Graphics& graphics, hinode::graphics::DX12GraphicsCommandList& cmdList)
	{
		this->mpImpl->begin(graphics, cmdList);
	}

	void RenderPhase::end(hinode::graphics::utility::Graphics& graphics, hinode::graphics::DX12GraphicsCommandList& cmdList)
	{
		this->mpImpl->end(graphics, cmdList);
	}

	bool RenderPhase::operator <(const RenderPhase& right)noexcept
	{
		return *this->mpImpl < right;
	}

	bool RenderPhase::operator >(const RenderPhase& right)noexcept
	{
		return *this->mpImpl > right;
	}

	bool RenderPhase::operator <=(const RenderPhase& right)noexcept
	{
		return *this->mpImpl <= right;
	}

	bool RenderPhase::operator >=(const RenderPhase& right)noexcept
	{
		return *this->mpImpl >= right;
	}

	bool RenderPhase::operator ==(const RenderPhase& right)noexcept
	{
		return *this->mpImpl == right;
	}

	bool RenderPhase::operator !=(const RenderPhase& right)noexcept
	{
		return *this->mpImpl != right;
	}

	RenderPhase::RENDERTARGET RenderPhase::rendertarget()const noexcept
	{
		return this->mpImpl->rendertarget();
	}

	int RenderPhase::priority()const noexcept
	{
		return this->mpImpl->priority();
	}

	const std::string& RenderPhase::name()const noexcept
	{
		return this->mpImpl->name();
	}

}