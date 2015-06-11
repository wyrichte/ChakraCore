//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class AuthoringServicesInfo;

    class AuthoringFactory
    {
    private:
        RefCountedPtr<AuthoringServicesInfo> m_authoringServicesInfo;
        RefCountedPtr<IAuthorServiceHost> m_host;
        ThreadContext::CollectCallBack *m_recyclerCallback;
        bool m_inCleanup;

        void CleanupPropertyCaches();
        bool StartConcurrentCleanup(ThreadContext *threadContext);

        static void _cdecl RecyclerCallback(void *context, RecyclerCollectCallBackFlags flags);
    public:
        struct Resources
        {
            static UINT HelpersJs();
            static UINT IntlHelpersJs();
            static UINT IntlJs();
        };

        AuthoringFactory() : m_authoringServicesInfo(nullptr), m_recyclerCallback(nullptr), m_inCleanup(false) { }

        virtual Js::ScriptContext *GetAuthoringScriptContext() = 0;
        virtual ArenaAllocator* GetAuthoringScriptAllocator() = 0;
        void CloseLanguageService();

        HRESULT GetColorizer(IAuthorColorizeText **result);
        HRESULT RegisterFile(IAuthorFile *file, IAuthorFileHandle **result);
        HRESULT GetFileAuthoring(IAuthorFileContext* context, IAuthorFileAuthoring **result);
        HRESULT Cleanup(VARIANT_BOOL exhaustive);
        HRESULT SetHost(IAuthorServiceHost *host);
        HRESULT Work();

        static bool GetResourceText(UINT resourceId, LPCUTF8 *text, DWORD* textLen);
        static void DecommitUnusedPages(PageAllocator *pageAllocator);
    };
}