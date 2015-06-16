/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once
#define DIRECT_AUTHOR_TEST

#define IfFlagSet(value ,flag) ((value & flag) == flag)

#define IfNullGo(expr, result) \
 do {                          \
    if ((expr) == NULL) {      \
        IfFailGo ((result));   \
    }                          \
 } while (0)



class CAuthoringHost : public IUnknown
{
    long refCount;
    unsigned int _threadId;
    HANDLE hThread;
    Message<CAuthoringHost> _message;
    void *_callbackData;
    void(*_callback)(void *);
    UINT_PTR _timer;

    static VOID CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
    void Timer();
    HRESULT SetCallbackTimer(DWORD dwTime);
    static DWORD __stdcall TimerLoop(LPVOID arg);
    static const DWORD HURRY_DELAY = 1;
    
public:
    void CallAfter(DWORD dwTime, void(*callback)(void *), void *data);
    void KillCallback();
    struct HostLoopArgs
    {
        LPCWSTR jscriptPath;
    };
    static unsigned int HostLoop(void* args);

public:
    CAuthoringHost();
    virtual ~CAuthoringHost();
    CComPtr<IAuthorServices> _authoringServices;

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    HRESULT Initialize(LPCWSTR targetdll);

};

std::wstring FindJScript9LS();

