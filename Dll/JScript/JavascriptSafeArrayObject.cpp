//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
namespace Js
{

FunctionInfo JavascriptSafeArrayObject::EntryInfo::NewInstance(JavascriptSafeArrayObject::NewInstance, FunctionInfo::SkipDefaultNewObject);
FunctionInfo JavascriptSafeArrayObject::EntryInfo::ToLbound(JavascriptSafeArrayObject::EntryToLbound, FunctionInfo::ErrorOnNew);
FunctionInfo JavascriptSafeArrayObject::EntryInfo::ToUbound(JavascriptSafeArrayObject::EntryToUbound, FunctionInfo::ErrorOnNew);
FunctionInfo JavascriptSafeArrayObject::EntryInfo::ToDimensions(JavascriptSafeArrayObject::EntryToDimensions, FunctionInfo::ErrorOnNew);
FunctionInfo JavascriptSafeArrayObject::EntryInfo::ToGetItem(JavascriptSafeArrayObject::EntryToGetItem, FunctionInfo::ErrorOnNew);
FunctionInfo JavascriptSafeArrayObject::EntryInfo::ToArray(JavascriptSafeArrayObject::EntryToArray, FunctionInfo::ErrorOnNew);
FunctionInfo JavascriptSafeArrayObject::EntryInfo::ValueOf(JavascriptSafeArrayObject::EntryValueOf, FunctionInfo::ErrorOnNew);

void JavascriptSafeArrayObject::InitPrototype(ScriptSite* scriptSite)
{
    Assert(!m_safeArray);
    m_safeArray = scriptSite->CreateJavascriptSafeArray(VT_VARIANT, null);
}

Var JavascriptSafeArrayObject::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ScriptContext* scriptContext = function->GetScriptContext();

    // SkipDefaultNewObject function flag should have revent the default object
    // being created, except when call true a host dispatch
    Assert(!(callInfo.Flags & CallFlags_New) || args[0] == null
        || JavascriptOperators::GetTypeId(args[0]) == TypeIds_HostDispatch);

    if(args.Info.Count < 2)
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedVBArray, L"VBArray");
    }

    Var result;
    if (!JavascriptSafeArray::Is(args[1]))
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedVBArray, L"VBArray");
    }
    JavascriptSafeArray* safeArrayObj = JavascriptSafeArray::FromVar(args[1]);


    if (!(callInfo.Flags & CallFlags_New))
    {
        //
        // Called as a function. In V5.8 a copy of the safe array is created
        //
        result = safeArrayObj->GetScriptSite()->CreateJavascriptSafeArray(safeArrayObj->GetSafeArrayVarType(), safeArrayObj->GetSafeArray());

    }
    else
    {
        // called as a new, create a wrap object. We avoid a safe array copy.
        result = safeArrayObj->GetScriptSite()->CreateJavascriptSafeArrayObject(safeArrayObj);
    }
    return result;
}

Var JavascriptSafeArrayObject::EntryToLbound(RecyclableObject* function, CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ScriptContext* scriptContext = function->GetScriptContext();

    Assert(!(callInfo.Flags & CallFlags_New)); 

    if (args.Info.Count > 0)
    {
        SAFEARRAY * psa;
        switch (JavascriptOperators::GetTypeId(args[0]))
        {
        case TypeIds_SafeArray:
            psa = JavascriptSafeArray::FromVar(args[0])->GetSafeArray();
            break;

        case TypeIds_SafeArrayObject:
            psa = JavascriptSafeArrayObject::FromVar(args[0])->GetJavascriptSafeArray()->GetSafeArray();
            break;

        default:
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.lbound");
        }

        ulong iDim = 0; 

        if (args.Info.Count > 1)
        {
            int32 dimValue;
            if ( !JavascriptConversion::ToInt32Finite(args[1], scriptContext, &dimValue))
            {
                JavascriptError::ThrowRangeError(scriptContext, VBSERR_OutOfBounds);
            }
            iDim = dimValue - 1;
        }

        if (psa == 0 || iDim >= psa->cDims)
        {
            JavascriptError::ThrowRangeError(scriptContext, VBSERR_OutOfBounds);
        }

        long result = psa->rgsabound[psa->cDims - iDim - 1].lLbound;
        return JavascriptNumber::ToVar(result, scriptContext);

    }
    else
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.lbound");
    }
}

