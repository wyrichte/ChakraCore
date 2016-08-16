/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include <EnginePch.h>
#include "mshtmdid.h"
#include "DispIdHelper.h"
#include "typeinfobuilder.h"

#include "Types\DynamicObjectEnumerator.h"
#include "Types\DynamicObjectSnapshotEnumerator.h"
#include "Types\DynamicObjectSnapshotEnumeratorWPCache.h"
#include "Library\ForInObjectEnumerator.h"

// Useful constants:

#define	pszTLibFileName   OLESTR("JSDeb.tlb")
#define	pszTLibName       OLESTR("JScriptTypeLib")
#define	pszTLibDoc        OLESTR("JScript Type Library")
#define	pszTInfoDoc       OLESTR("JScript Type Info")

#if _WIN32 || _WIN64
const SYSKIND syskind = SYS_WIN32;
#elif _WIN16
const SYSKIND syskind = SYS_WIN16;
#else
#error Neither _WIN16, _WIN32, nor _WIN64 is defined
#endif

HRESULT GetStdOleTypeLib(ITypeLib **pptlib)
{
    AssertMem(pptlib);
    ITypeLib * ptlib;

    ptlib = NULL;
    *pptlib = NULL;

    HRESULT hr;
    // Try to load the v2 StdOLE type library.
    hr = LoadRegTypeLib(IID_StdOle, STDOLE2_MAJORVERNUM, STDOLE2_MINORVERNUM,
        STDOLE2_LCID, &ptlib);

    if (FAILED(hr))
        return hr;

    if (NULL == ptlib)
        return HR(E_FAIL);

    *pptlib = ptlib;
    return NOERROR;
}

HRESULT GetDispatchTypeInfo(ITypeInfo **ppti)
{
    AssertMem(ppti);
    *ppti = NULL;

    HRESULT hr;
    ITypeLib *ptlibStdOle;
    if (FAILED(hr = GetStdOleTypeLib(&ptlibStdOle)))
        return hr;
    hr = ptlibStdOle->GetTypeInfoOfGuid(__uuidof(IDispatch), ppti);
    ptlibStdOle->Release();
    return hr;
}

TypeInfoBuilder::TypeInfoBuilder()
{
    m_cvarFunc = 0;
    m_cvarVar = 0;
    m_pcti = NULL;
    m_pctl = NULL;
}

TypeInfoBuilder::~TypeInfoBuilder()
{
    if (NULL != m_pcti)
        m_pcti->Release();
    if (NULL != m_pctl)
        m_pctl->Release();
}

void TypeInfoBuilder::Release(void)
{
    delete this;
}

