/*++

Library name:

  apostol-core

Module Name:

  Log.hpp

Notices:

   Apostol Core (Log)

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef APOSTOL_LOG_HPP
#define APOSTOL_LOG_HPP
//----------------------------------------------------------------------------------------------------------------------

#define LOG_MAX_ERROR_STR     2048
//----------------------------------------------------------------------------------------------------------------------

#define INVALID_FILE         (-1)
#define FILE_ERROR           (-1)
//----------------------------------------------------------------------------------------------------------------------

#define FILE_RDONLY          O_RDONLY
#define FILE_WRONLY          O_WRONLY
#define FILE_RDWR            O_RDWR
#define FILE_CREATE_OR_OPEN  O_CREAT
#define FILE_OPEN            0
#define FILE_TRUNCATE        (O_CREAT|O_TRUNC)
#define FILE_APPEND          (O_WRONLY|O_APPEND)
#define FILE_NONBLOCK        O_NONBLOCK
//----------------------------------------------------------------------------------------------------------------------

#define FILE_DEFAULT_ACCESS  0644
#define FILE_OWNER_ACCESS    0600
//----------------------------------------------------------------------------------------------------------------------

#define APP_LOG_STDERR            0
#define APP_LOG_EMERG             1
#define APP_LOG_ALERT             2
#define APP_LOG_CRIT              3
#define APP_LOG_ERR               4
#define APP_LOG_WARN              5
#define APP_LOG_NOTICE            6
#define APP_LOG_INFO              7
#define APP_LOG_DEBUG             8
//----------------------------------------------------------------------------------------------------------------------

#define APP_LOG_DEBUG_CORE        0x010u
#define APP_LOG_DEBUG_ALLOC       0x020u
#define APP_LOG_DEBUG_MUTEX       0x040u
#define APP_LOG_DEBUG_EVENT       0x080u
#define APP_LOG_DEBUG_HTTP        0x100u
#define APP_LOG_DEBUG_ALL         0x7f0u
#define APP_LOG_DEBUG_CONNECTION  0x800u
//----------------------------------------------------------------------------------------------------------------------

#ifndef log_pid
#define log_pid                  MainThreadID
#endif
//----------------------------------------------------------------------------------------------------------------------

#define log_tid                  0
#define LOG_TID_T_FMT            "%d"
//----------------------------------------------------------------------------------------------------------------------

static string_t err_levels[] = {
        CreateNullString,
        CreateString("emerg"),
        CreateString("alert"),
        CreateString("crit"),
        CreateString("error"),
        CreateString("warn"),
        CreateString("notice"),
        CreateString("info"),
        CreateString("debug")
};
//----------------------------------------------------------------------------------------------------------------------

static u_int GetLogLevelByName(LPCTSTR lpszName) {
    for (size_t I = 1; I < chARRAY(err_levels); ++I) {
        if (SameText(lpszName, err_levels[I].str))
            return I;
    }
    return 0;
};
//----------------------------------------------------------------------------------------------------------------------

static inline ssize_t write_fd(int fd, void *buf, size_t n)
{
    return write(fd, buf, n);
}
//----------------------------------------------------------------------------------------------------------------------

static inline void write_stderr(LPCSTR text)
{
    (void) write_fd(STDERR_FILENO, (char *) text, strlen(text));
}
//----------------------------------------------------------------------------------------------------------------------

#define write_console        write_fd
//----------------------------------------------------------------------------------------------------------------------

extern "C++" {

namespace Apostol {

    namespace Log {

        class CLog;
        //--------------------------------------------------------------------------------------------------------------

        extern CLog *GLog;
        //--------------------------------------------------------------------------------------------------------------

        typedef CLog log_t;

        //--------------------------------------------------------------------------------------------------------------

        //-- CFile -----------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

#define FILE_CLOSE_MESSAGE   _T("Could not close file: \"%s\" error: ")
        //--------------------------------------------------------------------------------------------------------------

#ifdef _GLIBCXX_FUNCTIONAL
        typedef std::function<void (Pointer Sender, int Error, LPCTSTR lpFormat, va_list args)> COnFilerErrorEvent;
#else
        typedef void (* COnFilerErrorEvent) (Pointer Sender, int Error, LPCTSTR lpFormat, va_list args);
#endif
        //--------------------------------------------------------------------------------------------------------------

        enum CLogType { ltError = 0, ltAccess, ltPostgres, ltStream, ltDebug };
        //--------------------------------------------------------------------------------------------------------------

        class CFile: public CObject {

            int m_hHandle;

            int m_iFlags;

            size_t m_uOffset;

            LPTSTR m_lpFileName;

            COnFilerErrorEvent m_OnFilerError;


        private:

            void SetFileName(LPCSTR AValue);

            void DoFilerError(int AError, LPCTSTR lpFormat, ...);

        public:

            explicit CFile(LPCSTR AFileName, int AFlags);

            ~CFile() override;

            int Handle() const { return m_hHandle; }

            char *FileName() { return m_lpFileName; }

            void Open();
            void Close(bool ASafe = false);

            ssize_t Read(char *buf, size_t size, size_t offset);
            ssize_t Write(char *buf, size_t size, size_t offset);

            const COnFilerErrorEvent &getOnFilerError() { return m_OnFilerError; };

            void setOnFilerError(COnFilerErrorEvent && Value) { m_OnFilerError = Value; };

        }; // class CFile

        //--------------------------------------------------------------------------------------------------------------

        //-- CLogFile --------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CLogFile: public CFile, public CCollectionItem {
        private:

            u_int m_uLevel;

            CLogType m_LogType;

        public:

            explicit CLogFile(CLog *ALog, LPCSTR AFileName);

            ~CLogFile() override = default;

            u_int Level() const { return m_uLevel; }
            void Level(u_int Value) { m_uLevel = Value; };

            CLogType LogType() { return m_LogType; }
            void LogType(CLogType Value) { m_LogType = Value; };

        }; // class CLogFile

        //--------------------------------------------------------------------------------------------------------------

        //-- CLogComponent ---------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CLogComponent {
        public:
            CLogComponent();
            ~CLogComponent();
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CLog ------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CLog: public CSysErrorComponent, public CCollection {
            typedef CCollection inherited;
        private:

            u_int       m_uLevel;
            int         m_CurrentIndex;
            bool        m_fUseStdErr;
            time_t      m_DiskFullTime;

        protected:

            static char *StrError(int AError, char *AStr, size_t ASize);
            static char *ErrNo(char *ADest, char *ALast, int AError);
            void ErrorCore(u_int ALevel, int AError, LPCSTR AFormat, CLogType ALogType, va_list args);

            void SetLevel(u_int Value);

            void CheckCurrentIndex();
            void SetCurrentIndex(int Index);

        public:

            CLog();

            explicit CLog(LPCSTR  AFileName, u_int ALevel);

            inline static class CLog *CreateLog() { return GLog = new CLog(); };

            inline static void DestroyLog() { delete GLog; };

            ~CLog() override = default;

            CLogFile *AddLogFile(LPCSTR AFileName, u_int ALevel = APP_LOG_STDERR);

            void Error(u_int ALevel, int AErrNo, LPCSTR AFormat, ...);
            void Error(u_int ALevel, int AErrNo, LPCSTR AFormat, va_list args);

            void Debug(u_int ALevel, LPCSTR AFormat, ...);
            void Debug(u_int ALevel, LPCSTR AFormat, va_list args);

            void Notice(LPCSTR AFormat, ...);
            void Notice(LPCSTR AFormat, va_list args);

            void Message(LPCSTR AFormat, ...);
            void Message(LPCSTR AFormat, va_list args);

            void Access(LPCSTR AFormat, ...);
            void Access(LPCSTR AFormat, va_list args);

            void Stream(LPCSTR AFormat, ...);
            void Stream(LPCSTR AFormat, va_list args);

            void Postgres(u_int ALevel, LPCSTR AFormat, ...);
            void Postgres(u_int ALevel, LPCSTR AFormat, va_list args);

            u_int Level() const { return m_uLevel; }
            void Level(u_int Value) { SetLevel(Value); };

            time_t DiskFullTime() const { return m_DiskFullTime; };
            void DiskFullTime(time_t Value) { m_DiskFullTime = Value; };

            bool UseStdErr() const { return m_fUseStdErr; };
            void UseStdErr(bool Value) { m_fUseStdErr = Value; };

            void RedirectStdErr();

            int CurrentIndex() const { return m_CurrentIndex; }
            void CurrentIndex(int Index) { SetCurrentIndex(Index); };

            CLogFile *Current();
            CLogFile *First();
            CLogFile *Last();
            CLogFile *Prior();
            CLogFile *Next();

            CLogFile *LogFiles(int Index) { return (CLogFile *) inherited::GetItem(Index); }
            void LogFiles(int Index, CLogFile *Item) { inherited::SetItem(Index, Item); }

        }; // class CLog
    }
}

using namespace Apostol::Log;
}
//----------------------------------------------------------------------------------------------------------------------

#endif // APOSTOL_LOG_HPP
