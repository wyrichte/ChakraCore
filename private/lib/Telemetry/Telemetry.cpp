//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "TelemetryPch.h"
#include <strsafe.h>
#include "ChakraVersion.h"
#include "ieconfig.h"
#include "globalthreadstate.h"
#include <telemetry\MicrosoftTelemetry.h>
#include "Base\ThreadContextTLSEntry.h"
#include "Base\ThreadBoundThreadContextManager.h"

// GUID for "ChakraProvider_V0.1": {FC7BA620-EB50-483D-97A0-72D8268A14B5}

TRACELOGGING_DEFINE_PROVIDER(g_hTraceLoggingProv,
    "Microsoft.Web.Platform.Chakra",
    (0xfc7ba620, 0xeb50, 0x483d, 0x97, 0xa0, 0x72, 0xd8, 0x26, 0x8a, 0x14, 0xb5),
    TraceLoggingOptionMicrosoftTelemetry());


#include "TelemetryMacros.h"

WCHAR *g_ProcessExclusionList[] = {
    _u("jshost"),
    _u("jc"),
    _u("slate"),
    _u("mshtmpad"),
    _u("te.processhost"),
    _u("jdtest"),
    _u("jsglass"),
    _u("loader42")
};


// Creating wrapper for atExit Scenario as we want to tackle OOM and other exceptions.
void __cdecl firePackageTelemetryAtExit() 
{
  if (g_TraceLoggingClient != nullptr && !(g_TraceLoggingClient->GetNodeTelemetryProvider()->IsPackageTelemetryFired()))
  {
    HRESULT hr = NOERROR;
    BEGIN_TRANSLATE_OOM_TO_HRESULT
      g_TraceLoggingClient->GetNodeTelemetryProvider()->FirePackageTelemetryHelper();
    END_TRANSLATE_OOM_TO_HRESULT(hr);
  }
}

DWORD Telemetry::initialized = FALSE;
void Telemetry::EnsureInitializeForJSRT()
{
    if (::InterlockedExchange(&initialized, TRUE) == FALSE)
    {
        atexit(firePackageTelemetryAtExit);
    }
}

void Telemetry::OnJSRTThreadContextClose()
{
    if (g_TraceLoggingClient != nullptr && !(g_TraceLoggingClient->GetNodeTelemetryProvider()->IsPackageTelemetryFired()))
    {
        g_TraceLoggingClient->GetNodeTelemetryProvider()->FirePackageTelemetryHelper();
    }
}

TraceLoggingClient *g_TraceLoggingClient = NULL;

TraceLoggingClient::TraceLoggingClient() : shouldLogTelemetry(true)
{
    // Check if we're running in a process from which telemetry should
    // not be logged.  We'll default to logging telemetry if the process
    // name cannot be determined for some reason.
    WCHAR fullPath[MAX_PATH];
    WCHAR fname[MAX_PATH];
    DWORD dwResult = GetModuleFileName(NULL, fullPath, _countof(fullPath));
    if (dwResult != 0)
    {
        if (_wsplitpath_s(fullPath, NULL, 0, NULL, 0, fname, _countof(fname), NULL, 0) == 0)
        {
            for (int i = 0; i < _countof(g_ProcessExclusionList); ++i)
            {
                if (_wcsicmp(fname, g_ProcessExclusionList[i]) == 0)
                {
                    shouldLogTelemetry = false;
                    break;
                }
            }
        }
    }

    TraceLoggingRegister(g_hTraceLoggingProv);
}

TraceLoggingClient::~TraceLoggingClient()
{
    TraceLoggingUnregister(g_hTraceLoggingProv);
}

NodeTelemetryProvider* TraceLoggingClient::GetNodeTelemetryProvider()
{
    return &node;
}

void TraceLoggingClient::ResetTelemetryStats(ThreadContext* threadContext)
{
    if (threadContext != NULL)
    {
        threadContext->ResetLangStats();
#ifdef ENABLE_DIRECTCALL_TELEMETRY
        threadContext->directCallTelemetry.Reset();
#endif
    }
}


void TraceLoggingClient::FireChakraInitTelemetry(DWORD host, bool isJSRT)
{
    if (!this->throttle.isThrottled())
    {
        // TODO: add experimental flag detection to this event
        TraceLogChakra(
            TL_CHAKRAINIT,
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT")
            );
    }
}

