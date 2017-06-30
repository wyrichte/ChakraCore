/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#include "guids.h"
#include "var.h"
#include "Library\JavascriptSymbol.h"
#include "Library\JavascriptVariantDate.h"

using namespace PlatformAgnostic;

const long kcchMaxConstBstr = 15;
struct ConstBstr
{
    // The layout of this parallels a variant.
    short vt;
    short sw1;
    short sw2;
    short sw3;
    const OLECHAR *psz;
    long lw;

    VARIANT *Pvar(void) { return (VARIANT *)this; }
};

/***************************************************************************
Creates a static constant bstr.
***************************************************************************/
#define StaticBstr(name, str) \
    struct StaticBstr_##name { \
        ULONG cb; \
        OLECHAR sz[sizeof(str)]; \
    }; \
    const StaticBstr_##name name = { \
    sizeof(OLESTR(str)) - sizeof(OLECHAR), \
    OLESTR(str) \
};

#define DefConstBstr(name, str) \
    StaticBstr(g_sbstr_##name, str) \
    ConstBstr NEAR cbstr##name = \
    { \
        VT_BSTR, 0, 0, 0, \
        g_sbstr_##name.sz, 0 \
    };

DefConstBstr(Undefined, "undefined")
DefConstBstr(Empty, "")
DefConstBstr(Null, "null")
DefConstBstr(True, "true")
DefConstBstr(False, "false")
DefConstBstr(Inf, "Infinity")
DefConstBstr(NegInf, "-Infinity")
DefConstBstr(NaN, "NaN")

/***************************************************************************
Misc BSTR functions. The parameters are LPCOLESTR instead of BSTR so
we can pass const things.
***************************************************************************/
inline long CbBstr(LPCOLESTR bstr)
{
    if (bstr == NULL)
        return 0;
    ULONG len = ((ULONG *)bstr)[-1];
    return (long)len;
}

inline long CchRawBstr(LPCOLESTR bstr)
{
    if (bstr == NULL)
        return 0;
    ULONG len = ((ULONG *)bstr)[-1] / sizeof(OLECHAR);
    return (long)len;
}

BOOL FNumber(VARIANT *pvar){return (pvar->vt == VT_I4 || pvar->vt == VT_R8);}

// This is here instead of in DispatchHelper to avoid referencing DateImplementation in the header
class DispatchHelperInternal
{
public:
    static HRESULT GetDateDefaultStringBstr(VARIANT *pvarRes, DateTime::YMD *pymd, Js::DateImplementation::TZD *ptzd, ulong noDateTime, Js::ScriptContext *scriptContext);
};

//-----------------------------------------------------------------------------------
//
// MarshalJsVarsToVariants
//
// Put the Var values into the array of VARIANTs, clearing the ones we've marshaled
// if we fail before we're done.
//
//-----------------------------------------------------------------------------------

HRESULT DispatchHelper::MarshalJsVarsToVariants(Js::Var *pAtom, VARIANT *pVar, int count)
{
    AssertMem(pVar);
    AssertMem(pAtom);

    HRESULT hr = S_OK;
    int i;

    for (i = 0; i < count; i++)
    {
        hr = MarshalJsVarToVariant(pAtom[count - i - 1], &pVar[i]);
        if (FAILED(hr))
        {
            int j;
            for (j = 0; j < i; j++)
            {
                VariantClear(&pVar[j]);
            }
            return hr;
        }
    }

    return hr;
}

HRESULT DispatchHelper::MarshalJsVarsToVariantsNoThrow(Js::Var *pAtom, VARIANT *pVar, int count)
{
    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        MarshalJsVarsToVariants(pAtom, pVar, count);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

void DispatchHelper::MarshalJsVarToDispatchVariant(Js::Var var,VARIANT *pVar)
{
    //
    // For the DynamicObject types, create a proxy, hook it to
    // the object, and return as an IDispatch.
    //

    JavascriptDispatch*   jsdisp;
    Js::DynamicObject  *obj = Js::DynamicObject::FromVar(var);
    jsdisp = JavascriptDispatch::Create<false>(obj);
    AssertMsg(jsdisp->GetObject() == var, "Bad dispatch map entry");
    pVar->vt = VT_DISPATCH;
    HRESULT hr = jsdisp->QueryInterface(__uuidof(IDispatchEx), (void**)&pVar->pdispVal);
    Assert(hr == S_OK);
}

HRESULT DispatchHelper::MarshalJsVarToVariantNoThrowWithLeaveScript(Js::Var var, VARIANT * pVar, Js::ScriptContext * scriptContext)
{
    HRESULT hr = S_OK;
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        MarshalJsVarToVariantNoThrow(var, pVar, scriptContext);
    }
    END_LEAVE_SCRIPT(scriptContext);
    return hr;
}

HRESULT DispatchHelper::MarshalJsVarToVariantNoThrow(Js::Var var, VARIANT * pVar, Js::ScriptContext * scriptContext)
{
    HRESULT hr = S_OK;
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        hr = MarshalJsVarToVariant(var, pVar);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

//-----------------------------------------------------------------------------------
//
// MarshalJsVarToVariant
//
// Put the given Var value into the given VARIANT.
//
//-----------------------------------------------------------------------------------

HRESULT DispatchHelper::MarshalJsVarToVariant(Js::Var var,VARIANT *pVar)
{
    HRESULT hr = S_OK;

    AssertMem(pVar);
    Assert(var != nullptr);

    VariantInit(pVar);
    if (var)
    {
        Js::JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray<Js::Var>(var);
        switch (Js::JavascriptOperators::GetTypeId(var))
        {
            case Js::TypeIds_Undefined:
            {
                // The undefined var.
                pVar->vt = VT_EMPTY;
                break;
            }

            case Js::TypeIds_Null:
            {
                // The undefined var.
                pVar->vt = VT_NULL;
                break;
            }

            case Js::TypeIds_Boolean:
            {
                //
                // Pass Boolean as BOOL
                //
                pVar->vt = VT_BOOL;
                pVar->boolVal = Js::JavascriptBoolean::FromVar(var)->GetValue() ? VARIANT_TRUE : VARIANT_FALSE;
                break;
            }

            case Js::TypeIds_Integer:
            {
                //
                // Pass Smi as I4.
                //
                pVar->vt = VT_I4;
                pVar->lVal = Js::TaggedInt::ToInt32(var);
                break;
            }

            // int64 and uint64 are ie10/win8 only; downgrade to double for legacy interface
            // ok to lose precision here.
            case Js::TypeIds_Int64Number:
            {
                pVar->vt = VT_R8;
                pVar->dblVal = static_cast<double>(Js::JavascriptInt64Number::FromVar(var)->GetValue());
                break;
            }

            case Js::TypeIds_UInt64Number:
            {
                pVar->vt = VT_R8;
                pVar->dblVal = static_cast<double>(Js::JavascriptUInt64Number::FromVar(var)->GetValue());
                break;
            }

            case Js::TypeIds_Number:
            {
                //
                // Pass Number as double.
                //
                pVar->vt = VT_R8;
                pVar->dblVal = Js::JavascriptNumber::GetValue(var);
                break;
            }

            case Js::TypeIds_String:
            {
                Js::JavascriptString *str = Js::JavascriptString::FromVar(var);
                if (str == str->GetLibrary()->GetNullString())
                {
                    pVar->bstrVal = NULL;
                }
                else
                {
                    pVar->bstrVal = SysAllocStringLen(str->GetString(), str->GetLength());
                    if (pVar->bstrVal == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                pVar->vt = VT_BSTR; // Change vt after bstrVal has been set to prevent invalid variant
                break;
            }

            case Js::TypeIds_Object:
            case Js::TypeIds_Function:
            case Js::TypeIds_Array:
            case Js::TypeIds_NativeIntArray:
            case Js::TypeIds_NativeFloatArray:
            case Js::TypeIds_ES5Array:
            case Js::TypeIds_Date:
            case Js::TypeIds_WinRTDate:
            case Js::TypeIds_RegEx:
            case Js::TypeIds_Error:
            case Js::TypeIds_BooleanObject:
            case Js::TypeIds_NumberObject:
            case Js::TypeIds_StringObject:
            case Js::TypeIds_GlobalObject:
            case Js::TypeIds_ModuleRoot:
            case Js::TypeIds_Arguments:
            case Js::TypeIds_ActivationObject:
            case Js::TypeIds_Int8Array:
            case Js::TypeIds_Uint8Array:
            case Js::TypeIds_Uint8ClampedArray:
            case Js::TypeIds_Int16Array:
            case Js::TypeIds_Uint16Array:
            case Js::TypeIds_Int32Array:
            case Js::TypeIds_Uint32Array:
            case Js::TypeIds_Float32Array:
            case Js::TypeIds_Float64Array:
            case Js::TypeIds_ArrayBuffer:
            case Js::TypeIds_DataView:
            case Js::TypeIds_Int64Array:
            case Js::TypeIds_Uint64Array:
            case Js::TypeIds_CharArray:
            case Js::TypeIds_BoolArray:
            case Js::TypeIds_Map:
            case Js::TypeIds_Set:
            case Js::TypeIds_WeakMap:
            case Js::TypeIds_WeakSet:
            case Js::TypeIds_SymbolObject:
            case Js::TypeIds_Proxy:
            case Js::TypeIds_ArrayIterator:
            case Js::TypeIds_MapIterator:
            case Js::TypeIds_SetIterator:
            case Js::TypeIds_StringIterator:
            case Js::TypeIds_Generator:
            case Js::TypeIds_Promise:
            {
                MarshalJsVarToDispatchVariant(var, pVar);
                break;
            }

            case Js::TypeIds_Symbol:
            {
                Js::JavascriptSymbol* sym = Js::JavascriptSymbol::FromVar(var);
                Js::JavascriptString* str = Js::JavascriptSymbol::ToString(sym->GetValue(), sym->GetScriptContext());

                if (str == str->GetLibrary()->GetNullString())
                {
                    pVar->bstrVal = NULL;
                }
                else
                {
                    pVar->bstrVal = SysAllocStringLen(str->GetString(), str->GetLength());
                    if (pVar->bstrVal == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                pVar->vt = VT_BSTR; // Change vt after bstrVal has been set to prevent invalid variant

                break;
            }

            case Js::TypeIds_VariantDate:
            {
                pVar->vt = VT_DATE;
                pVar->dblVal = Js::JavascriptVariantDate::FromVar(var)->GetValue();
                break;
            }

            case Js::TypeIds_HostDispatch:
            {
                Js::RecyclableObject  *obj = Js::RecyclableObject::FromVar(var);
                HostDispatch *pExternal = static_cast<HostDispatch*>(obj);
                IDispatch* dispatch = pExternal->GetDispatch();
                if (dispatch != NULL)
                {
                    pVar->vt = VT_DISPATCH;
                    pVar->pdispVal = dispatch;
                }
                else
                {
                    VARIANT* hostVariant = pExternal->GetVariant();
                    if (NULL == hostVariant)
                    {
                        hr = MapHr(E_ACCESSDENIED);
                    }
                    else
                    {
                        hr = VariantCopy(pVar, hostVariant);
                    }
                }
                break;
            }

            /* These are not user visible objects, they should get passed the host
                unless the runtime is doing so, which currently it doesn't */
            case Js::TypeIds_Enumerator:
            case Js::TypeIds_HostObject:
            default:
            {
                Js::RecyclableObject* jsInstance = Js::RecyclableObject::FromVar(var);
                if (jsInstance->IsExternal())
                {
                    MarshalJsVarToDispatchVariant(var, pVar);
                }
                else
                {
                    AssertMsg(FALSE, "Var type not supported as value returned to host");
                    hr = E_NOTIMPL;
                }
                break;
            }
        }
    }
    else
    {
        pVar->vt = VT_NULL;
    }
    return hr;
}

HRESULT DispatchHelper::MarshalFrameDisplayToVariantNoScript(Js::FrameDisplay *pDisplay, VARIANT *pVar)
{
    Assert(!ThreadContext::GetContextForCurrentThread()->IsScriptActive());
    Js::Var *pJsVar = (Js::Var*)((char*)pDisplay + Js::FrameDisplay::GetOffsetOfScopes());
    return MarshalJsVarsToVariants(pJsVar, pVar, pDisplay->GetLength());
}

HRESULT DispatchHelper::MarshalDispParamToArgumentsNoThrowNoScript(
    __in DISPPARAMS* dispParams,
    __in Js::Var thisPointer,
    __in Js::ScriptContext * scriptContext,
    __in Js::RecyclableObject *callee,
    __out Js::Arguments* argument)
{
    HRESULT hr = NOERROR;
    Js::Var* vars;
    int cvar = dispParams->cArgs;
    BOOL hasThis = DispParamsContainThis(dispParams);
    argument->Info.Count = cvar;
    if (!hasThis)
    {
        argument->Info.Count++;
    }
    BEGIN_TRANSLATE_OOM_TO_HRESULT
    {
        vars = RecyclerNewArrayZ(scriptContext->GetRecycler(), Js::Var, argument->Info.Count);
        argument->Values = vars;

        int start = 0;
        if (hasThis)
        {
            hr = MarshalVariantToJsVar(&dispParams->rgvarg[0], &vars[0], scriptContext);
            start++;
        }
        else
        {
            vars[0] = thisPointer;
        }
        if (FAILED(hr))
        {
            return hr;
        }

        if (start < cvar)
        {
            // Check to see if the last incoming arg is a frame display. It is if the type is SAFEARRAY.
            // We used to verify that the call target was eval, but now JS functions are protected from accessing
            // the frame display.
            VARIANT *pVar = &dispParams->rgvarg[start];
            if (pVar->vt == VT_SAFEARRAY)
            {
                // Create a new frame display from the contents of the SAFEARRAY and tell the callee that
                // it's coming.
                hr = MarshalVariantToFrameDisplay(pVar, (Js::FrameDisplay**)&vars[argument->Info.Count - 1], scriptContext);
                uint flags = argument->Info.Flags | CallFlags_ExtraArg;
                argument->Info.Flags = (Js::CallFlags)flags;
                start++;
                vars--;
            }
        }

        if (SUCCEEDED(hr))
        {
            for (int i = start; i < cvar; i++)
            {
                hr = MarshalVariantToJsVar(&dispParams->rgvarg[i], &vars[start + argument->Info.Count - i - 1], scriptContext);
                if (FAILED(hr))
                {
                    return hr;
                }
            }
        }
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);

    return hr;
}

HRESULT DispatchHelper::MarshalVariantToFrameDisplay(VARIANT *pVar, Js::FrameDisplay **ppDisplay, Js::ScriptContext *scriptContext)
{
    Assert(pVar->vt == VT_SAFEARRAY);
    SAFEARRAY *pArray = pVar->parray;
    Assert(pArray && pArray->cDims == 1);

    uint16 length = (uint16)pArray->cbElements;
    if (length == 0)
    {
        *ppDisplay = (Js::FrameDisplay*)&Js::NullFrameDisplay;
        return NOERROR;
    }

    *ppDisplay = RecyclerNewPlus(scriptContext->GetRecycler(), length * sizeof(Js::Var), Js::FrameDisplay, length);
    Js::Var *pJsVar = (Js::Var*)((char*)*ppDisplay + Js::FrameDisplay::GetOffsetOfScopes());
    HRESULT hr;
    for (uint i = 0; i < length; i++)
    {
        hr = MarshalVariantToJsVar(&((VARIANT*)pArray->pvData)[i], &pJsVar[length - i - 1], scriptContext);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    return NOERROR;
}

Js::Var DispatchHelper::MarshalBSTRToJsVar(Js::ScriptContext * scriptContext, BSTR bstr)
{
    if (bstr)
    {
        return Js::JavascriptString::NewCopyBuffer(bstr, SysStringLen(bstr), scriptContext);
    }
    return scriptContext->GetLibrary()->GetNullString();
}

HRESULT DispatchHelper::MarshalIDispatchToJsVar(Js::ScriptContext* scriptContext, IDispatch * pdispVal, Js::Var * var)
{
    CComPtr<IJavascriptDispatchLocalProxy> pProxy;
    HRESULT hr = NOERROR;

    {
        AUTO_NO_EXCEPTION_REGION;
        hr = pdispVal->QueryInterface(__uuidof(IJavascriptDispatchLocalProxy), (void**)&pProxy);
    }

    if (FAILED(hr) || !pProxy)
    {
        *var = HostDispatch::Create(scriptContext, pdispVal, FALSE);
        hr = NOERROR;
    }
    else
    {
        JavascriptDispatch* javascriptDispatch = static_cast<JavascriptDispatch*>((IJavascriptDispatchLocalProxy*)pProxy);
        Js::DynamicObject *obj = javascriptDispatch->GetObject();

        if (NULL == obj)
        {
            hr = E_ACCESSDENIED;
        }
        else
        {
            *var = Js::CrossSite::MarshalVar(scriptContext, obj);
            hr = NOERROR;
        }
    }
    return hr;
}

HRESULT DispatchHelper::MarshalVariantToJsVarDerefed(VARIANT *pVar, Js::Var *pAtom, Js::ScriptContext* scriptContext)
{
    HRESULT hr = S_OK;
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());
    switch (pVar->vt)
    {
        case VT_EMPTY:
        {
            *pAtom = scriptContext->GetLibrary()->GetUndefined();
            break;
        }

        case VT_NULL:
        {
            *pAtom = scriptContext->GetLibrary()->GetNull();
            break;
        }

        case VT_I1:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->cVal,scriptContext);
            break;
        }

        case VT_UI1:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->bVal,scriptContext);
            break;
        }

        case VT_I2:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->iVal,scriptContext);
            break;
        }

        case VT_UI2:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->uiVal,scriptContext);
            break;
        }

        case VT_I4:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->lVal,scriptContext);
            break;
        }

        case VT_UI4:
        {
            *pAtom = Js::JavascriptNumber::ToVar((uint32)pVar->ulVal,scriptContext);
            break;
        }

        // REVIEW: should we convert VT_I8 /VT_UI8 to int64Number?
        //  *pAtom = Js::JavascriptInt64Number::ToVar(pVar->llVal, scriptContext);
        case VT_I8:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->llVal,scriptContext);
            break;
        }

        case VT_UI8:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->ullVal,scriptContext);
            break;
        }

        case VT_INT:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->intVal,scriptContext);
            break;
        }

        case VT_UINT:
        {
            *pAtom = Js::JavascriptNumber::ToVar(pVar->uintVal,scriptContext);
            break;
        }

        case VT_R4:
        {
            // Don't use ToVar here, as it may create a tagged int from the fltVal, and the host may
            // expect to receive a float if we have to pass it back. (IEBVT: #1096)
            *pAtom = Js::JavascriptNumber::NewWithCheck(pVar->fltVal, scriptContext);
            break;
        }

        case VT_R8:
        {
            // Don't use ToVar here, as it may create a tagged int from the fltVal, and the host may
            // expect to receive a float if we have to pass it back. (IEBVT: #1096)
            *pAtom = Js::JavascriptNumber::NewWithCheck(pVar->dblVal, scriptContext);
            break;
        }

        case VT_BOOL:
        {
            *pAtom = Js::JavascriptBoolean::ToVar(pVar->boolVal != VARIANT_FALSE, scriptContext);
            break;
        }

        case VT_BSTR:
        {
            *pAtom = MarshalBSTRToJsVar(scriptContext, pVar->bstrVal);
            break;
        }

        case VTE_BYREF_BSTR:
        {
            *pAtom = MarshalBSTRToJsVar(scriptContext, *pVar->pbstrVal);
            break;
        }


        case VT_UNKNOWN:
        {
            if (pVar->punkVal == nullptr)
            {
                *pAtom = scriptContext->GetLibrary()->GetNull();
                break;
            }
            CComPtr<IDispatch> dispatch;
            {
                AUTO_NO_EXCEPTION_REGION;
                hr = pVar->punkVal->QueryInterface(__uuidof(IDispatch), (void**)&dispatch);
            }
            if (SUCCEEDED(hr) && dispatch)
            {
                *pAtom = HostDispatch::Create(scriptContext, (IDispatch*)dispatch);
            }
            else
            {
                *pAtom = HostDispatch::Create(scriptContext, pVar);
            }
            hr = NOERROR;
        }
        break;

        case VT_DISPATCH:
        {
            if (pVar->pdispVal == nullptr)
            {
                *pAtom = scriptContext->GetLibrary()->GetNull();
                break;
            }

            CComPtr<ITracker> tracker;
            IDispatch* pdispVal = pVar->pdispVal;
            CComPtr<IDispatchEx> pDispEx;
            {
                AUTO_NO_EXCEPTION_REGION;
                if (SUCCEEDED(pdispVal->QueryInterface(IID_PPV_ARGS(&pDispEx))) && pDispEx)
                {
                    // We'll use the IDispatchEx implementation to ask for ITracker in case any proxies want to handle IDispatchEx/ITracker.
                    pdispVal = pDispEx;
                }

                hr = pdispVal->QueryInterface(IID_ITrackerJS9, (void**)&tracker);
            }

            if (SUCCEEDED(hr) && tracker)
            {
                *pAtom = HostDispatch::Create(scriptContext, (ITracker*)tracker);
            }
            else
            {
                IDispatch* pDispatch = nullptr;
                hr = pdispVal->QueryInterface(__uuidof(IDispatch), (void**)&pDispatch);
                if (SUCCEEDED(hr) && pDispatch)
                {
                    hr = MarshalIDispatchToJsVar(scriptContext, pDispatch, pAtom);
                    pDispatch->Release();
                }
                else
                {
                    *pAtom = HostDispatch::Create(scriptContext, pVar);
                }
            }

            break;
        }

        case VTE_ARRAY_BYTE:
        case VTE_ARRAY:
        {
            // JavascriptSafeArray/JavascriptSafeArrayObject is deprecated for all hos types.
            // Do what's the default: just wrap it with HostDispatch and use as opaque object.
            // This was previously wrapped in a SafeArray type
            goto LDefault;
        }

        case VTE_BYREF_VARIANT:
        {
            hr = MarshalVariantToJsVar(pVar->pvarVal, pAtom, scriptContext);
            break;
        }

        case VT_ERROR:
        {
            //
            // Marshal VT_ERROR as undefined. There are legacy code in IE which uses VT_ERROR to report no arguments.
            // Example: showModalDialog when called without vArguments and engine tries to access window.dialogArguments IE returns
            // VT_ERROR and scode 0. Legacy IE8 engine stored an interal simple type VT_ERROR. But having a typeIds for such corner
            // case is costly hence marshaling them as undefined. VT_ERROR may get lost in roundtrips but there are very few known
            // cases where VT_ERROR is passed to the engine.
            // TODO: Need to revisit this if there are too many cases of VT_ERROR. Adding a typeId corresponding to VT_ERROR is a solution.
            //
            *pAtom = scriptContext->GetLibrary()->GetUndefined();
            break;
        }

        case VT_DATE:
        {
            *pAtom = scriptContext->GetLibrary()->CreateVariantDate(pVar->dblVal);
            break;
        }

        default:
