/*++

Library name:

  apostol-core

Module Name:

  Module.hpp

Notices:

  Apostol Core

Author:

  Copyright (c) Prepodobny Alen

  mailto: alienufo@inbox.ru
  mailto: ufocomp@gmail.com

--*/

#ifndef APOSTOL_MODULE_HPP
#define APOSTOL_MODULE_HPP

#define APOSTOL_MODULE_UID_LENGTH    42

#define APOSTOL_INDEX_FILE "index.html"

extern "C++" {

namespace Apostol {

    namespace Module {

        class CModuleProcess;
        //--------------------------------------------------------------------------------------------------------------

        typedef std::function<void (CHTTPServerConnection *AConnection)> COnMethodHandlerEvent;
        //--------------------------------------------------------------------------------------------------------------

        CString b2a_hex(const unsigned char *byte_arr, int size);
        CString hmac_sha256(const CString &key, const CString &data);
        //--------------------------------------------------------------------------------------------------------------

        CString SHA1(const CString &data);
        //--------------------------------------------------------------------------------------------------------------

        CString LongToString(unsigned long Value);
        //--------------------------------------------------------------------------------------------------------------

        LPCTSTR StrWebTime(time_t Time, LPTSTR lpszBuffer, size_t Size);
        //--------------------------------------------------------------------------------------------------------------

        CString GetUID(unsigned int len);
        CString ApostolUID();

        //--------------------------------------------------------------------------------------------------------------

        //-- CMethodHandler --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CMethodHandler: CObject {
        private:

            bool m_Allow;
            COnMethodHandlerEvent m_Handler;

        public:

            CMethodHandler(bool Allow, COnMethodHandlerEvent && Handler): CObject(),
                m_Allow(Allow), m_Handler(Handler) {

            };

            bool Allow() const { return m_Allow; };

            void Handler(CHTTPServerConnection *AConnection) {
                if (m_Allow && m_Handler)
                    m_Handler(AConnection);
            }
        };
        //--------------------------------------------------------------------------------------------------------------
#ifdef WITH_POSTGRESQL
        class CJob: CCollectionItem {
        private:

            CString m_Identity;

            CString m_CacheFile;

            CReply m_Reply;

            CPQPollQuery *m_pPollQuery;

            CStringList m_Data;

        public:

            explicit CJob(CCollection *ACCollection);

            ~CJob() override = default;

            CString &Identity() { return m_Identity; };
            const CString &Identity() const { return m_Identity; };

            CString &CacheFile() { return m_CacheFile; };
            const CString &CacheFile() const { return m_CacheFile; };

            CPQPollQuery *PollQuery() { return m_pPollQuery; };
            void PollQuery(CPQPollQuery *Value) { m_pPollQuery = Value; };

            CReply &Reply() { return m_Reply; };
            const CReply &Reply() const { return m_Reply; };

            CStringList &Data() { return m_Data; }
            const CStringList& Data() const { return m_Data; }
        };
        //--------------------------------------------------------------------------------------------------------------

        class CJobManager: CCollection {
            typedef CCollection inherited;
        private:

            CJob *Get(int Index);
            void Set(int Index, CJob *Value);

        public:

            CJobManager(): CCollection(this) {

            }

            CJob *Add(CPQPollQuery *Query);

            CJob *FindJobById(const CString &Id);
            CJob *FindJobByQuery(CPQPollQuery *Query);

            CJob *Jobs(int Index) { return Get(Index); }
            void Jobs(int Index, CJob *Value) { Set(Index, Value); }

        };
#endif
        //--------------------------------------------------------------------------------------------------------------

        //-- CApostolModule --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CModuleManager;
        //--------------------------------------------------------------------------------------------------------------

        enum CModuleStatus { msUnknown = -1, msDisabled, msEnabled };
        //--------------------------------------------------------------------------------------------------------------

        class CApostolModule: public CCollectionItem, public CGlobalComponent {
        private:

            CString m_ModuleName;

            CModuleProcess *m_pModuleProcess;

            CStringPairs m_Sites;

            mutable CString m_AllowedMethods;
            mutable CString m_AllowedHeaders;

            bool m_Sniffer;

        protected:

            int m_Version;
#ifdef WITH_POSTGRESQL
            CJobManager *m_pJobs;
#endif
            CStringList *m_pMethods;

            CStringList m_Headers;

            CModuleStatus m_ModuleStatus;

            virtual void InitMethods() abstract;

            void InitSites(const CSites &Sites);

            virtual void DoHead(CHTTPServerConnection *AConnection);
            virtual void DoGet(CHTTPServerConnection *AConnection);
            virtual void DoOptions(CHTTPServerConnection *AConnection);

            virtual void CORS(CHTTPServerConnection *AConnection);

            virtual void MethodNotAllowed(CHTTPServerConnection *AConnection);

            const CString& GetAllowedMethods() const;
            const CString& GetAllowedHeaders() const;

#ifdef WITH_POSTGRESQL
            virtual void DoPostgresQueryExecuted(CPQPollQuery *APollQuery);
            virtual void DoPostgresQueryException(CPQPollQuery *APollQuery, Delphi::Exception::Exception *AException);
#endif
        public:

            explicit CApostolModule(CModuleProcess *AProcess, const CString& ModuleName);

            ~CApostolModule() override;

            CString& ModuleName() { return m_ModuleName; }
            const CString& ModuleName() const { return m_ModuleName; }

            CModuleStatus ModuleStatus() { return m_ModuleStatus; }

            const CString& AllowedMethods() { return GetAllowedMethods(); };
            const CString& AllowedHeaders() { return GetAllowedHeaders(); };

            virtual bool Enabled() abstract;
            virtual bool Sniffer() { return m_Sniffer; };

            virtual bool CheckConnection(CHTTPServerConnection *AConnection);

            virtual void Initialization(CModuleProcess *AProcess) {};
            virtual void Finalization(CModuleProcess *AProcess) {};

            virtual void BeforeExecute(CModuleProcess *AProcess) {};
            virtual void AfterExecute(CModuleProcess *AProcess) {};

            virtual void Heartbeat();
            virtual void Execute(CHTTPServerConnection *AConnection);

            static CString GetUserAgent(CHTTPServerConnection *AConnection);
            static CString GetOrigin(CHTTPServerConnection *AConnection);
            static CString GetProtocol(CHTTPServerConnection *AConnection);
            static CString GetHost(CHTTPServerConnection *AConnection);

            const CString& GetRoot(const CString &Host) const;
            const CStringList& GetSiteConfig(const CString &Host) const;

            CHTTPClient *GetClient(const CString &Host, uint16_t Port);

            CHTTPServer &Server();
            const CHTTPServer &Server() const;
#ifdef WITH_POSTGRESQL
            CPQServer &PQServer();
            const CPQServer &PQServer() const;

            CPQPollQuery *GetQuery(CPollConnection *AConnection);

            static void EnumQuery(CPQResult *APQResult, CPQueryResult& AResult);
            static void QueryToResults(CPQPollQuery *APollQuery, CPQueryResults& AResults);

            bool StartQuery(CHTTPServerConnection *AConnection, const CStringList& SQL);

            bool ExecSQL(const CStringList &SQL, CPollConnection *AConnection = nullptr,
                         COnPQPollQueryExecutedEvent && OnExecuted = nullptr,
                         COnPQPollQueryExceptionEvent && OnException = nullptr);

            static void PQResultToList(CPQResult *Result, CStringList &List);
            static void PQResultToJson(CPQResult *Result, CString &Json, bool DataArray = false, const CString &ObjectName = CString());
#endif
            static void ContentToJson(CRequest *ARequest, CJSON& Json);
            static void ListToJson(const CStringList &List, CString &Json, bool DataArray = false, const CString &ObjectName = CString());

            static void ExceptionToJson(int ErrorCode, const std::exception &e, CString& Json);

            static void WSDebugRequest(CWebSocket *ARequest);
            static void WSDebugReply(CWebSocket *AReply);
            static void WSDebugConnection(CHTTPServerConnection *AConnection);

            static void DebugRequest(CRequest *ARequest);
            static void DebugReply(CReply *AReply);

            static void DebugConnection(CHTTPServerConnection *AConnection);

            static void Redirect(CHTTPServerConnection *AConnection, const CString& Location, bool SendNow = false);

            static CString TryFiles(const CString &Root, const CStringList &uris, const CString &Location);

            void SendResource(CHTTPServerConnection *AConnection, const CString &Path, LPCTSTR AContentType = nullptr,
                bool SendNow = false, const CStringList& TryFiles = CStringList()) const;

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CModuleManager --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CModuleManager: public CCollection {
            typedef CCollection inherited;
        private:

            void ExecuteModule(CHTTPServerConnection *AConnection, CApostolModule *AModule);

        protected:

            virtual void DoInitialization(CApostolModule *AModule) abstract;
            virtual void DoFinalization(CApostolModule *AModule) abstract;

            virtual void DoBeforeExecuteModule(CApostolModule *AModule) abstract;
            virtual void DoAfterExecuteModule(CApostolModule *AModule) abstract;

        public:

            explicit CModuleManager(): CCollection(this) {

            };

            CString ModulesNames();

            void Initialization();
            void Finalization();

            void HeartbeatModules();

            void ExecuteModules(CTCPConnection *AConnection);

            int ModuleCount() { return inherited::Count(); };
            void DeleteModule(int Index) { inherited::Delete(Index); };

            CApostolModule *Modules(int Index) { return (CApostolModule *) inherited::GetItem(Index); }
            void Modules(int Index, CApostolModule *Value) { inherited::SetItem(Index, (CCollectionItem *) Value); }
        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CModuleProcess --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CModuleProcess: public CServerProcess, public CModuleManager {
        protected:

            void DoInitialization(CApostolModule *AModule) override;
            void DoFinalization(CApostolModule *AModule) override;

            void DoBeforeExecuteModule(CApostolModule *AModule) override;
            void DoAfterExecuteModule(CApostolModule *AModule) override;

            void DoTimer(CPollEventHandler *AHandler) override;
            bool DoExecute(CTCPConnection *AConnection) override;

        public:

            CModuleProcess();

            ~CModuleProcess() override = default;

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CWorkerProcess --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CWorkerProcess: public CModuleProcess {
        public:

            CWorkerProcess();

            ~CWorkerProcess() override = default;

        };

        //--------------------------------------------------------------------------------------------------------------

        //-- CHelperProcess --------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------------

        class CHelperProcess: public CModuleProcess {
        public:

            CHelperProcess();

            ~CHelperProcess() override = default;

        };

    }
}

using namespace Apostol::Module;
}
//----------------------------------------------------------------------------------------------------------------------

#endif //APOSTOL_MODULE_HPP
