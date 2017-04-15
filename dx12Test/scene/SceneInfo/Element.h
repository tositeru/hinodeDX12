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

	/// @brief scene��Ԃ̗v�f�̋��ʏ������܂Ƃ߂�����
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

		/// @brief �L���ȏ�Ԃ��H
		///
		/// ���̊֐���false��Ԃ��Ƃ��͖����I�ɑ�����s��Ȃ�����A���s�Ώۂ��疳������܂��B
		/// @retval bool
		bool isEnable()const noexcept
		{
			return this->mIsEnable;
		}

		/// @brief�@
		void setEnableFlag(bool flag)noexcept
		{
			this->mIsEnable = flag;
		}

		/// @brief ���\�[�X���쐬�����������͍쐬�r�����ǂ�����\���t���O
		///
		/// true��Ԃ��P�[�X�ł��f�[�^�̃A�b�v���[�h���̂��������Ă��Ȃ��ꍇ������܂��̂ŁA���ӂ��Ă�������
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