LDefault:
        {
            *pAtom = HostDispatch::Create(scriptContext, pVar);
            break;
        }
    }
    return hr;
}

HRESULT DispatchHelper::MarshalVariantToJsVarDerefedNoThrow(VARIANT *pVar, Js::Var *pAtom, Js::ScriptContext* scriptContext)
{
    HRESULT hr = S_OK;
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        hr = MarshalVariantToJsVarDerefed(pVar, pAtom, scriptContext);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr);
    return hr;
}

HRESULT DispatchHelper::MarshalVariantToJsVarNoThrowNoScript(VARIANT *pVarIn, Js::Var *pAtom, Js::ScriptContext* scriptContext, VariantPropertyFlag VariantPropertyFlag)
{
    Assert(!scriptContext->GetThreadContext()->IsScriptActive());
    HRESULT hr = NOERROR;
    BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
    {
        hr = MarshalVariantToJsVar(pVarIn, pAtom, scriptContext);
    }
    END_TRANSLATE_OOM_TO_HRESULT(hr)
    return hr;
}

HRESULT DispatchHelper::MarshalVariantToJsVarWithLeaveScript(VARIANT *pVarIn, Js::Var *pAtom, Js::ScriptContext* scriptContext, VariantPropertyFlag VariantPropertyFlag)
{
    HRESULT hr = NOERROR;
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = MarshalVariantToJsVarNoThrowNoScript(pVarIn, pAtom, scriptContext, VariantPropertyFlag);
    }
    END_LEAVE_SCRIPT(scriptContext);
    return hr;
}

