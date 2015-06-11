//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

#include "ieconfig.h"
#include "globalthreadstate.h"
#include <telemetry\MicrosoftTelemetry.h>

// GUID for "ChakraProvider_V0.1": {FC7BA620-EB50-483D-97A0-72D8268A14B5}

TRACELOGGING_DEFINE_PROVIDER(g_hTraceLoggingProv,
    "Microsoft.Web.Platform.Chakra",
    (0xfc7ba620, 0xeb50, 0x483d, 0x97, 0xa0, 0x72, 0xd8, 0x26, 0x8a, 0x14, 0xb5),
    TraceLoggingOptionMicrosoftTelemetry());


#ifdef DBG
#define TL_BINARYFLAVOR "CHK"
#else
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#define TL_BINARYFLAVOR "FRETEST"
#else
#define TL_BINARYFLAVOR "FRE"
#endif
#endif

#if defined(_M_IX86)
#define TL_BINARYARCH "x86"
#elif defined(_M_X64)
#define TL_BINARYARCH "amd64"
#elif defined(_M_ARM)
#define TL_BINARYARCH "arm32"
#elif defined(_M_ARM64)
#define TL_BINARYARCH "arm64"
#else
#error Unknown architecture
#endif


#define TraceLogChakra(name, ...) \
    if (g_TraceLoggingClient->GetShouldLogTelemetry() == true) { \
        TraceLoggingWrite( \
        g_hTraceLoggingProv, \
        name, \
        TraceLoggingKeyword(MICROSOFT_KEYWORD_MEASURES), \
        TraceLoggingString(VER_IEVERSION_STR, "binaryVersion"), \
        TraceLoggingString(TL_BINARYFLAVOR, "binaryFlavor"), \
        TraceLoggingString(TL_BINARYARCH, "binaryArch"), \
        __VA_ARGS__ \
        ); \
    }

WCHAR *g_ProcessExclusionList[] = {
    L"jshost",
    L"jc",
    L"slate",
    L"mshtmpad",
    L"te.processhost"
};


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

    SetIsHighResPerfCounterAvailable(); // Used as a telemetry point

    TraceLoggingRegister(g_hTraceLoggingProv);
}

TraceLoggingClient::~TraceLoggingClient()
{
    TraceLoggingUnregister(g_hTraceLoggingProv);
}

void TraceLoggingClient::SetIsHighResPerfCounterAvailable()
{
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq))
    {
        this->isHighResAvail = false;
    }
    else
    {
        this->isHighResAvail = true;
    }
}

void TraceLoggingClient::ResetTelemetryStats(ThreadContext* threadContext)
{
    if (threadContext != NULL)
    {
        threadContext->GetRecycler()->ResetGCPauseStats();
        threadContext->ResetLangStats();
        threadContext->ResetJITStats();
        threadContext->ResetParserStats();
    }
}

