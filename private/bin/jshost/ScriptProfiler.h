#pragma once

enum FunctionCallType
{
    SCRIPT_FUNCTION,    // scriptId, functionId
    NATIVE_FUNCTION     // functionName, scriptType
};

struct FunctionCallInfo
{
    FunctionCallType functionCallType;
    union
    {
        struct
        {
            PROFILER_TOKEN scriptId;
            PROFILER_TOKEN functionId;
        };
        PROFILER_SCRIPT_TYPE scriptType;
    };
    std::wstring functionName;

    FunctionCallInfo(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId)
        : functionCallType(SCRIPT_FUNCTION), scriptId(scriptId), functionId(functionId)
    {}

    FunctionCallInfo(const WCHAR *pwszFunctionName, PROFILER_SCRIPT_TYPE type)
        : functionCallType(NATIVE_FUNCTION), functionName(pwszFunctionName), scriptType(type)
    {}

    bool operator==(const FunctionCallInfo& other) const;
    bool operator!=(const FunctionCallInfo& other) const;
};

struct ScriptInfo
{
    PROFILER_TOKEN scriptId;
    std::map<PROFILER_TOKEN, std::wstring> listOffunctions;
};

class ScriptProfiler : public IActiveScriptProfilerCallback3
{
public:
    ScriptProfiler(void);
    ~ScriptProfiler(void);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

    // IActiveScriptProfilerCallback methods

    STDMETHOD(Initialize)(DWORD dwContext);
    STDMETHOD(Shutdown)(HRESULT hrReason);
    STDMETHOD(ScriptCompiled)(
        PROFILER_TOKEN id,
        PROFILER_SCRIPT_TYPE type,
        IUnknown *pDocumentContext);
    STDMETHOD(FunctionCompiled)(
        PROFILER_TOKEN functionId,
        PROFILER_TOKEN scriptId,
        const WCHAR *pwszFunctionName,
        const WCHAR *pwszFunctionNameHint,
        IUnknown *pDocumentContext);
    STDMETHOD(OnFunctionEnter)(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId);
    STDMETHOD(OnFunctionExit)(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId);

    // IActiveScriptProfilerCallback2
    STDMETHOD(OnFunctionEnterByName)(const WCHAR *pwszFunctionName, PROFILER_SCRIPT_TYPE type);
    STDMETHOD(OnFunctionExitByName)(const WCHAR *pwszFunctionName, PROFILER_SCRIPT_TYPE type);

    // IActiveScriptProfilerCallback3
    STDMETHOD(SetWebWorkerId)(DWORD webWorkerId) { return S_OK; }

    void ValidateEvents();
    const WCHAR* GetFunctionName(PROFILER_TOKEN scriptId, PROFILER_TOKEN functionId) const;
    WCHAR *GetText(IUnknown *debugDocument);
    void CheckFunctionEnter();

private:
    long m_refCount;
    bool m_fProfileOn;

    std::map<PROFILER_TOKEN, ScriptInfo*> m_scriptMetaDataList;
    std::stack<FunctionCallInfo> m_functionCallStack;

    // Currently script profiler can output 2 kinds of logs:
    //  (1) FunctionCompiled events with LineColumn info.
    //  (2) Function call tree.
    //
    // These two kinds of logs can intermingle with each other and the order is unstable, thus
    // can't be used in test baseline. To avoid this problem, output FunctionCompiled logs but
    // cache function call tree logs. Flush the cached logs at shutdown.
    //
    std::list<std::wstring> m_functionCompiledLog;
    std::list<std::wstring> m_logs;
};
