//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#pragma once
class ProjectionExternalLibrary : public Js::ExternalLibraryBase
{
public:
    ProjectionExternalLibrary();
    void Initialize(Js::JavascriptLibrary* library);
    Js::JavascriptFunction* GetWinRTPromiseConstructor();
    Js::DynamicType * GetWinRTDateType() const { return winrtDateType; }
    Js::DynamicType * GetWinRTErrorType() const { return winrtErrorType; }
    Js::JavascriptError* CreateWinRTError(IErrorInfo* perrinfo, Js::RestrictedErrorStrings * proerrstr);
    static bool __cdecl InitializeWinRTErrorConstructor(Js::DynamicObject* constructor, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode);
    static bool __cdecl InitializeWinRTErrorPrototype(Js::DynamicObject* prototype, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode);
    void InitializeWinRTPromiseConstructor();
    Js::JavascriptFunction* GetWinRTErrorConstructor() const { return winrtErrorConstructor; }
    Js::DynamicObject* GetWinRTErrorPrototype() const { return winrtErrorPrototype; }
    WinRTPromiseEngineInterfaceExtensionObject* GetWinRTPromiseEngineInterfaceExtensionObject() const { return winRTPromiseExtension; }

private:
    Js::DynamicType * winrtDateType;
    Js::DynamicType * winrtErrorType;
    Js::RuntimeFunction* winrtErrorConstructor;
    Js::DynamicObject* winrtErrorPrototype;
    Js::JavascriptFunction* winRTPromiseConstructor;
    WinRTPromiseEngineInterfaceExtensionObject* winRTPromiseExtension;
};