HRESULT DispatchHelper::MarshalVariantToJsVar(VARIANT *pVarIn, Js::Var *pAtom, Js::ScriptContext* scriptContext, VariantPropertyFlag variantPropertyFlag)
{
    AssertMem(pVarIn);
    AssertMem(pAtom);

    HRESULT hr = S_OK;
    VARIANT * pVar = pVarIn;
    if (pVar == NULL || pAtom == NULL)
    {
        return E_INVALIDARG;
    }

    if (VariantIsReturnValue(variantPropertyFlag) && (pVarIn->vt & VT_BYREF))
    {
        hr = DISP_E_TYPEMISMATCH;
        return hr;
    }


    VARIANT var;
    if (((pVar->vt & VT_BYREF) != 0)
        && ((pVar->vt & ~VT_BYREF) !=  VT_ARRAY)
        && ((pVar->vt & ~VT_BYREF)  != VT_BSTR)
        && ((pVar->vt & ~VT_BYREF) != VT_VARIANT))
    {
        if (((pVar->vt & ~VT_BYREF)  == VT_EMPTY) ||
             ((pVar->vt & ~VT_BYREF) == VT_NULL))
        {
            return E_INVALIDARG;
        }
        VariantInit(&var);
        hr = VariantCopyInd(&var, pVar);
        if (FAILED(hr))
        {
            return hr;
        }
        pVar = &var;
    }
    else
    {
        // VariantCopy on BSTR will change NULL pointer to an empty string and that changes the
        // syntax. don't do the VaraintCopy on BSTR: we don't need it as we are not change anything
        // for BSTR in SimplifyVariant anyhow.
        if (!VariantIsReturnValue(variantPropertyFlag) && pVar->vt != VT_BSTR)
        {
            VariantInit(&var);
            hr = VariantCopy(&var, pVar);
            if (FAILED(hr))
            {
                return hr;
            }
            pVar = &var;
        }
    }

    hr = SimplifyVariant(pVar);
    if (FAILED(hr))
    {
        if (pVar == &var)
        {
            VariantClear(pVar);
        }

        return hr;
    }

    hr = MarshalVariantToJsVarDerefedNoThrow(pVar, pAtom, scriptContext);

    AssertMsg(FAILED(hr) || *pAtom != NULL, "MarshalVariantToJsVar returning null");
    if (pVar == &var)
    {
        VariantClear(pVar);
    }

    return hr;
}

