//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace Memory { class Recycler; }
class JavascriptDispatch: public FinalizableObject, IDispatchEx, public IJavascriptDispatchLocalProxy, public IJavascriptDispatchRemoteProxy
{
    friend class ScriptEngine;
    friend class Recycler;
    friend class ScriptSite;
    friend class AutoActiveCallPointer;
public:
    template <bool inScript>
    static JavascriptDispatch * Create(Js::DynamicObject* pgcScriptObject);

    /****************************************
    IUnknown methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    /****************************************
    IDispatch methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, __in_ecount(cpsz) LPOLESTR *prgpsz, UINT cpsz, LCID lcid, __in_ecount(cpsz) DISPID *prgid);
    virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarRes, EXCEPINFO *pexcepinfo, UINT *puArgErr);

    /****************************************
    IDispatchEx methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE GetDispID(BSTR bstr, DWORD grfdex, DISPID *pid) sealed;
    virtual HRESULT STDMETHODCALLTYPE InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller) sealed;
    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName(BSTR bstr, DWORD grfdex);
    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(DISPID id);
    virtual HRESULT STDMETHODCALLTYPE GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex);
    virtual HRESULT STDMETHODCALLTYPE GetMemberName(DISPID id, BSTR *pbstr);
    virtual HRESULT STDMETHODCALLTYPE GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid);
    virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent(IUnknown **ppunk);


    /****************************************
    IJavascriptDispatchRemoteProxy methods
    ****************************************/
    virtual HRESULT STDMETHODCALLTYPE GetTypeId(int* objectType);
    virtual HRESULT STDMETHODCALLTYPE HasInstance(VARIANT instance, BOOL * result, EXCEPINFO *pei, IServiceProvider *pspCaller);
    virtual HRESULT STDMETHODCALLTYPE InvokeBuiltInOperation(BuiltInOperation operation, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);
    virtual HRESULT STDMETHODCALLTYPE PrivateInvokeEx(DISPID id, LCID lcid, DWORD dwFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);

    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override;
    virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

    IDispatch * GetThis() {this->AddRef();  return static_cast<IDispatch*>(this); }

    inline Js::DynamicObject* GetObject(void) { return scriptObject; }

    HRESULT ResetToScriptSite(ScriptSite* newScriptSite);
    void ResetToNULL();
    void SetAsGCTracked();

protected:
    JavascriptDispatch(
        __in Js::DynamicObject* pgcScriptObject,
        __in ScriptSite *scriptSite);

    Js::DynamicObject* scriptObject;
    ScriptSite *scriptSite;

    // We cache this the string to hold a reference with the assumption that getting a reference
    // to the disp id means that the caller is going to set the property soon so we dont want the
    // associated property string to be reclaimed
    typedef JsUtil::BaseDictionary<int, Js::PropertyRecord const*, Recycler, PowerOf2SizePolicy> Int32InternalStringMap;
    Int32InternalStringMap* dispIdPropertyStringMap;
    void CachePropertyId(Js::PropertyRecord const* propertyRecord, BOOL isPropertyId = true);
    HRESULT ResetContentToNULL();

    BOOL VerifyCallingThread();

private:
    volatile ULONG refCount;
    bool isGCTracked : 1;
    bool isInCall : 1;
    bool isFinalized: 1;
#if DBG
    bool isFinishCreated : 1;
#endif
    class DispIdEnumerator * dispIdEnumerator;

    LIST_ENTRY linkList;


    Js::ScriptContext * GetScriptContext();
    void CreateDispIdEnumerator();

    HRESULT InvokeOnMember(
        DISPID id,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS *pdp,
        VARIANT *pvarRes,
        EXCEPINFO *pei,
        IServiceProvider *pspCaller);

    HRESULT InvokeOnSelf(
        DISPID              id,
        LCID                lcid,
        WORD                wFlags,
        DISPPARAMS *        pdp,
        VARIANT *           pvarRes,
        EXCEPINFO *         pei,
        IServiceProvider *  pspCaller);

