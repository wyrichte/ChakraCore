//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#include "EnginePch.h"
#include "DiagnosticsScriptObject.h"

#include "Types\PathTypeHandler.h"
#include "Types\PropertyIndexRanges.h"
#include "Types\SimpleDictionaryPropertyDescriptor.h"
#include "Types\SimpleDictionaryTypeHandler.h"

ActiveScriptExternalLibrary::ActiveScriptExternalLibrary()
    : ExternalLibraryBase(),
    dispMemberProxyType(nullptr),
    hostDispatchType(nullptr),
    hostObjectType(nullptr),
    diagnosticsScriptObject(nullptr)
{
}

void ActiveScriptExternalLibrary::InitializeTypes()
{
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    Js::DynamicObject * dispMemberProxyPrototype = library->CreateObject();
    dispMemberProxyType = Js::StaticType::New(scriptContext, Js::TypeIds_HostDispatch,
        dispMemberProxyPrototype, DispMemberProxy::DefaultInvoke);

    hostDispatchType = Js::StaticType::New(scriptContext, Js::TypeIds_HostDispatch,
        library->GetNull(), HostDispatch::Invoke);

    hostDispatchType->SetHasSpecialPrototype(true);

    moduleRootType = Js::DynamicType::New(scriptContext, Js::TypeIds_ModuleRoot, library->GetObjectPrototype(), nullptr,
        Js::SimpleDictionaryTypeHandler::New(scriptContext->GetRecycler(), 0, 0, 0, true, true), true, true);

    hostObjectType = Js::DynamicType::New(scriptContext, Js::TypeIds_HostObject, library->GetNull(), nullptr,
        Js::SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), true, true);
}

void ActiveScriptExternalLibrary::InitializeDiagnosticsScriptObject()
{
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    diagnosticsScriptObject = Js::DiagnosticsScriptObject::New(scriptContext->GetRecycler(), library->GetObjectType());
    library->AddMember(javascriptLibrary->GetGlobalObject(),
        scriptContext->GetOrAddPropertyIdTracked(L"diagnosticsScript"),
        this->diagnosticsScriptObject);

    library->AddFunctionToLibraryObjectWithPropertyName(diagnosticsScriptObject, L"getStackTrace", &Js::DiagnosticsScriptObject::EntryInfo::GetStackTrace, 1);
    library->AddFunctionToLibraryObjectWithPropertyName(diagnosticsScriptObject, L"debugEval", &Js::DiagnosticsScriptObject::EntryInfo::DebugEval, 3);
#ifdef EDIT_AND_CONTINUE
    library->AddFunctionToLibraryObjectWithPropertyName(diagnosticsScriptObject, L"editSource", &Js::DiagnosticsScriptObject::EntryInfo::EditSource, 2);
#endif
}

HRESULT ActiveScriptExternalLibrary::Initialize(Js::JavascriptLibrary* library)
{
    ExternalLibraryBase::Initialize(library);
    InitializeTypes();

    if (scriptContext->IsDiagnosticsScriptContext())
    {
        InitializeDiagnosticsScriptObject();
    }
    return NOERROR;
}

void ActiveScriptExternalLibrary::SetDispatchInvoke(Js::JavascriptMethod dispatchInvoke)
{
    if (this->dispMemberProxyType != nullptr)
    {
        this->dispMemberProxyType->SetDispatchInvoke(dispatchInvoke);
    }
}