inline BOOL DispatchHelper::AlreadyRecorded(HRESULT hr)
{
    return  SCRIPT_E_RECORDED  == hr ||
            SCRIPT_E_REPORTED  == hr ||
            SCRIPT_E_PROPAGATE == hr;
}

HRESULT DispatchHelper::GetDispatchValue(ScriptSite* scriptSite, IDispatch* pdisp, DISPID id, VARIANT* pvarRes)
{
    IfNullReturnError(scriptSite, E_INVALIDARG);
    HRESULT hr = S_OK;
    IDispatchEx *pDispEx = NULL;
    EXCEPINFO ei;
    DISPPARAMS dp = {0};
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (NULL == scriptEngine)
    {
        return E_UNEXPECTED;
    }

    hr = pdisp->QueryInterface(__uuidof(IDispatchEx), (void**)&pDispEx);
    if (SUCCEEDED(hr) && pDispEx)
    {
        DispatchExCaller *pdc = NULL;
        hr = scriptSite->GetDispatchExCaller(&pdc);
        if (SUCCEEDED(hr))
        {
            hr = pDispEx->InvokeEx(id, scriptEngine->GetInvokeVersion(), DISPATCH_PROPERTYGET, &dp, pvarRes, &ei, pdc);
            scriptSite->ReleaseDispatchExCaller(pdc);
        }
        pDispEx->Release();
    }
    else
    {
        hr = pdisp->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT/*lcid*/,DISPATCH_PROPERTYGET, &dp, pvarRes, &ei, NULL);
    }

    return hr;
}

