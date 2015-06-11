//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{

    #define EngineInterfaceObject_CommonFunctionProlog(function, callInfo) \
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault); \
        RUNTIME_ARGUMENTS(args, callInfo); \
        Assert(!(callInfo.Flags & CallFlags_New)); \
        unsigned argCount = args.Info.Count; \
        ScriptContext* scriptContext = function->GetScriptContext(); \
        AssertMsg(argCount > 0, "Should always have implicit 'this'"); \

    class EngineInterfaceObject : public DynamicObject
    {
    private:
        DEFINE_VTABLE_CTOR(EngineInterfaceObject, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(EngineInterfaceObject);

        DynamicObject* commonNativeInterfaces;
        DynamicObject* intlNativeInterfaces;
        DynamicObject* promiseNativeInterfaces;

#ifdef ENABLE_INTL_OBJECT
        JavascriptFunction* dateToLocaleString;
        JavascriptFunction* dateToLocaleTimeString;
        JavascriptFunction* dateToLocaleDateString;
        JavascriptFunction* numberToLocaleString;
        JavascriptFunction* stringLocaleCompare;

        FunctionBody* intlByteCode;
#endif
        FunctionBody* promiseByteCode;

        void EnsureIntlByteCode(_In_ ScriptContext * scriptContext);
        void EnsurePromiseByteCode(_In_ ScriptContext * scriptContext);

    public:
        EngineInterfaceObject(DynamicType * type) : DynamicObject(type) {}

        static EngineInterfaceObject* New(Recycler * recycler, DynamicType * type);
        static bool Is(Var aValue);
        static EngineInterfaceObject* FromVar(Var aValue);

        void Initialize();
#if DBG
        void DumpIntlByteCode(_In_ ScriptContext * scriptContext);
        void DumpPromiseByteCode(_In_ ScriptContext * scriptContext);
#endif

        static void __cdecl InitializeCommonNativeInterfaces(DynamicObject* engineInterface, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializeIntlNativeInterfaces(DynamicObject* intlNativeInterfaces, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);
        static void __cdecl InitializePromiseNativeInterfaces(DynamicObject* promiseNativeInterfaces, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

#ifdef ENABLE_INTL_OBJECT
        void InjectIntlLibraryCode(_In_ ScriptContext * scriptContext, DynamicObject* intlObject);

        JavascriptFunction* GetDateToLocaleString() { return dateToLocaleString; }
        JavascriptFunction* GetDateToLocaleTimeString() { return dateToLocaleTimeString; }
        JavascriptFunction* GetDateToLocaleDateString() { return dateToLocaleDateString; }
        JavascriptFunction* GetNumberToLocaleString() { return numberToLocaleString; }
        JavascriptFunction* GetStringLocaleCompare() { return stringLocaleCompare; }
#endif

#ifdef ENABLE_PROJECTION
        Js::Var GetPromiseConstructor(_In_ ScriptContext * scriptContext);
#endif

        class EntryInfo
        {
        public:
            static NoProfileFunctionInfo Intl_RaiseAssert;

            static NoProfileFunctionInfo Intl_IsWellFormedLanguageTag;
            static NoProfileFunctionInfo Intl_NormalizeLanguageTag;
            static NoProfileFunctionInfo Intl_ResolveLocaleLookup;
            static NoProfileFunctionInfo Intl_ResolveLocaleBestFit;
            static NoProfileFunctionInfo Intl_GetDefaultLocale;
            static NoProfileFunctionInfo Intl_GetExtensions;
            static NoProfileFunctionInfo Intl_CompareString;
            static NoProfileFunctionInfo Intl_CurrencyDigits;
            static NoProfileFunctionInfo Intl_FormatNumber;

            static NoProfileFunctionInfo Intl_CacheNumberFormat;
            static NoProfileFunctionInfo Intl_CreateDateTimeFormat;

            static NoProfileFunctionInfo Intl_BestFitFormatter;
            static NoProfileFunctionInfo Intl_LookupMatcher;
            static NoProfileFunctionInfo Intl_FormatDateTime;
            static NoProfileFunctionInfo Intl_GetPatternForLocale;

            static NoProfileFunctionInfo Intl_RegisterBuiltInFunction;
            static NoProfileFunctionInfo Intl_GetHiddenObject;
            static NoProfileFunctionInfo Intl_SetHiddenObject;

            static NoProfileFunctionInfo GetErrorMessage;
            static NoProfileFunctionInfo LogDebugMessage;
            static NoProfileFunctionInfo TagPublicLibraryCode;
            static NoProfileFunctionInfo Promise_EnqueueTask;

#ifndef GlobalBuiltIn
#define GlobalBuiltIn(global, method) \
            static NoProfileFunctionInfo Intl_BuiltIn_##global##_##method##; \
    
#define GlobalBuiltInConstructor(global)

#define BuiltInRaiseException(exceptionType, exceptionID) \
     static NoProfileFunctionInfo Intl_BuiltIn_raise##exceptionID;

#define BuiltInRaiseException1(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID)
#define BuiltInRaiseException2(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID)
#define BuiltInRaiseException3(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID##_3)

#include "EngineInterfaceObjectBuiltIns.h"

#undef BuiltInRaiseException
#undef BuiltInRaiseException1
#undef BuiltInRaiseException2
#undef BuiltInRaiseException3
#undef GlobalBuiltInConstructor
#undef GlobalBuiltIn
#endif
            static NoProfileFunctionInfo Intl_BuiltIn_SetPrototype;
            static NoProfileFunctionInfo Intl_BuiltIn_GetArrayLength;
            static NoProfileFunctionInfo Intl_BuiltIn_RegexMatch;
            static NoProfileFunctionInfo Intl_BuiltIn_CallInstanceFunction;
        };
        
        static Var EntryIntl_RaiseAssert(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryIntl_IsWellFormedLanguageTag(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_NormalizeLanguageTag(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_ResolveLocaleLookup(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_ResolveLocaleBestFit(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_GetDefaultLocale(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_GetExtensions(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_CompareString(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_CurrencyDigits(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_FormatNumber(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryIntl_CacheNumberFormat(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_CreateDateTimeFormat(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryIntl_FormatDateTime(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryIntl_RegisterBuiltInFunction(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_GetHiddenObject(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIntl_SetHiddenObject(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryIntl_BuiltIn_SetPrototype(RecyclableObject *function, CallInfo callInfo, ...);
        static Var EntryIntl_BuiltIn_GetArrayLength(RecyclableObject *function, CallInfo callInfo, ...);
        static Var EntryIntl_BuiltIn_RegexMatch(RecyclableObject *function, CallInfo callInfo, ...);
        static Var EntryIntl_BuiltIn_CallInstanceFunction(RecyclableObject *function, CallInfo callInfo, ...);

        static Var Entry_GetErrorMessage(RecyclableObject *function, CallInfo callInfo, ...);
        static Var Entry_LogDebugMessage(RecyclableObject *function, CallInfo callInfo, ...);
        static Var Entry_TagPublicLibraryCode(RecyclableObject *function, CallInfo callInfo, ...);
        static Var EntryPromise_EnqueueTask(RecyclableObject *function, CallInfo callInfo, ...);
        
#ifndef GlobalBuiltIn
#define GlobalBuiltIn(global, method) 
    
#define GlobalBuiltInConstructor(global)

#define BuiltInRaiseException(exceptionType, exceptionID) \
        static Var EntryIntl_BuiltIn_raise##exceptionID(RecyclableObject *function, CallInfo callInfo, ...);

#define BuiltInRaiseException1(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID)
#define BuiltInRaiseException2(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID)
#define BuiltInRaiseException3(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID##_3)

#include "EngineInterfaceObjectBuiltIns.h"

#undef BuiltInRaiseException
#undef BuiltInRaiseException1
#undef BuiltInRaiseException2
#undef BuiltInRaiseException3
#undef GlobalBuiltInConstructor
#undef GlobalBuiltIn
#endif

    private:
        static void deletePrototypePropertyHelper(ScriptContext* scriptContext, DynamicObject* intlObject, Js::PropertyId objectPropertyId, Js::PropertyId getterFunctionId);
        static WindowsGlobalizationAdapter* GetWindowsGlobalizationAdapter(_In_ ScriptContext*);
        static void prepareWithFractionIntegerDigits(ScriptContext* scriptContext, Windows::Globalization::NumberFormatting::INumberRounderOption* rounderOptions, 
            Windows::Globalization::NumberFormatting::INumberFormatterOptions* formatterOptions, uint16 minFractionDigits, uint16 maxFractionDigits, uint16 minIntegerDigits);
        static void prepareWithSignificantDigits(ScriptContext* scriptContext, Windows::Globalization::NumberFormatting::INumberRounderOption* rounderOptions, Windows::Globalization::NumberFormatting::INumberFormatter *numberFormatter,
            Windows::Globalization::NumberFormatting::INumberFormatterOptions* formatterOptions, uint16 minSignificantDigits, uint16 maxSignificantDigits);

#ifdef ENABLE_INTL_OBJECT
        void cleanUpIntl(ScriptContext* scriptContext, DynamicObject* intlObject);
#endif
    };
}
