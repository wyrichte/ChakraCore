//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
using namespace Windows::Globalization;
#pragma warning(push)
#pragma warning(disable:4309) // truncation of constant value
#ifdef ENABLE_INTL_OBJECT
#if _M_AMD64 
#include "InJavascript\Intl.js.bc.64b.h"
#else
#include "InJavascript\Intl.js.bc.32b.h"
#endif
#endif
#ifdef ENABLE_PROJECTION
#if _M_AMD64 
#include "InJavascript\Promise.js.bc.64b.h"
#else
#include "InJavascript\Promise.js.bc.32b.h"
#endif
#endif
#pragma warning(pop)

#define IfFailThrowHr(op) \
    if (FAILED(hr=(op))) \
    { \
    JavascriptError::MapAndThrowError(scriptContext, hr); \
    } \

#define IfFailAssertAndThrowHr(op) \
    if (FAILED(hr=(op))) \
    { \
    AssertMsg(false, "HRESULT was a failure."); \
    JavascriptError::MapAndThrowError(scriptContext, hr); \
    } \

#define IfFailAssertMsgAndThrowHr(op, msg) \
    if (FAILED(hr=(op))) \
    { \
    AssertMsg(false, msg); \
    JavascriptError::MapAndThrowError(scriptContext, hr); \
    } \

#define HandleOOMSOEHR(hr) \
    if (hr == E_OUTOFMEMORY) \
    { \
    JavascriptError::ThrowOutOfMemoryError(scriptContext); \
    } \
    else if(hr == VBSERR_OutOfStack) \
    { \
    JavascriptError::ThrowStackOverflowError(scriptContext); \
    } \

#define SetPropertyOn(obj, propID, value) \
    obj->SetProperty(propID, value, Js::PropertyOperationFlags::PropertyOperation_None, nullptr) \

#define SetStringPropertyOn(obj, propID, propValue) \
    SetPropertyOn(obj, propID, Js::JavascriptString::NewCopySz(propValue, scriptContext)) \

#define SetHSTRINGPropertyOn(obj, propID, hstringValue) \
    SetStringPropertyOn(obj, propID, wgl->WindowsGetStringRawBuffer(hstringValue, &length)) \

#define SetPropertyLOn(obj, literalProperty, value) \
    obj->SetProperty(Js::JavascriptString::NewCopySz(literalProperty, scriptContext), value, Js::PropertyOperationFlags::PropertyOperation_None, nullptr) \

#define SetStringPropertyLOn(obj, literalProperty, propValue) \
    SetPropertyLOn(obj, literalProperty, Js::JavascriptString::NewCopySz(propValue, scriptContext)) \

#define SetHSTRINGPropertyLOn(obj, literalProperty, hstringValue) \
    SetStringPropertyLOn(obj, literalProperty, wgl->WindowsGetStringRawBuffer(hstringValue, &length)) \

#define SetPropertyBuiltInOn(obj, builtInPropID, value) \
    SetPropertyOn(obj, Js::PropertyIds::builtInPropID, value) \

#define SetStringPropertyBuiltInOn(obj, builtInPropID, propValue) \
    SetPropertyBuiltInOn(obj, builtInPropID, Js::JavascriptString::NewCopySz(propValue, scriptContext))

#define SetHSTRINGPropertyBuiltInOn(obj, builtInPropID, hstringValue) \
    SetStringPropertyBuiltInOn(obj, builtInPropID, wgl->WindowsGetStringRawBuffer(hstringValue, &length)) \

#define GetPropertyFrom(obj, propertyID) \
    Js::JavascriptOperators::GetProperty(obj, propertyID, &propertyValue, scriptContext) \

#define GetPropertyLFrom(obj, propertyName) \
    GetPropertyFrom(obj, scriptContext->GetOrAddPropertyIdTracked(propertyName, wcslen(propertyName)))

#define GetPropertyBuiltInFrom(obj, builtInPropID) \
    GetPropertyFrom(obj, Js::PropertyIds::builtInPropID) \

#define GetTypedPropertyBuiltInFrom(obj, builtInPropID, Type) \
    (GetPropertyFrom(obj, Js::PropertyIds::builtInPropID) && Type::Is(propertyValue)) \

#define HasPropertyOn(obj, propID) \
    Js::JavascriptOperators::HasProperty(obj, propID) \

#define HasPropertyBuiltInOn(obj, builtInPropID) \
    HasPropertyOn(obj, Js::PropertyIds::builtInPropID) \

#define HasPropertyLOn(obj, propertyName) \
    HasPropertyOn(obj, scriptContext->GetOrAddPropertyIdTracked(propertyName, wcslen(propertyName)))

namespace Js
{    

    class AutoCOMJSObject : public FinalizableObject
    {
        IInspectable *instance;

    public:
        DEFINE_VTABLE_CTOR_NOBASE(AutoCOMJSObject);

        AutoCOMJSObject(IInspectable *object) 
            : instance(object)
        { }

        static AutoCOMJSObject * New(Recycler * recycler, IInspectable *object)
        {
            return RecyclerNewFinalized(recycler, AutoCOMJSObject, object);
        }

        void Finalize(bool isShutdown) override
        {

        }

        void Dispose(bool isShutdown) override
        {
            instance->Release();
        }
        void Mark(Recycler * recycler) override
        {

        }

        IInspectable *GetInstance()
        {
            return instance;
        }
    };


    class AutoHSTRING
    {
        PREVENT_COPY(AutoHSTRING)

    private:
        HSTRING value;
    public:
        HSTRING *operator&() { Assert(value == nullptr); return &value; }
        HSTRING operator*() const { Assert(value != nullptr); return value; }

        AutoHSTRING()
            : value(null)
        { }

        ~AutoHSTRING()
        {
            Clear();
        }

        void Clear()
        {
            if(value != null)
            {
                WindowsDeleteString(value);
                value = null;
            }
        }
    };

    //Public
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_RaiseAssert(EngineInterfaceObject::EntryIntl_RaiseAssert);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_IsWellFormedLanguageTag(EngineInterfaceObject::EntryIntl_IsWellFormedLanguageTag);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_NormalizeLanguageTag(EngineInterfaceObject::EntryIntl_NormalizeLanguageTag);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_ResolveLocaleLookup(EngineInterfaceObject::EntryIntl_ResolveLocaleLookup);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_ResolveLocaleBestFit(EngineInterfaceObject::EntryIntl_ResolveLocaleBestFit);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_GetDefaultLocale(EngineInterfaceObject::EntryIntl_GetDefaultLocale);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_GetExtensions(EngineInterfaceObject::EntryIntl_GetExtensions);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_CompareString(EngineInterfaceObject::EntryIntl_CompareString);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_CurrencyDigits(EngineInterfaceObject::EntryIntl_CurrencyDigits);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_FormatNumber(EngineInterfaceObject::EntryIntl_FormatNumber);

    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_CacheNumberFormat(EngineInterfaceObject::EntryIntl_CacheNumberFormat);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_CreateDateTimeFormat(EngineInterfaceObject::EntryIntl_CreateDateTimeFormat);

    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_FormatDateTime(EngineInterfaceObject::EntryIntl_FormatDateTime);

    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_RegisterBuiltInFunction(EngineInterfaceObject::EntryIntl_RegisterBuiltInFunction);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_GetHiddenObject(EngineInterfaceObject::EntryIntl_GetHiddenObject);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_SetHiddenObject(EngineInterfaceObject::EntryIntl_SetHiddenObject);

    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::GetErrorMessage(EngineInterfaceObject::Entry_GetErrorMessage);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::LogDebugMessage(EngineInterfaceObject::Entry_LogDebugMessage);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::TagPublicLibraryCode(EngineInterfaceObject::Entry_TagPublicLibraryCode);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Promise_EnqueueTask(EngineInterfaceObject::EntryPromise_EnqueueTask);

#ifndef GlobalBuiltIn
#define GlobalBuiltIn(global, method) \
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_BuiltIn_##global##_##method##(global##::##method##); \

#define GlobalBuiltInConstructor(global)

#define BuiltInRaiseException(exceptionType, exceptionID) \
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_BuiltIn_raise##exceptionID(EngineInterfaceObject::EntryIntl_BuiltIn_raise##exceptionID); \

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

    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_BuiltIn_GetArrayLength(EngineInterfaceObject::EntryIntl_BuiltIn_GetArrayLength);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_BuiltIn_SetPrototype(EngineInterfaceObject::EntryIntl_BuiltIn_SetPrototype);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_BuiltIn_RegexMatch(EngineInterfaceObject::EntryIntl_BuiltIn_RegexMatch);
    NoProfileFunctionInfo EngineInterfaceObject::EntryInfo::Intl_BuiltIn_CallInstanceFunction(EngineInterfaceObject::EntryIntl_BuiltIn_CallInstanceFunction);

    WindowsGlobalizationAdapter* EngineInterfaceObject::GetWindowsGlobalizationAdapter(_In_ ScriptContext * scriptContext)
    {
        return scriptContext->GetThreadContext()->GetWindowsGlobalizationAdapter();
    }

    EngineInterfaceObject * EngineInterfaceObject::New(Recycler * recycler, DynamicType * type)
    {
        return NewObject<EngineInterfaceObject>(recycler, type);
    }

