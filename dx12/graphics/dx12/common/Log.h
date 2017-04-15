#pragma once

#include <fstream>
#include <memory>
#include <sstream>
#include <functional>

#include "MacroUtility.h"

namespace hinode
{
    namespace graphics
    {
        class Log
        {
        public:
			static void sStandbyLogFile(const char* logFilePath = "hinodeGraphicsLog.txt")_noexcept;
			static void sStandbyLogFile(std::shared_ptr<std::ofstream>& pUsedOutputStream)_noexcept;

			static std::shared_ptr<std::ofstream> sGetLogStream()_noexcept;

			static std::stringstream sMakeErrorMessagePrefix(const char* fileName, int lineNo, const char* className, const char* funcName);

        private:
            static std::shared_ptr<std::ofstream> spLogStream;

        public:
            enum TYPE {
                eERROR,
                eINFO,
                eMEMO,
            };

            enum KEYWORD {
                eSP,	//' '
                eCOM,   //','
                eBR,    //\n
            };

        public:
            Log(TYPE type)_noexcept;
            Log(TYPE type, const char* fileName, int lineNo, const char* className, const char* funcName)_noexcept;
			Log(Log&& log);
            ~Log();

			Log& operator=(Log&& right)noexcept;

            template<typename T>
            Log& operator<<(const T n)_noexcept
            {
                this->mMessage << n;
                return *this;
            }

            Log& operator<<(const KEYWORD n)_noexcept
            {
                static const char tbl[] = {
                    ' ',//eSP
                    ',',//eCOM
                };
                if (n == eBR) {
                    this->mMessage << std::endl;
                }else{
                    this->mMessage << tbl[n];
                }
                return *this;
            }

            void clear()_noexcept;
            void write()_noexcept;

            void setAutoWrote(bool enable)_noexcept;

            void setType(TYPE type)_noexcept;
            void setFileName(const char*name)_noexcept;
            void setLineNo(int lineNo)_noexcept;
            void setClassName(const char*name)_noexcept;
            void setFuncName(const char*name)_noexcept;

        private:
            void writeError()_noexcept;
            void writeInfo()_noexcept;
            void writeMemo()_noexcept;

        private:
            TYPE mType;
            std::string mFileName;
            int mLine;
            std::string mClassName;
            std::string mFuncName;
            std::stringstream mMessage;
            bool mIsAutoWroteLog;
        };

        //--------------------------------------------------------------
        //	Log(Log::eERROR)のファイル名と行番号を省略するための関数
        //
#   	define HINODE_GRAPHICS_ERROR_LOG(className, funcName) \
            ::hinode::graphics::Log(::hinode::graphics::Log::eERROR, __FILE__, __LINE__, (className), (funcName))

    }
}
