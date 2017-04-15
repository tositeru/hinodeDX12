#pragma once
#ifndef _HINODE_GRAPHICS_COMMON_
#define _HINODE_GRAPHICS_COMMON_

#include <functional>
#include <string>
#include <cstring>
#include <algorithm>

#include "MacroUtility.h"
#include "Pimpl.h"
//#include "NotCopyPimpl.hpp"
#include "TypeDefine.h"
#include "Log.h"
#include "Exception.h"

namespace hinode
{
    namespace graphics
	{
		//
		//	finally構文を実現するためのクラス
		//
		class Finally
		{
			std::function< void() > f;
		public:
			explicit Finally(std::function< void() > f)
				: f(f) { }

			Finally(Finally const &) = delete;
			void operator = (Finally const &) = delete;

			~Finally()
			{
				f();
			}
		};

		///	@brief memsetのテンプレート版
		/// @tparam T
		/// @param[in] dest
		/// @param[in] val
		template<typename T> void setMemory(T* dest, int val)
		{
			memset(dest, val, sizeof(T));
		}

		template<typename T> void safeDelete(T** p){
			if (*p) { delete (*p); *p = nullptr; }
		}

		template<typename T> void safeArrayDelete(T** p){
			if (*p) { delete[] (*p); *p = nullptr; }
		}

		/// @brief ポインタのキャストを行う
		/// @tparam T
		/// @tparam U
		/// @param[in] p
		/// @retval const T*
		template<typename T, typename U> T* pointerCast(U* p){
			return static_cast<T*>(static_cast<void*>(p));
		}
	
		/// @brief ポインタのキャストを行う(const版)
		/// @tparam T
		/// @tparam U
		/// @param[in] p
		/// @retval const T*
		template<typename T, typename U> const T* pointerCast(const U* p){
			return static_cast<const T*>(static_cast<const void*>(p));
		}

		/// @brief 安全にRelease()を呼び出すための関数
		/// @param[in] p
		template<typename T> void safeRelease(T** p){
			if (*p) { (*p)->Release(); *p = nullptr; }
		}

		/// @brief ファイルパスからディレクトリの部分を取り出す
		/// @tparam T std::stringとchar*どちらでも対応できるようにするため使用
		/// @param[in] filepath
		/// @retval std::string
		template<typename T>
		std::string makePath(const T& filepath)
		{
			std::string path = filepath;
			std::replace_if(path.begin(), path.end(), [](char& c) { return c == '\\'; }, '/');
			auto pos = path.find_last_of('/');
			unless(std::string::npos == pos) {
				path = path.substr(0, pos + 1);
			}
			return path;
		}

		/// @brief ファイルのサイズを返す
		/// @param[in] fp
		/// @retval int
		inline int getFilesize(FILE* fp)_noexcept{
			fseek(fp, 0, SEEK_END);
			int size = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			return size;
		}

		/// @brief 配列の要素数を返す
		/// @tparam T
		/// @tparam size
		/// @param[in] const T(&)[size]
		/// @retval size_t
		template<typename T, size_t size>
		inline size_t arrayLength(const T(&)[size]) {
			return size;
		}

		/// @brief マルチバイト文字列からワイド文字列に変換する
		/// @param[in] pMBStr
		/// @retval std::wstring
		inline std::wstring toWideString(const char* pMBStr)
		{
			std::wstring result;
			size_t length;
			auto e = mbstowcs_s(&length, nullptr, 0, pMBStr, _TRUNCATE);
			unless(0 == e) {
				throw HINODE_GRAPHICS_CREATE_EXCEPTION(RunTimeErrorException, "ResourceUplaoder", "loadTextureFromWICFile") << "指定されたファイルを開くことに失敗しました。 e=" << e;
			}
			result.resize(length);
			e = mbstowcs_s(&length, &result[0], result.size(), pMBStr, _TRUNCATE);
			return result;
		}

		/// @brief マルチバイト文字列からワイド文字列に変換する
		/// @param[in] pMBStr
		/// @retval std::wstring
		inline std::wstring toWideString(const std::string& pMBStr)
		{
			return toWideString(pMBStr.c_str());
		}
	}
}

//開発環境がQt MinGW32の時
#ifdef __MINGW32__
#ifndef _DEFINE_MAKE_UNIQUE
#define _DEFINE_MAKE_UNIQUE
#include <memory>
namespace std
{
    template<typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) {
        return unique_ptr<T>(new T(forward<Args>(args)...));
    }
}
#endif

#endif

#endif