Var JavascriptSafeArrayObject::EntryToUbound(RecyclableObject* function, CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ScriptContext* scriptContext = function->GetScriptContext();

    Assert(!(callInfo.Flags & CallFlags_New)); 

    if (args.Info.Count > 0)
    {
        SAFEARRAY * psa;
        switch (JavascriptOperators::GetTypeId(args[0]))
        {
        case TypeIds_SafeArray:
            psa = JavascriptSafeArray::FromVar(args[0])->GetSafeArray();
            break;

        case TypeIds_SafeArrayObject:
            psa = JavascriptSafeArrayObject::FromVar(args[0])->GetJavascriptSafeArray()->GetSafeArray();
            break;

        default:
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.ubound");
        }

        ulong iDim = 0; 

        if (args.Info.Count > 1)
        {
            int32 dimValue;
            if ( !JavascriptConversion::ToInt32Finite(args[1], scriptContext, &dimValue))
            {
                JavascriptError::ThrowRangeError(scriptContext, VBSERR_OutOfBounds);
            }
            iDim = dimValue - 1;
        }

        if (psa == 0 || iDim >= psa->cDims)
        {
            JavascriptError::ThrowRangeError(scriptContext, VBSERR_OutOfBounds);
        }

        long result = psa->rgsabound[psa->cDims - iDim - 1].lLbound +
                        psa->rgsabound[psa->cDims - iDim - 1].cElements - 1;
        return JavascriptNumber::ToVar(result, scriptContext);

    }
    else
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.ubound");
    }
}

Var JavascriptSafeArrayObject::EntryToDimensions(RecyclableObject* function, CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ScriptContext* scriptContext = function->GetScriptContext();

    Assert(!(callInfo.Flags & CallFlags_New)); 

    if (args.Info.Count > 0)
    {
        SAFEARRAY * psa;
        switch (JavascriptOperators::GetTypeId(args[0]))
        {
        case TypeIds_SafeArray:
            psa = JavascriptSafeArray::FromVar(args[0])->GetSafeArray();
            break;

        case TypeIds_SafeArrayObject:
            psa = JavascriptSafeArrayObject::FromVar(args[0])->GetJavascriptSafeArray()->GetSafeArray();
            break;

        default:
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.dimensions");
        }

        return JavascriptNumber::ToVar((psa == 0) ? 0 : psa->cDims, scriptContext);        
    }
    else
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.dimensions");
    }
}

Var JavascriptSafeArrayObject::EntryToGetItem(RecyclableObject* function, CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ScriptContext* scriptContext = function->GetScriptContext();

    Assert(!(callInfo.Flags & CallFlags_New)); 

    if (args.Info.Count <= 0)
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.getItem");
    }

    // we have some dimension args
    JavascriptSafeArray* javascriptSafeArray;
    SAFEARRAY * psa;

    //get this arg , psa and active script site
    switch (JavascriptOperators::GetTypeId(args[0]))
    {
    case TypeIds_SafeArray:
        javascriptSafeArray = JavascriptSafeArray::FromVar(args[0]);
        break;

    case TypeIds_SafeArrayObject:
        javascriptSafeArray = JavascriptSafeArrayObject::FromVar(args[0])->GetJavascriptSafeArray();
        break;

    default:
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.getItem");
    }
    psa = javascriptSafeArray->GetSafeArray();
    if (psa == 0)
    {
        JavascriptError::ThrowRangeError(scriptContext, VBSERR_OutOfBounds);
    }

    // prepare the array of arguments for SafeArrayGetElement
    const int dimMax = 10;
    long autoIndexArray[dimMax];
    long*  heapIndexArray = NULL;

    long *indexArray = autoIndexArray;

    uint indexCount = args.Info.Count - 1;

    if (psa->cDims != indexCount)
    {
        JavascriptError::ThrowRangeError(scriptContext, VBSERR_OutOfBounds);
    }

    if (indexCount > dimMax)
    {
        uint nAlloc = UInt32Math::Mul(indexCount, sizeof(long), Js::Throw::OutOfMemory);
        heapIndexArray = (long *) malloc(nAlloc);
        if (NULL == heapIndexArray)
        {
            JavascriptError::ThrowOutOfMemoryError(scriptContext);
        }
        indexArray = heapIndexArray;
    }

    HRESULT hr = S_OK;
    VARIANT varT;
    VariantInit(&varT);

    for (uint k = 0 ; k < indexCount ; ++k)
    {
        int32 indexValue;
        if ( !JavascriptConversion::ToInt32Finite(args[k+1], scriptContext, &indexValue))
        {
            hr = VBSERR_OutOfBounds;
            goto LFail;
        }
        indexArray[k] = indexValue;
    }

    VARTYPE vtElement = javascriptSafeArray->GetSafeArrayVarType() & ~VT_ARRAY;
    Assert((vtElement == VT_VARIANT && psa->cbElements == sizeof(VARIANT))
        || (vtElement == VT_UI1 && psa->cbElements == sizeof(BYTE)));

    //call SafeArrayGetElement and marshal the result
    switch (vtElement)
    {
    case VT_UI1:
        varT.vt = vtElement;
        hr = SafeArrayGetElement(psa, indexArray, &varT.bVal);
        break;

    case VT_VARIANT:
        hr = SafeArrayGetElement(psa, indexArray, &varT);
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }
    if (FAILED(hr))
    {
        goto LFail;
    }

    Var var = NULL;
    hr = DispatchHelper::MarshalVariantToJsVar(&varT, &var, javascriptSafeArray->GetScriptContext());
    if (FAILED(hr))
    {
        goto LFail;
    }

    //Successful get element
    VariantClear(&varT);
    if (NULL != heapIndexArray)
    {
        free(heapIndexArray);
    }
    return var;

