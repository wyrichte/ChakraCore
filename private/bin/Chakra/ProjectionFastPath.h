//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace Projection
{
    void FastPathPopulateRuntimeClassThis(RuntimeClassThis * runtimeClassThis, ProjectionContext * projectionContext);
    Js::JavascriptWinRTFunction * TryGetProjectionFastPath(PropertyId nameId, RtABIMETHODSIGNATURE method, Signature * thunkSignature, ThisInfo * thisInfo, ProjectionContext * projectionContext, bool fConstructor);

    template<bool throwErrorOnTypeMismatch>
    IUnknown * TryGetInterface(Var thisArg, MetadataStringId expectedTypeId, const IID & iid, MetadataStringId methodNameId, Js::ScriptContext * scriptContext, bool * isDefaultInterface);

    INT64 GetDateTimeOfJavascriptDate(Var varInput, ProjectionContext *projectionContext);

#if DBG
    void IncrementInvokeCount();
#else
    inline void IncrementInvokeCount() {}
#endif
    template <bool isStatic>
    static Js::Var DefaultFastPathThunk(Js::RecyclableObject* function, Js::CallInfo callInfo, ...);
}