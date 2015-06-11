//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#ifdef ENABLE_FOUNDATION_OBJECT

using namespace Windows::Foundation::Diagnostics;

#define IfFailReturnNULL(op) \
    if (FAILED(hr=(op))) { return NULL; }; \

namespace Js
{    
    __inline DelayLoadWinRtString* WindowsFoundationAdapter::GetWinRtStringLibrary(_In_ ScriptContext* scriptContext)
    {
        return scriptContext->GetThreadContext()->GetWinRTStringLibrary();
    }

    __inline DelayLoadWinRtFoundation* WindowsFoundationAdapter::GetWinRtFoundationLibrary(_In_ ScriptContext* scriptContext)
    {
        return scriptContext->GetThreadContext()->GetWinRtFoundationLibrary();
    }

    IActivationFactory* WindowsFoundationAdapter::GetAsyncCausalityTracerActivationFactory(_In_ ScriptContext* scriptContext)
    {
        if (! asyncCausalityTracerActivationFactory)
        {
            HRESULT hr;
            HSTRING hString;
            HSTRING_HEADER hStringHdr;
            LPCWSTR factoryName = L"Windows.Foundation.Diagnostics.AsyncCausalityTracer";
            IID factoryIID = __uuidof(IAsyncCausalityTracerStatics);
            
            IfFailReturnNULL(GetWinRtStringLibrary(scriptContext)->WindowsCreateStringReference(factoryName, wcslen(factoryName), &hStringHdr, &hString));
            IfFailReturnNULL(GetWinRtFoundationLibrary(scriptContext)->RoGetActivationFactory(hString, factoryIID, &asyncCausalityTracerActivationFactory));
            
            Assert(asyncCausalityTracerActivationFactory != NULL);
        }

        return asyncCausalityTracerActivationFactory;
    }

    IAsyncCausalityTracerStatics* WindowsFoundationAdapter::GetAsyncCausalityTracerStatics(_In_ ScriptContext* scriptContext)
    {
        if (! asyncCausalityTracerStatics)
        {
            IActivationFactory* factory = GetAsyncCausalityTracerActivationFactory(scriptContext);
            if (!factory) 
            {
                return NULL;
            }

            HRESULT hr;
            IfFailReturnNULL(factory->QueryInterface(__uuidof(IAsyncCausalityTracerStatics), reinterpret_cast<void**>(&asyncCausalityTracerStatics)));

            Assert(asyncCausalityTracerStatics != NULL);
        }

        return asyncCausalityTracerStatics;
    }

    HRESULT WindowsFoundationAdapter::TraceOperationCreation(
        _In_ ScriptContext* scriptContext, 
        _In_ INT traceLevel, 
        _In_ INT source, 
        _In_ GUID platformId, 
        _In_ UINT64 operationId, 
        _In_z_ PCWSTR operationName, 
        _In_ UINT64 relatedContext)
    {
        HRESULT hr;
        HSTRING hString;
        HSTRING_HEADER hStringHdr;

        Assert(traceLevel <= CausalityTraceLevel::CausalityTraceLevel_Verbose && traceLevel >= CausalityTraceLevel_Required);
        Assert(source <= CausalitySource::CausalitySource_System && source >= CausalitySource_Application);

        IFFAILRET(GetWinRtStringLibrary(scriptContext)->WindowsCreateStringReference(operationName, wcslen(operationName), &hStringHdr, &hString));
        
        IAsyncCausalityTracerStatics* tracerStatics = GetAsyncCausalityTracerStatics(scriptContext);

        if (!tracerStatics)
        {
            return E_UNEXPECTED;
        }

        return tracerStatics->TraceOperationCreation((CausalityTraceLevel)traceLevel, (CausalitySource)source, platformId, operationId, hString, relatedContext);
    }

