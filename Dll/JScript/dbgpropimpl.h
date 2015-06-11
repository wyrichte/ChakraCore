//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#if _WIN64
typedef IDebugApplicationThread64 IDebugBitCorrectApplicationThread;
#else
typedef IDebugApplicationThread IDebugBitCorrectApplicationThread;
#endif

class DebugHelper
{
public:
    static bool IsScriptSiteClosed(ScriptSite* site, HRESULT* hr)
    {
        if (site->IsClosed() || site->GetScriptEngine()->IsInClosedState())
        {
            *hr = E_ABORT;
            return true;
        }
        return false;
    }

    static bool IsScriptEngineClosed(ScriptEngine* engine, HRESULT* hr)
    {
        if (engine->IsInClosedState())
        {
            *hr = E_ABORT;
            return true;
        }
        return false;
    }
};


class DebugProperty sealed : public IDebugProperty, public IDebugPropertyObjectMutation, public IDebugThreadCall
{
    ULONG m_refCount;
    WeakArenaReference<Js::IDiagObjectModelDisplay>* pModelWeakRef;
    CComQIPtr<IDebugBitCorrectApplicationThread> m_pApplicationThread;
    CComPtr<ScriptEngine> pScriptEngine;
    CComPtr <IUnknown> spunkParent;
    DWORD m_setDbgPropInfoAttributes;
    // Used for letting the debugger know that the item belongs to Return value sub-tree, that way we can tell them this is fake node.
    bool m_isInReturnValueHierarchy;

    enum
    {
        xthread_GetPropertyInfo,
        xthread_EnumMembers,
        xthread_SetValueAsString,
        xthread_CanSetMutationBreakpoint,
        xthread_SetMutationBreakpoint
    };

    struct GetPropertyInfoArgs
    {
        DBGPROP_INFO_FLAGS dwFieldSpec;
        UINT nRadix;
        DebugPropertyInfo *pPropertyInfo;
    };

    struct EnumMembersArgs
    {
        EnumMembersArgs(DWORD _dwFieldSpec, UINT _nRadix, REFIID _refiid, IEnumDebugPropertyInfo** ppepi);

        DBGPROP_INFO_FLAGS dwFieldSpec;
        UINT nRadix;
        REFIID refiid;
        IEnumDebugPropertyInfo **ppepi;
    };

    struct SetValueAsStringArgs
    {
        LPCOLESTR pszValue;
        UINT nRadix;
    };

    struct SetMutationBreakpointArgs
    {
        SetMutationBreakpointArgs(BOOL setOnObject, MutationType type, IMutationBreakpoint **mutationBreakpoint);
        BOOL setOnObject;
        MutationType type;
        IMutationBreakpoint **mutationBreakpoint;
    };

    struct CanSetMutationBreakpointArgs
    {
        CanSetMutationBreakpointArgs(BOOL setOnObject, MutationType type, BOOL *canSet);
        BOOL setOnObject;
        MutationType type;
        BOOL *canSet;
    };


    HRESULT PrependParentNameToChildFullName(
        _In_z_ BSTR parentName,
        _In_z_ BSTR childFullName,
        _In_ bool isParentNameLiteralProperty,
        _Out_ BSTR* combinedFullName);

    HRESULT PrependParentArrayNameToChildFullName(
        _In_z_ BSTR parentName,
        _In_ uint parentNameLength,
        _In_z_ BSTR childFullName,
        _In_ uint childFullNameLength,
        _Out_ BSTR* combinedFullName);

    HRESULT PrependParentNameToChildFullName(
        _In_z_ BSTR parentName,
        _In_ uint parentNameLength,
        _In_z_ BSTR childFullName,
        _In_ uint childFullNameLength,
        _Out_ BSTR* combinedFullName);

    HRESULT PrependNonLiteralPropertyNameToChildFullName(
        _In_z_ BSTR parentName,
        _In_ uint parentNameLength,
        _In_z_ BSTR childFullName,
        _In_ uint childFullNameLength,
        _Out_ BSTR* combinedFullName);

    HRESULT PrependDottedElementNameToChildFullName(
        _In_z_ BSTR parentName,
        _In_ uint parentNameLength,
        _In_z_ BSTR childFullName,
        _In_ uint childFullNameLength,
        _Out_ BSTR* combinedFullName);

    HRESULT GetFullName(_Out_ BSTR * pbstrFullName);
    
    template<typename Function>
    void EnumEscapeCharacters(_In_reads_z_(strLength) PCWSTR str, uint strLength, Function function);
    HRESULT GetEscapedCharacterCount(_In_reads_z_(strLength) PCWSTR str, uint strLength, _Out_ uint* count);
    HRESULT EscapeCharacters(_Out_writes_all_(destinationLength) PWCHAR destination, uint destinationLength, _In_reads_z_(sourceLength) PCWSTR source, uint sourceLength);

    Var GetVarFromModelDisplayWeakRef();
public:
    DebugProperty(WeakArenaReference<Js::IDiagObjectModelDisplay>* pModel,
                  IDebugApplicationThread* pApplicationThread,
                  ScriptEngine* scriptEngine,
                  __in_opt IUnknown * punkParent,
                  bool isInReturnValueHierarchy,
                  DWORD setDbgPropInfoAttributes = 0);

    ~DebugProperty();

