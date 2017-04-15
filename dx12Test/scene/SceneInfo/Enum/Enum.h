#pragma once

#include <string>
#include <unordered_map>
#include <algorithm>

/// @brief �e���v���[�g�����ɓn���ꂽ�񋓌^�𕶎�����֘A�t���邽�߂̃N���X
template<class EnumType, size_t defaultValue=0> class Enum {
public:
	static EnumType sToValue(std::string keyward)
	{
		std::transform(keyward.begin(), keyward.end(), keyward.begin(), tolower);//�������ɕϊ�
		auto it = sHash.find(keyward);
		if (sHash.end() == it) {
			throw std::invalid_argument("�Ή����Ă��Ȃ��L�[���[�h���n����܂���. keyward=" + keyward);
		}
		return it->second;
	}

	static const std::string& sToStr(EnumType value)
	{
		for (auto& it : sHash) {
			if (it.second == value) {
				return it.first;
			}
		}
		throw std::invalid_argument("�Ή����Ă��Ȃ��L�[���[�h�l���n����܂���. value=" + std::to_string(static_cast<size_t>(value)));
	}

	static bool sIsExist(std::string keyward)
	{
		std::transform(keyward.begin(), keyward.end(), keyward.begin(), tolower);//�������ɕϊ�
		return sHash.end() != sHash.find(keyward);
	}

	static const std::unordered_map<std::string, EnumType> sHash;

public:
	Enum()
		: mValue(static_cast<EnumType>(defaultValue))
	{}
	Enum(const EnumType& value)
		: mValue(value)
	{}
	Enum(const std::string& keyward)
		: mValue(sToValue(keyward))
	{}

	EnumType value()const noexcept { return this->mValue; }
	const std::string& str()const noexcept { return sToStr(this->value()); }

	Enum& operator=(const Enum& right) {
		this->mValue = right.mValue;
		return *this;
	}

	Enum& operator=(const std::string& right)
	{
		this->mValue = sToValue(right);
		return *this;
	}

	Enum& operator=(const EnumType& right)
	{
		this->mValue = right;
		return *this;
	}

	operator EnumType()const
	{
		return mValue;
	}

private:
	EnumType mValue;
};

/// @brief Enum�̔h���N���X�̊ȈՒ�`���s���}�N��
#define DEFINE_ENUM_CLASS(className, enumType) \
class className : public Enum<enumType>{ \
public: \
	className() : Enum<enumType>() {} \
	className(const enumType& value) : Enum<enumType>(value) {} \
	className(const std::string& keyward) : Enum<enumType>(sToValue(keyward)) {} \
};

/// @brief Enum���N���X���Ő錾�������ɕK�v�ƂȂ郁���o���`����}�N��
/// �A�N�Z�X�X�R�[�v�Ɉ������������Ƃ��Ɏg�p�g�p���Ă�������
#define DEFINE_REQUIRE_MEMBER_OF_ENUM_IN_CLASS(className, enumType) \
public: \
	className() : mEnum() {} \
	className(const enumType& value) : mEnum(value) {} \
	className(const std::string& keyward) : mEnum(toValue(keyward)) {} \
	Enum<enumType>* operator->()noexcept{return &this->mEnum;}\
	const Enum<enumType>* operator->()const noexcept{return &this->mEnum;}\
private: \
	Enum<enumType> mEnum;

/// @brief Enum�̃n�b�V��������������Ƃ��Ɏg���Ă�������
#define ENUM_HASH_TYPE(enumType) template<> const std::unordered_map<std::string, enumType> Enum<enumType>::sHash
