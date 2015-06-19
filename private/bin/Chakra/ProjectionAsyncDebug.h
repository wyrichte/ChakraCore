//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Projection
{
    class ProjectionAsyncDebug
    {
    public:

        enum IdentifyAsyncOperationsParameterType
        {
            IdentifyAsyncOperationsParameterType_In      = 0,
            IdentifyAsyncOperationsParameterType_Out     = 1,
            IdentifyAsyncOperationsParameterType_InOut   = 2
        };

        ProjectionAsyncDebug(ProjectionContext* projectionContext);
        ~ProjectionAsyncDebug();

        bool IsAsyncType(
            __in RtTYPE type);

        AsyncDebug::AsyncOperationId InstrumentAsyncMethodCall(
            __in_opt ThisInfo* thisInfo,
            __in RtABIMETHODSIGNATURE signature);

        bool InstrumentAsyncMethodCallByScanningParameters(
            __in_opt ThisInfo* thisInfo,
            __in RtABIMETHODSIGNATURE signature, 
            __out RtABIPARAMETER* asyncParameter,
            __out AsyncDebug::AsyncOperationId* asyncOperationId,
            __in_opt IdentifyAsyncOperationsParameterType parameterType = IdentifyAsyncOperationsParameterType_Out);

        AsyncDebug::AsyncOperationId BeginAsyncMethodCall(
            __in MetadataStringId runtimeClassNameId,
            __in MetadataStringId methodNameId);
        
    private:
        
        ProjectionAsyncDebug();

        ProjectionContext* m_projectionContext;
    };
}