HRESULT DispatchHelper::SimplifyVariant(VARIANT* var)
{
    switch (var->vt)
    {
    case VT_DISPATCH:
    case VT_UNKNOWN:
        break;

    case VT_DATE:
        break;

#if !defined(WINCE)
    case VT_I8:
        if (LONG_MIN <= var->llVal && var->llVal <= LONG_MAX)
        {
            var->vt = VT_I4;
            var->lVal = (LONG)var->llVal;
            goto I4Case;
        }
        goto DefaultCase;

    case VT_UI8:
        if (var->ullVal <= ULONG_MAX)
        {
            var->vt = VT_UI4;
            var->ulVal = (ULONG)var->ullVal;
            goto UI4Case;
        }
        goto DefaultCase;
#endif

        // Convert all numeric types to R8 and I4.
    case VT_UI1:
        var->lVal = (long)var->bVal;
        var->vt = VT_I4;
        break;
    case VT_I2:
        var->lVal = (long)var->iVal;
        var->vt = VT_I4;
        break;
#if !defined(WINCE)
I4Case:
#endif
    case VT_I4:
        break;

    case VT_I1:
        var->lVal = (long)var->cVal;
        var->vt = VT_I4;
        break;
    case VT_UI2:
        var->lVal = (long)var->uiVal;
        var->vt = VT_I4;
        break;
    case VT_INT:
        var->lVal = (long)var->intVal;
        var->vt = VT_I4;
        break;
    case VT_UINT:
        if (var->uintVal <= 0x7FFFFFFF)
        {
            var->lVal = (long)var->uintVal;
            var->vt = VT_I4;
        }
        else
        {
            var->dblVal = (double)var->uintVal;
            var->vt = VT_R8;
        }
        break;

#if  !defined(WINCE)
UI4Case:
#endif
    case VT_UI4:
        if (var->ulVal <= 0x7FFFFFFF)
        {
            var->lVal = (long)var->ulVal;
            var->vt = VT_I4;
        }
        else
        {
            var->dblVal = (double)var->ulVal;
            var->vt = VT_R8;
        }
        break;
    case VT_DECIMAL:

        var->dblVal = Js::NumberUtilities::DblFromDecimal(&var->decVal);

        var->vt = VT_R8;
        break;

    case VT_R4:
        var->dblVal = (double)var->fltVal;
        var->vt = VT_R8;
        break;
    case VT_R8:
        break;
    case VT_CY:
        var->dblVal = ((double)var->cyVal.Hi * (double)4294967296.0
            + (double)var->cyVal.Lo) / 10000;
        var->vt = VT_R8;
        break;

#if  !defined(WINCE)
DefaultCase:
#endif
    default:
        if(var->vt & VT_BYREF)
        {
            break;  //JS 5.8 behavior
        }
        // v5.8 register with GC the BSTR and VB_ARRAY variants. It shouldn't be the case here
        break;
    }

    return NOERROR;
}

