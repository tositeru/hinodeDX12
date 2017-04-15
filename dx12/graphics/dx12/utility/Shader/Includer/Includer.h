#pragma once

#include <d3d12.h>

#include <string>
namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			class Includer : public ID3DInclude
			{
			public:
				static const std::string& sSystemDirPath()noexcept;
				static const std::wstring& sSystemDirPathWString()noexcept;

			public:
				Includer(const std::string& localDir);
				Includer(const std::wstring& localDir);

				/// @brief 
				virtual HRESULT __stdcall Close(LPCVOID pData)override;
				virtual HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)override;

			private:
				void init(const char* localDir);

			private:
				std::string mLocalDir;
			};

		}
	}
}