    HRESULT GetPropertyIdWithFlagWithScriptEnter(__in BSTR bstrName, DWORD grfdex, __out Js::PropertyId* propertyId, uint32* indexVal, __out BOOL* pIsPropertyId, __out Js::PropertyRecord const ** propertyRecord);
    HRESULT OP_InitPropertyWithScriptEnter(Var instance, PropertyId propertyId, Var newValue);
    HRESULT GetPropertyIdWithFlag(__in BSTR bstrName, DWORD grfdex, __out Js::PropertyId* propertyId, uint32* indexVal, __out BOOL* pIsPropertyId, __out Js::PropertyRecord const ** propertyRecord);
    HRESULT GetRecordedError(HRESULT hr, EXCEPINFO * pei);
    HRESULT GetTypeInfoWithScriptEnter(UINT iti, LCID lcid, ITypeInfo **ppti);
    HRESULT NextWithScriptEnter(ULONG celt, VARIANT *rgVar, ULONG *pCeltFetched);
    HRESULT SkipWithScriptEnter(ULONG celt);
    HRESULT ResetWithScriptEnter();
    HRESULT GetNextDispIDWithScriptEnter(DWORD grfdex, DISPID id, DISPID *pid);
    HRESULT CreateSafeArrayOfPropertiesWithScriptEnter(VARIANT* pvarRes);

    HRESULT QueryObjectInterface(REFIID iid, void** obj);
    BOOL HasInstanceWithScriptEnter(Var instance);

    Js::PropertyId GetEnumeratorCurrentPropertyId();
    HRESULT CreateSafeArrayOfProperties(__out VARIANT* pvarRes);
    HRESULT GetTargetScriptSite(__in Js::RecyclableObject* obj, __out ScriptSite** targetScriptSite);

    void RemoveFromDispatchMap();

    void SetIsInCall() {Assert(!isInCall); isInCall = true; }
    void ResetIsInCall() { isInCall = false; }
    bool IsInCall() const { return isInCall; }
    HRESULT VerifyOnEntry(bool isValidThreadScope);

    static const unsigned short k_dispAll = DISPATCH_METHOD | DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT |
        DISPATCH_PROPERTYPUTREF | DISPATCH_CONSTRUCT;
    static const unsigned short k_dispCallOrGet = DISPATCH_PROPERTYGET | DISPATCH_METHOD;
    static const unsigned short k_dispCallOrConstruct = DISPATCH_METHOD | DISPATCH_CONSTRUCT;
    static const unsigned short k_dispPutOrPutRef = DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF;

#ifdef TRACK_JS_DISPATCH

    void LogAlloc();
    void LogFree(bool isShutdown = false);

    struct TrackNode
    {
        JavascriptDispatch * javascriptDispatch;
        StackBackTrace * stackBackTrace;
        bool hadShutdown;
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
        StackBackTraceNode * refCountStackBackTraces;
#endif
        TrackNode* prev;
        TrackNode* next;
    };

    static const int StackTraceDepth = 20;
    static const int StackToSkip = 0;
    static CriticalSection s_cs;
    static TrackNode* allocatedList;

    TrackNode * trackNode;
#endif
#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
    static void PrintJavascriptRefCountStackTraces();
    friend class JavascriptDispatchLeakOutput;
#endif
#if DEBUG
    BOOL AssertValid(void)
    {
        AssertMem(this);
        return TRUE;
    }
#endif //DEBUG

};

class AutoActiveCallPointer
{
private:
    BOOL wasInCall;
    Js::RecyclableObject* localObject;
    JavascriptDispatch* javascriptDispatch;
public:
    AutoActiveCallPointer(JavascriptDispatch* currentDispatch):
        javascriptDispatch(currentDispatch)
    {
        wasInCall = currentDispatch->IsInCall();
        localObject = javascriptDispatch->GetObject();
        if (!wasInCall)
        {
            currentDispatch->SetIsInCall();
        }
    }

    ~AutoActiveCallPointer();
};