inline BOOL IsTrue(VARIANT *var)
{
    switch (var->vt)
    {
    default:
        return FALSE;
    case VT_BOOL:
        return VARIANT_FALSE != var->boolVal;
    case VT_I4:
        return 0 != var->lVal;
    case VT_R8:
        return 0.0 != var->dblVal && !Js::JavascriptNumber::IsNan(var->dblVal);
    case VT_BSTR:
        return 0 < CbBstr(var->bstrVal);
    case VT_UNKNOWN:
    case VT_DISPATCH:
        return NULL != var->pdispVal;
    }
}

BOOL DispatchHelper::ConvertToScalarCore(VARIANT *pvarSrc, VARIANT  *pvarDst, int vt, Js::ScriptContext *const scriptContext)
{
    Assert(scriptContext);

    if (pvarSrc->vt == vt)
    {
        *pvarDst = *pvarSrc;
        return TRUE;
    }

    switch (vt)
    {
    case VT_R8:
        switch (pvarSrc->vt)
        {
        case VT_EMPTY:
            pvarDst->dblVal = Js::JavascriptNumber::NaN;
            break;
        case VT_NULL:
            pvarDst->dblVal = 0;
            break;
        case VT_DATE:
            pvarDst->dblVal = Js::DateImplementation::GetTvUtc(Js::DateImplementation::JsLocalTimeFromVarDate(pvarSrc->dblVal), scriptContext);
            break;
        case VT_BOOL:
            pvarDst->dblVal = (double)(pvarSrc->boolVal != VARIANT_FALSE);
            break;
        case VT_I4:
            pvarDst->dblVal = (double)pvarSrc->lVal;
            break;

        case VT_BSTR:
            pvarDst->dblVal = BstrToDbl(pvarSrc->bstrVal, scriptContext);
            break;
        }
        pvarDst->vt = VT_R8;
        break;

    case VT_I4:
        switch (pvarSrc->vt)
        {
        default:
            return FALSE;
        case VT_EMPTY:
        case VT_NULL:
            pvarDst->lVal = 0;
            break;
        case VT_DATE:
            pvarDst->lVal = LwFromDbl(Js::DateImplementation::GetTvUtc(Js::DateImplementation::JsLocalTimeFromVarDate(pvarSrc->dblVal), scriptContext));
            break;
        case VT_BOOL:
            pvarDst->lVal = (pvarSrc->boolVal != VARIANT_FALSE);
            break;
        case VT_R8:
            pvarDst->lVal = LwFromDbl(pvarSrc->dblVal);
            break;
        case VT_BSTR:
            pvarDst->lVal = LwFromDbl(BstrToDbl(pvarSrc->bstrVal, scriptContext));
            break;
        }
        pvarDst->vt = VT_I4;
        break;

    case VT_BOOL:
        {
            pvarDst->vt = VT_BOOL;
            pvarDst->boolVal = IsTrue(pvarSrc) ? VARIANT_TRUE : VARIANT_FALSE;
            break;
        }

    case VT_DATE:
        switch (pvarSrc->vt)
        {
        default:
            return FALSE;
        case VT_R8:
            pvarDst->dblVal = Js::DateImplementation::VarDateFromJsUtcTime(pvarSrc->dblVal, scriptContext);
            break;
        case VT_BOOL:
            pvarDst->dblVal = Js::DateImplementation::VarDateFromJsUtcTime((double)(pvarSrc->boolVal != VARIANT_FALSE), scriptContext);
            break;
        case VT_I4:
            pvarDst->dblVal = Js::DateImplementation::VarDateFromJsUtcTime((double)pvarSrc->lVal, scriptContext);
            break;
        case VT_BSTR:
            {
                double dbl;
                if(!Js::DateImplementation::UtcTimeFromStrCore(pvarSrc->bstrVal, SysStringLen(pvarSrc->bstrVal), dbl, scriptContext))
                {
                    return FALSE;
                }
                pvarDst->dblVal = Js::DateImplementation::VarDateFromJsUtcTime(dbl, scriptContext);
                break;
            }
        }
        pvarDst->vt = VT_DATE;
        break;

    default:
        AssertMsg(FALSE, "Who's passing a bogus vt to ConvertToScalar?");
        return FALSE;
    }

    return TRUE;
}

