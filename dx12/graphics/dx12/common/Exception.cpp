#include "stdafx.h"

#include "Exception.h"

#include "Log.h"
#include <Windows.h>

namespace hinode
{
	namespace graphics
	{
        std::function<void(const Exception*)> Exception::sWriteLog = [](const Exception* pException) {
			Log(Log::eERROR,
				pException->filename().c_str(),
				pException->line(),
				pException->className().c_str(),
				pException->funcName().c_str())
				<< pException->message();
		};

		void Exception::sSetWroteLogFunc(std::function<void(const Exception*)> f)
		{
			Exception::sWriteLog = f;
		}

		Exception::Exception(
			const char* filename,
			int line,
			const char* className,
			const char* funcName,
            Exception* pPrevException)
			: mIsAutoWroteLog(true)
			, mFileName(filename)
			, mLine(line)
			, mClassName(className)
			, mFuncName(funcName)
		{
            if (pPrevException) {
				pPrevException->writeLog();
			}
		}

        Exception::Exception(Exception& right)
            : mFileName(right.mFileName)
            , mLine(right.mLine)
            , mClassName(right.mClassName)
            , mFuncName(right.mFuncName)
        {
            mMessage << right.mMessage.str();
            right.mIsAutoWroteLog = false;
        }

        Exception::Exception(Exception&& right)
            : mFileName(right.mFileName)
            , mLine(right.mLine)
            , mClassName(right.mClassName)
            , mFuncName(right.mFuncName)
		{
            mMessage << right.mMessage.str();
			right.mIsAutoWroteLog = false;
		}

		Exception::~Exception()
		{
			if( this->mIsAutoWroteLog ) {
				this->writeLog();
			}
		}

		void Exception::writeLog()
		{
			sWriteLog(this);
			this->offAutoWroteLog();
		}

		void Exception::onAutoWroteLog() { this->mIsAutoWroteLog = true; }
		void Exception::offAutoWroteLog() { this->mIsAutoWroteLog = false; }

		std::stringstream& Exception::stream() {return this->mMessage; }
		const std::stringstream& Exception::stream()const { return this->mMessage; }

		const std::string Exception::message()const { return this->mMessage.str(); }
		const std::string& Exception::filename()const { return this->mFileName; }
		int Exception::line()const { return this->mLine; }
		const std::string& Exception::className()const { return this->mClassName; }
		const std::string& Exception::funcName()const { return this->mFuncName; }
	}
}