    bool EngineInterfaceObject::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_EngineInterfaceObject;
    }

    EngineInterfaceObject* EngineInterfaceObject::FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "aValue is actually an EngineInterfaceObject");

        return static_cast<EngineInterfaceObject *>(RecyclableObject::FromVar(aValue));
    }

    void EngineInterfaceObject::Initialize()
    {
        Recycler* recycler = this->GetRecycler();
        ScriptContext* scriptContext = this->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        // CommonNativeInterfaces is used as a prototype for the other native interface objects 
        // to share the common APIs without requiring everyone to access EngineInterfaceObject.Common.
        this->commonNativeInterfaces = DynamicObject::New(recycler,
            DynamicType::New(scriptContext, TypeIds_Object, library->GetObjectPrototype(), null,
            DeferredTypeHandler<InitializeCommonNativeInterfaces>::GetDefaultInstance()));
        library->AddMember(this, Js::PropertyIds::Common, this->commonNativeInterfaces);
        
#ifdef ENABLE_INTL_OBJECT
        if (scriptContext->GetConfig()->IsIntlEnabled())
        {
            this->intlNativeInterfaces = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, this->commonNativeInterfaces, null,
                DeferredTypeHandler<InitializeIntlNativeInterfaces>::GetDefaultInstance()));
            library->AddMember(this, Js::PropertyIds::Intl, this->intlNativeInterfaces);
        }
#endif

#ifdef ENABLE_PROJECTION
        if (scriptContext->GetConfig()->IsWinRTEnabled())
        {
            this->promiseNativeInterfaces = DynamicObject::New(recycler,
                DynamicType::New(scriptContext, TypeIds_Object, this->commonNativeInterfaces, null,
                DeferredTypeHandler<InitializePromiseNativeInterfaces>::GetDefaultInstance()));
            library->AddMember(this, Js::PropertyIds::Promise, this->promiseNativeInterfaces);
        }
#endif
    }
#if DBG
    void EngineInterfaceObject::DumpIntlByteCode(ScriptContext* scriptContext)
    {
        Output::Print(L"Dumping Intl Byte Code:");
        this->EnsureIntlByteCode(scriptContext);
        Js::ByteCodeDumper::DumpRecursively(intlByteCode);
    }

    void EngineInterfaceObject::DumpPromiseByteCode(ScriptContext* scriptContext)
    {
        Output::Print(L"Dumping Promise Byte Code:");
        this->EnsurePromiseByteCode(scriptContext);
        Js::ByteCodeDumper::DumpRecursively(promiseByteCode);
    }
#endif
    void EngineInterfaceObject::InitializeCommonNativeInterfaces(DynamicObject* commonNativeInterfaces, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode) 
    {
        typeHandler->Convert(commonNativeInterfaces, mode, 38);

        ScriptContext* scriptContext = commonNativeInterfaces->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

#ifndef GlobalBuiltIn
#define GlobalBuiltIn(global, method) \
    library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtIn##global##method, &EngineInterfaceObject::EntryInfo::Intl_BuiltIn_##global##_##method##, 1); \

#define GlobalBuiltInConstructor(global) SetPropertyOn(commonNativeInterfaces, Js::PropertyIds::##global##, library->Get##global##Constructor());

#define BuiltInRaiseException(exceptionType, exceptionID) \
    library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::raise##exceptionID, &EngineInterfaceObject::EntryInfo::Intl_BuiltIn_raise##exceptionID, 1); \

#define BuiltInRaiseException1(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID)
#define BuiltInRaiseException2(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID)
#define BuiltInRaiseException3(exceptionType, exceptionID) BuiltInRaiseException(exceptionType, exceptionID##_3)

#include "EngineInterfaceObjectBuiltIns.h"

#undef BuiltInRaiseException
#undef BuiltInRaiseException1
#undef BuiltInRaiseException2
#undef BuiltInRaiseException3
#undef GlobalBuiltIn
#undef GlobalBuiltInConstructor
#endif

        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInSetPrototype, &EngineInterfaceObject::EntryInfo::Intl_BuiltIn_SetPrototype, 1);
        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInGetArrayLength, &EngineInterfaceObject::EntryInfo::Intl_BuiltIn_GetArrayLength, 1);
        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInRegexMatch, &EngineInterfaceObject::EntryInfo::Intl_BuiltIn_RegexMatch, 1);
        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInCallInstanceFunction, &EngineInterfaceObject::EntryInfo::Intl_BuiltIn_CallInstanceFunction, 1);

        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInJavascriptObjectCreate, &JavascriptObject::EntryInfo::Create, 1);
        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInJavascriptObjectPreventExtensions, &JavascriptObject::EntryInfo::PreventExtensions, 1);
        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInJavascriptObjectGetOwnPropertyDescriptor, &JavascriptObject::EntryInfo::GetOwnPropertyDescriptor, 1);

        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::builtInGlobalObjectEval, &GlobalObject::EntryInfo::Eval, 2);

        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::getErrorMessage, &EngineInterfaceObject::EntryInfo::GetErrorMessage, 1);
        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::logDebugMessage, &EngineInterfaceObject::EntryInfo::LogDebugMessage, 1);
        library->AddFunctionToLibraryObject(commonNativeInterfaces, Js::PropertyIds::tagPublicLibraryCode, &EngineInterfaceObject::EntryInfo::TagPublicLibraryCode, 1);

        commonNativeInterfaces->SetHasNoEnumerableProperties(true);
    }

    void EngineInterfaceObject::InitializeIntlNativeInterfaces(DynamicObject* intlNativeInterfaces, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(intlNativeInterfaces, mode, 16);

        ScriptContext* scriptContext = intlNativeInterfaces->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::raiseAssert, &EngineInterfaceObject::EntryInfo::Intl_RaiseAssert, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::isWellFormedLanguageTag, &EngineInterfaceObject::EntryInfo::Intl_IsWellFormedLanguageTag, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::normalizeLanguageTag, &EngineInterfaceObject::EntryInfo::Intl_NormalizeLanguageTag, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::compareString, &EngineInterfaceObject::EntryInfo::Intl_CompareString, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::resolveLocaleLookup, &EngineInterfaceObject::EntryInfo::Intl_ResolveLocaleLookup, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::resolveLocaleBestFit, &EngineInterfaceObject::EntryInfo::Intl_ResolveLocaleBestFit, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::getDefaultLocale, &EngineInterfaceObject::EntryInfo::Intl_GetDefaultLocale, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::getExtensions, &EngineInterfaceObject::EntryInfo::Intl_GetExtensions, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::formatNumber, &EngineInterfaceObject::EntryInfo::Intl_FormatNumber, 1);

        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::cacheNumberFormat, &EngineInterfaceObject::EntryInfo::Intl_CacheNumberFormat, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::createDateTimeFormat, &EngineInterfaceObject::EntryInfo::Intl_CreateDateTimeFormat, 1);

        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::currencyDigits, &EngineInterfaceObject::EntryInfo::Intl_CurrencyDigits, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::formatDateTime, &EngineInterfaceObject::EntryInfo::Intl_FormatDateTime, 1);

        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::registerBuiltInFunction, &EngineInterfaceObject::EntryInfo::Intl_RegisterBuiltInFunction, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::getHiddenObject, &EngineInterfaceObject::EntryInfo::Intl_GetHiddenObject, 1);
        library->AddFunctionToLibraryObject(intlNativeInterfaces, Js::PropertyIds::setHiddenObject, &EngineInterfaceObject::EntryInfo::Intl_SetHiddenObject, 1);

        intlNativeInterfaces->SetHasNoEnumerableProperties(true);
    }

    void EngineInterfaceObject::InitializePromiseNativeInterfaces(DynamicObject* promiseNativeInterfaces, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode)
    {
        typeHandler->Convert(promiseNativeInterfaces, mode, 9);
        
        ScriptContext* scriptContext = promiseNativeInterfaces->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();
        
        // Promise has a dependency on the Debug object type being constructed so
        // undefer it here.
        DynamicObject* debugObject = library->GetDebugObject();
        debugObject->EnsureObjectReady();

        // msTraceAsyncCallbackStarting([asyncOperationId: number=-1], [workType: number=1], [logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msTraceAsyncCallbackStarting, &AsyncDebug::EntryInfo::BeginAsyncCallback, 3);
        // msTraceAsyncCallbackCompleted([logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msTraceAsyncCallbackCompleted, &AsyncDebug::EntryInfo::CompleteAsyncCallback, 1);
        // msTraceAsyncOperationCompleted([asyncOperationID: number=-1], [status: number=1], [logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msTraceAsyncOperationCompleted, &AsyncDebug::EntryInfo::CompleteAsyncOperation, 3);
        // msUpdateAsyncCallbackRelation([relatedAsyncOperationID: number=-1], [relationType: number=5], [logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msUpdateAsyncCallbackRelation, &AsyncDebug::EntryInfo::UpdateAsyncCallbackStatus, 3);

        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::enqueueTask, &EngineInterfaceObject::EntryInfo::Promise_EnqueueTask, 2);

        // MS_ASYNC_OP_STATUS_SUCCESS: number=1.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_OP_STATUS_SUCCESS, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Completed, scriptContext), PropertyNone);
        // MS_ASYNC_OP_STATUS_CANCELED: number=2.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_OP_STATUS_CANCELED, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Canceled, scriptContext), PropertyNone);
        // MS_ASYNC_OP_STATUS_ERROR: number=3.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_OP_STATUS_ERROR, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Error, scriptContext), PropertyNone);
        // MS_ASYNC_CALLBACK_STATUS_ERROR: number=4.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_CALLBACK_STATUS_ERROR, Js::JavascriptNumber::New(AsyncDebug::AsyncCallbackStatus_Error, scriptContext), PropertyNone);

        // setNonUserCodeExceptions(enableNonUserCodeExceptions: boolean):undefined. This is a setter.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::setNonUserCodeExceptions, library->GetDebugObjectNonUserSetterFunction(), PropertyNone);

        promiseNativeInterfaces->SetHasNoEnumerableProperties(true);
    }

    void EngineInterfaceObject::deletePrototypePropertyHelper(ScriptContext* scriptContext, DynamicObject* intlObject, Js::PropertyId objectPropertyId, Js::PropertyId getterFunctionId)
    {
        DynamicObject *prototypeVal = nullptr;
        DynamicObject *functionObj = nullptr;
        Var propertyValue;
        Var getter;
        Var setter;

        if (!GetPropertyFrom(intlObject, objectPropertyId))
        {
            AssertMsg(false, "Error.");
            return;
        }

        if (!GetPropertyBuiltInFrom(DynamicObject::FromVar(propertyValue), prototype))
        {
            AssertMsg(false, "Can't be null, otherwise Intl library wasn't initialized correctly");
            return;
        }

        if (!GetPropertyBuiltInFrom(prototypeVal = DynamicObject::FromVar(propertyValue), resolvedOptions))
        {
            AssertMsg(false, "If these operations result in false, Intl tests will detect them");
            return;
        }

        (functionObj = DynamicObject::FromVar(propertyValue))->SetConfigurable(Js::PropertyIds::prototype, true);
        functionObj->DeleteProperty(Js::PropertyIds::prototype, Js::PropertyOperationFlags::PropertyOperation_None);

        JavascriptOperators::GetOwnAccessors(prototypeVal, getterFunctionId, &getter, &setter, scriptContext);
        (functionObj = DynamicObject::FromVar(getter))->SetConfigurable(Js::PropertyIds::prototype, true);
        functionObj->DeleteProperty(Js::PropertyIds::prototype, Js::PropertyOperationFlags::PropertyOperation_None);
    }
    
