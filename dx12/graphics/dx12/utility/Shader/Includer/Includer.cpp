#include "stdafx.h"

#define NOMINMAX

#include "Includer.h"

#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

#include "ShaderSystemRootPath.h"

namespace hinode
{
	namespace graphics
	{
		namespace utility
		{
			const std::string& Includer::sSystemDirPath()noexcept
			{
				return gShaderSystemRootPath;
			}

			const std::wstring& Includer::sSystemDirPathWString()noexcept
			{
				return gShaderSystemRootPathW;
			}

			Includer::Includer(const std::string& localDir)
			{
				this->init(localDir.c_str());
			}

			Includer::Includer(const std::wstring& localDir)
			{
				int length = WideCharToMultiByte(CP_ACP, 0, localDir.c_str(), static_cast<int>(localDir.size()), nullptr, 0, nullptr, nullptr);
				if (length == 0) {
					auto error = GetLastError();
					int hoge = 0;
				}
				std::vector<char> buf;
				buf.resize(length+1, '\0');
				length = WideCharToMultiByte(CP_ACP, 0, localDir.c_str(), static_cast<int>(localDir.size()), buf.data(), length, nullptr, nullptr);

				this->init(buf.data());
			}

			void Includer::init(const char* localDir)
			{
				this->mLocalDir = localDir;

				std::replace(this->mLocalDir.begin(), this->mLocalDir.end(), '\\', '/');
				int tail = static_cast<int>(this->mLocalDir.size());
				bool isFilename = false;
				for (int i = static_cast<int>(this->mLocalDir.size()) - 1; 0 <= i; --i) {
					if (this->mLocalDir[i] == '.') {
						isFilename = true;
					}
					if (this->mLocalDir[i] == '/') {
						tail = std::max(0, i);
						break;
					}
				}

				if (isFilename) {
					this->mLocalDir = this->mLocalDir.substr(0, tail);
				}
			}

			HRESULT __stdcall Includer::Close(LPCVOID pData)
			{
				if (pData) {
					char* buf = (char*)pData;
					delete[] buf;
				}
				return S_OK;
			}

			HRESULT __stdcall Includer::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
			{
				try {
					std::ifstream includeFile;

					std::string filepath;
					switch (IncludeType) {
					case D3D_INCLUDE_LOCAL: {
						filepath = this->mLocalDir + "/" + pFileName;
						includeFile.open(filepath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
						if (includeFile.is_open()) {
							break;
						}
						//開くのに失敗したら、システムディレクトの方を探す
					}
					case D3D_INCLUDE_SYSTEM:
						filepath = Includer::sSystemDirPath() + "/" + pFileName;
						includeFile.open(filepath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
						break;
					}

					if (includeFile.is_open()) {
						size_t fileSize = static_cast<size_t>(includeFile.tellg());
						char* buf = new char[fileSize];
						includeFile.seekg(0, std::ios::beg);
						includeFile.read(buf, fileSize);
						includeFile.close();
						*ppData = buf;
						*pBytes = static_cast<UINT>(fileSize);
					} else {
						return E_FAIL;
					}
					return S_OK;
				} catch (std::exception&) {
					return E_FAIL;
				}
				return S_OK;
			}

		}
	}
}