LFail:
    VariantClear(&varT);
    if (NULL != heapIndexArray)
    {
        free(heapIndexArray);
    }
    JavascriptError::MapAndThrowError(scriptContext, hr);
}

//JsVBArrayToArray converts a VB-style safe array to the equivalent
//JScript Array object.  All multi-dimensional stuff is "flattened" --
//we treat the safe array as a big linear block of data.

Var JavascriptSafeArrayObject::EntryToArray(RecyclableObject* function, CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ScriptContext* scriptContext = function->GetScriptContext();

    Assert(!(callInfo.Flags & CallFlags_New)); 

    if (args.Info.Count <= 0)
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.toArray");
    }

    // we have some dimension args
    JavascriptSafeArray* javascriptSafeArray;
    SAFEARRAY * psa;

    //get this arg , psa and active script site
    switch (JavascriptOperators::GetTypeId(args[0]))
    {
    case TypeIds_SafeArray:
        javascriptSafeArray = JavascriptSafeArray::FromVar(args[0]);
        break;

    case TypeIds_SafeArrayObject:
        javascriptSafeArray = JavascriptSafeArrayObject::FromVar(args[0])->GetJavascriptSafeArray();
        break;

    default:
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.toArray");
    }
    psa = javascriptSafeArray->GetSafeArray();
    uint len = javascriptSafeArray->GetSafeArrayLength();
    JavascriptArray* jsArray = scriptContext->GetLibrary()->CreateArray(len);

    Var undefined = javascriptSafeArray->GetScriptContext()->GetLibrary()->GetUndefined();

    VARTYPE vtElement = javascriptSafeArray->GetSafeArrayVarType() & ~VT_ARRAY;
    for (uint k = 0; k < len ; k++)
    {
        Var var = NULL;
        VARIANT varT;
        switch (vtElement)
        {
        case VT_UI1:
            varT.vt = vtElement;
            varT.bVal = ((BYTE*)psa->pvData)[k];
            break;

        case VT_VARIANT:
            varT = ((VARIANT*)psa->pvData)[k];
            break;

        default:
            JavascriptError::MapAndThrowError(scriptContext, E_NOTIMPL); // We don't support other SafeArray
            break;
        }
        
        HRESULT hr = DispatchHelper::MarshalVariantToJsVar(&varT, &var, javascriptSafeArray->GetScriptContext());
        if (FAILED(hr))
        {
            JavascriptError::MapAndThrowError(scriptContext, hr);
        }

        if (var != undefined) //IE8 ignores VT_EMPTY
        {
            jsArray->DirectSetItemAt(k, var);
        }
    }
    return jsArray;
}

Var JavascriptSafeArrayObject::EntryValueOf(RecyclableObject* function, CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ScriptContext* scriptContext = function->GetScriptContext();

    Assert(!(callInfo.Flags & CallFlags_New)); 

    if (args.Info.Count <= 0)
    {
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.valueOf");
    }

    JavascriptSafeArray* javascriptSafeArray;

    //get this arg 
    switch (JavascriptOperators::GetTypeId(args[0]))
    {
    case TypeIds_SafeArray:
        javascriptSafeArray = JavascriptSafeArray::FromVar(args[0]);
        break;

    case TypeIds_SafeArrayObject:
        javascriptSafeArray = JavascriptSafeArrayObject::FromVar(args[0])->GetJavascriptSafeArray();
        break;

    default:
        JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedVBArray, L"VBArray.prototype.valueOf");
    }
    return javascriptSafeArray;
}
}