HRESULT TypeInfoBuilder::AddFunction(Js::Var javascriptObject,
                                     __in LPCOLESTR functionName, MEMBERID id)
{
    HRESULT hr = NOERROR;
    LPCOLESTR *paramNames;
    ELEMDESC *elemDesc;
    FUNCDESC funcdesc;
    
    Assert(Js::JavascriptFunction::Is(javascriptObject));
    Js::JavascriptFunction* javascriptFunction = Js::JavascriptFunction::FromVar(javascriptObject);
    Js::ScriptContext* scriptContext = javascriptFunction->GetScriptContext();
    int paramCount = Js::JavascriptConversion::ToInt32(Js::JavascriptOperators::OP_GetProperty(javascriptObject, Js::PropertyIds::length, scriptContext), scriptContext);

    // no further overflow check is needed after we can ensure paramCount is no larger than short
    if ((short)paramCount != paramCount)
    {
        return E_OUTOFMEMORY;
    }

    funcdesc.memid             = id;
    funcdesc.funckind          = FUNC_DISPATCH;
    funcdesc.invkind           = INVOKE_FUNC;
    funcdesc.lprgscode         = NULL;
    funcdesc.cScodes           = -1;
    funcdesc.callconv          = CC_STDCALL;
    funcdesc.cParamsOpt        = 0;
    funcdesc.oVft              = 0;
    funcdesc.wFuncFlags        = 0x00;
    funcdesc.cParams           = (SHORT)paramCount;
    funcdesc.lprgelemdescParam = NULL;

    memset(&funcdesc.elemdescFunc, 0x00, sizeof(ELEMDESC));
    funcdesc.elemdescFunc.tdesc.vt = VT_VARIANT;

    int allocSize;
    if (paramCount > 0)
    {
        allocSize = sizeof(ELEMDESC) * paramCount;
        elemDesc = (ELEMDESC *)_alloca(allocSize);

        memset(elemDesc, 0x00, allocSize);
        funcdesc.lprgelemdescParam = elemDesc;
        for (int i = 0 ; i < funcdesc.cParams ; ++i)
            elemDesc[i].tdesc.vt  = VT_VARIANT;
    }

    hr = m_pcti->AddFuncDesc(m_cvarFunc, &funcdesc);
    if (SUCCEEDED(hr))
    {
        allocSize = sizeof(LPWSTR) * (paramCount + 1);
        paramNames = (LPCOLESTR *)_alloca(allocSize);

        memset(paramNames, 0, allocSize);
        paramNames[0] = functionName;
        HashTbl *h = NULL;
        UTF8Scanner *scan = NULL;
        if (paramCount > 0)
        {
            // The parameter list in AST is not available after bytecode generator
            // just reparse it here.
            // We need to reparse it only if it's not built in and not a library function
            if(javascriptFunction->IsScriptFunction())
            {
                Js::FunctionProxy* pFuncInfo = javascriptFunction->GetFunctionInfo()->GetFunctionProxy();
                Js::Utf8SourceInfo* sourceInfo = pFuncInfo->GetUtf8SourceInfo();

                bool isLibraryCode = false;

                if (sourceInfo && !sourceInfo->GetIsLibraryCode()) 
                {
                    isLibraryCode = true;
                }

                if (!isLibraryCode)
                {
                    Js::ParseableFunctionInfo* pFuncBody = NULL;

                    // If the function is defer deserialized, need to deserialize it here
                    // If it's defer parse, that's fine since we still have the source info so 
                    // we can use it directly without parsing the whole function
                    // We could probably optimize the defer deserialize case later
                    if (pFuncInfo->IsDeferredDeserializeFunction())
                    {
                        pFuncBody = javascriptFunction->GetFunctionProxy()->EnsureDeserialized();
                    }
                    else
                    {
                        // Make sure that we're either defer parsed or not defered at all
                        Assert(pFuncInfo->IsDeferredParseFunction() || !pFuncInfo->IsDeferred());
                        pFuncBody = javascriptFunction->GetFunctionBody();
                    }

                    Token tok;
                    ErrHandler err;
                    h = HashTbl::Create(256, NULL);
                    scan = UTF8Scanner::Create(NULL, h, &tok, &err, scriptContext);
                    scan->SetText(pFuncBody->GetSource(_u("TypeInfoBuilder::AddFunction")), 0, pFuncBody->LengthInBytes(), 0, 0);
                    int params = 0;
                    scan->Scan();
                    while (tok.tk != tkEOF)
                    {
                        if (tok.tk == tkLParen) break;
                        scan->Scan();
                    }
                    while (scan->Scan() != tkEOF && params < paramCount)
                    {
                        switch (tok.tk)
                        {
                        case tkID:
                            paramNames[params + 1] = tok.GetIdentifier(h)->Psz();
                            params++;
                            continue;
                        case tkComma:
                            continue;
                        }

                        break;
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            // add this
            paramCount++;

            hr = m_pcti->SetFuncAndParamNames(m_cvarFunc, (LPOLESTR *)paramNames, paramCount);
            // if there is ambiguous name error returned from oleaut32, it means that we have case insensitive name
            // collision and tlb does not like it. To avoid failing the call all together and desotry c# dynamic functionality,
            // we can just ignore the case insensitive duplicated name as if it doesn't exist. It will cause missing property 
            // in generated type library but it's better than not generating it as well.
            if (hr == 0x8002802c)
            {
                hr = m_pcti->DeleteFuncDesc(m_cvarFunc);
                RELEASEPTR(h);
                RELEASEPTR(scan);
                return hr;
            }
        }

        RELEASEPTR(h);
        RELEASEPTR(scan);
    }

    if (FAILED(hr))
        return hr;

    ++m_cvarFunc;

    return NOERROR;
}


HRESULT TypeInfoBuilder::AddVar(__in LPCOLESTR pszName, MEMBERID id)
{
    VARDESC vardesc;
    HRESULT hr;

    vardesc.memid                = id;
    vardesc.elemdescVar.tdesc.vt = VT_VARIANT;
    vardesc.varkind              = VAR_DISPATCH;
    vardesc.lpstrSchema          = NULL;
    vardesc.wVarFlags            = 0x00;

    hr = m_pcti->AddVarDesc(m_cvarVar, &vardesc);
    if (FAILED(hr))
        return hr;

    hr = m_pcti->SetVarName(m_cvarVar, (LPOLESTR)pszName);
    if (FAILED(hr))
    {
        // if there is ambiguous name error returned from oleaut32, it means that we have case insensitive name
        // collision and tlb does not like it. To avoid failing the call all together and desotry c# dynamic functionality,
        // we can just ignore the case insensitive duplicated name as if it doesn't exist. It will cause missing property 
        // in generated type library but it's better than not generating it as well.
        if (hr == 0x8002802c)
        {
            hr = m_pcti->DeleteVarDesc(m_cvarVar);
            return hr;
        }
        return hr;
    }

    ++m_cvarVar;
    return hr;
}


HRESULT TypeInfoBuilder::AddItemToTypeInfo(Js::Var javascriptObject,
                                           __in LPCOLESTR pszName, MEMBERID id,
                                           BOOL fIsExternal)
{
    HRESULT hr;

    if (fIsExternal)
    {
        // We need to offset our DISPID return value to be in a known range for external objects.
        // See use of DISPID_EXPANDO_BASE for JavascriptDispatch
        id = PropertyIdToExpandoDispId(id);
    }

    // we don't really have function body etc. for trampoline anyhow, let's treat them as properties.
    if (Js::JavascriptOperators::GetTypeId(javascriptObject) == Js::TypeIds_Function &&
        !(Js::JavascriptObject::FromVar(javascriptObject))->IsExternal())
    {
        hr = AddFunction(javascriptObject, pszName, id);
    }
    else
    {
        hr = AddVar(pszName, id);
    }

    return hr;

}

Js::Var  TypeInfoBuilder::GetPropertyNoThrow(Js::DynamicObject* dynamicObject, Js::PropertyId propertyId, Js::ScriptContext* scriptContext)
{
    Js::Var prop;
    try
    {
        prop = Js::JavascriptOperators::OP_GetProperty(dynamicObject, propertyId, scriptContext);
    }
    catch(Js::JavascriptExceptionObject *)
    {
        return NULL;
    }
    return prop;
}
    
HRESULT TypeInfoBuilder::AddJavascriptObject(Js::DynamicObject* dynamicObject)
{
    HRESULT hr = NOERROR;

    m_cvarFunc = 0;
    m_cvarVar = 0;

    BOOL fExternal = dynamicObject->IsExternal();

    Js::ScriptContext* scriptContext = dynamicObject->GetScriptContext();
    Js::ForInObjectEnumerator forinEnumerator(dynamicObject, scriptContext);
   
    while (true)
    {
        Js::PropertyId propId = Js::Constants::NoProperty;
        Js::Var propertyIndex = forinEnumerator.MoveAndGetNext(propId);
        if (propertyIndex == nullptr)
        {
            break;
        }
        Js::JavascriptString *propertyName = Js::JavascriptString::FromVar(propertyIndex);
        Js::PropertyRecord const * propRecord;
        if (propId == Js::Constants::NoProperty)
        {
            scriptContext->GetOrAddPropertyRecord(propertyName->GetString(), propertyName->GetLength(), &propRecord);
            propId = propRecord->GetPropertyId();
        }

        if (dynamicObject->IsEnumerable(propId))
        {
            Js::Var prop = GetPropertyNoThrow(dynamicObject, propId, scriptContext);
            if (prop == NULL)
                continue;

            if (FAILED(hr = AddItemToTypeInfo(prop, propertyName->GetSz(), propId, fExternal)))
            {
                return hr;
            }
        }
    }
    return hr;
}

HRESULT TypeInfoBuilder::Create(__in LPCOLESTR pszName, LCID lcid,
                                TypeInfoBuilder * * ppbuilder)
{
    HRESULT hr = NOERROR;
    HREFTYPE hreftype;

    TypeInfoBuilder * pbuilder = NULL;
    ITypeInfo *ptiStdDisp = NULL;

    pbuilder = new TypeInfoBuilder;
    if (NULL == pbuilder)
    {
        hr = HR(E_OUTOFMEMORY);
        goto LFail;
    }

    ITypeLib *ptlibStdOle;
    if (FAILED(hr = GetStdOleTypeLib(&ptlibStdOle)))
    {
        goto LFail;
    }
    hr = ptlibStdOle->GetTypeInfoOfGuid(__uuidof(IDispatch), &ptiStdDisp);
    ptlibStdOle->Release();

    if (FAILED(hr))
        goto LFail;

#if _WIN32 || _WIN64
    hr = CreateTypeLib2(syskind, pszTLibFileName,
        (ICreateTypeLib2 **) &pbuilder->m_pctl);
#else
#error Neither _WIN16, _WIN32, nor _WIN64 is defined
#endif

    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pctl->SetLcid(lcid);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pctl->SetName(pszTLibName);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pctl->SetDocString(pszTLibDoc);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pctl->SetVersion(SCRIPT_ENGINE_MAJOR_VERSION, SCRIPT_ENGINE_MINOR_VERSION);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pctl->SetGuid(IID_IScriptTypeLib);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pctl->SetLibFlags(0x00);
    if (FAILED(hr))
        goto LFail;

    {
        CComPtr<ICreateTypeInfo> typeInfo = nullptr;
        hr = pbuilder->m_pctl->CreateTypeInfo((OLECHAR *)pszName,
            TKIND_DISPATCH, &typeInfo);
        if (FAILED(hr))
            goto LFail;
        hr = typeInfo->QueryInterface(__uuidof(ICreateTypeInfo2), (void**)&pbuilder->m_pcti);
        if (FAILED(hr))
            goto LFail;
    }

    hr = pbuilder->m_pcti->SetDocString(pszTInfoDoc);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pcti->SetGuid(IID_IScriptTypeInfo);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pcti->SetVersion(SCRIPT_ENGINE_MAJOR_VERSION, SCRIPT_ENGINE_MINOR_VERSION);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pcti->AddRefTypeInfo(ptiStdDisp, &hreftype);
    if (FAILED(hr))
        goto LFail;

    hr = pbuilder->m_pcti->AddImplType(0, hreftype);
    if (FAILED(hr))
        goto LFail;

    hr = NOERROR;
    *ppbuilder = pbuilder;
    pbuilder = NULL;

LFail:

    if (NULL != pbuilder)
        pbuilder->Release();
    if (NULL != ptiStdDisp)
        ptiStdDisp->Release();


    return hr;
}

HRESULT TypeInfoBuilder::GetTypeInfo(ITypeInfo * * ppti)
{
    AssertMem(ppti);

    HRESULT hr;
    ITypeInfo * pti = NULL;

    hr = m_pcti->QueryInterface(__uuidof(ITypeInfo), (void **)&pti);
    if (FAILED(hr))
        goto LFail;

    hr = m_pcti->LayOut();
    if (FAILED(hr))
        goto LFail;

#if WRITE_TYPELIB
    // Save the type library to disk for debugging purposes.
    hr = m_pctl->SaveAllChanges();
    if (FAILED(hr))
        goto LFail;
#endif // WRITE_TYPELIB

    hr = NOERROR;
    *ppti = pti;
    pti = NULL;

LFail:

    if (NULL != pti)
        pti->Release();

    return hr;
}

