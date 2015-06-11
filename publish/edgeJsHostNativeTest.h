/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "activscp.h"

typedef  HRESULT (STDMETHODCALLTYPE *JsHostOnScriptErrorCallback)(IActiveScriptError *, void* context);
typedef  HRESULT (__stdcall *JsHostLoadScriptFileCallback)(void* jsHostScriptSite, LPCWSTR filename);
typedef  HRESULT (__stdcall *JsHostCreateNewEngineCallback)(IActiveScriptDirect** jsScriptDirect, void** jsHostScriptSite, bool freeAtShutdown);
typedef  HRESULT (__stdcall *JsHostCreateThreadServiceCallback)(IActiveScriptGarbageCollector** trackerService);
typedef  HRESULT (__stdcall *JsHostShutdownScriptSiteCallback)(void* jsHostScriptSite);

class JsHostOnScriptErrorHelper
{
private:
    JsHostOnScriptErrorCallback m_callback;
    void* cookie;

public:
    JsHostOnScriptErrorHelper() : m_callback(NULL) {}
    void SetCallback(JsHostOnScriptErrorCallback onScriptErrorCallback, void* context) { m_callback = onScriptErrorCallback; this->cookie = context;}
    void* GetContext() const { return cookie; }
    JsHostOnScriptErrorCallback GetCallback() { return m_callback; }
};

struct JsHostNativeTestArguments
{
    IActiveScript* activeScript;
    void* jsHostScriptSite;
    HINSTANCE jscriptLibrary;
    JsHostOnScriptErrorHelper* onScriptErrorHelper;
    JsHostLoadScriptFileCallback loadScriptFile;
    JsHostCreateNewEngineCallback createNewEngine;
    JsHostShutdownScriptSiteCallback shutdownScriptSite;
    JsHostCreateThreadServiceCallback createThreadService;
    int flagCount;
    LPWSTR* flags;
};

const LPCSTR nativeTestEntryPointName = "RunNativeTest";
typedef HRESULT (__stdcall *NativeTestEntryPoint)(JsHostNativeTestArguments* args);


