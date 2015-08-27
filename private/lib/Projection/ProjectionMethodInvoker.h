//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#if defined(_M_X64)
extern "C" HRESULT amd64_ProjectionCall(void* methodAddress, void* unknown, void* paramStack, ulong argsSize);
#elif defined(_M_ARM)
extern "C" HRESULT arm_ProjectionCall(void* entryPoint, ProjectionModel::ApcsCallLayout* callLayout);
#elif defined(_M_ARM64)
extern "C" HRESULT arm64_ProjectionCall(void* entryPoint, ProjectionModel::ApcsCallLayout* callLayout);
#endif

namespace Projection
{
    class ProjectionMethodInvoker
    {
        RtABIMETHODSIGNATURE signature;
#ifdef _M_ARM32_OR_ARM64
        ProjectionModel::ApcsCallLayout callLayout;
        ProjectionModel::ParameterLocation* parameterLocations; // Array of locations for each parameter (not including "this" parameter).
#else
        byte * stack;
#endif
        ProjectionMarshaler marshal;
        Js::Var ReadOutParameters(Js::Arguments arguments, ParameterMarker* parameterMarker = nullptr);
        void BuildCallStack(IUnknown * _this, Js::Arguments arguments, bool isDelegate = false);

    public:
        ProjectionMethodInvoker(RtABIMETHODSIGNATURE signature, ProjectionContext * projectionContext);
        ~ProjectionMethodInvoker() { }
        HRESULT InvokeUnknown(IUnknown * _this, int vtableIndex, Js::Arguments arguments, bool isDefaultInterface = false, bool isDelegate = false);
        Js::Var ReadOutOrThrow(HRESULT hr, bool boundsToUndefined, Js::Arguments args, ParameterMarker* parameterMarker = nullptr);
        void SetIsInAsyncInterface() { marshal.SetIsInAsyncInterface(); }
    };
}