HRESULT DispatchHelper::ConvertToScalar(VARIANT *pvarSrc, VARIANT  *pvarDst, int vt, Js::ScriptContext *const scriptContext)
{
    Assert(scriptContext);

    if (ConvertToScalarCore(pvarSrc, pvarDst, vt, scriptContext))
    {
        return NOERROR;
    }
    return E_NOTIMPL; //Need to record error code?
}

HRESULT DispatchHelper::JscriptChangeType(VARIANT* src, VARIANT* dst, VARTYPE vtNew, Js::ScriptContext *const scriptContext)
{
    Assert(scriptContext);

    HRESULT hr;
    IfFailedReturn(SimplifyVariant(src));
    switch (vtNew)
    {
    default:
        return E_NOTIMPL;   // bug to bug compat: this is what old engine returns.
    case VT_BOOL:
        IFFAILGO(ConvertToScalar(src, dst, VT_BOOL, scriptContext));
        break;
    case VT_DATE:
        IFFAILGO(ConvertToScalar(src, dst, VT_DATE, scriptContext));
        break;
    case VT_R8:
        IFFAILGO(ConvertToScalar(src, dst, VT_R8, scriptContext));
        break;
    case VT_R4:
        IFFAILGO(ConvertToScalar(src, dst, VT_R8, scriptContext));
        dst->fltVal = (float)dst->dblVal;
        dst->vt = VT_R4;
        break;
    case VT_UI1:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->bVal = (byte)(dst->lVal);
        dst->vt = VT_UI1;
        break;
    case VT_I2:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->iVal = (short)(dst->lVal);
        dst->vt = VT_I2;
        break;
    case VT_I4:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        break;
    case VT_I1:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->cVal = (char)(dst->lVal);
        dst->vt = VT_I1;
        break;
    case VT_UI2:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->uiVal = (ushort)dst->lVal;
        dst->vt = VT_UI2;
        break;
    case VT_INT:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->intVal = (int)dst->lVal;
        dst->vt = VT_INT;
        break;
    case VT_UINT:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->uintVal = (uint)dst->lVal;
        dst->vt = VT_UINT;
        break;
    case VT_UI4:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->ulVal = (ulong)dst->lVal;
        dst->vt = VT_UI4;
        break;
    case VT_UI8:
        IFFAILGO(ConvertToScalar(src, dst, VT_I4, scriptContext));
        dst->ullVal = (ULONGLONG)dst->lVal;
        dst->vt = VT_UI8;
        break;
    case VT_BSTR:
        IFFAILGO(ConvertToString(src, dst, scriptContext));
        break;
    }
    Assert(dst->vt == vtNew);
    return NOERROR;
LReturn:
    dst->vt = VT_EMPTY;
    return hr;
}

double DispatchHelper::BstrToDbl(BSTR bstr, Js::ScriptContext *const scriptContext)
{
    const Js::CharClassifier *charClassifier = scriptContext->GetCharClassifier();
    double dbl;
    const OLECHAR *pch;
    long cch = CchRawBstr(bstr);

    if (0 == cch)
        return 0;

    for (pch = bstr; charClassifier->IsWhiteSpace(*pch); pch++)
        ;
    if (0 == *pch)
        return 0;

    if (*pch == '0' && (pch[1] == 'x' || pch[1] == 'X'))
    {
        // Hex
        const OLECHAR *pchT = pch + 2;

        dbl = Js::NumberUtilities::DblFromHex(pchT, &pch);
        if (pchT == pch)
            goto LNan;
    }
    else
        dbl = Js::NumberUtilities::StrToDbl(pch, &pch, scriptContext);

    while (charClassifier->IsWhiteSpace(*pch))
        pch++;
    if (pch != bstr + cch)
    {
LNan:
        dbl = Js::JavascriptNumber::NaN;
    }

    return dbl;
}

HRESULT DispatchHelper::GetStringForNumber(VARIANT *src, __out_ecount(pszLen) OLECHAR * psz, int pszLen)
{
    IfNullReturnError(src, E_INVALIDARG);
    AssertMem(psz);
    Assert(pszLen > 20);

    Assert(FNumber(src));

    long lwT;
    double dblT;
    HRESULT hr = NOERROR;

    switch(src->vt)
    {
        case VT_I4:
            lwT = src->lVal;
            _ltow(lwT, psz, 10);
            break;

        case VT_R8:
            dblT = src->dblVal;
            if (!Js::NumberUtilities::IsFinite(dblT))
            {
                if (Js::JavascriptNumber::IsNan(dblT))
                    wcscpy_s(psz, pszLen, cbstrNaN.Pvar()->bstrVal);
                else
                    if(dblT < 0)
                        wcscpy_s(psz, pszLen, cbstrNegInf.Pvar()->bstrVal);
                    else
                        wcscpy_s(psz, pszLen, cbstrInf.Pvar()->bstrVal);
            }
            else if (!Js::NumberUtilities::FDblToStr(dblT, psz, pszLen))
                return E_OUTOFMEMORY;
            break;

        default:
            Assert(FALSE);
    }
    return hr;
}

