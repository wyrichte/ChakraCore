//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Projection
{
    using namespace ProjectionModel;

    ProjectionAsyncDebug::ProjectionAsyncDebug() : 
        m_projectionContext(nullptr)
    {
    }

    ProjectionAsyncDebug::ProjectionAsyncDebug(ProjectionContext* projectionContext) : 
        m_projectionContext(projectionContext)
    {
    }

    ProjectionAsyncDebug::~ProjectionAsyncDebug()
    {
        m_projectionContext = nullptr;
    }

    // Info:        Determine if a type will be special-projected to a promise.
    // Parameters:  type - The type to check for async behavior.
    // Returns:     True if type is an async type. 
    bool ProjectionAsyncDebug::IsAsyncType(RtTYPE type)
    {
        auto concrete = ByRefType::Is(type) ? ByRefType::From(type)->pointedTo : type;

        // If type is not an InterfaceType it cannot be IAsyncInfo.
        if (!InterfaceType::Is(concrete))
        {
            return false;
        }

        auto typeDefType = TypeDefinitionType::From(concrete);
        HTYPE htype;
        RuntimeClassTypeInformation* typeInformation;
                    
        if (m_projectionContext->GetProjectionWriter()->TryEnsureRuntimeClassTypeExists(typeDefType, &htype, &typeInformation) &&
            typeInformation->GetThisInfo()->GetSpecialization() && 
            typeInformation->GetThisInfo()->GetSpecialization()->specializationType == specPromiseSpecialization)
        {
            return true;
        }

        return false;
    }

    // Info:        Gather information about the method call and pass it along to AsyncDebug.
    //              Note: This method assumes async debugging is enabled.
    // Parameters:  thisInfo - The ThisInfo for the this we are using to call the method, if any.
    //              signature - The method signature we should search parameters of.
    //              action - If begin, call to begin new async operations for each parameter.
    //                       If cancel, pop off the async id for each parameter.
    AsyncDebug::AsyncOperationId ProjectionAsyncDebug::InstrumentAsyncMethodCall(ThisInfo* thisInfo, RtABIMETHODSIGNATURE signature)
    {
        // This (signature->runtimeClassNameId) is usually MetadataStringIdNil for non-factory methods.
        auto runtimeClassTypeId = signature->runtimeClassNameId;

        // Get actual runtime class name from ThisInfo.
        if (thisInfo)
        {
            runtimeClassTypeId = thisInfo->GetTypeId();
        }

        return BeginAsyncMethodCall(runtimeClassTypeId, signature->nameId);
    }

    // Info:        Search through all parameters in signature for any async parameters.
    //              If we find multiple async parameters, return the last one via asyncParameter.
    // Parameters:  thisInfo - The ThisInfo for the this we are using to call the method, if any.
    //              signature - The method signature we should search parameters of.
    //              asyncParameter - The RtABIPARAMETER which is determined to be async.
    //              asyncOperationId - Id of the async operation assigned.
    //              parameterType - Search only in, out, or in/out parameters. Default Out.
    // Returns:     True if the method call was instrumented.
    bool ProjectionAsyncDebug::InstrumentAsyncMethodCallByScanningParameters(
            __in_opt ThisInfo* thisInfo,
            __in RtABIMETHODSIGNATURE signature, 
            __out RtABIPARAMETER* asyncParameter,
            __out UINT64* asyncOperationId,
            __in_opt IdentifyAsyncOperationsParameterType parameterType)
    {
        size_t outArgumentCount = signature->GetParameters()->allParameters->Count() - signature->inParameterCount;
        int asyncParametersFound = 0;

        if ((outArgumentCount == 0 && parameterType == IdentifyAsyncOperationsParameterType_Out) ||
            (signature->inParameterCount == 0 && parameterType == IdentifyAsyncOperationsParameterType_In))
        {
            return false;
        }

        // Identify IAsyncInfo parameters.
        signature->GetParameters()->allParameters->Cast<RtABIPARAMETER>()->IterateN([&](int parameterLocationIndex, RtABIPARAMETER parameter) {
            if ((parameter->isOut && (parameterType == IdentifyAsyncOperationsParameterType_Out || parameterType == IdentifyAsyncOperationsParameterType_InOut)) ||
                (parameter->isIn && (parameterType == IdentifyAsyncOperationsParameterType_In || parameterType == IdentifyAsyncOperationsParameterType_InOut)))
            {
                // Intentionally overwrite asyncParameter if we find multiple async parameters. 
                // According to spec, multiple async parameters are illegal and the retval parameter is required to be last in the parameter list.
                if (IsAsyncType(parameter->type))
                {
                    *asyncParameter = parameter;

                    asyncParametersFound++;
                }
            }
        });

        if (asyncParametersFound > 0)
        {
            // Instrument generates an id and calls down to begin tracing for the async operation.
            // Do this outside the loop above so we only instrument the method once, even if it had multiple async parameters.
            *asyncOperationId = InstrumentAsyncMethodCall(thisInfo, signature);

            return true;
        }

        return false;
    }

    AsyncDebug::AsyncOperationId ProjectionAsyncDebug::BeginAsyncMethodCall(MetadataStringId runtimeClassNameId, MetadataStringId methodNameId)
    {
        return AsyncDebug::BeginAsyncOperationForWinRTMethodCall(
            m_projectionContext->StringOfId(runtimeClassNameId),
            m_projectionContext->StringOfId(methodNameId),
            m_projectionContext->GetScriptContext());
    }
}
