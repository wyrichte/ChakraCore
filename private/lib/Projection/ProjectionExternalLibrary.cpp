//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#include "ProjectionPch.h"

#include "Types\DeferredTypeHandler.h"
#include "Types\PathTypeHandler.h"

ProjectionExternalLibrary::ProjectionExternalLibrary() :
    winrtDateType(nullptr),
    winrtErrorType(nullptr),
    winrtErrorPrototype(nullptr),
    winrtErrorConstructor(nullptr)
{

}

void ProjectionExternalLibrary::Initialize(Js::JavascriptLibrary* library)
{
    ExternalLibraryBase::Initialize(library);
    Js::JavascriptFunction* nativeErrorPrototype = nullptr;

    winrtDateType = Js::DynamicType::New(scriptContext, Js::TypeIds_WinRTDate, library->GetDatePrototype(), nullptr,
        Js::SimplePathTypeHandler::New(scriptContext, library->GetRootPath(), 0, 0, 0, true, true), true, true);

    if (scriptContext->GetConfig()->IsES6PrototypeChain())
    {
        nativeErrorPrototype = library->GetErrorConstructor();
    }

    if (scriptContext->GetConfig()->IsWinRTEnabled())
    {
        winrtErrorConstructor = library->CreateBuiltinConstructor(&Js::JavascriptError::EntryInfo::NewWinRTErrorInstance,
            Js::DeferredTypeHandler<InitializeWinRTErrorConstructor>::GetDefaultInstance(),
            nativeErrorPrototype);
        library->AddFunction(library->GetGlobalObject(), Js::PropertyIds::WinRTError, winrtErrorConstructor);

        winrtErrorPrototype = RecyclerNew(library->GetRecycler(), Js::JavascriptError,
            Js::DynamicType::New(scriptContext, Js::TypeIds_Error, library->GetErrorPrototype(), nullptr,
                Js::DeferredTypeHandler<InitializeWinRTErrorPrototype, Js::DefaultDeferredTypeFilter, true>::GetDefaultInstance()),
            /*isExternalError*/FALSE, /*isPrototype*/TRUE);

        winrtErrorType = Js::DynamicType::New(scriptContext, Js::TypeIds_Error, winrtErrorPrototype, nullptr,
            Js::SimplePathTypeHandler::New(scriptContext, library->GetRootPath(), 0, 0, 0, true, true), true, true);

        winRTPromiseExtension = RecyclerNew(library->GetRecycler(), WinRTPromiseEngineInterfaceExtensionObject, scriptContext);
        library->GetEngineInterfaceObject()->SetEngineExtension(Js::EngineInterfaceExtensionKind::EngineInterfaceExtensionKind_WinRTPromise, winRTPromiseExtension);
    }
}

Js::JavascriptError* ProjectionExternalLibrary::CreateWinRTError(IErrorInfo* perrinfo, Js::RestrictedErrorStrings * proerrstr)
{
    // If WinRT isn't enabled, create an error of type kjstError instead.
    if (!scriptContext->GetConfig()->IsWinRTEnabled())
    {
        return scriptContext->GetLibrary()->CreateError();
    }
    AssertMsg(winrtErrorType, "Where's winrtErrorType?");
    Js::JavascriptLibrary *library = scriptContext->GetLibrary(); 
    Js::JavascriptError *pError = nullptr;
    if (perrinfo != nullptr)
    {
        Js::JavascriptErrorDebug *pErrorDebug = RecyclerNewFinalized(library->GetRecycler(), Js::JavascriptErrorDebug, perrinfo, GetWinRTErrorType());
        Js::JavascriptError::SetErrorType(pErrorDebug, kjstWinRTError);
        if (proerrstr != nullptr)
        {
            pErrorDebug->SetRestrictedErrorStrings(proerrstr);
        }
        pError = static_cast<Js::JavascriptError*>(pErrorDebug);
    }
    else
    {
        pError = RecyclerNew(library->GetRecycler(), Js::JavascriptError, winrtErrorType);
        Js::JavascriptError::SetErrorType(pError, kjstWinRTError);
    }
    return pError;
}

Js::JavascriptFunction* ProjectionExternalLibrary::GetWinRTPromiseConstructor()
{
    if (this->winRTPromiseConstructor == nullptr)
    {
        this->InitializeWinRTPromiseConstructor();
    }

    Assert(this->winRTPromiseConstructor != nullptr);

    return this->winRTPromiseConstructor;
}

bool ProjectionExternalLibrary::InitializeWinRTErrorConstructor(Js::DynamicObject* constructor, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode)
{
    typeHandler->Convert(constructor, mode, 3);
    Js::ScriptContext* scriptContext = constructor->GetScriptContext();
    ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ProjectionContext* projectionContext = scriptSite->GetScriptEngine()->GetProjectionContext();
    Assert(projectionContext != nullptr);
    Js::JavascriptLibrary* library = constructor->GetLibrary();
    library->AddMember(constructor, Js::PropertyIds::prototype, projectionContext->GetProjectionExternalLibrary()->GetWinRTErrorPrototype(), PropertyNone);
    library->AddMember(constructor, Js::PropertyIds::length, Js::TaggedInt::ToVarUnchecked(1), PropertyNone);
    if (scriptContext->GetConfig()->IsES6FunctionNameEnabled())
    {
        Js::PropertyAttributes prototypeNameMessageAttributes = PropertyConfigurable;
        library->AddMember(constructor, Js::PropertyIds::name, library->CreateStringFromCppLiteral(_u("WinRTError")), prototypeNameMessageAttributes);
    }
    constructor->SetHasNoEnumerableProperties(true);
    return true;
}

bool ProjectionExternalLibrary::InitializeWinRTErrorPrototype(Js::DynamicObject* prototype, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode)
{
    typeHandler->Convert(prototype, mode, 4);
    Js::ScriptContext* scriptContext = prototype->GetScriptContext();
    ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ProjectionContext* projectionContext = scriptSite->GetScriptEngine()->GetProjectionContext();
    Assert(projectionContext != nullptr);
    Js::JavascriptLibrary* library = prototype->GetLibrary();
    library->AddMember(prototype, Js::PropertyIds::constructor, projectionContext->GetProjectionExternalLibrary()->GetWinRTErrorConstructor());
    bool hasNoEnumerableProperties = true;
    Js::PropertyAttributes prototypeNameMessageAttributes = PropertyConfigurable | PropertyWritable;
    library->AddMember(prototype, Js::PropertyIds::name, library->CreateStringFromCppLiteral(_u("WinRTError")), prototypeNameMessageAttributes);
    library->AddMember(prototype, Js::PropertyIds::message, library->GetEmptyString(), prototypeNameMessageAttributes);
    library->AddFunctionToLibraryObject(prototype, Js::PropertyIds::toString, &Js::JavascriptError::EntryInfo::ToString, 0);
    prototype->SetHasNoEnumerableProperties(hasNoEnumerableProperties);
    return true;
}

void ProjectionExternalLibrary::InitializeWinRTPromiseConstructor()
{
    ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ProjectionContext* projectionContext = scriptSite->GetScriptEngine()->GetProjectionContext();
    Assert(projectionContext != nullptr);

    this->winRTPromiseConstructor = Js::JavascriptFunction::FromVar(GetWinRTPromiseEngineInterfaceExtensionObject()->GetPromiseConstructor(scriptContext));
}