#ifdef ENABLE_INTL_OBJECT
    void EngineInterfaceObject::cleanUpIntl(ScriptContext *scriptContext, DynamicObject* intlObject)
    {
        this->dateToLocaleString = nullptr;
        this->dateToLocaleTimeString = nullptr;
        this->dateToLocaleDateString = nullptr;
        this->numberToLocaleString = nullptr;
        this->stringLocaleCompare = nullptr;

        //Failed to setup Intl; Windows.Globalization.dll is most likely missing.
        if(HasPropertyBuiltInOn(intlObject, Collator))
        {
            intlObject->DeleteProperty(Js::PropertyIds::Collator, Js::PropertyOperationFlags::PropertyOperation_None);
        }
        if(HasPropertyBuiltInOn(intlObject, NumberFormat))
        {
            intlObject->DeleteProperty(Js::PropertyIds::NumberFormat, Js::PropertyOperationFlags::PropertyOperation_None);
        }
        if(HasPropertyBuiltInOn(intlObject, DateTimeFormat))
        {
            intlObject->DeleteProperty(Js::PropertyIds::DateTimeFormat, Js::PropertyOperationFlags::PropertyOperation_None);
        }
    }

    void EngineInterfaceObject::EnsureIntlByteCode(_In_ ScriptContext * scriptContext)
    {
        if (this->intlByteCode == nullptr)
        {
            SourceContextInfo * sourceContextInfo = scriptContext->GetSourceContextInfo(Js::Constants::NoHostSourceContext, NULL);
            
            Assert(sourceContextInfo != nullptr);

            SRCINFO si;
            memset(&si, 0, sizeof(si));
            si.sourceContextInfo = sourceContextInfo;
            SRCINFO *hsi = scriptContext->AddHostSrcInfo(&si);
            ulong flags = fscrIsLibraryCode | (CONFIG_FLAG(CreateFunctionProxy) && !scriptContext->IsProfiling() ? fscrAllowFunctionProxy : 0);

            HRESULT hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, (LPCUTF8)nullptr, hsi, (byte*)Library_Bytecode_intl, nullptr, &this->intlByteCode);

            IfFailAssertMsgAndThrowHr(hr, "Failed to deserialize Intl.js bytecode - very probably the bytecode needs to be rebuilt.");
        }
    }

    void EngineInterfaceObject::EnsurePromiseByteCode(_In_ ScriptContext * scriptContext)
    {
        if (this->promiseByteCode == nullptr)
        {
            SourceContextInfo* sourceContextInfo = scriptContext->GetSourceContextInfo(Js::Constants::NoHostSourceContext, nullptr);

            Assert(sourceContextInfo != nullptr);

            SRCINFO si;
            memset(&si, 0, sizeof(si));
            si.sourceContextInfo = sourceContextInfo;
            SRCINFO* hsi = scriptContext->AddHostSrcInfo(&si);

            // Mark the Promise bytecode as internal library code - the real stack frames will be ignored during a stackwalk.
            // If we aren't profiling and function proxies are enabled, allow the bytecode to be built into a FunctionProxy instead of a FunctionBody.
            ulong flags = fscrIsLibraryCode | (CONFIG_FLAG(CreateFunctionProxy) && !scriptContext->IsProfiling() ? fscrAllowFunctionProxy : 0);
            HRESULT hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, (LPCUTF8)nullptr, hsi, (byte*)Library_Bytecode_promise, nullptr, &this->promiseByteCode);

            AssertMsg(SUCCEEDED(hr), "Failed to deserialize Promise.js bytecode - very probably the bytecode needs to be rebuilt.");
            IfFailThrowHr(hr);
        }
    }

    void EngineInterfaceObject::InjectIntlLibraryCode(_In_ ScriptContext * scriptContext, DynamicObject* intlObject)
    {
        JavascriptExceptionObject *pExceptionObject = nullptr;

        try{            
            this->EnsureIntlByteCode(scriptContext);
            
            Assert(intlByteCode != nullptr);

            HRESULT hr;
            //Ensure we have initialized all appropriate COM objects for the adapter (we will be using them now)
            IfFailAssertMsgAndThrowHr(GetWindowsGlobalizationAdapter(scriptContext)->EnsureInitialized(scriptContext), "Failed to initialize COM interfaces, verify correct version of globalization dll is used.");

            Js::ScriptFunction *function = scriptContext->GetLibrary()->CreateScriptFunction(intlByteCode->GetNestedFunc(0)->EnsureDeserialized());

            // If we are profiling, we need to register the script to the profiler callback, so the script compiled event will be sent.
            if (scriptContext->IsProfiling())
            {
                scriptContext->RegisterScript(function->GetFunctionProxy());
            }
            // Mark we are profiling library code already, so that any initialization library code called here won't be reported to profiler
            AutoProfilingUserCode autoProfilingUserCode(scriptContext->GetThreadContext(), /*isProfilingUserCode*/false);

            Js::Var args[] = { scriptContext->GetLibrary()->GetUndefined(), this };
            Js::CallInfo callInfo(Js::CallFlags_Value, _countof(args));
            JavascriptFunction::CallRootFunctionInScript(function, Js::Arguments(callInfo, args));

            //Delete prototypes on functions
            deletePrototypePropertyHelper(scriptContext, intlObject, Js::PropertyIds::Collator, Js::PropertyIds::compare);
            deletePrototypePropertyHelper(scriptContext, intlObject, Js::PropertyIds::NumberFormat, Js::PropertyIds::format);
            deletePrototypePropertyHelper(scriptContext, intlObject, Js::PropertyIds::DateTimeFormat, Js::PropertyIds::format);

        }
        catch (JavascriptExceptionObject* exceptionObject) 
        { 
            pExceptionObject = exceptionObject; 
        } 

        if(pExceptionObject) 
        { 
            cleanUpIntl(scriptContext, intlObject);
            if(pExceptionObject == ThreadContext::GetContextForCurrentThread()->GetPendingOOMErrorObject() ||
                pExceptionObject == ThreadContext::GetContextForCurrentThread()->GetPendingSOErrorObject())
            {
                scriptContext->GetLibrary()->ResetIntlObject();
                pExceptionObject = pExceptionObject->CloneIfStaticExceptionObject(scriptContext); 
                throw pExceptionObject; 
            }
            JavascriptError::ThrowTypeError(scriptContext, JSERR_IntlNotAvailable);
        }
    }
#endif

#ifdef ENABLE_PROJECTION
    Js::Var EngineInterfaceObject::GetPromiseConstructor(_In_ ScriptContext * scriptContext)
    {
        if (!scriptContext->VerifyAlive()) // Can't initialize if scriptContext closed, will need to run script
        {
            return nullptr;
        }

        JavascriptLibrary* library = scriptContext->GetLibrary();
        this->EnsurePromiseByteCode(scriptContext);

        Assert(promiseByteCode != nullptr);

        Js::ScriptFunction* function = library->CreateScriptFunction(promiseByteCode->GetNestedFunc(0)->EnsureDeserialized());

        // If we are profiling, we need to register the script to the profiler callback, so the script compiled event will be sent.
        if (scriptContext->IsProfiling())
        {
            scriptContext->RegisterScript(function->GetFunctionProxy());
        }
        // Mark we are profiling library code already, so that any initialization library code called here won't be reported to profiler
        AutoProfilingUserCode autoProfilingUserCode(scriptContext->GetThreadContext(), /*isProfilingUserCode*/false);

        Js::Var args[] = { scriptContext->GetLibrary()->GetUndefined(), this };
        Js::CallInfo callInfo(Js::CallFlags_Value, _countof(args));
        Js::Var value = JavascriptFunction::CallRootFunctionInScript(function, Js::Arguments(callInfo, args));

        return value;
    }
