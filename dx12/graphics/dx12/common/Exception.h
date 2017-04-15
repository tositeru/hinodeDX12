#pragma once

#include <sstream>
#include <functional>

namespace hinode
{
	namespace graphics
	{
		class Exception
		{
		public:
			static void sSetWroteLogFunc(std::function<void(const Exception*)> f);
		private:
			static std::function<void(const Exception*)> sWriteLog;

		public:
			explicit Exception(const char* filename, int line, const char* className, const char* funcName, Exception* pPrevException=nullptr);
            Exception(Exception& right);
            Exception(Exception&& right);
			~Exception();

			void writeLog();

			template<typename T>
			Exception& operator<<(T v)
			{
				this->mMessage << v;
				return *this;
			}

			void onAutoWroteLog();
			void offAutoWroteLog();

			std::stringstream& stream();
			const std::stringstream& stream()const;

			const std::string& filename()const;
			int line()const;
			const std::string& className()const;
			const std::string& funcName()const;
			const std::string message()const;

		private:
			std::string mFileName;
			int mLine;
			std::string mClassName;
			std::string mFuncName;
			std::stringstream mMessage;
			bool mIsAutoWroteLog;
        };

        //Exceptionを継承した例外クラスを定義するマクロ
#define DEFINE_HINODE_GRAPHICS_EXCEPTION(exceptionName) \
		class exceptionName : public Exception { \
		public: \
			explicit exceptionName(const char* filename, int line, const char* className, const char* funcName, Exception* prevException=nullptr) \
		        : Exception(filename, line, className, funcName, prevException) \
        		    {} \
			template<typename T> exceptionName& operator<<(T v) {\
				this->stream() << v; \
				return *this; \
			} \
		};

		DEFINE_HINODE_GRAPHICS_EXCEPTION(FailedToCreateException);
		DEFINE_HINODE_GRAPHICS_EXCEPTION(RunTimeErrorException);
        DEFINE_HINODE_GRAPHICS_EXCEPTION(InvalidArgumentsException);
        
#define HINODE_GRAPHICS_CREATE_EXCEPTION(exceptionName, className, funcName) \
	 ::hinode::graphics:: exceptionName (__FILE__, __LINE__, (className), (funcName))
#define HINODE_GRAPHICS_CREATE_EXCEPTION_WITH_PREV_EXCEPTION(exceptionName, className, funcName, prevException) \
	 ::hinode::graphics:: exceptionName (__FILE__, __LINE__, (className), (funcName), &prevException)

	}
}