    // IUnknown

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);

    // IDebugProperty

    HRESULT STDMETHODCALLTYPE GetPropertyInfo( 
        /* [in] */ DBGPROP_INFO_FLAGS dwFieldSpec,
        /* [in] */ UINT nRadix,
        /* [out] */ __RPC__out DebugPropertyInfo *pPropertyInfo);

    HRESULT STDMETHODCALLTYPE GetExtendedInfo( 
        /* [in] */ ULONG cInfos,
        /* [size_is][in] */ __RPC__in_ecount_full(cInfos) GUID *rgguidExtendedInfo,
        /* [size_is][out] */ __RPC__out_ecount_full(cInfos) VARIANT *rgvar);

    HRESULT STDMETHODCALLTYPE SetValueAsString( 
        /* [in] */ __RPC__in LPCOLESTR pszValue,
        /* [in] */ UINT nRadix);

    HRESULT STDMETHODCALLTYPE EnumMembers( 
        /* [in] */ DBGPROP_INFO_FLAGS dwFieldSpec,
        /* [in] */ UINT nRadix,
        /* [in] */ __RPC__in REFIID refiid,
        /* [out] */ __RPC__deref_out_opt IEnumDebugPropertyInfo **ppepi);

    HRESULT STDMETHODCALLTYPE GetParent( 
        /* [out] */ __RPC__deref_out_opt IDebugProperty **ppDebugProp);

    // IDebugThreadCall

    HRESULT STDMETHODCALLTYPE ThreadCallHandler(
        /* [in] */ DWORD_PTR dwParam1,
        /* [in] */ DWORD_PTR dwParam2,
        /* [in] */ DWORD_PTR dwParam3);

    // This is a helper function which will try to set the value to the debug address. If it fails to do that, it will
    // try to construct the expression such as "fullname = pszValue".
    static HRESULT BuildExprAndEval(BSTR bstrFullName,
                                    DebugProperty *dbgProp,
                                    Js::IDiagObjectAddress* address,
                                    Js::ScriptContext *scriptContext,
                                    Js::DiagStackFrame* frame,
                                    LPCOLESTR pszValue); 

    // IDebugPropertyObjectMutation

    STDMETHODIMP CanSetMutationBreakpoint(
        /* [in] */ BOOL setOnObject,
        /* [in] */ MutationType type,
        /* [out] */ __RPC__out BOOL *canSet);

    STDMETHODIMP SetMutationBreakpoint(
        /* [in] */ BOOL setOnObject,
        /* [in] */ MutationType type,
        /* [out] */ __RPC__deref_out_opt IMutationBreakpoint **mutationBreakpoint);
};

class DebugPropertySetValueCallback sealed : public IDebugSetValueCallback
{
private:
    CComQIPtr<IDebugBitCorrectApplicationThread> pApplicationThread;
    CComPtr<ScriptEngine> pScriptEngine;
    Js::IDiagObjectAddress* pAddress;
    ULONG m_refCount;

public:
    DebugPropertySetValueCallback(IDebugApplicationThread* pApplicationThread,
                  ScriptEngine* scriptEngine);

    void SetDiagAddress(Js::IDiagObjectAddress* _pAddress);
    Js::IDiagObjectAddress * GetDiagAddress() const
    {
        return pAddress;
    }

    // IUnknown

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);

    // IDebugSetValueCallback
    HRESULT STDMETHODCALLTYPE SetValue(VARIANT *pvarNode,
        DISPID dispid,
        ULONG clIndicies,
        LONG *prglIndicies,
        LPCOLESTR pszValue,
        UINT nRadix,
        BSTR *pbstrError);
};

class EnumDebugPropertyInfo sealed : public IEnumDebugPropertyInfo, public IDebugThreadCall
{
    ULONG m_refCount;
    ULONG current;
    UINT m_nRadix;
    WeakArenaReference<Js::IDiagObjectModelWalkerBase>* pWalkerWeakRef;
    CComQIPtr<IDebugBitCorrectApplicationThread> m_pApplicationThread;
    CComPtr<ScriptEngine> m_pScriptEngine;
    CComPtr <IUnknown> spunkDbgProp;
    DBGPROP_INFO_FLAGS m_dwFieldSpec;
    // Needs to pass the field to the DebugProperty.
    bool m_isInReturnValueHierarchy;

    enum
    {
        xthread_GetCount,
        xthread_Next
    };

    struct NextArgs
    {
        ULONG celt;
        DebugPropertyInfo* pi;
        ULONG* pCeltsfetched;
    };

public:
    EnumDebugPropertyInfo(WeakArenaReference<Js::IDiagObjectModelWalkerBase>* pWalker,
                          IDebugApplicationThread* pApplicationThread,
                          ScriptEngine* scriptEngine,
                          __in_opt IUnknown * punkDbgProp,
                          DBGPROP_INFO_FLAGS dwFieldSpec,
                          UINT nRadix,
                          bool isInReturnValueHierarchy);
    ~EnumDebugPropertyInfo();

    // IUnknown

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv);

    // IEnumDebugPropertyInfo

    HRESULT STDMETHODCALLTYPE Next( 
        /* [in] */ ULONG celt,
        /* [out] */ DebugPropertyInfo *pi,
        /* [out] */ ULONG *pcEltsfetched);

    HRESULT STDMETHODCALLTYPE Skip( 
        /* [in] */ ULONG celt);

    HRESULT STDMETHODCALLTYPE Reset( void);

    HRESULT STDMETHODCALLTYPE Clone( 
        /* [out] */ __RPC__deref_out_opt IEnumDebugPropertyInfo **ppepi);

    HRESULT STDMETHODCALLTYPE GetCount( 
        /* [out] */ __RPC__out ULONG *pcelt);

    // IDebugThreadCall

    HRESULT STDMETHODCALLTYPE ThreadCallHandler(
        /* [in] */ DWORD_PTR dwParam1,
        /* [in] */ DWORD_PTR dwParam2,
        /* [in] */ DWORD_PTR dwParam3);
private:

    // Only to be called from the Next function.
    HRESULT NextInternal(ULONG celt, DebugPropertyInfo *pi, ULONG *pcEltsfetched);
};