// Fired whenever we close a page by navigating away
void TraceLoggingClient::FireSiteNavigation(const char16 *url, GUID activityId, DWORD host, bool isJSRT)
{
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

    // TODO: use current threadContext to retrieve thread-local data, turn it into
    // a specific schema, then log the event.


    // Workflow of Logging Telemetry
    // 1. Get the Telemetry point either as a value or as a pointer/reference if the structure is too big
    // 2. Report the telemetry 
    // 3. RESET the VALUE to INITIAL Values in the ResetMethod Below. If the value being reported is single scalar then use GetAndReset<Telemetry>()
    //    else do resetting in the Tracelogging Telemetry Stats API.

    //******** Treat this AS things to think about while adding NEW Telemetry****************
    // 1. Update the Reset count of your telemetry, if applicable
    // 2. Is it a perf metric, does it makes sense to report data when ScriptContext is in debug mode?
    // 3. If so then using isAnyScriptContextInDebugMode (variable defined below) would suffice?


    // printing GC Pause stats
    if (threadContext != NULL)
    {
        Js::LanguageStats* langStats = threadContext->GetLanguageStats();   

        if (langStats != NULL && CONFIG_ISENABLED(Js::ES5LangTelFlag))
        {
            Output::Print(_u("Array.isArray count: %d\n"), langStats->ArrayisArray.callCount);
            Output::Print(_u("Array.prototype.indexOf count: %d\n"), langStats->ArrayIndexOf.callCount);
            Output::Print(_u("Array.prototype.every count: %d\n"), langStats->ArrayEvery.callCount);
            Output::Print(_u("Array.prototype.filter count: %d\n"), langStats->ArrayFilter.callCount);
            Output::Print(_u("Array.prototype.forEach count: %d\n"), langStats->ArrayForEach.callCount);
            Output::Print(_u("Array.prototype.lastIndexOf count: %d\n"), langStats->ArrayLastIndexOf.callCount);
            Output::Print(_u("Array.prototype.map count: %d\n"), langStats->ArrayMap.callCount);
            Output::Print(_u("Array.prototype.reduce count: %d\n"), langStats->ArrayReduce.callCount);
            Output::Print(_u("Array.prototype.reduceRight count: %d\n"), langStats->ArrayReduceRight.callCount);
            Output::Print(_u("Array.prototype.some count: %d\n"), langStats->ArraySome.callCount);
            Output::Print(_u("Object.keys count: %d\n"), langStats->ObjectKeys.callCount);
            Output::Print(_u("Object.getOwnPropertyNames count: %d\n"), langStats->ObjectGetOwnPropertyNames.callCount);
            Output::Print(_u("Object.create count: %d\n"), langStats->ObjectCreate.callCount);
            Output::Print(_u("Object.defineProperties count: %d\n"), langStats->ObjectDefineProperties.callCount);
            Output::Print(_u("Object.freeze count: %d\n"), langStats->ObjectFreeze.callCount);
            Output::Print(_u("Object.seal count: %d\n"), langStats->ObjectSeal.callCount);
            Output::Print(_u("Object.getPrototypeOf count: %d\n"), langStats->ObjectGetPrototypeOf.callCount);
            Output::Print(_u("Object.isFrozen count: %d\n"), langStats->ObjectIsFrozen.callCount);
            Output::Print(_u("Object.isSealed count: %d\n"), langStats->ObjectIsSealed.callCount);
            Output::Print(_u("Object.isExtensible count: %d\n"), langStats->ObjectIsExtensible.callCount);
            Output::Print(_u("Object.preventExtensions count: %d\n"), langStats->ObjectPreventExtension.callCount);
            Output::Print(_u("Date.prototype.toISOString count: %d\n"), langStats->DateToISOString.callCount);
            Output::Print(_u("Function.prototype.bind count: %d\n"), langStats->FunctionBind.callCount);
            // Debug Mode call count
            Output::Print(_u("Array.isArray debug mode call count: %d\n"), langStats->ArrayisArray.debugModeCallCount);
            Output::Print(_u("Array.prototype.indexOf debug mode call count: %d\n"), langStats->ArrayIndexOf.debugModeCallCount);
            Output::Print(_u("Array.prototype.every debug mode call count: %d\n"), langStats->ArrayEvery.debugModeCallCount);
            Output::Print(_u("Array.prototype.filter debug mode call count: %d\n"), langStats->ArrayFilter.debugModeCallCount);
            Output::Print(_u("Array.prototype.forEach debug mode call count: %d\n"), langStats->ArrayForEach.debugModeCallCount);
            Output::Print(_u("Array.prototype.lastIndexOf debug mode call count: %d\n"), langStats->ArrayLastIndexOf.debugModeCallCount);
            Output::Print(_u("Array.prototype.map debug mode call count: %d\n"), langStats->ArrayMap.debugModeCallCount);
            Output::Print(_u("Array.prototype.reduce debug mode call count: %d\n"), langStats->ArrayReduce.debugModeCallCount);
            Output::Print(_u("Array.prototype.reduceRight debug mode call count: %d\n"), langStats->ArrayReduceRight.debugModeCallCount);
            Output::Print(_u("Array.prototype.some debug mode call count: %d\n"), langStats->ArraySome.debugModeCallCount);
            Output::Print(_u("Object.keys debug mode call count: %d\n"), langStats->ObjectKeys.debugModeCallCount);
            Output::Print(_u("Object.getOwnPropertyNames debug mode call count: %d\n"), langStats->ObjectGetOwnPropertyNames.debugModeCallCount);
            Output::Print(_u("Object.create debug mode call count: %d\n"), langStats->ObjectCreate.debugModeCallCount);
            Output::Print(_u("Object.defineProperties debug mode call count: %d\n"), langStats->ObjectDefineProperties.debugModeCallCount);
            Output::Print(_u("Object.freeze debug mode call count: %d\n"), langStats->ObjectFreeze.debugModeCallCount);
            Output::Print(_u("Object.seal debug mode call count: %d\n"), langStats->ObjectSeal.debugModeCallCount);
            Output::Print(_u("Object.getPrototypeOf debug mode call count: %d\n"), langStats->ObjectGetPrototypeOf.debugModeCallCount);
            Output::Print(_u("Object.isFrozen debug mode call count: %d\n"), langStats->ObjectIsFrozen.debugModeCallCount);
            Output::Print(_u("Object.isSealed debug mode call count: %d\n"), langStats->ObjectIsSealed.debugModeCallCount);
            Output::Print(_u("Object.isExtensible debug mode call count: %d\n"), langStats->ObjectIsExtensible.debugModeCallCount);
            Output::Print(_u("Object.preventExtensions debug mode call count: %d\n"), langStats->ObjectPreventExtension.debugModeCallCount);
            Output::Print(_u("Date.prototype.toISOString debug mode call count: %d\n"), langStats->DateToISOString.debugModeCallCount);
            Output::Print(_u("Function.prototype.bind debug mode call count: %d\n"), langStats->FunctionBind.debugModeCallCount);
        }

        if (langStats != NULL && CONFIG_ISENABLED(Js::ES6LangTelFlag))
        {
            Output::Print(_u("GetOwnPropertySymbolsCount %d\n"), langStats->GetOwnPropertySymbols.callCount);
            Output::Print(_u("GetOwnPropertySymbolsDebugModeCallCount %d\n"), langStats->GetOwnPropertySymbols.debugModeCallCount);
            Output::Print(_u("Log10Count %d\n"), langStats->Log10.callCount);
            Output::Print(_u("Log10DebugModeCount %d\n"), langStats->Log10.debugModeCallCount);
            Output::Print(_u("Log1pCount %d\n"), langStats->Log1p.callCount);
            Output::Print(_u("Log1pDebugModeCallCount %d\n"), langStats->Log1p.debugModeCallCount);
            Output::Print(_u("Log2Count %d\n"), langStats->Log2.callCount);
            Output::Print(_u("Log2DebugModeCallCount %d\n"), langStats->Log2.debugModeCallCount);
            Output::Print(_u("SinhCount %d\n"), langStats->Sinh.callCount);
            Output::Print(_u("SinhDebugModeCallCount %d\n"), langStats->Sinh.debugModeCallCount);
            Output::Print(_u("CoshCount %d\n"), langStats->Cosh.callCount);
            Output::Print(_u("CoshDebugModeCallCount %d\n"), langStats->Cosh.debugModeCallCount);
            Output::Print(_u("tanhCountCount %d\n"), langStats->Tanh.callCount);
            Output::Print(_u("tanhDebugModeCallCount %d\n"), langStats->Tanh.debugModeCallCount);
            Output::Print(_u("AsinhCount %d\n"), langStats->Asinh.callCount);
            Output::Print(_u("AsinhDebugModeCallCount %d\n"), langStats->Asinh.debugModeCallCount);
            Output::Print(_u("AcoshCount %d\n"), langStats->Acosh.callCount);
            Output::Print(_u("AcoshDebugModeCallCount %d\n"), langStats->Acosh.debugModeCallCount);
            Output::Print(_u("AtanhCount %d\n"), langStats->Atanh.callCount);
            Output::Print(_u("AtanhDebugModeCallCount %d\n"), langStats->Atanh.debugModeCallCount);
            Output::Print(_u("HypotCount %d\n"), langStats->Hypot.callCount);
            Output::Print(_u("HypotDebugModeCallCount %d\n"), langStats->Hypot.debugModeCallCount);
            Output::Print(_u("CbrtCount %d\n"), langStats->Cbrt.callCount);
            Output::Print(_u("CbrtDebugModeCallCount %d\n"), langStats->Cbrt.debugModeCallCount);
            Output::Print(_u("TruncCount %d\n"), langStats->Trunc.callCount);
            Output::Print(_u("TruncDebugModeCallCount %d\n"), langStats->Trunc.debugModeCallCount);
            Output::Print(_u("SignCount %d\n"), langStats->Sign.callCount);
            Output::Print(_u("SignDebugModeCallCount %d\n"), langStats->Sign.debugModeCallCount);
            Output::Print(_u("ImulCount %d\n"), langStats->Imul.callCount);
            Output::Print(_u("ImulDebugModeCallCount %d\n"), langStats->Imul.debugModeCallCount);
            Output::Print(_u("Clz32Count %d\n"), langStats->Clz32.callCount);
            Output::Print(_u("Clz32DebugModeCallCount %d\n"), langStats->Clz32.debugModeCallCount);
            Output::Print(_u("FroundCount %d\n"), langStats->Fround.callCount);
            Output::Print(_u("FroundDebugModeCallCount %d\n"), langStats->Fround.debugModeCallCount);
            Output::Print(_u("IsNaNCount %d\n"), langStats->IsNaN.callCount);
            Output::Print(_u("IsNaNDebugModeCallCount %d\n"), langStats->IsNaN.debugModeCallCount);
            Output::Print(_u("IsFiniteCount %d\n"), langStats->IsFinite.callCount);
            Output::Print(_u("IsFiniteDebugModeCallCount %d\n"), langStats->IsFinite.debugModeCallCount);
            Output::Print(_u("IsIntegerCount %d\n"), langStats->IsInteger.callCount);
            Output::Print(_u("IsIntegerDebugModeCallCount %d\n"), langStats->IsInteger.debugModeCallCount);
            Output::Print(_u("IsSafeIntegerCount %d\n"), langStats->IsSafeInteger.callCount);
            Output::Print(_u("IsSafeIntegerDebugModeCallCount %d\n"), langStats->IsSafeInteger.debugModeCallCount);
            Output::Print(_u("StartsWithCount %d\n"), langStats->StartsWith.callCount);
            Output::Print(_u("StartsWithDebugModeCallCount %d\n"), langStats->StartsWith.debugModeCallCount);
            Output::Print(_u("EndsWithCount %d\n"), langStats->EndsWith.callCount);
            Output::Print(_u("EndsWithDebugModeCallCount %d\n"), langStats->EndsWith.debugModeCallCount);
            Output::Print(_u("ContainsCount %d\n"), langStats->Contains.callCount);
            Output::Print(_u("ContainsDebugModeCallCount %d\n"), langStats->Contains.debugModeCallCount);
            Output::Print(_u("RepeatCount %d\n"), langStats->Repeat.callCount);
            Output::Print(_u("RepeatDebugModeCallCount %d\n"), langStats->Repeat.debugModeCallCount);
            Output::Print(_u("PromiseCount %d\n"), langStats->Promise.callCount);
            Output::Print(_u("PromiseDebugModeCallCount %d\n"), langStats->Promise.debugModeCallCount);
            Output::Print(_u("LetCount %d\n"), langStats->Let.parseCount);
            Output::Print(_u("LambdaCount %d\n"), langStats->Lambda.parseCount);
            Output::Print(_u("ConstCount %d\n"), langStats->Const.parseCount);
            Output::Print(_u("SuperCount %d\n"), langStats->Super.parseCount);
            Output::Print(_u("AsmJSFunctionCount %d\n"), langStats->AsmJSFunction.parseCount);
            Output::Print(_u("StrictModeFunctionCount %d\n"), langStats->StrictModeFunction.parseCount);
            Output::Print(_u("ClassCount %d\n"), langStats->Class.parseCount);
            Output::Print(_u("StringTemplatesCount %d\n"), langStats->StringTemplates.parseCount);
            Output::Print(_u("GeneratorsCount %d\n"), langStats->Generator.parseCount);
            Output::Print(_u("RestCount %d\n"), langStats->Rest.parseCount);
            Output::Print(_u("SpreadCount %d\n"), langStats->SpreadFeature.parseCount);
            Output::Print(_u("DefaultCount %d\n"), langStats->DefaultArgFunction.parseCount);
            Output::Print(_u("StickyRegexFlagCount %d\n"), langStats->StickyRegexFlag.parseCount);
            Output::Print(_u("UnicodeRegexFlagCount %d\n"), langStats->UnicodeRegexFlag.parseCount);
            Output::Print(_u("Array.prototype.includes count: %d\n"), langStats->ArrayIncludes.callCount);
            Output::Print(_u("Array.prototype.includes debug mode call count: %d\n"), langStats->ArrayIncludes.debugModeCallCount);

        }

        if (url != NULL && (CONFIG_ISENABLED(Js::ES6LangTelFlag) || CONFIG_ISENABLED(Js::ES5LangTelFlag)))
        {
            Output::Print(_u("Navigated from site: %s\n"), url);
        }

        if (!this->throttle.isThrottled())
        {

            // Note: must be thread-safe.
            
            TraceLogChakra(
                TL_ES5BUILTINS,
                TraceLoggingGuid(activityId, "activityId"),
                TraceLoggingUInt32(langStats->ArrayisArray.callCount, "arrayisArrayCount"),
                TraceLoggingUInt32(langStats->ArrayIndexOf.callCount, "arrayIndexOfCount"),
                TraceLoggingUInt32(langStats->ArrayEvery.callCount, "arrayEveryCount"),
                TraceLoggingUInt32(langStats->ArrayFilter.callCount, "arrayFilterCount"),
                TraceLoggingUInt32(langStats->ArrayForEach.callCount, "arrayForEachCount"),
                TraceLoggingUInt32(langStats->ArrayLastIndexOf.callCount, "arrayLastIndexOfCount"),
                TraceLoggingUInt32(langStats->ArrayMap.callCount, "arrayMapCount"),
                TraceLoggingUInt32(langStats->ArrayReduce.callCount, "arrayReduceCount"),
                TraceLoggingUInt32(langStats->ArrayReduceRight.callCount, "arrayReduceRightCount"),
                TraceLoggingUInt32(langStats->ArraySome.callCount, "arraySomeCount"),
                TraceLoggingUInt32(langStats->ObjectCreate.callCount, "objectCreateCount"),
                TraceLoggingUInt32(langStats->ObjectDefineProperties.callCount, "objectDefinePropertiesCount"),
                TraceLoggingUInt32(langStats->ObjectFreeze.callCount, "objectFreezeCount"),
                TraceLoggingUInt32(langStats->ObjectSeal.callCount, "objectSealCount"),
                TraceLoggingUInt32(langStats->ObjectGetOwnPropertyNames.callCount, "objectGetOwnPropertyNamesCount"),
                TraceLoggingUInt32(langStats->ObjectGetPrototypeOf.callCount, "objectGetPrototypeOfCount"),
                TraceLoggingUInt32(langStats->ObjectIsExtensible.callCount, "objectIsExtensibleCount"),
                TraceLoggingUInt32(langStats->ObjectIsFrozen.callCount, "objectIsFrozenCount"),
                TraceLoggingUInt32(langStats->ObjectIsSealed.callCount, "objectIsSealedCount"),
                TraceLoggingUInt32(langStats->ObjectKeys.callCount, "objectKeysCount"),
                TraceLoggingUInt32(langStats->ObjectPreventExtension.callCount, "objectPreventExtensionCount"),
                TraceLoggingUInt32(langStats->DateToISOString.callCount, "dateToISOStringCount"),
                TraceLoggingUInt32(langStats->FunctionBind.callCount, "functionBindCount"),
                TraceLoggingUInt32(langStats->StringTrim.callCount, "stringTrimCount"),
                TraceLoggingUInt32(langStats->ArrayisArray.debugModeCallCount, "arrayisArrayDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayIndexOf.debugModeCallCount, "arrayIndexOfDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayEvery.debugModeCallCount, "arrayEveryDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayFilter.debugModeCallCount, "arrayFilterDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayForEach.debugModeCallCount, "arrayForEachDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayLastIndexOf.debugModeCallCount, "arrayLastIndexOfDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayMap.debugModeCallCount, "arrayMapDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayReduce.debugModeCallCount, "arrayReduceDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayReduceRight.debugModeCallCount, "arrayReduceRightDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArraySome.debugModeCallCount, "arraySomeDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectCreate.debugModeCallCount, "objectCreateDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectDefineProperties.debugModeCallCount, "objectDefinePropertiesDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectFreeze.debugModeCallCount, "objectFreezeDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectSeal.debugModeCallCount, "objectSealDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectGetOwnPropertyNames.debugModeCallCount, "objectGetOwnPropertyNamesDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectGetPrototypeOf.debugModeCallCount, "objectGetPrototypeOfDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectIsExtensible.debugModeCallCount, "objectIsExtensibleDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectIsFrozen.debugModeCallCount, "objectIsFrozenDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectIsSealed.debugModeCallCount, "objectIsSealedDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectKeys.debugModeCallCount, "objectKeysDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ObjectPreventExtension.debugModeCallCount, "objectPreventExtensionDebugModeCallCount"),
                TraceLoggingUInt32(langStats->DateToISOString.debugModeCallCount, "dateToISOStringDebugModeCallCount"),
                TraceLoggingUInt32(langStats->FunctionBind.debugModeCallCount, "functionBindDebugModeCallCount"),
                TraceLoggingUInt32(langStats->StringTrim.debugModeCallCount, "stringTrimDebugModeCallCount"),
                TraceLoggingUInt32(host, "HostingInterface"),
                TraceLoggingBool(isJSRT, "isJSRT"),
                TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
                );


            TraceLogChakra(
                TL_ES6BUILTINS,
                TraceLoggingGuid(activityId, "activityId"),
                TraceLoggingUInt32(langStats->GetOwnPropertySymbols.callCount, "GetOwnPropertySymbolsCount"),
                TraceLoggingUInt32(langStats->GetOwnPropertySymbols.debugModeCallCount, "GetOwnPropertySymbolsDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Log10.callCount, "Log10Count"),
                TraceLoggingUInt32(langStats->Log10.debugModeCallCount, "Log10DebugModeCount"),
                TraceLoggingUInt32(langStats->Log1p.callCount, "Log1pCountCount"),
                TraceLoggingUInt32(langStats->Log1p.debugModeCallCount, "Log1pDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Log2.callCount, "Log2Count"),
                TraceLoggingUInt32(langStats->Log2.debugModeCallCount, "Log2DebugModeCallCount"),
                TraceLoggingUInt32(langStats->Sinh.callCount, "SinhCount"),
                TraceLoggingUInt32(langStats->Sinh.debugModeCallCount, "SinhDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Cosh.callCount, "CoshCount"),
                TraceLoggingUInt32(langStats->Cosh.debugModeCallCount, "CoshDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Tanh.callCount, "TanhCountCount"),
                TraceLoggingUInt32(langStats->Tanh.debugModeCallCount, "TanhDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Asinh.callCount, "AsinhCount"),
                TraceLoggingUInt32(langStats->Asinh.debugModeCallCount, "AsinhDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Acosh.callCount, "AcoshCount"),
                TraceLoggingUInt32(langStats->Acosh.debugModeCallCount, "AcoshDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Atanh.callCount, "AtanhCount"),
                TraceLoggingUInt32(langStats->Atanh.debugModeCallCount, "AtanhDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Hypot.callCount, "HypotCount"),
                TraceLoggingUInt32(langStats->Hypot.debugModeCallCount, "HypotDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Cbrt.callCount, "CbrtCount"),
                TraceLoggingUInt32(langStats->Cbrt.debugModeCallCount, "CbrtDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Trunc.callCount, "TruncCount"),
                TraceLoggingUInt32(langStats->Trunc.debugModeCallCount, "TruncDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Sign.callCount, "SignCount"),
                TraceLoggingUInt32(langStats->Sign.debugModeCallCount, "SignDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Imul.callCount, "ImulCount"),
                TraceLoggingUInt32(langStats->Imul.debugModeCallCount, "ImulDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Clz32.callCount, "Clz32Count"),
                TraceLoggingUInt32(langStats->Clz32.debugModeCallCount, "Clz32DebugModeCallCount"),
                TraceLoggingUInt32(langStats->Fround.callCount, "FroundCount"),
                TraceLoggingUInt32(langStats->Fround.debugModeCallCount, "FroundDebugModeCallCount"),
                TraceLoggingUInt32(langStats->IsNaN.callCount, "IsNaNCount"),
                TraceLoggingUInt32(langStats->IsNaN.debugModeCallCount, "IsNaNDebugModeCallCount"),
                TraceLoggingUInt32(langStats->IsFinite.callCount, "IsFiniteCount"),
                TraceLoggingUInt32(langStats->IsFinite.debugModeCallCount, "IsFiniteDebugModeCallCount"),
                TraceLoggingUInt32(langStats->IsInteger.callCount, "IsIntegerCount"),
                TraceLoggingUInt32(langStats->IsInteger.debugModeCallCount, "IsIntegerDebugModeCallCount"),
                TraceLoggingUInt32(langStats->IsSafeInteger.callCount, "IsSafeIntegerCount"),
                TraceLoggingUInt32(langStats->IsSafeInteger.debugModeCallCount, "IsSafeIntegerDebugModeCallCount"),
                TraceLoggingUInt32(langStats->StartsWith.callCount, "StartsWithCount"),
                TraceLoggingUInt32(langStats->StartsWith.debugModeCallCount, "StartsWithDebugModeCallCount"),
                TraceLoggingUInt32(langStats->EndsWith.callCount, "EndsWithCount"),
                TraceLoggingUInt32(langStats->EndsWith.debugModeCallCount, "EndsWithDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Contains.callCount, "ContainsCount"),
                TraceLoggingUInt32(langStats->Contains.debugModeCallCount, "ContainsDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Repeat.callCount, "RepeatCount"),
                TraceLoggingUInt32(langStats->Repeat.debugModeCallCount, "RepeatDebugModeCallCount"),
                TraceLoggingUInt32(langStats->ArrayIncludes.callCount, "arrayIncludesCount"),
                TraceLoggingUInt32(langStats->ArrayIncludes.debugModeCallCount, "arrayIncludesDebugModeCallCount"),
                TraceLoggingUInt32(host, "HostingInterface"),
                TraceLoggingBool(isJSRT, "isJSRT"),
                TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
                );


            TraceLogChakra(
                TL_ES6CTORS,
                TraceLoggingGuid(activityId, "activityId"),
                TraceLoggingUInt32(langStats->WeakMap.callCount, "WeakMapCount"),
                TraceLoggingUInt32(langStats->WeakMap.debugModeCallCount, "WeakMapDebugModeCallCount"),
                TraceLoggingUInt32(langStats->WeakSet.callCount, "WeakSetCount"),
                TraceLoggingUInt32(langStats->WeakSet.debugModeCallCount, "WeakSetDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Set.callCount, "SetCount"),
                TraceLoggingUInt32(langStats->Set.debugModeCallCount, "SetDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Proxy.callCount, "ProxyCount"),
                TraceLoggingUInt32(langStats->Proxy.debugModeCallCount, "ProxyDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Symbol.callCount, "SymbolCount"),
                TraceLoggingUInt32(langStats->Symbol.debugModeCallCount, "SymbolDebugModeCallCount"),
                TraceLoggingUInt32(langStats->Map.callCount, "MapCount"),
                TraceLoggingUInt32(langStats->Map.debugModeCallCount, "MapDebugModeCallCount"),
                TraceLoggingUInt32(host, "HostingInterface"),
                TraceLoggingBool(isJSRT, "isJSRT"),
                TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
                );

            TraceLogChakra(
                TL_TABUILTINS,
                TraceLoggingGuid(activityId, "activityId"),
                TraceLoggingUInt32(langStats->TAFrom.callCount, "TAFromCount"),
                TraceLoggingUInt32(langStats->TAFrom.debugModeCallCount, "TAFromDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAOf.callCount, "TAOfCount"),
                TraceLoggingUInt32(langStats->TAOf.debugModeCallCount, "TAOfDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TACopyWithin.callCount, "TACopyWithinCount"),
                TraceLoggingUInt32(langStats->TACopyWithin.debugModeCallCount, "TACopyWithinDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAEntries.callCount, "TAEntriesCount"),
                TraceLoggingUInt32(langStats->TAEntries.debugModeCallCount, "TAEntriesDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAEvery.callCount, "TAEveryCount"),
                TraceLoggingUInt32(langStats->TAEvery.debugModeCallCount, "TAEveryDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAFilter.callCount, "TAFilterCount"),
                TraceLoggingUInt32(langStats->TAFilter.debugModeCallCount, "TAFilterDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAFill.callCount, "TAFillCount"),
                TraceLoggingUInt32(langStats->TAFill.debugModeCallCount, "TAFillDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAFind.callCount, "TAFindCount"),
                TraceLoggingUInt32(langStats->TAFind.debugModeCallCount, "TAFindDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAFindIndex.callCount, "TAFindIndexCount"),
                TraceLoggingUInt32(langStats->TAFindIndex.debugModeCallCount, "TAFindIndexDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAForEach.callCount, "TAForEachCount"),
                TraceLoggingUInt32(langStats->TAForEach.debugModeCallCount, "TAForEachDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAIndexOf.callCount, "TAIndexOfCount"),
                TraceLoggingUInt32(langStats->TAIndexOf.debugModeCallCount, "TAIndexOfDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAIncludes.callCount, "TAIncludesCount"),
                TraceLoggingUInt32(langStats->TAIncludes.debugModeCallCount, "TAIncludesDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAJoin.callCount, "TAJoinCount"),
                TraceLoggingUInt32(langStats->TAJoin.debugModeCallCount, "TAJoinDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAKeys.callCount, "TAKeysCount"),
                TraceLoggingUInt32(langStats->TAKeys.debugModeCallCount, "TAKeysDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TALastIndexOf.callCount, "TALastIndexOfCount"),
                TraceLoggingUInt32(langStats->TALastIndexOf.debugModeCallCount, "TALastIndexOfDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAMap.callCount, "TAMapCount"),
                TraceLoggingUInt32(langStats->TAMap.debugModeCallCount, "TAMapDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAReduce.callCount, "TAReduceCount"),
                TraceLoggingUInt32(langStats->TAReduce.debugModeCallCount, "TAReduceDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAReduceRight.callCount, "TAReduceRightCount"),
                TraceLoggingUInt32(langStats->TAReduceRight.debugModeCallCount, "TAReduceRightDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAReverse.callCount, "TAReverseCount"),
                TraceLoggingUInt32(langStats->TAReverse.debugModeCallCount, "TAReverseDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TASome.callCount, "TASomeCount"),
                TraceLoggingUInt32(langStats->TASome.debugModeCallCount, "TASomeDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TASort.callCount, "TASortCount"),
                TraceLoggingUInt32(langStats->TASort.debugModeCallCount, "TASortDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TASubArray.callCount, "TASubArrayCount"),
                TraceLoggingUInt32(langStats->TASubArray.debugModeCallCount, "TASubArrayDebugModeCallCount"),
                TraceLoggingUInt32(langStats->TAValues.callCount, "TAValuesCount"),
                TraceLoggingUInt32(langStats->TAValues.debugModeCallCount, "TAValuesDebugModeCallCount"),
                TraceLoggingUInt32(host, "HostingInterface"),
                TraceLoggingBool(isJSRT, "isJSRT"),
                TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
                );

            TraceLogChakra(
                TL_ES6LANGFEATURES,
                TraceLoggingGuid(activityId, "activityId"),
                TraceLoggingUInt32(langStats->Let.parseCount, "LetCount"),
                TraceLoggingUInt32(langStats->Lambda.parseCount, "LambdaCount"),
                TraceLoggingUInt32(langStats->StrictModeFunction.parseCount, "StrictModeFunctionCount"),
                TraceLoggingUInt32(langStats->Super.parseCount, "SuperCount"),
                TraceLoggingUInt32(langStats->Class.parseCount, "ClassCount"),
                TraceLoggingUInt32(langStats->AsmJSFunction.parseCount, "AsmJSFunctionCount"),
                TraceLoggingUInt32(langStats->StringTemplates.parseCount, "StringTemplatesCount"),
                TraceLoggingUInt32(langStats->Const.parseCount, "ConstCount"),
                TraceLoggingUInt32(langStats->Rest.parseCount, "RestCount"),
                TraceLoggingUInt32(langStats->SpreadFeature.parseCount, "SpreadCount"),
                TraceLoggingUInt32(langStats->Generator.parseCount, "GeneratorsCount"),
                TraceLoggingUInt32(langStats->UnicodeRegexFlag.parseCount, "UnicodeRegexFlagCount"),
                TraceLoggingUInt32(langStats->StickyRegexFlag.parseCount, "StickyRegexFlagCount"),
                TraceLoggingUInt32(langStats->DefaultArgFunction.parseCount, "DefaultArgFunctionCount"),
                TraceLoggingUInt32(host, "HostingInterface"),
                TraceLoggingBool(isJSRT, "isJSRT"),
                TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
                );

#ifdef ENABLE_DIRECTCALL_TELEMETRY
            // This is called inside a block that just checked whether we should
            // throttle or not - no need to check again in the callee so call the function
            // that just directly logs instead.

            // Note: the IE perf team uses this heavily, so don't touch it
            FireDomTelemetry(activityId);
#endif

            ResetTelemetryStats(threadContext);
        }
    }
}

#ifdef ENABLE_DIRECTCALL_TELEMETRY
void TraceLoggingClient::FirePeriodicDomTelemetry(GUID activityId)
{
    if (!this->throttle.isThrottled())
    {
        FireDomTelemetry(activityId);
    }
}

void TraceLoggingClient::FireDomTelemetry(GUID activityId)
{
    void *data;
    uint16 dataSize;
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
    threadContext->directCallTelemetry.GetBinaryData(&data, &dataSize);

    TraceLogChakra(
        TL_DIRECTCALLRAW,
        TraceLoggingGuid(activityId, "activityId"),
        TraceLoggingUInt64(threadContext->directCallTelemetry.GetFrequency(), "Frequency"),
        TraceLoggingUInt64(reinterpret_cast<uint64>(threadContext->GetTridentLoadAddress()), "TridentLoadAddress"),
        TraceLoggingBinary(data, dataSize, "Data")
        );
}
#endif

#ifdef ENABLE_DIRECTCALL_TELEMETRY_STATS
void TraceLoggingClient::FireDomTelemetryStats(double tracelogTimeMs, double logTimeMs)
{
    TraceLogChakra(
        TL_DIRECTCALLTIME,
        TraceLoggingFloat64(tracelogTimeMs, "TraceLogTimeInMs"),
        TraceLoggingFloat64(logTimeMs, "MaxLogTimeInMs")
        );
}
#endif


CEventTraceProperties::CEventTraceProperties()
    : m_pEventTraceProperties(reinterpret_cast<EVENT_TRACE_PROPERTIES*>(m_rgData))
{
    memset(m_rgData, 0, sizeof(m_rgData));

    // Configure core structure
    m_pEventTraceProperties->Wnode.BufferSize = sizeof(m_rgData);
    m_pEventTraceProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;

    m_pEventTraceProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    m_pEventTraceProperties->LoggerNameOffset = m_pEventTraceProperties->LogFileNameOffset + s_cbLogFileName;

    // Set up defaults
    m_pEventTraceProperties->Wnode.ClientContext = 1; // EVENT_TRACE_CLOCK_PERFCOUNTER;
    m_pEventTraceProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
}


CEventTraceProperties::operator EVENT_TRACE_PROPERTIES*()
{
    return m_pEventTraceProperties;
}

EVENT_TRACE_PROPERTIES& CEventTraceProperties::Properties()
{
    return *m_pEventTraceProperties;
}

HRESULT CEventTraceProperties::SetLogFileName(_In_ LPCWSTR wszLogFileName)
{
    WCHAR* pDestination = reinterpret_cast<WCHAR*>(m_rgData + m_pEventTraceProperties->LogFileNameOffset);
    return StringCbCopy(pDestination, s_cbLogFileName, wszLogFileName);
}

HRESULT CEventTraceProperties::SetLoggerName(_In_ LPCWSTR wszLoggerName)
{
    WCHAR* pDestination = reinterpret_cast<WCHAR*>(m_rgData + m_pEventTraceProperties->LoggerNameOffset);
    return StringCbCopy(pDestination, s_cbLoggerName, wszLoggerName);
}

CEtwSession::CEtwSession(_In_ LPCWSTR wszLoggerName, _In_ LPCWSTR wszLogFileName, _In_ SessionScope sessionScope)
    : m_rgData(),
    m_SessionHandle()
{
    m_rgData.SetLoggerName(wszLoggerName);
    m_rgData.SetLogFileName(wszLogFileName);

    if (sessionScope == SessionScope_InProcess)
    {
        m_rgData.Properties().LogFileMode |= EVENT_TRACE_PRIVATE_LOGGER_MODE | EVENT_TRACE_PRIVATE_IN_PROC;
    }
    else
    {
        m_rgData.Properties().LogFileMode |= EVENT_TRACE_INDEPENDENT_SESSION_MODE;
    }

    {
        ULONG status = StartTrace(&m_SessionHandle, wszLoggerName, m_rgData);

        if (status != ERROR_SUCCESS)
        {
            throw "Error!";
        }
    }
}

CEtwSession::~CEtwSession()
{
    if (m_SessionHandle)
    {
        (void)ControlTrace(m_SessionHandle, NULL, m_rgData, EVENT_TRACE_CONTROL_STOP);

        m_SessionHandle = 0;
    }
}

HRESULT CEtwSession::EnableProvider(_In_ GUID const& ProviderId)
{
    if (!m_SessionHandle)
    {
        return E_UNEXPECTED;
    }

    ULONG status = EnableTraceEx2(m_SessionHandle, &ProviderId, EVENT_CONTROL_CODE_ENABLE_PROVIDER, 0, 0, 0, INFINITE, NULL);

    if (status != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    return S_OK;
}
