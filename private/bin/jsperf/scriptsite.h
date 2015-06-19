#pragma once

class ScriptSite : IActiveScriptSite
{
public:
    ScriptSite();
    ~ScriptSite();
private:
    ScriptSite(ScriptSite const&);
    void operator=(ScriptSite const&);
public:

    // IUnknown interface
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

    // IActiveScriptSite interface
    HRESULT STDMETHODCALLTYPE GetLCID(LCID *plcid) { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
    HRESULT STDMETHODCALLTYPE GetDocVersionString(BSTR *pbstrVersion) { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE OnScriptTerminate(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo) { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE OnStateChange(SCRIPTSTATE ssScriptState) { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE OnScriptError(IActiveScriptError *pscripterror);
    HRESULT STDMETHODCALLTYPE OnEnterScript( void) { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE OnLeaveScript( void) { return E_NOTIMPL; }


    // static methods
    static HRESULT Create(ScriptSite ** scriptSite);
    static HRESULT Shutdown(ScriptSite * scriptSite);

    // public interface
    HRESULT ParseScript(LPCOLESTR contents);
    HRESULT GetGlobalValue(LPCOLESTR pszStr, int *val);
    HRESULT InvokeGlobalFunction(LPCOLESTR pszStr);
    

private:
    HRESULT CreateScriptEngine();
    HRESULT StopScriptEngine();
    HRESULT GetActiveScript(IActiveScript ** activeScript);
    HRESULT GetDispID(LPCOLESTR pszStr, DISPID *dispid);

    DWORD m_refCount;
    DWORD m_activeScriptCookie;
    DWORD m_dwNextSourceCookie;

    IActiveScript *m_pActiveScript;
    IDispatch *m_pGlobalObject;

    std::map<std::wstring, DISPID> m_DISPIDs;
};