void TraceLoggingClient::FireSiteNavigation(const wchar_t *url, GUID activityId, DWORD host, bool isJSRT )
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
        Js::GCPauseStats stats = threadContext->GetRecycler()->GetGCPauseStats();
        Js::LanguageStats* langStats = threadContext->GetLanguageStats(); 
        uint scriptContextCount = threadContext->GetUnreleasedScriptContextCount(); // No need to reset it as its managed by Chakra
        size_t maxPAUB = 0;
        maxPAUB = PageAllocator::GetAndResetMaxUsedBytes(); // No need for a separate Reset Function as its just one scalar value
        ThreadBoundThreadContextManager::ResetMaxNumberActiveThreadContexts();
        double maxGlobalExecTime = threadContext->GetAndResetMaxGlobalFunctionExecTime(); // Just like above
        JITStats JITstats = threadContext->GetJITStats();
        ParserStats parserStats = threadContext->GetParserStats();

        bool isAnyScriptCtxtInDebugMode = false;

        Js::ScriptContext* contextList = threadContext->GetScriptContextList();
        while (contextList != NULL)
        {
            if (contextList->IsInDebugMode())
            {
                isAnyScriptCtxtInDebugMode = true;
                break;
            }
            contextList = contextList->next;
        }
        

        if (CONFIG_ISENABLED(Js::GCPauseTelFlag))
        {
            Output::Print(L"Max GC pause time is: %f ms\n", stats.maxGCPauseTime);
            Output::Print(L"Mean GC pause time is: %f ms\n", stats.meanGCPauseTime);
            Output::Print(L"GC Pauses < 3 MS :%d\n", stats.lessThan3MS);
            Output::Print(L"3ms < GC Pauses < 7ms :%d\n", stats.within3And7MS);
            Output::Print(L"7ms < GC Pauses < 10ms :%d\n", stats.within7And10MS);
            Output::Print(L"10ms < GC Pauses < 20ms :%d\n", stats.within10And20MS);
            Output::Print(L"20ms < GC Pauses < 50ms :%d\n", stats.within20And50MS);
            Output::Print(L"50ms < GC Pauses < 100ms :%d\n", stats.within50And100MS);
            Output::Print(L"100ms < GC Pauses < 300ms :%d\n", stats.within100And300MS);
            Output::Print(L"GC Pauses > 300ms :%d\n", stats.greaterThan300MS);
            Output::Print(L"Total GC pauseTime :%f ms\n", stats.totalGCPauseTime);
            Output::Print(L"Scriptsite close GC pauseTime :%f ms\n", stats.scriptSiteCloseGCTime);
            Output::Print(L"Unreleased Script Contexts from this URL:%d\n", scriptContextCount);
            Output::Print(L"Max PageAllocator Used Bytes Count:%d\n", maxPAUB);

            Output::Print(L"ParserStats\n");
            Output::Print(L"lessThan1ms: %I64d\n", parserStats.lessThan1ms);
            Output::Print(L"within1And3ms: %I64d\n", parserStats.within1And3ms);
            Output::Print(L"within3And10ms: %I64d\n", parserStats.within3And10ms);
            Output::Print(L"within10And20ms: %I64d\n", parserStats.within10And20ms);
            Output::Print(L"within20And50ms: %I64d\n", parserStats.within20And50ms);
            Output::Print(L"within50And100ms: %I64d\n", parserStats.within50And100ms);
            Output::Print(L"within100And300ms: %I64d\n", parserStats.within100And300ms);
            Output::Print(L"greaterThan300ms: %I64d\n", parserStats.greaterThan300ms);			
        }


        if (langStats != NULL && CONFIG_ISENABLED(Js::ES5LangTelFlag))
        {
            Output::Print(L"Array.isArray count: %d\n", langStats->ArrayisArrayCount.callCount);
            Output::Print(L"Array.prototype.indexOf count: %d\n", langStats->ArrayIndexOfCount.callCount);
            Output::Print(L"Array.prototype.every count: %d\n", langStats->ArrayEveryCount.callCount);
            Output::Print(L"Array.prototype.filter count: %d\n", langStats->ArrayFilterCount.callCount);
            Output::Print(L"Array.prototype.forEach count: %d\n", langStats->ArrayForEachCount.callCount);
            Output::Print(L"Array.prototype.lastIndexOf count: %d\n", langStats->ArrayLastIndexOfCount.callCount);
            Output::Print(L"Array.prototype.map count: %d\n", langStats->ArrayMapCount.callCount);
            Output::Print(L"Array.prototype.reduce count: %d\n", langStats->ArrayReduceCount.callCount);
            Output::Print(L"Array.prototype.reduceRight count: %d\n", langStats->ArrayReduceRightCount.callCount);
            Output::Print(L"Array.prototype.some count: %d\n", langStats->ArraySomeCount.callCount);
            Output::Print(L"Object.keys count: %d\n", langStats->ObjectKeysCount.callCount);
            Output::Print(L"Object.getOwnPropertyNames count: %d\n", langStats->ObjectGetOwnPropertyNamesCount.callCount);
            Output::Print(L"Object.create count: %d\n", langStats->ObjectCreateCount.callCount);
            Output::Print(L"Object.defineProperties count: %d\n", langStats->ObjectDefinePropertiesCount.callCount);
            Output::Print(L"Object.freeze count: %d\n", langStats->ObjectFreezeCount.callCount);
            Output::Print(L"Object.seal count: %d\n", langStats->ObjectSealCount.callCount);
            Output::Print(L"Object.getPrototypeOf count: %d\n", langStats->ObjectGetPrototypeOfCount.callCount);
            Output::Print(L"Object.isFrozen count: %d\n", langStats->ObjectIsFrozenCount.callCount);
            Output::Print(L"Object.isSealed count: %d\n", langStats->ObjectIsSealedCount.callCount);
            Output::Print(L"Object.isExtensible count: %d\n", langStats->ObjectIsExtensibleCount.callCount);
            Output::Print(L"Object.preventExtensions count: %d\n", langStats->ObjectPreventExtensionCount.callCount);
            Output::Print(L"Date.prototype.toISOString count: %d\n", langStats->DateToISOStringCount.callCount);
            Output::Print(L"Function.prototype.bind count: %d\n", langStats->FunctionBindCount.callCount);
            // Debug Mode call count
            Output::Print(L"Array.isArray debug mode call count: %d\n", langStats->ArrayisArrayCount.debugModeCallCount);
            Output::Print(L"Array.prototype.indexOf debug mode call count: %d\n", langStats->ArrayIndexOfCount.debugModeCallCount);
            Output::Print(L"Array.prototype.every debug mode call count: %d\n", langStats->ArrayEveryCount.debugModeCallCount);
            Output::Print(L"Array.prototype.filter debug mode call count: %d\n", langStats->ArrayFilterCount.debugModeCallCount);
            Output::Print(L"Array.prototype.forEach debug mode call count: %d\n", langStats->ArrayForEachCount.debugModeCallCount);
            Output::Print(L"Array.prototype.lastIndexOf debug mode call count: %d\n", langStats->ArrayLastIndexOfCount.debugModeCallCount);
            Output::Print(L"Array.prototype.map debug mode call count: %d\n", langStats->ArrayMapCount.debugModeCallCount);
            Output::Print(L"Array.prototype.reduce debug mode call count: %d\n", langStats->ArrayReduceCount.debugModeCallCount);
            Output::Print(L"Array.prototype.reduceRight debug mode call count: %d\n", langStats->ArrayReduceRightCount.debugModeCallCount);
            Output::Print(L"Array.prototype.some debug mode call count: %d\n", langStats->ArraySomeCount.debugModeCallCount);
            Output::Print(L"Object.keys debug mode call count: %d\n", langStats->ObjectKeysCount.debugModeCallCount);
            Output::Print(L"Object.getOwnPropertyNames debug mode call count: %d\n", langStats->ObjectGetOwnPropertyNamesCount.debugModeCallCount);
            Output::Print(L"Object.create debug mode call count: %d\n", langStats->ObjectCreateCount.debugModeCallCount);
            Output::Print(L"Object.defineProperties debug mode call count: %d\n", langStats->ObjectDefinePropertiesCount.debugModeCallCount);
            Output::Print(L"Object.freeze debug mode call count: %d\n", langStats->ObjectFreezeCount.debugModeCallCount);
            Output::Print(L"Object.seal debug mode call count: %d\n", langStats->ObjectSealCount.debugModeCallCount);
            Output::Print(L"Object.getPrototypeOf debug mode call count: %d\n", langStats->ObjectGetPrototypeOfCount.debugModeCallCount);
            Output::Print(L"Object.isFrozen debug mode call count: %d\n", langStats->ObjectIsFrozenCount.debugModeCallCount);
            Output::Print(L"Object.isSealed debug mode call count: %d\n", langStats->ObjectIsSealedCount.debugModeCallCount);
            Output::Print(L"Object.isExtensible debug mode call count: %d\n", langStats->ObjectIsExtensibleCount.debugModeCallCount);
            Output::Print(L"Object.preventExtensions debug mode call count: %d\n", langStats->ObjectPreventExtensionCount.debugModeCallCount);
            Output::Print(L"Date.prototype.toISOString debug mode call count: %d\n", langStats->DateToISOStringCount.debugModeCallCount);
            Output::Print(L"Function.prototype.bind debug mode call count: %d\n", langStats->FunctionBindCount.debugModeCallCount);
        }

        if (langStats != NULL && CONFIG_ISENABLED(Js::ES6LangTelFlag))
        {
            Output::Print(L"GetOwnPropertySymbolsCount %d\n", langStats->GetOwnPropertySymbolsCount.callCount);
            Output::Print(L"GetOwnPropertySymbolsDebugModeCallCount %d\n", langStats->GetOwnPropertySymbolsCount.debugModeCallCount);
            Output::Print(L"Log10Count %d\n", langStats->Log10Count.callCount);
            Output::Print(L"Log10DebugModeCount %d\n", langStats->Log10Count.debugModeCallCount);
            Output::Print(L"Log1pCountCount %d\n", langStats->Log1pCount.callCount);
            Output::Print(L"Log1pDebugModeCallCount %d\n", langStats->Log1pCount.debugModeCallCount);
            Output::Print(L"Log2Count %d\n", langStats->Log2Count.callCount);
            Output::Print(L"Log2DebugModeCallCount %d\n", langStats->Log2Count.debugModeCallCount);
            Output::Print(L"SinhCount %d\n", langStats->SinhCount.callCount);
            Output::Print(L"SinhDebugModeCallCount %d\n", langStats->SinhCount.debugModeCallCount);
            Output::Print(L"CoshCount %d\n", langStats->CoshCount.callCount);
            Output::Print(L"CoshDebugModeCallCount %d\n", langStats->CoshCount.debugModeCallCount);
            Output::Print(L"tanhCountCount %d\n", langStats->TanhCount.callCount);
            Output::Print(L"tanhDebugModeCallCount %d\n", langStats->TanhCount.debugModeCallCount);
            Output::Print(L"AsinhCount %d\n", langStats->AsinhCount.callCount);
            Output::Print(L"AsinhDebugModeCallCount %d\n", langStats->AsinhCount.debugModeCallCount);
            Output::Print(L"AcoshCount %d\n", langStats->AcoshCount.callCount);
            Output::Print(L"AcoshDebugModeCallCount %d\n", langStats->AcoshCount.debugModeCallCount);
            Output::Print(L"AtanhCount %d\n", langStats->AtanhCount.callCount);
            Output::Print(L"AtanhDebugModeCallCount %d\n", langStats->AtanhCount.debugModeCallCount);
            Output::Print(L"HypotCount %d\n", langStats->HypotCount.callCount);
            Output::Print(L"HypotDebugModeCallCount %d\n", langStats->HypotCount.debugModeCallCount);
            Output::Print(L"CbrtCount %d\n", langStats->CbrtCount.callCount);
            Output::Print(L"CbrtDebugModeCallCount %d\n", langStats->CbrtCount.debugModeCallCount);
            Output::Print(L"TruncCount %d\n", langStats->TruncCount.callCount);
            Output::Print(L"TruncDebugModeCallCount %d\n", langStats->TruncCount.debugModeCallCount);
            Output::Print(L"SignCount %d\n", langStats->SignCount.callCount);
            Output::Print(L"SignDebugModeCallCount %d\n", langStats->SignCount.debugModeCallCount);
            Output::Print(L"ImulCount %d\n", langStats->ImulCount.callCount);
            Output::Print(L"ImulDebugModeCallCount %d\n", langStats->ImulCount.debugModeCallCount);
            Output::Print(L"Clz32Count %d\n", langStats->Clz32Count.callCount);
            Output::Print(L"Clz32DebugModeCallCount %d\n", langStats->Clz32Count.debugModeCallCount);
            Output::Print(L"FroundCount %d\n", langStats->FroundCount.callCount);
            Output::Print(L"FroundDebugModeCallCount %d\n", langStats->FroundCount.debugModeCallCount);
            Output::Print(L"IsNaNCount %d\n", langStats->IsNaNCount.callCount);
            Output::Print(L"IsNaNDebugModeCallCount %d\n", langStats->IsNaNCount.debugModeCallCount);
            Output::Print(L"IsFiniteCount %d\n", langStats->IsFiniteCount.callCount);
            Output::Print(L"IsFiniteDebugModeCallCount %d\n", langStats->IsFiniteCount.debugModeCallCount);
            Output::Print(L"IsIntegerCount %d\n", langStats->IsIntegerCount.callCount);
            Output::Print(L"IsIntegerDebugModeCallCount %d\n", langStats->IsIntegerCount.debugModeCallCount);
            Output::Print(L"IsSafeIntegerCount %d\n", langStats->IsSafeIntegerCount.callCount);
            Output::Print(L"IsSafeIntegerDebugModeCallCount %d\n", langStats->IsSafeIntegerCount.debugModeCallCount);
            Output::Print(L"StartsWithCount %d\n", langStats->StartsWithCount.callCount);
            Output::Print(L"StartsWithDebugModeCallCount %d\n", langStats->StartsWithCount.debugModeCallCount);
            Output::Print(L"EndsWithCount %d\n", langStats->EndsWithCount.callCount);
            Output::Print(L"EndsWithDebugModeCallCount %d\n", langStats->EndsWithCount.debugModeCallCount);
            Output::Print(L"ContainsCount %d\n", langStats->ContainsCount.callCount);
            Output::Print(L"ContainsDebugModeCallCount %d\n", langStats->ContainsCount.debugModeCallCount);
            Output::Print(L"RepeatCount %d\n", langStats->RepeatCount.callCount);
            Output::Print(L"RepeatDebugModeCallCount %d\n", langStats->RepeatCount.debugModeCallCount);
            Output::Print(L"PromiseCount %d\n", langStats->PromiseCount.callCount);
            Output::Print(L"PromiseDebugModeCallCount %d\n", langStats->PromiseCount.debugModeCallCount);
            Output::Print(L"LetCount %d\n", langStats->LetCount.parseCount);
            Output::Print(L"LambdaCount %d\n", langStats->LambdaCount.parseCount);
            Output::Print(L"ConstCount %d\n", langStats->ConstCount.parseCount);
            Output::Print(L"SuperCount %d\n", langStats->SuperCount.parseCount);
            Output::Print(L"AsmJSFunctionCount %d\n", langStats->AsmJSFunctionCount.parseCount);
            Output::Print(L"StrictModeFunctionCount %d\n", langStats->StrictModeFunctionCount.parseCount);
            Output::Print(L"ClassCount %d\n", langStats->ClassCount.parseCount);
            Output::Print(L"StringTemplatesCount %d\n", langStats->StringTemplatesCount.parseCount);
            Output::Print(L"GeneratorsCount %d\n", langStats->GeneratorCount.parseCount);
            Output::Print(L"RestCount %d\n", langStats->RestCount.parseCount);
            Output::Print(L"SpreadCount %d\n", langStats->SpreadFeatureCount.parseCount);
            Output::Print(L"DefaultCount %d\n", langStats->DefaultArgFunctionCount.parseCount);
            Output::Print(L"StickyRegexFlagCount %d\n", langStats->StickyRegexFlagCount.parseCount);
            Output::Print(L"UnicodeRegexFlagCount %d\n", langStats->UnicodeRegexFlagCount.parseCount);

        }

        if (url != NULL && (CONFIG_ISENABLED(Js::ES6LangTelFlag) || CONFIG_ISENABLED(Js::ES5LangTelFlag) || CONFIG_ISENABLED(Js::GCPauseTelFlag)))
        {
            Output::Print(L"Navigated from site: %s\n", url);
        }
        
        // Note: must be thread-safe.
        TraceLogChakra(
            TL_ES5BUILTINS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt32(langStats->ArrayisArrayCount.callCount, "arrayisArrayCount"),
            TraceLoggingUInt32(langStats->ArrayIndexOfCount.callCount, "arrayIndexOfCount"),
            TraceLoggingUInt32(langStats->ArrayEveryCount.callCount, "arrayEveryCount"),
            TraceLoggingUInt32(langStats->ArrayFilterCount.callCount, "arrayFilterCount"),
            TraceLoggingUInt32(langStats->ArrayForEachCount.callCount, "arrayForEachCount"),
            TraceLoggingUInt32(langStats->ArrayLastIndexOfCount.callCount, "arrayLastIndexOfCount"),
            TraceLoggingUInt32(langStats->ArrayMapCount.callCount, "arrayMapCount"),
            TraceLoggingUInt32(langStats->ArrayReduceCount.callCount, "arrayReduceCount"),
            TraceLoggingUInt32(langStats->ArrayReduceRightCount.callCount, "arrayReduceRightCount"),
            TraceLoggingUInt32(langStats->ArraySomeCount.callCount, "arraySomeCount"),
            TraceLoggingUInt32(langStats->ObjectCreateCount.callCount, "objectCreateCount"),
            TraceLoggingUInt32(langStats->ObjectDefinePropertiesCount.callCount, "objectDefinePropertiesCount"),
            TraceLoggingUInt32(langStats->ObjectFreezeCount.callCount, "objectFreezeCount"),
            TraceLoggingUInt32(langStats->ObjectSealCount.callCount, "objectSealCount"),
            TraceLoggingUInt32(langStats->ObjectGetOwnPropertyNamesCount.callCount, "objectGetOwnPropertyNamesCount"),
            TraceLoggingUInt32(langStats->ObjectGetPrototypeOfCount.callCount, "objectGetPrototypeOfCount"),
            TraceLoggingUInt32(langStats->ObjectIsExtensibleCount.callCount, "objectIsExtensibleCount"),
            TraceLoggingUInt32(langStats->ObjectIsFrozenCount.callCount, "objectIsFrozenCount"),
            TraceLoggingUInt32(langStats->ObjectIsSealedCount.callCount, "objectIsSealedCount"),
            TraceLoggingUInt32(langStats->ObjectKeysCount.callCount, "objectKeysCount"),
            TraceLoggingUInt32(langStats->ObjectPreventExtensionCount.callCount, "objectPreventExtensionCount"),
            TraceLoggingUInt32(langStats->DateToISOStringCount.callCount, "dateToISOStringCount"),
            TraceLoggingUInt32(langStats->FunctionBindCount.callCount, "functionBindCount"),
            TraceLoggingUInt32(langStats->StringTrimCount.callCount, "stringTrimCount"),
            TraceLoggingUInt32(langStats->ArrayisArrayCount.debugModeCallCount, "arrayisArrayDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayIndexOfCount.debugModeCallCount, "arrayIndexOfDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayEveryCount.debugModeCallCount, "arrayEveryDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayFilterCount.debugModeCallCount, "arrayFilterDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayForEachCount.debugModeCallCount, "arrayForEachDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayLastIndexOfCount.debugModeCallCount, "arrayLastIndexOfDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayMapCount.debugModeCallCount, "arrayMapDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayReduceCount.debugModeCallCount, "arrayReduceDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArrayReduceRightCount.debugModeCallCount, "arrayReduceRightDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ArraySomeCount.debugModeCallCount, "arraySomeDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectCreateCount.debugModeCallCount, "objectCreateDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectDefinePropertiesCount.debugModeCallCount, "objectDefinePropertiesDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectFreezeCount.debugModeCallCount, "objectFreezeDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectSealCount.debugModeCallCount, "objectSealDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectGetOwnPropertyNamesCount.debugModeCallCount, "objectGetOwnPropertyNamesDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectGetPrototypeOfCount.debugModeCallCount, "objectGetPrototypeOfDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectIsExtensibleCount.debugModeCallCount, "objectIsExtensibleDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectIsFrozenCount.debugModeCallCount, "objectIsFrozenDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectIsSealedCount.debugModeCallCount, "objectIsSealedDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectKeysCount.debugModeCallCount, "objectKeysDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ObjectPreventExtensionCount.debugModeCallCount, "objectPreventExtensionDebugModeCallCount"),
            TraceLoggingUInt32(langStats->DateToISOStringCount.debugModeCallCount, "dateToISOStringDebugModeCallCount"),
            TraceLoggingUInt32(langStats->FunctionBindCount.debugModeCallCount, "functionBindDebugModeCallCount"),
            TraceLoggingUInt32(langStats->StringTrimCount.debugModeCallCount, "stringTrimDebugModeCallCount"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );


        TraceLogChakra(
            TL_ES6BUILTINS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt32(langStats->GetOwnPropertySymbolsCount.callCount, "GetOwnPropertySymbolsCount"),
            TraceLoggingUInt32(langStats->GetOwnPropertySymbolsCount.debugModeCallCount, "GetOwnPropertySymbolsDebugModeCallCount"),
            TraceLoggingUInt32(langStats->Log10Count.callCount, "Log10Count"),
            TraceLoggingUInt32(langStats->Log10Count.debugModeCallCount, "Log10DebugModeCount"),
            TraceLoggingUInt32(langStats->Log1pCount.callCount, "Log1pCountCount"),
            TraceLoggingUInt32(langStats->Log1pCount.debugModeCallCount, "Log1pDebugModeCallCount"),
            TraceLoggingUInt32(langStats->Log2Count.callCount, "Log2Count"),
            TraceLoggingUInt32(langStats->Log2Count.debugModeCallCount, "Log2DebugModeCallCount"),
            TraceLoggingUInt32(langStats->SinhCount.callCount, "SinhCount"),
            TraceLoggingUInt32(langStats->SinhCount.debugModeCallCount, "SinhDebugModeCallCount"),
            TraceLoggingUInt32(langStats->CoshCount.callCount, "CoshCount"),
            TraceLoggingUInt32(langStats->CoshCount.debugModeCallCount, "CoshDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TanhCount.callCount, "TanhCountCount"),
            TraceLoggingUInt32(langStats->TanhCount.debugModeCallCount, "TanhDebugModeCallCount"),
            TraceLoggingUInt32(langStats->AsinhCount.callCount, "AsinhCount"),
            TraceLoggingUInt32(langStats->AsinhCount.debugModeCallCount, "AsinhDebugModeCallCount"),
            TraceLoggingUInt32(langStats->AcoshCount.callCount, "AcoshCount"),
            TraceLoggingUInt32(langStats->AcoshCount.debugModeCallCount, "AcoshDebugModeCallCount"),
            TraceLoggingUInt32(langStats->AtanhCount.callCount, "AtanhCount"),
            TraceLoggingUInt32(langStats->AtanhCount.debugModeCallCount, "AtanhDebugModeCallCount"),
            TraceLoggingUInt32(langStats->HypotCount.callCount, "HypotCount"),
            TraceLoggingUInt32(langStats->HypotCount.debugModeCallCount, "HypotDebugModeCallCount"),
            TraceLoggingUInt32(langStats->CbrtCount.callCount, "CbrtCount"),
            TraceLoggingUInt32(langStats->CbrtCount.debugModeCallCount, "CbrtDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TruncCount.callCount, "TruncCount"),
            TraceLoggingUInt32(langStats->TruncCount.debugModeCallCount, "TruncDebugModeCallCount"),
            TraceLoggingUInt32(langStats->SignCount.callCount, "SignCount"),
            TraceLoggingUInt32(langStats->SignCount.debugModeCallCount, "SignDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ImulCount.callCount, "ImulCount"),
            TraceLoggingUInt32(langStats->ImulCount.debugModeCallCount, "ImulDebugModeCallCount"),
            TraceLoggingUInt32(langStats->Clz32Count.callCount, "Clz32Count"),
            TraceLoggingUInt32(langStats->Clz32Count.debugModeCallCount, "Clz32DebugModeCallCount"),
            TraceLoggingUInt32(langStats->FroundCount.callCount, "FroundCount"),
            TraceLoggingUInt32(langStats->FroundCount.debugModeCallCount, "FroundDebugModeCallCount"),
            TraceLoggingUInt32(langStats->IsNaNCount.callCount, "IsNaNCount"),
            TraceLoggingUInt32(langStats->IsNaNCount.debugModeCallCount, "IsNaNDebugModeCallCount"),
            TraceLoggingUInt32(langStats->IsFiniteCount.callCount, "IsFiniteCount"),
            TraceLoggingUInt32(langStats->IsFiniteCount.debugModeCallCount, "IsFiniteDebugModeCallCount"),
            TraceLoggingUInt32(langStats->IsIntegerCount.callCount, "IsIntegerCount"),
            TraceLoggingUInt32(langStats->IsIntegerCount.debugModeCallCount, "IsIntegerDebugModeCallCount"),
            TraceLoggingUInt32(langStats->IsSafeIntegerCount.callCount, "IsSafeIntegerCount"),
            TraceLoggingUInt32(langStats->IsSafeIntegerCount.debugModeCallCount, "IsSafeIntegerDebugModeCallCount"),
            TraceLoggingUInt32(langStats->StartsWithCount.callCount, "StartsWithCount"),
            TraceLoggingUInt32(langStats->StartsWithCount.debugModeCallCount, "StartsWithDebugModeCallCount"),
            TraceLoggingUInt32(langStats->EndsWithCount.callCount, "EndsWithCount"),
            TraceLoggingUInt32(langStats->EndsWithCount.debugModeCallCount, "EndsWithDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ContainsCount.callCount, "ContainsCount"),
            TraceLoggingUInt32(langStats->ContainsCount.debugModeCallCount, "ContainsDebugModeCallCount"),
            TraceLoggingUInt32(langStats->RepeatCount.callCount, "RepeatCount"),
            TraceLoggingUInt32(langStats->RepeatCount.debugModeCallCount, "RepeatDebugModeCallCount"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );


        TraceLogChakra(
            TL_ES6CTORS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt32(langStats->WeakMapCount.callCount, "WeakMapCount"),
            TraceLoggingUInt32(langStats->WeakMapCount.debugModeCallCount, "WeakMapDebugModeCallCount"),
            TraceLoggingUInt32(langStats->WeakSetCount.callCount, "WeakSetCount"),
            TraceLoggingUInt32(langStats->WeakSetCount.debugModeCallCount, "WeakSetDebugModeCallCount"),
            TraceLoggingUInt32(langStats->SetCount.callCount, "SetCount"),
            TraceLoggingUInt32(langStats->SetCount.debugModeCallCount, "SetDebugModeCallCount"),
            TraceLoggingUInt32(langStats->ProxyCount.callCount, "ProxyCount"),
            TraceLoggingUInt32(langStats->ProxyCount.debugModeCallCount, "ProxyDebugModeCallCount"),
            TraceLoggingUInt32(langStats->SymbolCount.callCount, "SymbolCount"),
            TraceLoggingUInt32(langStats->SymbolCount.debugModeCallCount, "SymbolDebugModeCallCount"),
            TraceLoggingUInt32(langStats->MapCount.callCount, "MapCount"),
            TraceLoggingUInt32(langStats->MapCount.debugModeCallCount, "MapDebugModeCallCount"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );

        TraceLogChakra(
            TL_TABUILTINS,
            TraceLoggingGuid(activityId, "activityId"),           
            TraceLoggingUInt32(langStats->TAFromCount.callCount, "TAFromCount"),
            TraceLoggingUInt32(langStats->TAFromCount.debugModeCallCount, "TAFromDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAOfCount.callCount, "TAOfCount"),
            TraceLoggingUInt32(langStats->TAOfCount.debugModeCallCount, "TAOfDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TACopyWithinCount.callCount, "TACopyWithinCount"),
            TraceLoggingUInt32(langStats->TACopyWithinCount.debugModeCallCount, "TACopyWithinDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAEntriesCount.callCount, "TAEntriesCount"),
            TraceLoggingUInt32(langStats->TAEntriesCount.debugModeCallCount, "TAEntriesDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAEveryCount.callCount, "TAEveryCount"),
            TraceLoggingUInt32(langStats->TAEveryCount.debugModeCallCount, "TAEveryDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAFilterCount.callCount, "TAFilterCount"),
            TraceLoggingUInt32(langStats->TAFilterCount.debugModeCallCount, "TAFilterDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAFillCount.callCount, "TAFillCount"),
            TraceLoggingUInt32(langStats->TAFillCount.debugModeCallCount, "TAFillDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAFindCount.callCount, "TAFindCount"),
            TraceLoggingUInt32(langStats->TAFindCount.debugModeCallCount, "TAFindDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAFindIndexCount.callCount, "TAFindIndexCount"),
            TraceLoggingUInt32(langStats->TAFindIndexCount.debugModeCallCount, "TAFindIndexDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAForEachCount.callCount, "TAForEachCount"),
            TraceLoggingUInt32(langStats->TAForEachCount.debugModeCallCount, "TAForEachDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAIndexOfCount.callCount, "TAIndexOfCount"),
            TraceLoggingUInt32(langStats->TAIndexOfCount.debugModeCallCount, "TAIndexOfDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAJoinCount.callCount, "TAJoinCount"),
            TraceLoggingUInt32(langStats->TAJoinCount.debugModeCallCount, "TAJoinDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAKeysCount.callCount, "TAKeysCount"),
            TraceLoggingUInt32(langStats->TAKeysCount.debugModeCallCount, "TAKeysDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TALastIndexOfCount.callCount, "TALastIndexOfCount"),
            TraceLoggingUInt32(langStats->TALastIndexOfCount.debugModeCallCount, "TALastIndexOfDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAMapCount.callCount, "TAMapCount"),
            TraceLoggingUInt32(langStats->TAMapCount.debugModeCallCount, "TAMapDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAReduceCount.callCount, "TAReduceCount"),
            TraceLoggingUInt32(langStats->TAReduceCount.debugModeCallCount, "TAReduceDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAReduceRightCount.callCount, "TAReduceRightCount"),
            TraceLoggingUInt32(langStats->TAReduceRightCount.debugModeCallCount, "TAReduceRightDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAReverseCount.callCount, "TAReverseCount"),
            TraceLoggingUInt32(langStats->TAReverseCount.debugModeCallCount, "TAReverseDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TASomeCount.callCount, "TASomeCount"),
            TraceLoggingUInt32(langStats->TASomeCount.debugModeCallCount, "TASomeDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TASortCount.callCount, "TASortCount"),
            TraceLoggingUInt32(langStats->TASortCount.debugModeCallCount, "TASortDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TASubArrayCount.callCount, "TASubArrayCount"),
            TraceLoggingUInt32(langStats->TASubArrayCount.debugModeCallCount, "TASubArrayDebugModeCallCount"),
            TraceLoggingUInt32(langStats->TAValuesCount.callCount, "TAValuesCount"),
            TraceLoggingUInt32(langStats->TAValuesCount.debugModeCallCount, "TAValuesDebugModeCallCount"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );

        TraceLogChakra(
            TL_ES6LANGFEATURES,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt32(langStats->LetCount.parseCount, "LetCount"),
            TraceLoggingUInt32(langStats->LambdaCount.parseCount, "LambdaCount"),
            TraceLoggingUInt32(langStats->StrictModeFunctionCount.parseCount, "StrictModeFunctionCount"),
            TraceLoggingUInt32(langStats->SuperCount.parseCount, "SuperCount"),
            TraceLoggingUInt32(langStats->ClassCount.parseCount, "ClassCount"),
            TraceLoggingUInt32(langStats->AsmJSFunctionCount.parseCount, "AsmJSFunctionCount"),
            TraceLoggingUInt32(langStats->StringTemplatesCount.parseCount, "StringTemplatesCount"),
            TraceLoggingUInt32(langStats->ConstCount.parseCount, "ConstCount"),
            TraceLoggingUInt32(langStats->RestCount.parseCount, "RestCount"),
            TraceLoggingUInt32(langStats->SpreadFeatureCount.parseCount, "SpreadCount"),
            TraceLoggingUInt32(langStats->GeneratorCount.parseCount, "GeneratorsCount"),
            TraceLoggingUInt32(langStats->UnicodeRegexFlagCount.parseCount, "UnicodeRegexFlagCount"),
            TraceLoggingUInt32(langStats->StickyRegexFlagCount.parseCount, "StickyRegexFlagCount"),
            TraceLoggingUInt32(langStats->DefaultArgFunctionCount.parseCount, "DefaultArgFunctionCount"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );

       

        // Note: must be thread-safe.
        TraceLogChakra(
            TL_GCPAUSESTATS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingFloat64(stats.maxGCPauseTime, "maxGCPauseTime"),
            TraceLoggingFloat64(stats.totalGCPauseTime, "totalGCPauseTime"),
            TraceLoggingFloat64(stats.meanGCPauseTime, "meanGCPauseTime"),
            TraceLoggingFloat64(stats.scriptSiteCloseGCTime, "scriptSiteCloseGCPauseTime"),
            TraceLoggingUInt32(stats.lessThan3MS, "lessThan3ms"),
            TraceLoggingUInt32(stats.within3And7MS, "within3And7ms"),
            TraceLoggingUInt32(stats.within7And10MS, "within7And10ms"),
            TraceLoggingUInt32(stats.within10And20MS, "within10And20ms"),
            TraceLoggingUInt32(stats.within20And50MS, "within20And50ms"),
            TraceLoggingUInt32(stats.greaterThan50MS, "greaterThan50ms"),
            TraceLoggingUInt32(stats.within50And100MS, "within50And100ms"),
            TraceLoggingUInt32(stats.within100And300MS, "within100And300ms"),
            TraceLoggingUInt32(stats.greaterThan300MS, "greaterThan300ms"),
            TraceLoggingUInt32(scriptContextCount, "unreleasedScriptContextCount"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingBool(isAnyScriptCtxtInDebugMode, "isAnyScriptContextInDebugMode"),
            TraceLoggingBool(isHighResAvail, "isHighResPerfCounterAvailable"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );

        // Note: must be thread-safe.
        TraceLogChakra(
            TL_JITTIMESTATS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt32(JITstats.lessThan5ms, "lessThan5ms"),
            TraceLoggingUInt32(JITstats.within5And10ms, "within5And10ms"),
            TraceLoggingUInt32(JITstats.within10And20ms, "within10And20ms"),
            TraceLoggingUInt32(JITstats.within20And50ms, "within20And50ms"),
            TraceLoggingUInt32(JITstats.within50And100ms, "within50And100ms"),
            TraceLoggingUInt32(JITstats.within100And300ms, "within100And300ms"),
            TraceLoggingUInt32(JITstats.greaterThan300ms, "greaterThan300ms"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingBool(isAnyScriptCtxtInDebugMode, "isAnyScriptContextInDebugMode"),
            TraceLoggingBool(isHighResAvail, "isHighResPerfCounterAvailable"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );

        TraceLogChakra(
            TL_GLOBALSTATS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingFloat64(maxGlobalExecTime, "maxGlobalFunctionExecTime"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingBool(isAnyScriptCtxtInDebugMode, "isAnyScriptContextInDebugMode"),
            TraceLoggingBool(isHighResAvail, "isHighResPerfCounterAvailable"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );


        TraceLogChakra(
            TL_PARSERSTATS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt64(parserStats.lessThan1ms, "lessThan1ms"),
            TraceLoggingUInt64(parserStats.within1And3ms, "within1And3ms"),
            TraceLoggingUInt64(parserStats.within3And10ms, "within3And10ms"),
            TraceLoggingUInt64(parserStats.within10And20ms, "within10And20ms"),
            TraceLoggingUInt64(parserStats.within20And50ms, "within20And50ms"),
            TraceLoggingUInt64(parserStats.within50And100ms, "within50And100ms"),
            TraceLoggingUInt64(parserStats.within100And300ms, "within100And300ms"),
            TraceLoggingUInt64(parserStats.greaterThan300ms, "greaterThan300ms"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingBool(isAnyScriptCtxtInDebugMode, "isAnyScriptContextInDebugMode"),
            TraceLoggingBool(isHighResAvail, "isHighResPerfCounterAvailable"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );

        TraceLogChakra(
            TL_MEMSTATS,
            TraceLoggingGuid(activityId, "activityId"),
            TraceLoggingUInt64(maxPAUB, "maxPAUB"),
            TraceLoggingUInt32(ThreadBoundThreadContextManager::s_maxNumberActiveThreadContexts, "maxActiveThreadContexts"),
            TraceLoggingUInt32(host, "HostingInterface"),
            TraceLoggingBool(isJSRT, "isJSRT"),
            TraceLoggingBool(isAnyScriptCtxtInDebugMode, "isAnyScriptContextInDebugMode"),
            TraceLoggingBool(isHighResAvail, "isHighResPerfCounterAvailable"),
            TraceLoggingPointer(threadContext->GetJSRTRuntime(), "JsrtRuntime")
            );

        ResetTelemetryStats(threadContext);

    }
}


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
