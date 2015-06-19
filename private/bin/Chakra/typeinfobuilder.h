//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once 
HRESULT GetStdOleTypeLib(ITypeLib **pptlib);

class TypeInfoBuilder
{
private:
    // Member data
    int m_cvarFunc;
    int m_cvarVar;
    ICreateTypeInfo2 * m_pcti;
    ICreateTypeLib2 * m_pctl;

    // Member functions
    HRESULT AddItemToTypeInfo(Js::Var var, __in LPCOLESTR pszName, MEMBERID id, BOOL fIsExternal);
    HRESULT AddFunction(Js::Var var, __in LPCOLESTR pszName, MEMBERID id);

    TypeInfoBuilder();
    ~TypeInfoBuilder();
    Js::Var  GetPropertyNoThrow(Js::DynamicObject* dynamicObject, Js::PropertyId propertyId, Js::ScriptContext* scriptContext);

public:

    static HRESULT Create(__in LPCOLESTR pszName, LCID lcid,
        TypeInfoBuilder ** ppbuilder);
    void Release(void);
    HRESULT AddJavascriptObject(Js::DynamicObject *javascriptObject);
    HRESULT AddVar(__in LPCOLESTR pszName, __in MEMBERID id);
    HRESULT GetTypeInfo(ITypeInfo** ppti);
};