#endif

    // First parameter is boolean.
    Var EngineInterfaceObject::EntryIntl_RaiseAssert(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(args.Info.Count < 2 || !JavascriptError::Is(args.Values[1]))
        {
            AssertMsg(false, "Intl's Assert platform API was called incorrectly.");
            return scriptContext->GetLibrary()->GetUndefined();
        }
#if DEBUG
        JavascriptExceptionOperators::Throw(JavascriptError::FromVar(args.Values[1]), scriptContext);
        AssertMsg(false, "Intl raised an assert in the JS implementation. An exception was thrown with the msg.")
#endif
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var EngineInterfaceObject::EntryIntl_IsWellFormedLanguageTag(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(args.Info.Count < 2 || !JavascriptString::Is(args.Values[1]))
        {
            // IsWellFormedLanguageTage of undefined or non-string is false
            return scriptContext->GetLibrary()->GetFalse();
        }

        JavascriptString *argString = JavascriptString::FromVar(args.Values[1]);
        return GetWindowsGlobalizationAdapter(scriptContext)->IsWellFormedLanguageTag(scriptContext, argString->GetSz()) ? 
            scriptContext->GetLibrary()->GetTrue() : scriptContext->GetLibrary()->GetFalse();
    }

    Var EngineInterfaceObject::EntryIntl_NormalizeLanguageTag(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(args.Info.Count < 2 || !JavascriptString::Is(args.Values[1]))
        {
            // NormalizeLanguageTag of undefined or non-string is undefined
            return scriptContext->GetLibrary()->GetUndefined();
        }

        JavascriptString *argString = JavascriptString::FromVar(args.Values[1]);
        WindowsGlobalizationAdapter* wga = GetWindowsGlobalizationAdapter(scriptContext);
        DelayLoadWindowsGlobalization* wsl = scriptContext->GetThreadContext()->GetWindowsGlobalizationLibrary();

        AutoHSTRING str;
        HRESULT hr;
        if(FAILED(hr = wga->NormalizeLanguageTag(scriptContext, argString->GetSz(), &str)))
        {
            HandleOOMSOEHR(hr);
            //If we can't normalize the tag; return undefined.
            return scriptContext->GetLibrary()->GetUndefined();
        }

        PCWSTR strBuf = wsl->WindowsGetStringRawBuffer(*str, NULL);
        JavascriptString *retVal = Js::JavascriptString::NewCopySz(strBuf, scriptContext);

        return retVal;
    }
    Var EngineInterfaceObject::EntryIntl_ResolveLocaleLookup(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(args.Info.Count < 2 || !JavascriptString::Is(args.Values[1]))
        {
            // NormalizeLanguageTag of undefined or non-string is undefined
            return scriptContext->GetLibrary()->GetUndefined();
        }

        JavascriptString *argString = JavascriptString::FromVar(args.Values[1]);

        WCHAR resolvedLocaleName[LOCALE_NAME_MAX_LENGTH];
        resolvedLocaleName[0] = '\0';

        ResolveLocaleName(argString->GetSz(),  resolvedLocaleName, _countof(resolvedLocaleName));
        if(resolvedLocaleName[0] == '\0')
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
        return JavascriptString::NewCopySz(resolvedLocaleName, scriptContext);
    }

    Var EngineInterfaceObject::EntryIntl_ResolveLocaleBestFit(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);
        DelayLoadWindowsGlobalization* wgl = scriptContext->GetThreadContext()->GetWindowsGlobalizationLibrary();
        WindowsGlobalizationAdapter* wga = GetWindowsGlobalizationAdapter(scriptContext);
        if(args.Info.Count < 2 || !JavascriptString::Is(args.Values[1]))
        {
            // NormalizeLanguageTag of undefined or non-string is undefined
            return scriptContext->GetLibrary()->GetUndefined();
        }


        PCWSTR passedLocale = JavascriptString::FromVar(args.Values[1])->GetSz();

        AutoCOMPtr<DateTimeFormatting::IDateTimeFormatter> formatter;
        HRESULT hr;
        if(FAILED(hr = wga->CreateDateTimeFormatter(scriptContext, L"longdate", &passedLocale, 1, nullptr, nullptr, &formatter)))
        {
            HandleOOMSOEHR(hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }
        AutoHSTRING locale;
        if(FAILED(hr = formatter->get_ResolvedLanguage(&locale)))
        {
            HandleOOMSOEHR(hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }

        return JavascriptString::NewCopySz(wgl->WindowsGetStringRawBuffer(*locale, NULL), scriptContext);
    }

    Var EngineInterfaceObject::EntryIntl_GetDefaultLocale(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        WCHAR defaultLocale[LOCALE_NAME_MAX_LENGTH];
        defaultLocale[0]='\0';

        if(GetUserDefaultLocaleName(defaultLocale, _countof(defaultLocale)) == 0 || defaultLocale[0]=='\0')
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        return JavascriptString::NewCopySz(defaultLocale, scriptContext);
    }
    Var EngineInterfaceObject::EntryIntl_GetExtensions(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        DelayLoadWindowsGlobalization* wgl = scriptContext->GetThreadContext()->GetWindowsGlobalizationLibrary();
        WindowsGlobalizationAdapter* wga = GetWindowsGlobalizationAdapter(scriptContext);
        if(args.Info.Count < 2 || !JavascriptString::Is(args.Values[1]))
        {
            // NormalizeLanguageTag of undefined or non-string is undefined
            return scriptContext->GetLibrary()->GetUndefined();
        }

        AutoCOMPtr<ILanguage> language;
        AutoCOMPtr<ILanguageExtensionSubtags> extensionSubtatgs;
        HRESULT hr;
        if(FAILED(hr = wga->CreateLanguage(scriptContext, JavascriptString::FromVar(args.Values[1])->GetSz(), &language)))
        {
            HandleOOMSOEHR(hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }

        if(FAILED(hr = language->QueryInterface(__uuidof(ILanguageExtensionSubtags), reinterpret_cast<void**>(&extensionSubtatgs))))
        {
            HandleOOMSOEHR(hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }
        Assert(extensionSubtatgs);

        AutoHSTRING singletonString;
        AutoCOMPtr<Windows::Foundation::Collections::IVectorView<HSTRING>> subtags;
        uint32 length;

        if(FAILED(hr = wgl->WindowsCreateString(L"u", 1, &singletonString)) ||  FAILED(hr = extensionSubtatgs->GetExtensionSubtags(*singletonString, &subtags)) || FAILED(subtags->get_Size(&length)))
        {
            HandleOOMSOEHR(hr);
            return scriptContext->GetLibrary()->GetUndefined();
        }
        JavascriptArray *toReturn = scriptContext->GetLibrary()->CreateArray(length);
        
        for(uint32 i = 0; i < length; i++)
        {
            AutoHSTRING str;
            if(!FAILED(hr = subtags->GetAt(i, &str)))
            {
                toReturn->SetItem(i, JavascriptString::NewCopySz(wgl->WindowsGetStringRawBuffer(*str, NULL), scriptContext), Js::PropertyOperationFlags::PropertyOperation_None);
            }
            else
            {
                HandleOOMSOEHR(hr);
            }
        }

        return toReturn;
    }

    Var EngineInterfaceObject::EntryIntl_CacheNumberFormat(RecyclableObject * function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);
        DelayLoadWindowsGlobalization* wgl = scriptContext->GetThreadContext()->GetWindowsGlobalizationLibrary();
        WindowsGlobalizationAdapter* wga = GetWindowsGlobalizationAdapter(scriptContext);

        //The passed object is the hidden state object
        if (args.Info.Count < 2 || !DynamicObject::Is(args.Values[1]))
        {
            // Call with undefined or non-number is undefined
            return scriptContext->GetLibrary()->GetUndefined();
        }

        HRESULT hr;
        Var propertyValue;
        JavascriptString* localeJSstr;
        DynamicObject* options = DynamicObject::FromVar(args.Values[1]);

        //Verify locale is present
        if(!GetTypedPropertyBuiltInFrom(options, __locale, JavascriptString) || (localeJSstr=JavascriptString::FromVar(propertyValue))->GetLength() <= 0)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        //First we have to determine which formatter(number, percent, or currency) we will be using.
        //Note some options might not be present.
        AutoCOMPtr<NumberFormatting::INumberFormatter> numberFormatter(nullptr);
        PCWSTR locale = localeJSstr->GetSz();
        uint16 formatterToUseVal = 0; // number is default, 1 is percent, 2 is currency
        if(GetTypedPropertyBuiltInFrom(options, __formatterToUse, TaggedInt) && (formatterToUseVal = TaggedInt::ToUInt16(propertyValue)) == 1)
        {
            //Use the percent formatter
            IfFailThrowHr(wga->CreatePercentFormatter(scriptContext, &locale, 1, &numberFormatter));
        }
        else if(formatterToUseVal == 2)
        {
            //Use the currency formatter
            AutoCOMPtr<NumberFormatting::ICurrencyFormatter> currencyFormatter(nullptr);
            if(!GetTypedPropertyBuiltInFrom(options, __currency, JavascriptString))
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            //API call retrieves a currency formatter, have to query its interface for numberFormatter
            IfFailThrowHr(GetWindowsGlobalizationAdapter(scriptContext)->CreateCurrencyFormatter(scriptContext, &locale, 1, JavascriptString::FromVar(propertyValue)->GetSz(), &currencyFormatter));

            if (GetTypedPropertyBuiltInFrom(options, __currencyDisplayToUse, TaggedInt)) // 0 is for symbol, 1 is for code, 2 is for name. Currently name isn't supported; so it will default to code in that case.
            {
                AutoCOMPtr<NumberFormatting::ICurrencyFormatter2> currencyFormatter2(nullptr);
                IfFailThrowHr(currencyFormatter->QueryInterface(__uuidof(NumberFormatting::ICurrencyFormatter2), reinterpret_cast<void**>(&currencyFormatter2)));

                if(TaggedInt::ToUInt16(propertyValue) == 0)
                {
                    IfFailThrowHr(currencyFormatter2->put_Mode(NumberFormatting::CurrencyFormatterMode::CurrencyFormatterMode_UseSymbol));
                }
                else
                {
                    IfFailThrowHr(currencyFormatter2->put_Mode(NumberFormatting::CurrencyFormatterMode::CurrencyFormatterMode_UseCurrencyCode));
                }
            }

            IfFailThrowHr(currencyFormatter->QueryInterface(__uuidof(NumberFormatting::INumberFormatter), reinterpret_cast<void**>(&numberFormatter)));
        }
        else
        {
            //Use the number formatter (default)
            IfFailThrowHr(wga->CreateNumberFormatter(scriptContext, &locale, 1, &numberFormatter));
        }
        Assert(numberFormatter);

        AutoCOMPtr<NumberFormatting::ISignedZeroOption> signedZeroOption(nullptr);
        IfFailThrowHr(numberFormatter->QueryInterface(__uuidof(NumberFormatting::ISignedZeroOption), reinterpret_cast<void**>(&signedZeroOption)));
        IfFailThrowHr(signedZeroOption->put_IsZeroSigned(true));

        //Configure non-digit related options
        AutoCOMPtr<NumberFormatting::INumberFormatterOptions> numberFormatterOptions(nullptr);
        IfFailThrowHr(numberFormatter->QueryInterface(__uuidof(NumberFormatting::INumberFormatterOptions), reinterpret_cast<void**>(&numberFormatterOptions)));
        Assert(numberFormatterOptions);

        if (GetTypedPropertyBuiltInFrom(options, __isDecimalPointAlwaysDisplayed, JavascriptBoolean))
        {
            IfFailThrowHr(numberFormatterOptions->put_IsDecimalPointAlwaysDisplayed((boolean)(JavascriptBoolean::FromVar(propertyValue)->GetValue())));
        }
        if (GetTypedPropertyBuiltInFrom(options, __useGrouping, JavascriptBoolean))
        {
            IfFailThrowHr(numberFormatterOptions->put_IsGrouped((boolean)(JavascriptBoolean::FromVar(propertyValue)->GetValue())));
        }

        //Get the numeral system and add it to the object since it will be located in the locale
        AutoHSTRING hNumeralSystem;
        AutoHSTRING hResolvedLanguage;
        uint32 length;
        IfFailThrowHr(numberFormatterOptions->get_NumeralSystem(&hNumeralSystem));
        SetHSTRINGPropertyBuiltInOn(options, __numberingSystem, *hNumeralSystem);

        IfFailThrowHr(numberFormatterOptions->get_ResolvedLanguage(&hResolvedLanguage));
        SetHSTRINGPropertyBuiltInOn(options, __locale, *hResolvedLanguage);

        AutoCOMPtr<NumberFormatting::INumberRounderOption> rounderOptions(nullptr);
        IfFailThrowHr(numberFormatter->QueryInterface(__uuidof(NumberFormatting::INumberRounderOption), reinterpret_cast<void**>(&rounderOptions)));
        Assert(rounderOptions);

        if(HasPropertyBuiltInOn(options, __minimumSignificantDigits) || HasPropertyBuiltInOn(options, __maximumSignificantDigits))
        {
            uint16 minSignificantDigits = 1, maxSignificantDigits = 21;
            //Do significant digit rounding
            if(GetTypedPropertyBuiltInFrom(options, __minimumSignificantDigits, TaggedInt))
            {
                minSignificantDigits = max<uint16>(min<uint16>(TaggedInt::ToUInt16(propertyValue), 21), 1);
            }
            if(GetTypedPropertyBuiltInFrom(options, __maximumSignificantDigits, TaggedInt))
            {
                maxSignificantDigits = max<uint16>(min<uint16>(TaggedInt::ToUInt16(propertyValue), 21), minSignificantDigits);
            }
            prepareWithSignificantDigits(scriptContext, rounderOptions, numberFormatter, numberFormatterOptions, minSignificantDigits, maxSignificantDigits);
        }
        else
        {
            uint16 minFractionDigits = 0, maxFractionDigits = 3, minIntegerDigits = 1;
            //Do fraction/integer digit rounding
            if(GetTypedPropertyBuiltInFrom(options, __minimumIntegerDigits, TaggedInt))
            {
                minIntegerDigits = max<uint16>(min<uint16>(TaggedInt::ToUInt16(propertyValue), 21), 1);
            }
            if(GetTypedPropertyBuiltInFrom(options, __minimumFractionDigits, TaggedInt))
            {
                minFractionDigits = min<uint16>(TaggedInt::ToUInt16(propertyValue), 20);//ToUInt16 will get rid of negatives by making them high
            }
            if(GetTypedPropertyBuiltInFrom(options, __maximumFractionDigits, TaggedInt))
            {
                maxFractionDigits = max(min<uint16>(TaggedInt::ToUInt16(propertyValue), 20), minFractionDigits);//ToUInt16 will get rid of negatives by making them high
            }
            prepareWithFractionIntegerDigits(scriptContext, rounderOptions, numberFormatterOptions, minFractionDigits, maxFractionDigits + (formatterToUseVal == 1 ? 2 : 0), minIntegerDigits);//extend max fractions for percent
        }

        //Set the object as a cache
        numberFormatter->AddRef();
        options->SetInternalProperty(Js::InternalPropertyIds::HiddenObject, AutoCOMJSObject::New(scriptContext->GetRecycler(), numberFormatter), Js::PropertyOperationFlags::PropertyOperation_None, NULL);

        return scriptContext->GetLibrary()->GetUndefined();
    }
    // Unlike CacheNumberFormat; this call takes an additional parameter to specify whether we are going to cache it.
    // We have to create this formatter twice; first time get the date/time patterns; and second time cache with correct format string.
    Var EngineInterfaceObject::EntryIntl_CreateDateTimeFormat(RecyclableObject * function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);
        DelayLoadWindowsGlobalization* wgl = scriptContext->GetThreadContext()->GetWindowsGlobalizationLibrary();
        WindowsGlobalizationAdapter* wga = GetWindowsGlobalizationAdapter(scriptContext);

        if(args.Info.Count < 3 || !DynamicObject::Is(args.Values[1]) || !JavascriptBoolean::Is(args.Values[2]))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        DynamicObject* obj = DynamicObject::FromVar(args.Values[1]);

        HRESULT hr;
        Var propertyValue;
        uint32 length;

        PCWSTR locale = GetTypedPropertyBuiltInFrom(obj, __locale, JavascriptString) ? JavascriptString::FromVar(propertyValue)->GetSz() : nullptr;
        PCWSTR templateString = GetTypedPropertyBuiltInFrom(obj, __templateString, JavascriptString) ? JavascriptString::FromVar(propertyValue)->GetSz() : nullptr;

        if (locale == nullptr || templateString == nullptr)
        {
            AssertMsg(false, "For some reason, locale and templateString aren't defined or aren't a JavascriptString.");
            return scriptContext->GetLibrary()->GetUndefined();
        }

        PCWSTR clock = GetTypedPropertyBuiltInFrom(obj, __windowsClock, JavascriptString) ? JavascriptString::FromVar(propertyValue)->GetSz() : nullptr ;

        AutoHSTRING hDummyCalendar;
        if(clock != nullptr)
        {
            //Because both calendar and clock are needed to pass into the datetimeformatter constructor (or neither); create a dummy one to get the value of calendar out so clock can be passed in with it.
            AutoCOMPtr<DateTimeFormatting::IDateTimeFormatter> dummyFormatter;
            IfFailThrowHr(wga->CreateDateTimeFormatter(scriptContext, templateString, &locale, 1, nullptr, nullptr, &dummyFormatter));

            IfFailThrowHr(dummyFormatter->get_Calendar(&hDummyCalendar));
        }

        //Now create the real formatter.
        AutoCOMPtr<DateTimeFormatting::IDateTimeFormatter> cachedFormatter;
        IfFailThrowHr(wga->CreateDateTimeFormatter(scriptContext, templateString, &locale, 1, 
            clock == nullptr ? nullptr : wgl->WindowsGetStringRawBuffer(*hDummyCalendar, &length), clock, &cachedFormatter));

        AutoHSTRING hCalendar;
        AutoHSTRING hClock;
        AutoHSTRING hLocale;
        AutoHSTRING hNumberingSystem;
        //In case the upper code path wasn't hit; extract the calendar string again so it can be set.
        IfFailThrowHr(cachedFormatter->get_Calendar(&hCalendar));
        SetHSTRINGPropertyBuiltInOn(obj, __windowsCalendar, *hCalendar);

        IfFailThrowHr(cachedFormatter->get_Clock(&hClock));
        SetHSTRINGPropertyBuiltInOn(obj, __windowsClock, *hClock);

        IfFailThrowHr(cachedFormatter->get_ResolvedLanguage(&hLocale));
        SetHSTRINGPropertyBuiltInOn(obj, __locale, *hLocale);

        //Get the numbering system
        IfFailThrowHr(cachedFormatter->get_NumeralSystem(&hNumberingSystem));
        SetHSTRINGPropertyBuiltInOn(obj, __numberingSystem, *hNumberingSystem);

        //Extract the pattern strings
        AutoCOMPtr<Windows::Foundation::Collections::IVectorView<HSTRING>> dateResult;
        IfFailThrowHr(cachedFormatter->get_Patterns(&dateResult));

        IfFailThrowHr(dateResult->get_Size(&length));

        JavascriptArray *patternStrings = scriptContext->GetLibrary()->CreateArray(length);

        for(uint32 i = 0; i < length; i++)
        {
            AutoHSTRING item;
            IfFailThrowHr(dateResult->GetAt(i, &item));
            patternStrings->SetItem(i, Js::JavascriptString::NewCopySz(wgl->WindowsGetStringRawBuffer(*item, NULL), scriptContext), PropertyOperation_None);
        }
        SetPropertyBuiltInOn(obj, __patternStrings, patternStrings);

        //This parameter tells us whether we are caching it this time around; or just validating pattern strings
        if((boolean)(JavascriptBoolean::FromVar(args.Values[2])->GetValue())) 
        {
            //If timeZone is undefined; then use the standard dateTimeFormatter to format in local time; otherwise use the IDateTimeFormatter2 to format using specified timezone (UTC)
            if(!GetPropertyBuiltInFrom(obj, __timeZone) || JavascriptOperators::IsUndefinedObject(propertyValue))
            {
                cachedFormatter->AddRef();
                obj->SetInternalProperty(Js::InternalPropertyIds::HiddenObject, AutoCOMJSObject::New(scriptContext->GetRecycler(), cachedFormatter), Js::PropertyOperationFlags::PropertyOperation_None, NULL);
            }
            else
            {
                AutoCOMPtr<DateTimeFormatting::IDateTimeFormatter2> tzCachedFormatter;
                IfFailThrowHr(cachedFormatter->QueryInterface(__uuidof(DateTimeFormatting::IDateTimeFormatter2), reinterpret_cast<void**>(&tzCachedFormatter))); 
                tzCachedFormatter->AddRef();

                //Set the object as a cache
                obj->SetInternalProperty(Js::InternalPropertyIds::HiddenObject, AutoCOMJSObject::New(scriptContext->GetRecycler(), tzCachedFormatter), Js::PropertyOperationFlags::PropertyOperation_None, NULL);
            }
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    DWORD getFlagsForSensitivity(LPCWSTR sensitivity)
    {
        if (wcscmp(sensitivity, L"base") == 0)
        {
            return LINGUISTIC_IGNOREDIACRITIC | LINGUISTIC_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH;
        }
        else if (wcscmp(sensitivity, L"accent") == 0)
        {
            return LINGUISTIC_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH;
        }
        else if (wcscmp(sensitivity, L"case") == 0)
        {
            return  NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH | LINGUISTIC_IGNOREDIACRITIC;
        }
        else if (wcscmp(sensitivity, L"variant") == 0)
        {
            return NORM_LINGUISTIC_CASING;
        }
        return 0;
    }
    // Takes arguments as follows(all required):
    //     - [1] - String 1 for comparison
    //     - [2] - String 2 for comparison
    //     - [3] - Locale string (or undefined)
    //     - [4] - Sensitivity string (or undefined)
    //     - [5] - IgnorePunctuation boolean (or undefined)
    //     - [6] - Numeric boolean (or undefined)
    Var EngineInterfaceObject::EntryIntl_CompareString(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(args.Info.Count < 7 || !JavascriptString::Is(args.Values[1]) || !JavascriptString::Is(args.Values[2]))
        {
            // CompareStringEx of undefined or non-strings is undefined
            return scriptContext->GetLibrary()->GetUndefined();
        }

        DWORD compareFlags = 0;
        JavascriptString* str1 = JavascriptString::FromVar(args.Values[1]);
        JavascriptString* str2 = JavascriptString::FromVar(args.Values[2]);

        WCHAR defaultLocale[LOCALE_NAME_MAX_LENGTH];
        const wchar_t *givenLocale = nullptr;
        defaultLocale[0] = '\0';

        if(!JavascriptOperators::IsUndefinedObject(args.Values[3], scriptContext))
        {
            if(!JavascriptString::Is(args.Values[3]))
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            givenLocale = JavascriptString::FromVar(args.Values[3])->GetSz();
        }

        if(!JavascriptOperators::IsUndefinedObject(args.Values[4], scriptContext))
        {
            if(!JavascriptString::Is(args.Values[4]))
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            compareFlags |= getFlagsForSensitivity(JavascriptString::FromVar(args.Values[4])->GetSz());
        }
        else 
        {
            compareFlags |= NORM_LINGUISTIC_CASING;
        }

        if(!JavascriptOperators::IsUndefinedObject(args.Values[5], scriptContext))
        {
            if(!JavascriptBoolean::Is(args.Values[5]))
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            else if((boolean)(JavascriptBoolean::FromVar(args.Values[5])->GetValue()))
            {
                compareFlags |= NORM_IGNORESYMBOLS;
            }
        }

        if(!JavascriptOperators::IsUndefinedObject(args.Values[6], scriptContext))
        {
            if(!JavascriptBoolean::Is(args.Values[6]))
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
            else if((boolean)(JavascriptBoolean::FromVar(args.Values[6])->GetValue()))
            {
                compareFlags |= SORT_DIGITSASNUMBERS;
            }
        }

        if(givenLocale == nullptr && GetUserDefaultLocaleName(defaultLocale, _countof(defaultLocale)) == 0)
        {
            JavascriptError::MapAndThrowError(scriptContext, HRESULT_FROM_WIN32(GetLastError()));
        }

        int compareResult = 0;
        BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"localeCompare")
        {
            wchar_t * aLeft = nullptr;
            wchar_t * aRight = nullptr;
            charcount_t size1 = 0;
            charcount_t size2 = 0;
            _NORM_FORM canonicalEquivalentForm = NormalizationC;
            if (!IsNormalizedString(canonicalEquivalentForm, str1->GetSz(), -1))
            {
                aLeft = str1->GetNormalizedString(canonicalEquivalentForm, tempAllocator, size1);
            }

            if (!IsNormalizedString(canonicalEquivalentForm, str2->GetSz(), -1))
            {
                aRight = str2->GetNormalizedString(canonicalEquivalentForm, tempAllocator, size2);
            }

            if (aLeft == nullptr)
            {
                aLeft = const_cast<wchar_t*>(str1->GetSz());
                size1 = str1->GetLength();
            }
            if (aRight == nullptr)
            {
                aRight = const_cast<wchar_t*>(str2->GetSz());
                size2 = str2->GetLength();
            }

            compareResult = CompareStringEx(givenLocale != nullptr ? givenLocale : defaultLocale, compareFlags, aLeft, size1, aRight, size2, NULL, NULL, 0);
        }
        END_TEMP_ALLOCATOR(tempAllocator, scriptContext);


        if (compareResult != 0)//CompareStringEx returns 1, 2, 3 on success;  2 is the strings are equal, 1 is the fist string is lexically less than second, 3 is reverse. 
        {
            return JavascriptNumber::ToVar(compareResult - 2, scriptContext);//Convert 1,2,3 to -1,0,1
        }

        JavascriptError::MapAndThrowError(scriptContext, HRESULT_FROM_WIN32(GetLastError()));
    }

    Var EngineInterfaceObject::EntryIntl_CurrencyDigits(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        HRESULT hr;

        if(args.Info.Count < 2 || !JavascriptString::Is(args.Values[1]))
        {
            // Call with undefined or non-string is undefined
            return scriptContext->GetLibrary()->GetFalse();
        }

        JavascriptString *argString = JavascriptString::FromVar(args.Values[1]);
        AutoCOMPtr<NumberFormatting::ICurrencyFormatter> currencyFormatter(nullptr);
        IfFailThrowHr(GetWindowsGlobalizationAdapter(scriptContext)->CreateCurrencyFormatterCode(scriptContext, argString->GetSz(), &currencyFormatter));
        AutoCOMPtr<NumberFormatting::INumberFormatterOptions> numberFormatterOptions;
        IfFailThrowHr(currencyFormatter->QueryInterface(__uuidof(NumberFormatting::INumberFormatterOptions), reinterpret_cast<void**>(&numberFormatterOptions)));
        Assert(numberFormatterOptions);
        INT32 fractionDigits;
        IfFailThrowHr(numberFormatterOptions->get_FractionDigits(&fractionDigits));
        return JavascriptNumber::ToVar(fractionDigits, scriptContext);
    }

    //Helper, this just prepares based on fraction and integer format options
    void EngineInterfaceObject::prepareWithFractionIntegerDigits(ScriptContext* scriptContext, NumberFormatting::INumberRounderOption* rounderOptions, 
        NumberFormatting::INumberFormatterOptions* formatterOptions, uint16 minFractionDigits, uint16 maxFractionDigits, uint16 minIntegerDigits)
    {
        HRESULT hr;
        WindowsGlobalizationAdapter* wga = GetWindowsGlobalizationAdapter(scriptContext);
        AutoCOMPtr<NumberFormatting::INumberRounder> numberRounder(nullptr);
        AutoCOMPtr<NumberFormatting::IIncrementNumberRounder> incrementNumberRounder(nullptr);

        IfFailThrowHr(wga->CreateIncrementNumberRounder(scriptContext, &numberRounder));
        IfFailThrowHr(numberRounder->QueryInterface(__uuidof(NumberFormatting::IIncrementNumberRounder), reinterpret_cast<void**>(&incrementNumberRounder)));
        Assert(incrementNumberRounder);
        IfFailThrowHr(incrementNumberRounder->put_RoundingAlgorithm(Windows::Globalization::NumberFormatting::RoundingAlgorithm::RoundingAlgorithm_RoundHalfAwayFromZero));

        IfFailThrowHr(incrementNumberRounder->put_Increment(pow(10.0, -maxFractionDigits)));
        IfFailThrowHr(rounderOptions->put_NumberRounder(numberRounder));

        IfFailThrowHr(formatterOptions->put_FractionDigits(minFractionDigits));
        IfFailThrowHr(formatterOptions->put_IntegerDigits(minIntegerDigits));
    }

    //Helper, this just prepares based on significant digits format options
    void EngineInterfaceObject::prepareWithSignificantDigits(ScriptContext* scriptContext, NumberFormatting::INumberRounderOption* rounderOptions, NumberFormatting::INumberFormatter *numberFormatter,
        NumberFormatting::INumberFormatterOptions* formatterOptions, uint16 minSignificantDigits, uint16 maxSignificantDigits)
    {
        HRESULT hr;
        WindowsGlobalizationAdapter* wga = GetWindowsGlobalizationAdapter(scriptContext);
        AutoCOMPtr<NumberFormatting::INumberRounder> numberRounder(nullptr);
        AutoCOMPtr<NumberFormatting::ISignificantDigitsNumberRounder> incrementNumberRounder(nullptr);
        AutoCOMPtr<NumberFormatting::ISignificantDigitsOption> significantDigitsOptions(nullptr);

        IfFailThrowHr(wga->CreateSignificantDigitsRounder(scriptContext, &numberRounder));
        IfFailThrowHr(numberRounder->QueryInterface(__uuidof(NumberFormatting::ISignificantDigitsNumberRounder), reinterpret_cast<void**>(&incrementNumberRounder)));
        Assert(incrementNumberRounder);
        IfFailThrowHr(incrementNumberRounder->put_RoundingAlgorithm(Windows::Globalization::NumberFormatting::RoundingAlgorithm::RoundingAlgorithm_RoundHalfAwayFromZero));
		
        IfFailThrowHr(incrementNumberRounder->put_SignificantDigits(maxSignificantDigits));
        IfFailThrowHr(rounderOptions->put_NumberRounder(numberRounder));

        IfFailThrowHr(numberFormatter->QueryInterface(__uuidof(NumberFormatting::ISignificantDigitsOption), reinterpret_cast<void**>(&significantDigitsOptions)));
        IfFailThrowHr(significantDigitsOptions->put_SignificantDigits(minSignificantDigits));
        Assert(significantDigitsOptions);

        //Clear minimum fraction digits as in the case of percent 2 is supplied
        IfFailThrowHr(formatterOptions->put_FractionDigits(0));
    }

    /*
    * This function has the following options:
    *  - Format as Percent.
    *  - Format as Number.
    *  - If significant digits are present, format using the significant digts;
    *  - Otherwise format using minimumFractionDigits, maximuimFractionDigits, minimumIntegerDigits
    */
    Var EngineInterfaceObject::EntryIntl_FormatNumber(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        DelayLoadWindowsGlobalization* wsl = scriptContext->GetThreadContext()->GetWindowsGlobalizationLibrary();

        //First argument is required and must be either a tagged integer or a number; second is also required and is the internal state object
        if (args.Info.Count < 3 || ! (TaggedInt::Is(args.Values[1]) || JavascriptNumber::Is(args.Values[1])) || !DynamicObject::Is(args.Values[2]))
        {
            // Call with undefined or non-number is undefined
            return scriptContext->GetLibrary()->GetUndefined();
        }

        DynamicObject *obj = DynamicObject::FromVar(args.Values[2]);

        NumberFormatting::INumberFormatter *numberFormatter;
        Var hiddenObject;
        obj->GetInternalProperty(obj, Js::InternalPropertyIds::HiddenObject, &hiddenObject, NULL, scriptContext);

        numberFormatter = static_cast<NumberFormatting::INumberFormatter *>(((AutoCOMJSObject *)hiddenObject)->GetInstance());

        AutoHSTRING result;
        HRESULT hr;
        if (TaggedInt::Is(args.Values[1]))
        {
            IfFailThrowHr(numberFormatter->FormatInt(TaggedInt::ToInt32(args.Values[1]), &result));
        }
        else
        {
            IfFailThrowHr(numberFormatter->FormatDouble(JavascriptNumber::GetValue(args.Values[1]), &result));
        }
        PCWSTR strBuf = wsl->WindowsGetStringRawBuffer(*result, NULL);
        JavascriptStringObject *retVal = scriptContext->GetLibrary()->CreateStringObject(Js::JavascriptString::NewCopySz(strBuf, scriptContext));
        return retVal;
    }

    Var EngineInterfaceObject::EntryIntl_FormatDateTime(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(args.Info.Count < 3 || !(TaggedInt::Is(args.Values[1]) || JavascriptNumber::Is(args.Values[1])) || !DynamicObject::Is(args.Values[2]))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        Windows::Foundation::DateTime winDate;
        HRESULT hr;
        if (TaggedInt::Is(args.Values[1]))
        {
            hr = Js::DateUtilities::ES5DateToWinRTDate(TaggedInt::ToInt32(args.Values[1]), &(winDate.UniversalTime));
        }
        else
        {
            hr = Js::DateUtilities::ES5DateToWinRTDate(JavascriptNumber::GetValue(args.Values[1]), &(winDate.UniversalTime));
        }
        if (FAILED(hr))
        {
            HandleOOMSOEHR(hr);
            // If conversion failed, double value is outside the range of WinRT DateTime
            Js::JavascriptError::ThrowRangeError(scriptContext, JSERR_OutOfDateTimeRange);
        }

        DynamicObject* obj = DynamicObject::FromVar(args.Values[2]);
        Var hiddenObject;
        
        obj->GetInternalProperty(obj, Js::InternalPropertyIds::HiddenObject, &hiddenObject, NULL, scriptContext);

        //We are going to perform the same check for timeZone as when caching the formatter.
        Var propertyValue;
        AutoHSTRING result;

        //If timeZone is undefined; then use the standard dateTimeFormatter to format in local time; otherwise use the IDateTimeFormatter2 to format using specified timezone (UTC)
        if(!GetPropertyBuiltInFrom(obj, __timeZone) || JavascriptOperators::IsUndefinedObject(propertyValue))
        {
            DateTimeFormatting::IDateTimeFormatter *formatter = static_cast<DateTimeFormatting::IDateTimeFormatter *>(((AutoCOMJSObject *)hiddenObject)->GetInstance());
            Assert(formatter);
            IfFailThrowHr(formatter->Format(winDate, &result));
        }
        else
        {
            DateTimeFormatting::IDateTimeFormatter2 *formatter = static_cast<DateTimeFormatting::IDateTimeFormatter2 *>(((AutoCOMJSObject *)hiddenObject)->GetInstance());
            Assert(formatter);
            HSTRING timeZone;
            HSTRING_HEADER timeZoneHeader;

            IfFailThrowHr(WindowsCreateStringReference(L"UTC", wcslen(L"UTC"), &timeZoneHeader, &timeZone));

            IfFailThrowHr(formatter->FormatUsingTimeZone(winDate, timeZone, &result));
        }
        PCWSTR strBuf = scriptContext->GetThreadContext()->GetWindowsGlobalizationLibrary()->WindowsGetStringRawBuffer(*result, NULL);

        return Js::JavascriptString::NewCopySz(strBuf, scriptContext);
    }

    /*
    * This function registers built in functions when Intl initializes.
    * Call with (Function : toRegister, integer : id) 
    * ID Mappings:
    *  - 0 for Date.prototype.toLocaleString
    *  - 1 for Date.prototype.toLocaleDateString
    *  - 2 for Date.prototype.toLocaleTimeString
    *  - 3 for Number.prototype.toLocaleString
    *  - 4 for String.prototype.localeComapre
    */
    Var EngineInterfaceObject::EntryIntl_RegisterBuiltInFunction(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        //This function will only be used during the construction of the Intl object, hence Asserts are in place.
        Assert(args.Info.Count >= 3 && JavascriptFunction::Is(args.Values[1]) && TaggedInt::Is(args.Values[2]));

        JavascriptFunction *func = JavascriptFunction::FromVar(args.Values[1]);
        int32 id = TaggedInt::ToInt32(args.Values[2]);

        Assert(id >= 0 && id < 5);
        EngineInterfaceObject* nativeEngineInterfaceObj = scriptContext->GetLibrary()->GetEngineInterfaceObject();

        switch(id)
        {
        case 0:
            nativeEngineInterfaceObj->dateToLocaleString = func;
            break;
        case 1:
            nativeEngineInterfaceObj->dateToLocaleDateString = func;
            break;
        case 2:
            nativeEngineInterfaceObj->dateToLocaleTimeString = func;
            break;
        case 3:
            nativeEngineInterfaceObj->numberToLocaleString = func;            
            break;
        case 4:
            nativeEngineInterfaceObj->stringLocaleCompare = func;
            break;
        default:
            Assert(false);//Shouldn't hit here, the previous assert should catch this.
            break;
        }

        //Don't need to return anything
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var EngineInterfaceObject::EntryIntl_GetHiddenObject(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(callInfo.Count < 2 || !DynamicObject::Is(args.Values[1])) 
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        DynamicObject* obj = DynamicObject::FromVar(args.Values[1]);
        Var hiddenObject;
        if(!obj->GetInternalProperty(obj, Js::InternalPropertyIds::HiddenObject, &hiddenObject, NULL, scriptContext))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
        return hiddenObject;
    }

    Var EngineInterfaceObject::EntryIntl_SetHiddenObject(RecyclableObject* function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(callInfo.Count < 3 || !DynamicObject::Is(args.Values[1]) || !DynamicObject::Is(args.Values[2]))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        DynamicObject* obj = DynamicObject::FromVar(args.Values[1]);
        DynamicObject* value = DynamicObject::FromVar(args.Values[2]);

        if(obj->SetInternalProperty(Js::InternalPropertyIds::HiddenObject, value, Js::PropertyOperationFlags::PropertyOperation_None, NULL))
        {
            return scriptContext->GetLibrary()->GetTrue();
        }
        else 
        {
            return scriptContext->GetLibrary()->GetFalse();
        }
    }

    /*
    * First parameter is the object onto which prototype should be set; second is the value
    */
    Var EngineInterfaceObject::EntryIntl_BuiltIn_SetPrototype(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(callInfo.Count < 3 || !DynamicObject::Is(args.Values[1]) || !RecyclableObject::Is(args.Values[2]))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        DynamicObject* obj = DynamicObject::FromVar(args.Values[1]);
        RecyclableObject* value = RecyclableObject::FromVar(args.Values[2]);

        obj->SetPrototype(value);

        return obj;
    }

    /*
    * First parameter is the array object.
    */
    Var EngineInterfaceObject::EntryIntl_BuiltIn_GetArrayLength(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if (callInfo.Count < 2)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        if (DynamicObject::IsAnyArray(args.Values[1]))
        {
            JavascriptArray* arr = JavascriptArray::FromAnyArray(args.Values[1]);
            return TaggedInt::ToVarUnchecked(arr->GetLength());
        }
        else
        {
            AssertMsg(false, "Object passed in with unknown type ID, verify Intl.js is correct.");
            return TaggedInt::ToVarUnchecked(0);
        }
    }

    /*
     * First parameter is the string on which to match.
     * Second parameter is the regex object
     */
    Var EngineInterfaceObject::EntryIntl_BuiltIn_RegexMatch(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if(callInfo.Count < 2 || !JavascriptString::Is(args.Values[1]) || !JavascriptRegExp::Is(args.Values[2]))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        JavascriptString *stringToUse = JavascriptString::FromVar(args.Values[1]);
        JavascriptRegExp *regexpToUse = JavascriptRegExp::FromVar(args.Values[2]);

        return RegexHelper::RegexMatchNoHistory(scriptContext, regexpToUse, stringToUse, false);
    }

    /*
    * First parameter is the function, then its the this arg; so at least 2 are needed.
    */
    Var EngineInterfaceObject::EntryIntl_BuiltIn_CallInstanceFunction(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        Assert(args.Info.Count <= 5);
        if(callInfo.Count < 3 || args.Info.Count > 5 || !JavascriptConversion::IsCallable(args.Values[1]) || !RecyclableObject::Is(args.Values[2]))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        RecyclableObject *func = RecyclableObject::FromVar(args.Values[1]);

        //Shift the arguments by 2 so argument at index 2 becomes the 'this' argument at index 0
        Var newVars[3];
        Js::Arguments newArgs(callInfo, newVars);

        for (uint i=0; i<args.Info.Count - 2; ++i)
        {
            newArgs.Values[i] = args.Values[i + 2];
        }

        newArgs.Info.Count = args.Info.Count - 2;

        return JavascriptFunction::CallFunction<true>(func, func->GetEntryPoint(), newArgs);
    }

    Var EngineInterfaceObject::Entry_GetErrorMessage(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);
        
        if (callInfo.Count < 2)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        int hr = Js::JavascriptConversion::ToInt32(args[1], scriptContext);
        int resourceId;

        switch(hr)
        {
        case ASYNCERR_NoErrorInErrorState:
            resourceId = 5200;
            break;
        case ASYNCERR_InvalidStatusArg:
            resourceId = 5201;
            break;
        case ASYNCERR_InvalidSenderArg:
            resourceId = 5202;
            break;
        default:
            AssertMsg(false, "Invalid HRESULT passed to GetErrorMessage. This shouldn't come from Promise.js - who called us?");
            return scriptContext->GetLibrary()->GetUndefined();
        }

        LCID lcid = GetUserLocale();
        const int strLength = 1024;
        OLECHAR errorString[strLength];

        if(FGetResourceString(resourceId, errorString, strLength, lcid))
        {
            return Js::JavascriptString::NewCopySz(errorString, scriptContext);
        }

        AssertMsg(false, "FGetResourceString returned false.");
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var EngineInterfaceObject::Entry_LogDebugMessage(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

#if DBG
        if (callInfo.Count < 2 || !JavascriptString::Is(args.Values[1]))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        JavascriptString* message = JavascriptString::FromVar(args.Values[1]);

        Output::Print(message->GetString());
        Output::Flush();
#endif
        
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var EngineInterfaceObject::Entry_TagPublicLibraryCode(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if (callInfo.Count >= 2 && JavascriptFunction::Is(args.Values[1]))
        {
            JavascriptFunction* func = JavascriptFunction::FromVar(args.Values[1]);
            func->GetFunctionProxy()->SetIsPublicLibraryCode();

            if (callInfo.Count >= 3 && JavascriptString::Is(args.Values[2]))
            {
                JavascriptString* customFunctionName = JavascriptString::FromVar(args.Values[2]);
                func->GetFunctionProxy()->EnsureDeserialized()->SetDisplayName(customFunctionName->GetString());
            }

            return func;
        }
        
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var EngineInterfaceObject::EntryPromise_EnqueueTask(RecyclableObject *function, CallInfo callInfo, ...)
    {
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo);

        if (callInfo.Count >= 2 && JavascriptFunction::Is(args.Values[1]))
        {
            JavascriptFunction* taskVar = JavascriptFunction::FromVar(args.Values[1]);
            scriptContext->GetLibrary()->EnqueueTask(taskVar);
        }
        
        return scriptContext->GetLibrary()->GetUndefined();
    }

#ifndef GlobalBuiltIn
#define GlobalBuiltIn(global, method) 
#define GlobalBuiltInConstructor(global) 

#define BuiltInRaiseException(exceptionType, exceptionID) \
    Var EngineInterfaceObject::EntryIntl_BuiltIn_raise##exceptionID(RecyclableObject *function, CallInfo callInfo, ...) \
    { \
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo); \
        \
        JavascriptError::Throw##exceptionType(scriptContext, JSERR_##exceptionID); \
    } 

#define BuiltInRaiseException1(exceptionType, exceptionID) \
    Var EngineInterfaceObject::EntryIntl_BuiltIn_raise##exceptionID(RecyclableObject *function, CallInfo callInfo, ...) \
    { \
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo); \
        \
        if(args.Info.Count < 2 || !JavascriptString::Is(args.Values[1])) \
        { \
            Assert(false); \
            JavascriptError::Throw##exceptionType(scriptContext, JSERR_##exceptionID); \
        } \
        JavascriptError::Throw##exceptionType##Var(scriptContext, JSERR_##exceptionID, JavascriptString::FromVar(args.Values[1])->GetSz()); \
    } 

#define BuiltInRaiseException2(exceptionType, exceptionID) \
    Var EngineInterfaceObject::EntryIntl_BuiltIn_raise##exceptionID(RecyclableObject *function, CallInfo callInfo, ...) \
    { \
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo); \
        \
        if(args.Info.Count < 3 || !JavascriptString::Is(args.Values[1]) || !JavascriptString::Is(args.Values[2])) \
        { \
            Assert(false); \
            JavascriptError::Throw##exceptionType(scriptContext, JSERR_##exceptionID); \
        } \
        JavascriptError::Throw##exceptionType##Var(scriptContext, JSERR_##exceptionID, JavascriptString::FromVar(args.Values[1])->GetSz(), JavascriptString::FromVar(args.Values[2])->GetSz()); \
    } 

#define BuiltInRaiseException3(exceptionType, exceptionID) \
    Var EngineInterfaceObject::EntryIntl_BuiltIn_raise##exceptionID##_3(RecyclableObject *function, CallInfo callInfo, ...) \
    { \
        EngineInterfaceObject_CommonFunctionProlog(function, callInfo); \
        \
        if(args.Info.Count < 4 || !JavascriptString::Is(args.Values[1]) || !JavascriptString::Is(args.Values[2]) || !JavascriptString::Is(args.Values[3])) \
        { \
            Assert(false); \
            JavascriptError::Throw##exceptionType(scriptContext, JSERR_##exceptionID); \
        } \
        JavascriptError::Throw##exceptionType##Var(scriptContext, JSERR_##exceptionID, JavascriptString::FromVar(args.Values[1])->GetSz(), JavascriptString::FromVar(args.Values[2])->GetSz(), JavascriptString::FromVar(args.Values[3])->GetSz()); \
    } 

#include "EngineInterfaceObjectBuiltIns.h"

#undef BuiltInRaiseException
#undef BuiltInRaiseException1
#undef BuiltInRaiseException2
#undef BuiltInRaiseException3
#undef GlobalBuiltIn
#undef GlobalBuiltInConstructor
#endif

}
