#include "stdafx.h"

#include "Log.h"

#include <cassert>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>

#include <Windows.h>

namespace hinode
{
    namespace graphics
    {
        std::shared_ptr<std::ofstream> Log::spLogStream = nullptr;

        void Log::sStandbyLogFile(const char *logFilePath)_noexcept
        {
            Log::spLogStream.reset();
            Log::spLogStream = std::make_shared<std::ofstream>(logFilePath, std::ios::binary);
            assert(Log::spLogStream->good());
        }

        void Log::sStandbyLogFile(std::shared_ptr<std::ofstream> &pUsedOutputStream)_noexcept
        {
            Log::spLogStream.reset();
            Log::spLogStream = pUsedOutputStream;
        }

        std::shared_ptr<std::ofstream> Log::sGetLogStream()_noexcept
        {
            return Log::spLogStream;
        }

		std::stringstream Log::sMakeErrorMessagePrefix(const char* fileName, int lineNo, const char* className, const char* funcName)
		{
			std::stringstream s;
			s <<
				fileName << "(" << lineNo << ") : " <<
				className << "::" << funcName << " : ";
			return s;
		}

        Log::Log(Log::TYPE type)_noexcept
            : Log(type, "", 0, "", "")
        {}

        Log::Log(Log::TYPE type, const char *fileName, int lineNo, const char *className, const char *funcName)_noexcept
            : mType(type)
            , mFileName(fileName)
            , mLine(lineNo)
            , mClassName(className)
            , mFuncName(funcName)
            , mIsAutoWroteLog(true)
        {}

		Log::Log(Log&& log)
			: mType(log.mType)
			, mFileName(log.mFileName)
			, mLine(log.mLine)
			, mClassName(log.mClassName)
			, mFuncName(log.mFuncName)
			, mIsAutoWroteLog(log.mIsAutoWroteLog)
		{
			this->mMessage = std::move(log.mMessage);
			log.mIsAutoWroteLog = false;
		}

		Log& Log::operator=(Log&& right)noexcept
		{
			this->mType = right.mType;
			this->mFileName = right.mFileName;
			this->mLine = right.mLine;
			this->mClassName = right.mClassName;
			this->mFuncName = right.mFuncName;
			this->mIsAutoWroteLog = right.mIsAutoWroteLog;
			this->mMessage = std::move(right.mMessage);
			right.mIsAutoWroteLog = false;
			return *this;
		}

        Log::~Log()_noexcept
        {
            if(this->mIsAutoWroteLog){
                this->write();
            }
        }

        void Log::clear()_noexcept
        {
            this->mMessage.clear();
        }

        void Log::write()_noexcept
        {
            switch(this->mType) {
            case eERROR: writeError(); break;
            case eINFO: writeInfo(); break;
            case eMEMO: writeMemo(); break;
            default:
                assert(false && "実装していない");
            }
            this->setAutoWrote(false);
        }

        void Log::setAutoWrote(bool enable)_noexcept
        {
            this->mIsAutoWroteLog = enable;
        }

        void Log::setType(Log::TYPE type)_noexcept
        {
            this->mType = type;
        }

        void Log::setFileName(const char *name)_noexcept
        {
            this->mFileName = name;
        }

        void Log::setLineNo(int lineNo)_noexcept
        {
            this->mLine = lineNo;
        }

        void Log::setClassName(const char *name)_noexcept
        {
            this->mClassName = name;
        }

        void Log::setFuncName(const char *name)_noexcept
        {
            this->mFuncName = name;
        }

        void Log::writeError()_noexcept
        {
            assert(Log::spLogStream != nullptr && "hinode::graphics::standByLogFile関数を呼んでください");

        #ifdef _DEBUG
            std::stringstream strStream = sMakeErrorMessagePrefix(this->mFileName.c_str(), this->mLine, this->mClassName.c_str(), this->mFuncName.c_str());
			strStream << this->mMessage.str() << std::endl;

            std::string msg = strStream.str();
			std::cout << msg.c_str();
			*Log::spLogStream << msg;

			OutputDebugStringA(msg.c_str());

        #else
            std::string msg = this->mMessage.str();
            *Log::spLogStream << msg;
		#endif
			DebugBreak();
		}

        void Log::writeInfo()_noexcept
        {
            assert(Log::spLogStream != nullptr && "hinode::graphics::standByLogFile関数を呼んでください");

            std::stringstream strStream;
            strStream << this->mMessage.str() << std::endl;
			auto msg = strStream.str();
#ifdef _DEBUG
            std::cout << msg.c_str();
			OutputDebugStringA(msg.c_str());
#endif
            *Log::spLogStream << strStream.str();
        }

        void Log::writeMemo()_noexcept
        {
        #ifdef _DEBUG
            assert(Log::spLogStream != nullptr);
            std::stringstream strStream;
            strStream << this->mMessage.str();
            std::cout << strStream.str().c_str() << std::endl;
        #endif
        }
    }
}
