#pragma once

#include "Enum/Enum.h"

namespace scene
{
	enum class ELEMENT_TYPE : size_t {
		eOBJECT,
		eMODEL,
		ePIPELINE,
		eRENDER_PHASE,
	};

	DEFINE_ENUM_CLASS(ElementType, ELEMENT_TYPE);

	/// @brief scene空間の要素の共通処理をまとめたもの
	class Element
	{
	public:
		Element()
			: mIsCreatedResource(false)
			, mIsEnable(false)
		{}

		Element(Element&& right)
			: mIsCreatedResource(right.mIsCreatedResource)
			, mIsEnable(right.mIsEnable)
		{}

		virtual ~Element() {}

		virtual const std::string& name()const noexcept = 0;

		/// @brief 有効な状態か？
		///
		/// この関数がfalseを返すときは明示的に操作を行わない限り、実行対象から無視されます。
		/// @retval bool
		bool isEnable()const noexcept
		{
			return this->mIsEnable;
		}

		/// @brief　
		void setEnableFlag(bool flag)noexcept
		{
			this->mIsEnable = flag;
		}

		/// @brief リソースを作成したもしくは作成途中かどうかを表すフラグ
		///
		/// trueを返すケースでもデータのアップロード自体が完了していない場合がありますので、注意してください
		/// @retval bool
		bool isCreatedResource()const noexcept
		{
			return this->mIsCreatedResource;
		}

	protected:
		virtual void reset()
		{
			this->mIsCreatedResource = false;
			this->mIsEnable = false;
		}

		void setCreatedResourceFlag(bool flag)noexcept
		{
			this->mIsCreatedResource = flag;
		}

	private:
		bool mIsCreatedResource = false;
		bool mIsEnable = false;
	};
}