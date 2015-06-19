//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class ScriptEngine;

class EventSink sealed : public IDispatch
{
public:

    static HRESULT Create(EventSink **ppsink, ScriptEngine *pos,
        IDispatch* pdisp, ITypeInfo *ptiCoClass);

    /****************************************
    IUnknown methods.
    ****************************************/
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /****************************************
    IDispatch methods.
    ****************************************/
    STDMETHOD(GetTypeInfoCount)(unsigned int *pctinfo);
    STDMETHOD(GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo **pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, __in_ecount(cpsz) OLECHAR **prgpsz, uint cpsz,
        LCID lcid, DISPID *prgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid,
        ushort wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
        EXCEPINFO *pexcepinfo, uint *puArgErr);

    /****************************************
    Event set management.
    ****************************************/
    // Connect the event set to the IConnectionPoint of the source
    HRESULT    Connect(void);

    // Disconnect the event set from the connection point
    HRESULT Disconnect(void);

    IDispatch* GetDispatch()      { return m_pdisp; }
    BOOL RegisterEventHandler(__in LPCOLESTR pszEvt, __in Js::DynamicObject *scriptObject);

private:
    struct EventInfo
    {
        DISPID dispID;            // DISPID of the event.
        BSTR bstrName;        // Name of the event.
        RecyclerRootPtr<Js::DynamicObject> scriptObject;    // Entry point.
    };

    class EventInfoComparer
    {
    public:
        static bool Equals(EventSink::EventInfo* src, EventSink::EventInfo* dst);    
        static int Compare(EventSink::EventInfo* src, EventSink::EventInfo* dst);
    };

private:        
    long m_refCount;
    ScriptEngine *m_pos;
    IID m_iid; // IID of the event set
    DWORD m_dwCookie; // The sink's Advise cookie
    IDispatch *m_pdisp; // IDispatch of the control we're sinking
    IConnectionPoint *m_pconn; // The connection point we are advising on

    // Event table, sorted by dispid.
    typedef JsUtil::List<EventInfo*, ArenaAllocator, false, Js::CopyRemovePolicy, SpecializedComparer<EventInfo *, EventInfoComparer>::TComparerType> EventInfoList;
    EventInfoList* eventInfoes;

    EventSink(void);
    ~EventSink(void);

    // Init the event set name and associated object

    HRESULT Init(ScriptEngine *pos, IDispatch* pdisp, ITypeInfo *ptiCoClass);

    // create the event set for this event sink
    HRESULT CreateEventSet(ITypeInfo *ptiCoClass);
    static HRESULT GetSourceTypeInfo(ITypeInfo *ptinfoCoClass, ITypeInfo **pptinfoSrc);

    // lookup an event by dispid
    HRESULT FindEvent(__in DISPID id, __deref_out EventInfo **ppevt);

    // sort the events by dispid
    void Sort(void);
};


