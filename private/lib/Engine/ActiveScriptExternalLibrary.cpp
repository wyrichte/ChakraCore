//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

#include "EnginePch.h"
#include "DiagnosticsScriptObject.h"
#include "ExternalObject.h"
#include "DOMFastPath.h"
#include "DOMFastPathInfo.h"
#include "JavascriptTypedObjectSlotAccessorFunction.h"

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
        Js::SimplePathTypeHandlerNoAttr::New(scriptContext, library->GetRootPath(), 0, 0, 0, true, true), true, true);
}

void ActiveScriptExternalLibrary::InitializeDiagnosticsScriptObject()
{
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    diagnosticsScriptObject = Js::DiagnosticsScriptObject::New(scriptContext->GetRecycler(), library->GetObjectType());
    library->AddMember(javascriptLibrary->GetGlobalObject(),
        scriptContext->GetOrAddPropertyIdTracked(_u("diagnosticsScript")),
        this->diagnosticsScriptObject);

    PropertyId getStackTracePropId = scriptContext->GetOrAddPropertyIdTracked(_u("getStackTrace"));
    library->getStackTrace = library->DefaultCreateFunction(&Js::DiagnosticsScriptObject::EntryInfo::GetStackTrace, 0, nullptr, nullptr, getStackTracePropId);
    library->AddMember(diagnosticsScriptObject, getStackTracePropId, library->GetStackTraceFunction());
    PropertyId debugEvalPropId = scriptContext->GetOrAddPropertyIdTracked(_u("debugEval"));
    library->debugEval = library->DefaultCreateFunction(&Js::DiagnosticsScriptObject::EntryInfo::DebugEval, 2, nullptr, nullptr, debugEvalPropId);
    library->AddMember(diagnosticsScriptObject, debugEvalPropId, library->GetDebugEval());
    library->AddFunctionToLibraryObjectWithPropertyName(diagnosticsScriptObject, _u("getConsoleScope"), &Js::DiagnosticsScriptObject::EntryInfo::GetConsoleScopeObject, 0);
#ifdef EDIT_AND_CONTINUE
    PropertyId editSourcePropId = scriptContext->GetOrAddPropertyIdTracked(_u("editSource"));
    library->editSource = library->DefaultCreateFunction(&Js::DiagnosticsScriptObject::EntryInfo::EditSource, 3, nullptr, nullptr, editSourcePropId);
    library->AddMember(diagnosticsScriptObject, editSourcePropId, library->GetEditSource());
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

Js::JavascriptFunction * ActiveScriptExternalLibrary::CreateSlotGetterFunction(bool isObject, unsigned int slotIndex, Js::FunctionInfo* functionInfo, int typeId, PropertyId nameId, ScriptMethod fallBack)
{
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();

    Js::DynamicType **slotGetterFunctionTypes = isObject ? objectSlotGetterFunctionTypes : typeSlotGetterFunctionTypes;
    IR::JnHelperMethod getterIRHelper = isObject ? DOMFastPathInfo::GetObjectGetterIRHelper(slotIndex) : DOMFastPathInfo::GetTypeGetterIRHelper(slotIndex);

    // GC should zero out the whole library; we shouldn't need to explicitly zero out
    if (slotGetterFunctionTypes[slotIndex] == nullptr)
    {
        slotGetterFunctionTypes[slotIndex] = library->CreateFunctionWithLengthType(functionInfo);
        if (JITManager::GetJITManager()->IsOOPJITEnabled() && JITManager::GetJITManager()->IsConnected())
        {
            PSCRIPTCONTEXT_HANDLE remoteScriptContext = this->scriptContext->GetRemoteScriptAddr();
            if (remoteScriptContext)
            {
                HRESULT hr = JITManager::GetJITManager()->AddDOMFastPathHelper(
                    remoteScriptContext,
                    (intptr_t)functionInfo,
                    (int)getterIRHelper);
                JITManager::HandleServerCallResult(hr, RemoteCallType::StateUpdate);
            }
        }
        else
        {
            scriptContext->AddToDOMFastPathHelperMap((intptr_t)functionInfo, getterIRHelper);
        }
    }
    return RecyclerNewEnumClass(library->GetRecycler(), EnumClass_1_Bit, Js::JavascriptTypedObjectSlotAccessorFunction, slotGetterFunctionTypes[slotIndex], functionInfo, typeId, nameId, fallBack);
}

Js::JavascriptFunction* ActiveScriptExternalLibrary::CreateSlotSetterFunction(bool isObject, unsigned int slotIndex, Js::FunctionInfo* functionInfo, int typeId, PropertyId nameId, ScriptMethod fallBack)
{
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();
    Js::DynamicType **slotSetterFunctionTypes = isObject ? objectSlotSetterFunctionTypes : typeSlotSetterFunctionTypes;
    // GC should zero out the whole library; we shouldn't need to explicitly zero out
    if (slotSetterFunctionTypes[slotIndex] == nullptr)
    {
        slotSetterFunctionTypes[slotIndex] = library->CreateFunctionWithLengthType(functionInfo);
    }

    return RecyclerNewEnumClass(library->GetRecycler(), EnumClass_1_Bit, Js::JavascriptTypedObjectSlotAccessorFunction, slotSetterFunctionTypes[slotIndex], functionInfo, typeId, nameId, fallBack);
}
