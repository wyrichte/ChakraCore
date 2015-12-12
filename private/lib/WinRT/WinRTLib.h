//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// Following references are for metadata consumption
#include "cor.h"
#include <corhdr.h>

// Following references are for WinRT consumption
#include <roapi.h>
#include <Windows.Foundation.h>

// For weak references
#include "WeakReference.h"

#include "EdgeScriptDirect.h"
#include "Exceptions\Throw.h"
#include "DataStructures\Option.h"
#include "DataStructures\Tuple.h"
#include "DataStructures\ImmutableList.h"

// All we need from the runtime is just the delay load library
#include "base\DelayLoadLibrary.h"

// Copied from ParserCommon.h, needed to get the VBSERR_OutOfStack
#define MAKE_HR(vbserr) (MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL, vbserr))
#include "rterror.h"

//#define WINRTFINDPREMATURECOLLECTION

#define IfFailedMapAndThrowHrWithInfo(scriptContext, hr)                                    \
    if (FAILED(hr))                                                                         \
    {                                                                                       \
        Js::JavascriptErrorDebug::MapAndThrowErrorWithInfo(scriptContext, hr);              \
    }


#define IfFailedMapAndThrowHr(scriptContext, hr)                                            \
    if (FAILED(hr))                                                                         \
    {                                                                                       \
        Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, hr);                      \
    }


#define IfNullMapAndThrowHr(scriptContext, expr, hr)                                        \
    if (!(expr))                                                                            \
    {                                                                                       \
        Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, hr);                      \
    }

// The below IfFailed*Throw*AfterScriptEnter macros need to be used to make sure that while throwing error,
// we can handle the OutOfMemory exception that might be resulting while creating error object.
// Its used in methods that do not use HRESULT as out value but Var as out values since we cannot make use of translated HRESULT in these cases.

#define IfFailedMapAndThrowHrWithInfoAfterScriptEnter(scriptContext, hr)                    \
    if (FAILED(hr))                                                                         \
    {                                                                                       \
        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)                                      \
        {                                                                                   \
            Js::JavascriptErrorDebug::MapAndThrowErrorWithInfo(scriptContext, hr);          \
        }                                                                                   \
        END_JS_RUNTIME_CALL(scriptContext)                                                  \
    }

#define IfFailedMapAndThrowHrAfterScriptEnter(scriptContext, hr)                            \
    if (FAILED(hr))                                                                         \
    {                                                                                       \
        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)                                      \
        {                                                                                   \
            Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, hr);                  \
        }                                                                                   \
        END_JS_RUNTIME_CALL(scriptContext)                                                  \
    }

#define IfNullMapAndThrowHrAfterScriptEnter(scriptContext, expr, hr)                        \
    if (!(expr))                                                                            \
    {                                                                                       \
        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)                                      \
        {                                                                                   \
            Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, hr);                  \
        }                                                                                   \
        END_JS_RUNTIME_CALL(scriptContext)                                                  \
    }

#define ThrowJavascriptErrorAfterScriptEnter(ThrowErrorFn, scriptContext, ...)              \
    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)                                          \
    {                                                                                       \
        Js::JavascriptError::ThrowErrorFn(scriptContext, __VA_ARGS__);                      \
    }                                                                                       \
    END_JS_RUNTIME_CALL(scriptContext)

#define ThrowErrorAfterScriptEnter(scriptContext, ErrorType)                                \
    BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)                                          \
    {                                                                                       \
        Js::Throw::ErrorType();                                                             \
    }                                                                                       \
    END_JS_RUNTIME_CALL(scriptContext)

#include "GUIDParser.h"
#include "Metadata.h"
#include "ProjectionModel.h"
#include "ModelHelper.h"
#include "Visitor.h"
#include "MetaDataLocator.h"
#include "AutoHSTRING.h"
#include "AutoHeapString.h"
#include "AutoStackItem.h"