    HRESULT WindowsFoundationAdapter::TraceOperationCompletion(
        _In_ ScriptContext* scriptContext, 
        _In_ INT traceLevel,
        _In_ INT source,
        _In_ GUID platformId,
        _In_ UINT64 operationId,
        _In_ INT status)
    {
        Assert(traceLevel <= CausalityTraceLevel::CausalityTraceLevel_Verbose && traceLevel >= CausalityTraceLevel_Required);
        Assert(source <= CausalitySource::CausalitySource_System && source >= CausalitySource_Application);
        Assert(status <= (INT)AsyncStatus::Error && status >= (INT)AsyncStatus::Started);

        IAsyncCausalityTracerStatics* tracerStatics = GetAsyncCausalityTracerStatics(scriptContext);

        if (!tracerStatics)
        {
            return E_UNEXPECTED;
        }

        return tracerStatics->TraceOperationCompletion((CausalityTraceLevel)traceLevel, (CausalitySource)source, platformId, operationId, (AsyncStatus)status);
    }

    HRESULT WindowsFoundationAdapter::TraceOperationRelation(
        _In_ ScriptContext* scriptContext, 
        _In_ INT traceLevel,
        _In_ INT source,
        _In_ GUID platformId, 
        _In_ UINT64 operationId, 
        _In_ INT relation)
    {
        Assert(traceLevel <= CausalityTraceLevel::CausalityTraceLevel_Verbose && traceLevel >= CausalityTraceLevel_Required);
        Assert(source <= CausalitySource::CausalitySource_System && source >= CausalitySource_Application);
        Assert(relation <= CausalityRelation::CausalityRelation_Error && relation >= CausalityRelation_AssignDelegate);

        IAsyncCausalityTracerStatics* tracerStatics = GetAsyncCausalityTracerStatics(scriptContext);

        if (!tracerStatics)
        {
            return E_UNEXPECTED;
        }

        return tracerStatics->TraceOperationRelation((CausalityTraceLevel)traceLevel, (CausalitySource)source, platformId, operationId, (CausalityRelation)relation);
    }

    HRESULT WindowsFoundationAdapter::TraceSynchronousWorkStart(
        _In_ ScriptContext* scriptContext, 
        _In_ INT traceLevel, 
        _In_ INT source, 
        _In_ GUID platformId, 
        _In_ UINT64 operationId, 
        _In_ INT work)
    {
        Assert(traceLevel <= CausalityTraceLevel::CausalityTraceLevel_Verbose && traceLevel >= CausalityTraceLevel_Required);
        Assert(source <= CausalitySource::CausalitySource_System && source >= CausalitySource_Application);
        Assert(work <= CausalitySynchronousWork::CausalitySynchronousWork_Execution && work >= CausalitySynchronousWork_CompletionNotification);

        IAsyncCausalityTracerStatics* tracerStatics = GetAsyncCausalityTracerStatics(scriptContext);

        if (!tracerStatics)
        {
            return E_UNEXPECTED;
        }

        return tracerStatics->TraceSynchronousWorkStart((CausalityTraceLevel)traceLevel, (CausalitySource)source, platformId, operationId, (CausalitySynchronousWork)work);
    }

    HRESULT WindowsFoundationAdapter::TraceSynchronousWorkCompletion(
        _In_ ScriptContext* scriptContext, 
        _In_ INT traceLevel, 
        _In_ INT source, 
        _In_ INT work)
    {
        Assert(traceLevel <= CausalityTraceLevel::CausalityTraceLevel_Verbose && traceLevel >= CausalityTraceLevel_Required);
        Assert(source <= CausalitySource::CausalitySource_System && source >= CausalitySource_Application);
        Assert(work <= CausalitySynchronousWork::CausalitySynchronousWork_Execution && work >= CausalitySynchronousWork_CompletionNotification);

        IAsyncCausalityTracerStatics* tracerStatics = GetAsyncCausalityTracerStatics(scriptContext);

        if (!tracerStatics)
        {
            return E_UNEXPECTED;
        }

        return tracerStatics->TraceSynchronousWorkCompletion((CausalityTraceLevel)traceLevel, (CausalitySource)source, (CausalitySynchronousWork)work);
    }

}
#endif
