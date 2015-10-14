//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#pragma once

class ActiveScriptExternalLibrary : public Js::ExternalLibraryBase
{
public:
    ActiveScriptExternalLibrary();
    HRESULT Initialize(Js::JavascriptLibrary* library);

    void InitializeDiagnosticsScriptObject(Js::DiagnosticsScriptObject* newDiagnosticsScriptObject);
    Js::DiagnosticsScriptObject* GetDiagnosticsScriptObect() { return diagnosticsScriptObject; }
    Js::StaticType * GetDispMemberProxyType() { return dispMemberProxyType; }
    Js::StaticType * GetHostDispatchType() { return hostDispatchType; }
    Js::DynamicType * GetHostObjectType() { return hostObjectType; }
    void SetDispatchInvoke(Js::JavascriptMethod dispatchInvoke);
    Js::DynamicType * GetModuleRootType() const { return moduleRootType; }

private:
    Js::StaticType * dispMemberProxyType;
    Js::StaticType * hostDispatchType;
    Js::DynamicType * hostObjectType;
    Js::DynamicType * moduleRootType;
    Js::DiagnosticsScriptObject * diagnosticsScriptObject;

    void InitializeTypes();
    void InitializeDiagnosticsScriptObject();
};
