//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

//
// Entry point interface into the Js debugging API, with provided test dataTarget. Used for unit testing.
//
#undef  INTERFACE
#define INTERFACE IJsDebug2
DECLARE_INTERFACE_IID_(IJsDebug2, IJsDebug, "86114F6F-B4C4-4654-B2B0-33AECE74476C")
{
    STDMETHOD(OpenVirtualProcess)(THIS_
        _In_ IUnknown* pTestDataTarget,
        _In_ bool validateDebugMode,
        _In_ DWORD processId,
        _In_ UINT64 runtimeJsBaseAddress,
        _In_ IJsDebugDataTarget* pDataTarget,
        _Out_ IJsDebugProcess** ppProcess
        ) PURE;
};

#undef  INTERFACE
#define INTERFACE IJsDebugProcessPrivate
DECLARE_INTERFACE_IID_(IJsDebugProcessPrivate, IJsDebugProcess, "7CBB695A-06E7-4FFF-A328-D2A3477F7D8C")
{
    STDMETHOD(InspectVar)(THIS_
        _In_ VOID* var,
        _Out_ IJsDebugProperty** ppDebugProperty
        ) PURE;
};
