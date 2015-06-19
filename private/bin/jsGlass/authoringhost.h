/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

class JsGlass;
struct CompletionsOperation;

class CAuthoringHost : public IUnknown
{
    friend CompletionsOperation;

    long refCount;
    JsGlass* _jsGlass;
    unsigned int _threadId;
    Message<CAuthoringHost> _message;
    void *_callbackData;
    void (*_callback)(void *);
    UINT_PTR _timer;

    static VOID CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    void Timer();
    HRESULT SetCallbackTimer(DWORD dwTime);
    void CallAfter(DWORD dwTime, void (*callback)(void *), void *data);
    void KillCallback();

public:
    struct HostLoopArgs
    {
        JsGlass* jsGlass;
        LPCWSTR jscriptPath;
    };
    static unsigned int HostLoop(void* args);

public:
    CAuthoringHost(JsGlass* jsGlass);
    virtual ~CAuthoringHost();

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    
    HRESULT Initialize(LPCWSTR targetdll);
#ifdef DIRECT_AUTHOR_TEST
    HRESULT GetTokenRanges(IUnknown *scriptEngine, LPCWSTR scriptCode);
    HRESULT GetRegions(IUnknown *scriptEngine, LPCWSTR scriptCode);
    HRESULT GetCompletions(IUnknown *scriptEngine, int count, __in_ecount(count) LPCWSTR scripts[]);
    HRESULT GetErrors(IUnknown *scriptEngine, LPCWSTR scriptCode);
    HRESULT GetAst(IUnknown *scriptEngine, LPCWSTR scriptCode);
    HRESULT GetQuickInfo(IUnknown *scriptEngine, LPCWSTR scriptCode);
    HRESULT ProcessCompletionsSession(IUnknown *scriptEngine, LPCWSTR scriptCode, LPCWSTR defaultPath);
    HRESULT GetFunctionHelp(IUnknown *scriptEngine, int count, __in_ecount(count) LPCWSTR scripts[]);
    HRESULT SplatterSession(IUnknown *scriptEngine, LPCWSTR scriptCode);
    HRESULT MultipleHostTypeCompletion(IUnknown *scriptEngine, int count, __in_ecount(count) LPCWSTR scripts[]);

private:
    HRESULT PerformGetCompletionRequest(IAuthorFileAuthoring* authoring, long offset, AuthorCompletionFlags flags, CComBSTR &json);
#endif
};
