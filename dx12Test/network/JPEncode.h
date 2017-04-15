#pragma once

#include <string>
#include <vector>
#include <codecvt>

/// @brief Shift-JISからUTF-8への変換をまとめたもの
///  参考サイト:http://nekko1119.hatenablog.com/entry/2017/01/02/054629
///
namespace jp_encode
{
	/// @brief 日本のロケール地を取得する
	/// @retval _local_t
	inline _locale_t getJPLocale()
	{
		return ::_create_locale(LC_ALL, "jpn");
	}

	/// @brief Shift-JISからワイド文字列に変換する
	/// @param[in] src
	/// @retval std::wstring
	inline std::wstring multi_to_wide_capi(const std::string& src)
	{
		std::size_t converted{};
		std::vector<wchar_t> dest(src.size(), L'\0');
		if (0 != ::_mbstowcs_s_l(&converted, dest.data(), dest.size(), src.data(), _TRUNCATE, getJPLocale())){
			throw std::system_error{errno, std::system_category()};
		}
		dest.resize(converted);
		return std::wstring(dest.begin(), dest.end());
	}

	/// @brief ワイド文字列からShift-JISに変換する
	/// @param src
	/// @retval std::string
	inline std::string wide_to_multi_capi(const std::wstring& src)
	{
		std::size_t converted{};
		std::vector<char> dest(src.size(), '\0');
		if (0 != ::_wcstombs_s_l(&converted, dest.data(), dest.size(), src.data(), _TRUNCATE, getJPLocale())) {
			throw std::system_error{ errno, std::system_category() };
		}
		dest.resize(converted);
		return std::string(dest.begin(), dest.end());
	}

	/// @brief UTF-8文字列からワイド文字列に変換する
	/// @param[in] src
	/// @retval std::wstring
	inline std::wstring utf8_to_wide_cppapi(const std::string& src)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(src);
	}

	/// @brief ワイド文字列からUTF-8文字列に変換する
	/// @param[in] src
	/// @retval std::string
	inline std::string wide_to_utf8_appapi(const std::wstring& src)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(src);
	}

	/// @brief Shift-JISからUTF-8文字列へ変換する
	/// @param[in] src
	/// @retval std::string
	inline std::string multi_to_utf8_cppapi(const std::string& src)
	{
		const auto wide = multi_to_wide_capi(src);
		return wide_to_utf8_appapi(wide);
	}

	/// @brief UTF-8文字列からShift-JISへ変換する
	/// @parma[in] src
	/// @retval std::string
	inline std::string utf8_to_multi_cppapi(const std::string& src)
	{
		const auto wide = utf8_to_wide_cppapi(src);
		return wide_to_multi_capi(wide);
	}

}
