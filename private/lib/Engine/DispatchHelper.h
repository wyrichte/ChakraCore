/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#pragma once


class DispatchHelper
{
public:
    inline static bool DispParamsContainThis(DISPPARAMS *_pdp)
    {
        return (_pdp->cNamedArgs != 0 && _pdp->rgdispidNamedArgs[0] == DISPID_THIS);
    }
    enum VariantPropertyFlag 
    {
        IsReturnValue = 0x1,
        VariantPropertyNone = 0
    };
    static HRESULT MarshalVariantToJsVarNoThrowNoScript(VARIANT *pVarIn, Js::Var *pAtom, Js::ScriptContext* scriptContext, VariantPropertyFlag VariantPropertyFlag = VariantPropertyNone);
    static HRESULT MarshalJsVarsToVariantsNoThrow(Js::Var *pAtom, VARIANT *pVar, int count);
    static HRESULT MarshalJsVarToVariantNoThrowWithLeaveScript(Js::Var var, VARIANT *pVar, Js::ScriptContext* scriptContext);
    static HRESULT MarshalFrameDisplayToVariantNoScript(Js::FrameDisplay *pDisplay, VARIANT *pVar);
    static HRESULT MarshalJsVarToVariantNoThrow(Js::Var var, VARIANT *pVar, Js::ScriptContext * scriptContext);
    static HRESULT MarshalVariantToJsVarWithLeaveScript(VARIANT *pVarIn, Js::Var *pAtom, Js::ScriptContext* scriptContext, VariantPropertyFlag VariantPropertyFlag = VariantPropertyNone);
    static HRESULT MarshalIDispatchToJsVarNoThrow(Js::ScriptContext * scriptContext, IDispatch * pdispVal, Js::Var * var);
    static HRESULT MarshalDispParamToArgumentsNoThrowNoScript(
        __in DISPPARAMS* dispParams,
        __in Js::Var thisPointer, 
        __in Js::ScriptContext * scriptContext, 
        __in Js::RecyclableObject *callee,
        __out Js::Arguments* argument);

    static HRESULT GetDispatchValue(ScriptSite* scriptSite, IDispatch* pdisp, DISPID id, VARIANT* pvarRes);
    static HRESULT JscriptChangeType(VARIANT* src, VARIANT* dst, VARTYPE vtNew, Js::ScriptContext *const scriptContext);

private:
    static HRESULT MarshalVariantToFrameDisplay(VARIANT *pVar, Js::FrameDisplay **ppDisplay, Js::ScriptContext *scriptContext);
    static HRESULT MarshalJsVarsToVariants(Js::Var *pAtom, VARIANT *pVar, int count);
    static HRESULT MarshalJsVarToVariant(Js::Var var, VARIANT *pVar);
    static HRESULT MarshalVariantToJsVar(VARIANT *pVar, Js::Var *pAtom, Js::ScriptContext * scriptContext, VariantPropertyFlag VariantPropertyFlag = VariantPropertyNone);
    static double JsLocalTimeFromVarDate(double dbl);
    static double BstrToDbl(BSTR bstr, Js::ScriptContext *const scriptContext /* can be 0 for standard behavior */);
    static double VarDateFromJsUtcTime(double dbl);
    static HRESULT SimplifyVariant(VARIANT* var);
    static HRESULT ConvertToString(VARIANT *src, VARIANT* dst, Js::ScriptContext *scriptContext);
    static void MarshalJsVarToDispatchVariant(Js::Var var,VARIANT *pVar);
    static HRESULT MarshalVariantToJsVarDerefed(VARIANT *pVar, Js::Var *pAtom, Js::ScriptContext* scriptContext);
    static Js::Var MarshalBSTRToJsVar(Js::ScriptContext * scriptContext, BSTR bstr);
    static HRESULT MarshalVariantToJsVarDerefedNoThrow(VARIANT *pVar, Js::Var *pAtom, Js::ScriptContext* scriptContext);
    static inline BOOL DispatchHelper::AlreadyRecorded(HRESULT hr);
    static HRESULT ConvertToScalar(VARIANT *pvarSrc, VARIANT  *pvarDst, int vt, Js::ScriptContext *const scriptContext);
    static BOOL ConvertToScalarCore(VARIANT *pvarSrc, VARIANT  *pvarDst, int vt, Js::ScriptContext *const scriptContext);
    static HRESULT GetStringForNumber(VARIANT *src, __out_ecount(pszLen) OLECHAR * psz, int pszLen);     
    static HRESULT ConvertVarDateToStr(double dbl, VARIANT *dst, Js::ScriptContext *scriptContext);
    static inline BOOL VariantIsReturnValue(VariantPropertyFlag VariantPropertyFlag)  {return (VariantPropertyFlag & IsReturnValue) != 0; }    
};