HRESULT DispatchHelperInternal::GetDateDefaultStringBstr(VARIANT *pvarRes, DateTime::YMD *pymd, Js::DateImplementation::TZD *ptzd, ulong noDateTime, Js::ScriptContext *scriptContext)
{
    AssertMem(pvarRes);
    AssertMem(pymd);
    AssertMem(ptzd);

    const int kcchMax = 256;
    OLECHAR sz[kcchMax];
    int hour, min;
    BuildString bs;

    if( !(noDateTime & Js::DateImplementation::DateTimeFlag::NoDate))
    {
        StringCchPrintfW(sz, ARRAYSIZE(sz), OLESTR("%s %s %d "),
            Js::g_rgpszDay[pymd->wday],
            Js::g_rgpszMonth[pymd->mon],
            pymd->mday + 1);
        bs.AppendSz(sz);
    }

    if(!(noDateTime & Js::DateImplementation::DateTimeFlag::NoTime))
    {
        swprintf_s(sz, ARRAYSIZE(sz), OLESTR("%02d:%02d:%02d "),
            (int)(pymd->time / 3600000),
            (int)((pymd->time / 60000) % 60),
            (int)((pymd->time / 1000) % 60));
        bs.AppendSz(sz);

        // Add the time zone designation and a space.
        if (0 == ptzd->minutes % 60 &&
            -8 <= (hour = ptzd->minutes / 60) && hour <= -5)
        {
            // Standard US time zone.
            hour = 2 * (-5 - hour);
            if (ptzd->fDst)
                hour++;
            //Assert(hour >= 0 && hour < CvFromRgv((Js::g_rgpszZone)));
            bs.AppendSz(Js::g_rgpszZone[hour]);
        }
        else
        {
            // Non-US time zone.
            bs.AppendSz(OLESTR("UTC"));
            min = ptzd->offset;
            if (0 != min)
            {
                if (min < 0)
                {
                    bs.AppendCh('-');
                    min = -min;
                }
                else
                    bs.AppendCh('+');
                hour = min / 60;
                min %= 60;
                swprintf_s(sz, ARRAYSIZE(sz), OLESTR("%02d%02d"), hour, min);
                bs.AppendSz(sz);
            }
        }
        if( !(noDateTime & Js::DateImplementation::DateTimeFlag::NoDate))
            bs.AppendCh(' ');
    }


    if( !(noDateTime & Js::DateImplementation::DateTimeFlag::NoDate))
    {
        // Add the year.
        if (pymd->year > 0)
        {
            oltoa(pymd->year, sz, 10);
            bs.AppendSz(sz);
        }
        else
        {
            oltoa(1 - pymd->year, sz, 10);
            bs.AppendSz(sz);
            bs.AppendSz(OLESTR(" B.C."));
        }
    }

    if (bs.FError() || NULL == (pvarRes->bstrVal = SysAllocString(bs.PszCur())))
        return(HR(E_OUTOFMEMORY));
    else
    {
        js_memcpy_s(pvarRes->bstrVal, (bs.CchCur() + 1) * sizeof(OLECHAR), bs.PszCur(), bs.CchCur()* sizeof(OLECHAR));
        pvarRes->vt= VT_BSTR;
    }
    bs.Reset();

    return NOERROR;

}

HRESULT DispatchHelper::ConvertVarDateToStr(double dbl, VARIANT *dst, Js::ScriptContext *scriptContext)
{
    Assert(scriptContext);

    Js::DateImplementation::TZD tzd;
    DateTime::YMD ymd;
    double tv = Js::DateImplementation::GetTvUtc(Js::DateImplementation::JsLocalTimeFromVarDate(dbl), scriptContext);

    tv = Js::DateImplementation::GetTvLcl(tv, scriptContext, &tzd);
    if (Js::JavascriptNumber::IsNan(tv))
    {
        return VariantCopy(dst, cbstrNaN.Pvar());
    }

    Js::DateImplementation::GetYmdFromTv(tv, &ymd);
    return DispatchHelperInternal::GetDateDefaultStringBstr(dst, &ymd, &tzd, 0, scriptContext);
}

// Convert from source VARAINT to dst VARIANT with VT_BSTR type.
// Copy the logic from old engine. Do CopyVaraint here to avoid
// memory leak for the places where we allocate BSTR.
HRESULT DispatchHelper::ConvertToString(VARIANT *src, VARIANT* dst, Js::ScriptContext *scriptContext)
{
    Assert(scriptContext);

    HRESULT hr = NOERROR;
    OLECHAR sz[256];
    // check the pointer before we de-reference it.
    if (NULL == dst)
    {
        return E_INVALIDARG;
    }

    switch (src->vt)
    {
    case VT_EMPTY:
        return VariantCopy(dst, cbstrUndefined.Pvar());

    case VT_NULL:
        return VariantCopy(dst, cbstrNull.Pvar());

    case VT_DATE:
        return ConvertVarDateToStr(src->dblVal, dst, scriptContext);

    case VT_I4:
    case VT_R8:
        IfFailedReturn(GetStringForNumber(src, sz, 256));
        {
            BSTR bstr = SysAllocString(sz);
            IfNullReturnError(bstr, E_OUTOFMEMORY);
            dst->vt = VT_BSTR;
            dst->bstrVal = bstr;
        }
        return NOERROR;

    case VT_BSTR:
        if (NULL == src->bstrVal)
        {
            return VariantCopy(dst, cbstrEmpty.Pvar());
        }
        return NOERROR;

    case VT_BOOL:
        return VariantCopy(dst, src->boolVal  != VARIANT_FALSE  ? cbstrTrue.Pvar() : cbstrFalse.Pvar());

    case VT_DISPATCH:
        if (NULL == src->pdispVal)
        {
            return VariantCopy(dst, cbstrNull.Pvar());
        }
        // fall thru
    default:
        return JSERR_NeedString;
    }
}



