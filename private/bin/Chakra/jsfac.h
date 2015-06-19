//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

// Description: defs for the jscript class factory

class CJScript9ClassFactory : public CClassFactory
{
public:
    CJScript9ClassFactory(void);

    inline REFIID GetTypeId(void) sealed { return CLSID_Chakra; }

    STDMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvObj);
protected:
    virtual ScriptEngine* AllocateEngine(__in LPOLESTR szLangName);
};

class CJScript9ThreadServiceClassFactory : public CClassFactory
{
public:
    CJScript9ThreadServiceClassFactory(void);

    inline REFIID GetTypeId(void) { return CLSID_ChakraThreadService; }

    STDMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvObj);
protected:
};

