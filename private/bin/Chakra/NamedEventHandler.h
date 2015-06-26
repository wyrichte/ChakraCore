//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

struct NamedItem;
class CScriptBody;

class BaseEventHandler
{
public:
    virtual void AddRef(void) = 0;
    virtual void Release(void) = 0;
    virtual IDispatch *GetDispatch(void) = 0;
    virtual LPCOLESTR GetEventHandlerName(void) = 0;
    virtual Js::DynamicObject *GetScriptObject(void) = 0;
    virtual BOOL ShouldPersist(void) = 0;
    virtual HRESULT Reset(void) = 0;
    virtual HRESULT Clone(__in ScriptEngine *pos, __deref_out BaseEventHandler **ppeh) = 0;
    virtual CScriptBody * GetUnderlyingScript(){ return NULL; }
};


// Named Item Event Handler. Comes from a call to AddScriptlet.
class NamedEventHandler sealed : public BaseEventHandler
{
private:
    long refCount;
    ScriptEngine *m_pos;
    NamedItem *m_pnid;
    LPWSTR m_pszSubItem;
    LPWSTR eventHandlerName;
    CScriptBody *scriptBody;
    RecyclerRootPtr<Js::DynamicObject> scriptObject;
    IDispatch *m_pdisp;
    BOOL shouldPersist : 1;
    BOOL m_fTried : 1;

    NamedEventHandler(void);
    ~NamedEventHandler(void);

    HRESULT Init(ScriptEngine *pos, NamedItem *pnid, LPCOLESTR pszSubItem,
        LPCOLESTR pszEvt, CScriptBody *pbody, BOOL fPersist);

public:
    static HRESULT Create(__out NamedEventHandler **ppneh, __in ScriptEngine *pos,
        __in NamedItem *pnid, __in LPCOLESTR pszSubItem, __in LPCOLESTR pszEvt,
        __in CScriptBody *pbody, __in BOOL fPersist);

    virtual void AddRef(void)
    { refCount++; }
    virtual void Release(void)
    {
        if (0 == --refCount)
            delete this;
    }
    virtual IDispatch *GetDispatch(void) override;
    virtual LPCOLESTR GetEventHandlerName(void)    { return eventHandlerName; }
    virtual Js::DynamicObject *GetScriptObject(void) override;
    virtual BOOL ShouldPersist(void)   { return shouldPersist; }
    virtual HRESULT Reset(void);
    virtual HRESULT Clone(__in ScriptEngine *pos, __deref_out BaseEventHandler **ppeh);

    virtual CScriptBody * GetUnderlyingScript(){ return scriptBody; }
};
