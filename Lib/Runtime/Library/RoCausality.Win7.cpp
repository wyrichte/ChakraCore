//  Copyright (c) Microsoft Corporation. All rights reserved.
// This is a snap of //fbl_rex_dev1/com/combase/RoDebug/RoCausality.cpp#7 (some pieces removed for chakra integration)

#include "stdafx.h"
#include "RoCausality.Win7.h"

// Maps for events
PCEVENT_DESCRIPTOR CreateAsynchronousOperationEventDescriptors[3][3] =
{
    {&CreateApplicationAsynchronousOperation_Required,  &CreateLibraryAsynchronousOperation_Required,  &CreateSystemAsynchronousOperation_Required},
    {&CreateApplicationAsynchronousOperation_Important, &CreateLibraryAsynchronousOperation_Important, &CreateSystemAsynchronousOperation_Important},
    {&CreateApplicationAsynchronousOperation_Verbose,   &CreateLibraryAsynchronousOperation_Verbose,   &CreateSystemAsynchronousOperation_Verbose  },
};

PCEVENT_DESCRIPTOR CompleteAsynchronousOperationEventDescriptors[3][3] =
{
    {&CompleteApplicationAsynchronousOperation_Required,  &CompleteLibraryAsynchronousOperation_Required,  &CompleteSystemAsynchronousOperation_Required},
    {&CompleteApplicationAsynchronousOperation_Important, &CompleteLibraryAsynchronousOperation_Important, &CompleteSystemAsynchronousOperation_Important},
    {&CompleteApplicationAsynchronousOperation_Verbose,   &CompleteLibraryAsynchronousOperation_Verbose,   &CompleteSystemAsynchronousOperation_Verbose  },
};

PCEVENT_DESCRIPTOR RelateAsynchronousOperationEventDescriptors[3][3] =
{
    {&RelateApplicationAsynchronousOperation_Required,  &RelateLibraryAsynchronousOperation_Required,  &RelateSystemAsynchronousOperation_Required},
    {&RelateApplicationAsynchronousOperation_Important, &RelateLibraryAsynchronousOperation_Important, &RelateSystemAsynchronousOperation_Important},
    {&RelateApplicationAsynchronousOperation_Verbose,   &RelateLibraryAsynchronousOperation_Verbose,   &RelateSystemAsynchronousOperation_Verbose  },
};

PCEVENT_DESCRIPTOR StartSynchronousWorkItemEventDescriptors[3][3] =
{
    {&StartApplicationSynchronousWorkItem_Required,  &StartLibrarySynchronousWorkItem_Required,  &StartSystemSynchronousWorkItem_Required},
    {&StartApplicationSynchronousWorkItem_Important, &StartLibrarySynchronousWorkItem_Important, &StartSystemSynchronousWorkItem_Important},
    {&StartApplicationSynchronousWorkItem_Verbose,   &StartLibrarySynchronousWorkItem_Verbose,   &StartSystemSynchronousWorkItem_Verbose  },
};

PCEVENT_DESCRIPTOR StartSynchronousOperationWorkItemEventDescriptors[3][3] =
{
    {&StartApplicationSynchronousOperationWorkItem_Required,  &StartLibrarySynchronousOperationWorkItem_Required,  &StartSystemSynchronousOperationWorkItem_Required},
    {&StartApplicationSynchronousOperationWorkItem_Important, &StartLibrarySynchronousOperationWorkItem_Important, &StartSystemSynchronousOperationWorkItem_Important},
    {&StartApplicationSynchronousOperationWorkItem_Verbose,   &StartLibrarySynchronousOperationWorkItem_Verbose,   &StartSystemSynchronousOperationWorkItem_Verbose  },
};

PCEVENT_DESCRIPTOR CompleteSynchronousWorkItemEventDescriptors[3][3] =
{
    {&CompleteApplicationSynchronousWorkItem_Required,  &CompleteLibrarySynchronousWorkItem_Required,  &CompleteSystemSynchronousWorkItem_Required},
    {&CompleteApplicationSynchronousWorkItem_Important, &CompleteLibrarySynchronousWorkItem_Important, &CompleteSystemSynchronousWorkItem_Important},
    {&CompleteApplicationSynchronousWorkItem_Verbose,   &CompleteLibrarySynchronousWorkItem_Verbose,   &CompleteSystemSynchronousWorkItem_Verbose  },
};

PCEVENT_DESCRIPTOR CompleteSynchronousOperationWorkItemEventDescriptors[3][3] =
{
    {&CompleteApplicationSynchronousOperationWorkItem_Required,  &CompleteLibrarySynchronousOperationWorkItem_Required,  &CompleteSystemSynchronousOperationWorkItem_Required},
    {&CompleteApplicationSynchronousOperationWorkItem_Important, &CompleteLibrarySynchronousOperationWorkItem_Important, &CompleteSystemSynchronousOperationWorkItem_Important},
    {&CompleteApplicationSynchronousOperationWorkItem_Verbose,   &CompleteLibrarySynchronousOperationWorkItem_Verbose,   &CompleteSystemSynchronousOperationWorkItem_Verbose  },
};

// Storage for log levels and sources as to whether we should log or not. This is set when we get the enable/disable callback from ETW. 
boolean gCausalityShouldLog[3][3] = {0};

FORCEINLINE HRESULT EnsureCausalityProviderRegistered()
{
    return HRESULT_FROM_WIN32(EventRegisterMicrosoft_Windows_AsynchronousCausality());
}

HRESULT UnregisterCausalityProvider()
{
    return HRESULT_FROM_WIN32(EventUnregisterMicrosoft_Windows_AsynchronousCausality());
}

FORCEINLINE HRESULT VerifyCommonParameters(
    _In_ CausalityTraceLevel traceLevel, 
    _In_ CausalitySource source)
{
    // Verify traceLevel
    if (traceLevel < CausalityTraceLevel_Required || traceLevel > CausalityTraceLevel_Verbose)
    {
        return E_INVALIDARG;
    }

    // Verify source are only set to the publicly available flags
    if (source < CausalitySource_Application || source > CausalitySource_System)
    {
        return E_INVALIDARG;
    }
    return S_OK;
}

FORCEINLINE BOOL ShouldLog(
    _In_ CausalityTraceLevel traceLevel, 
    _In_ CausalitySource source,
    _In_ UINT64 keywords)
{
    // If the provider is disabled we don't log anything
    if (!ASYNCHRONOUS_CAUSALITY_PROVIDER_Context.IsEnabled)
    {
        return FALSE;
    }
    // Check if the source is enabled
    if (gCausalityShouldLog[traceLevel][source])
    {
        return TRUE;
    }
    // Otherwise check if the message type is enabled
    else if ((traceLevel < ASYNCHRONOUS_CAUSALITY_PROVIDER_Context.Level) || (traceLevel == 0))
    {
        if ((keywords | ASYNCHRONOUS_CAUSALITY_PROVIDER_Context.MatchAnyKeyword) > 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

FORCEINLINE GUID GetLogicalThreadId()
{
    GUID logicalThreadId;
    if (FAILED(::CoGetCurrentLogicalThreadId(&logicalThreadId)))
    {
        // We want to still log even if we are not on a COM thread
        logicalThreadId = GUID_NULL;
    }
    return logicalThreadId;
}

template <typename T1, typename T2, typename T3, typename T4>
void InitializeEventDescriptors(PEVENT_DATA_DESCRIPTOR descriptor, T1 *data1, T2 *data2, T3 *data3, T4 *data4)
{
    EventDataDescCreate(&descriptor[0], data1, sizeof(T1));
    EventDataDescCreate(&descriptor[1], data2, sizeof(T2));
    EventDataDescCreate(&descriptor[2], data3, sizeof(T3));
    EventDataDescCreate(&descriptor[3], data4, sizeof(T4));
}

template <typename T1, typename T2, typename T3>
void InitializeEventDescriptors(PEVENT_DATA_DESCRIPTOR descriptor, T1 *data1, T2 *data2, T3 *data3, PCWSTR string)
{
    EventDataDescCreate(&descriptor[0], data1, sizeof(T1));
    EventDataDescCreate(&descriptor[1], data2, sizeof(T2));
    EventDataDescCreate(&descriptor[3], data3, sizeof(T3));

    // This specialization is only used by the creation events which expect the string in this slot
    EventDataDescCreate(&descriptor[2], 
                        (string != nullptr) ? string : L"NULL",
                        (string != nullptr) ? (ULONG)((wcslen(string) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));
}

template <typename T1, typename T2, typename T3>
void InitializeEventDescriptors(PEVENT_DATA_DESCRIPTOR descriptor, T1 *data1, T2 *data2, T3 *data3)
{
    EventDataDescCreate(&descriptor[0], data1, sizeof(T1));
    EventDataDescCreate(&descriptor[1], data2, sizeof(T2));
    EventDataDescCreate(&descriptor[2], data3, sizeof(T3));
}

FORCEINLINE HRESULT WriteEventPayload(
    _In_     CausalityTraceLevel       traceLevel,
    _In_     CausalitySource           source,
    _In_     ULONG                     countEventData, 
    _In_     PEVENT_DATA_DESCRIPTOR    eventData, 
    _In_     PCEVENT_DESCRIPTOR        eventDescriptor)
{
    return HRESULT_FROM_WIN32(::EventWrite(Microsoft_Windows_AsynchronousCausalityHandle, eventDescriptor, countEventData, eventData));
}

HRESULT RoCausalityTraceAsyncOperationCreation(
    _In_     CausalityTraceLevel     traceLevel,
    _In_     CausalitySource         source,
    _In_     GUID                    platformId,
    _In_     UINT64                  operationId, 
    _In_opt_ PCWSTR                  operationName,
    _In_opt_ UINT64                  relatedId)
{
    HRESULT hr = EnsureCausalityProviderRegistered();
    if (SUCCEEDED(hr))
    {
        hr = VerifyCommonParameters(traceLevel, source);
    }
    if (SUCCEEDED(hr) && ShouldLog(traceLevel, source, ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_AsynchronousOperation))
    {
        // Create the event data payload
        const ULONG countEventData = 4; 
        EVENT_DATA_DESCRIPTOR eventData[countEventData];
        InitializeEventDescriptors(eventData, &platformId, &operationId, &relatedId, operationName);
        // Write the event
        hr = WriteEventPayload(traceLevel, source, countEventData, eventData, CreateAsynchronousOperationEventDescriptors[traceLevel][source]);
    }
    return hr;
}

HRESULT RoCausalityTraceAsyncOperationCompletion(
    _In_     CausalityTraceLevel       traceLevel,
    _In_     CausalitySource           source,
    _In_     GUID                      platformId,
    _In_     UINT64                    operationId,
    _In_     AsyncStatus               completionStatus)
{
    HRESULT hr = EnsureCausalityProviderRegistered();
    if (SUCCEEDED(hr))
    {
        hr = VerifyCommonParameters(traceLevel, source);
        if (SUCCEEDED(hr) && (completionStatus < AsyncStatus::Completed|| completionStatus > AsyncStatus::Error))
        {
            hr = E_INVALIDARG;
        }
    }
    if (SUCCEEDED(hr) && ShouldLog(traceLevel, source, ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_AsynchronousOperation))
    {
        // Create the event data payload
        const ULONG countEventData = 3; 
        EVENT_DATA_DESCRIPTOR eventData[countEventData];
        // Need to cast to a UINT8 type to match etw manifest
        UINT8 completionType = static_cast<UINT8>(completionStatus);
        InitializeEventDescriptors(eventData, &platformId, &operationId, &completionType);
        // Write the event
        hr = WriteEventPayload(traceLevel, source, countEventData, eventData, CompleteAsynchronousOperationEventDescriptors[traceLevel][source]);
    }
    return hr;
}

HRESULT RoCausalityTraceAsyncOperationRelation(
    _In_     CausalityTraceLevel     traceLevel,
    _In_     CausalitySource         source,
    _In_     GUID                    platformId,
    _In_     UINT64                  operationId,
    _In_     CausalityRelation       relation)
{
    HRESULT hr = EnsureCausalityProviderRegistered();
    if (SUCCEEDED(hr))
    {
        hr = VerifyCommonParameters(traceLevel, source);
        if (SUCCEEDED(hr) && (relation < CausalityRelation_AssignDelegate || relation > CausalityRelation_Error))
        {
            hr = E_INVALIDARG;
        }
    }
    if (SUCCEEDED(hr) && ShouldLog(traceLevel, source, ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_Relation))
    {
        // Create the event data payload
        const ULONG countEventData = 3; 
        EVENT_DATA_DESCRIPTOR eventData[countEventData];
        // Need to cast to a UINT8 type to match etw manifest
        UINT8 relationType = static_cast<UINT8>(relation);
        InitializeEventDescriptors(eventData, &platformId, &operationId, &relationType);
        // Write the event
        hr = WriteEventPayload(traceLevel, source, countEventData, eventData, RelateAsynchronousOperationEventDescriptors[traceLevel][source]);
    }
    return hr;
}

HRESULT RoCausalityTraceSynchronousWorkItemStart(
    _In_     CausalityTraceLevel      traceLevel,
    _In_     CausalitySource          source,
    _In_     GUID                     platformId,
    _In_     UINT64                   operationId,
    _In_     CausalitySynchronousWork work)
{
    HRESULT hr = EnsureCausalityProviderRegistered();
    if (SUCCEEDED(hr))
    {
        hr = VerifyCommonParameters(traceLevel, source);
        if (SUCCEEDED(hr) && (work < CausalitySynchronousWork_CompletionNotification || work > CausalitySynchronousWork_Execution))
        {
            hr = E_INVALIDARG;
        }
    }
    // Is the synchronous work item "internal" to the operation?
    // We handle the keywords differently in this case.
    if (work == CausalitySynchronousWork_Execution)
    {
        if (SUCCEEDED(hr) && ShouldLog(traceLevel, source, ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_OperationWork))
        {
            // Create the event data payload
            const ULONG countEventData = 3; 
            EVENT_DATA_DESCRIPTOR eventData[countEventData];
            // Need to cast to a UINT8 type to match etw manifest
            UINT8 workType = static_cast<UINT8>(work);
            InitializeEventDescriptors(eventData, &platformId, &operationId, &workType);
            // Write the event
            hr = WriteEventPayload(traceLevel, source, countEventData, eventData, StartSynchronousOperationWorkItemEventDescriptors[traceLevel][source]);
        }
    }
    else
    {
        if (SUCCEEDED(hr) && ShouldLog(traceLevel, source, ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_SynchronousWorkItem))
        {
            // Create the event data payload
            const ULONG countEventData = 3; 
            EVENT_DATA_DESCRIPTOR eventData[countEventData];
            // Need to cast to a UINT8 type to match etw manifest
            UINT8 workType = static_cast<UINT8>(work);
            InitializeEventDescriptors(eventData, &platformId, &operationId, &workType);
            // Write the event
            hr = WriteEventPayload(traceLevel, source, countEventData, eventData, StartSynchronousWorkItemEventDescriptors[traceLevel][source]);
        }
    }
    return hr;
}

HRESULT RoCausalityTraceSynchronousWorkItemCompletion(
    _In_     CausalityTraceLevel      traceLevel,
    _In_     CausalitySource          source,
    _In_     CausalitySynchronousWork work)
{
    HRESULT hr = EnsureCausalityProviderRegistered();
    if (SUCCEEDED(hr))
    {
        hr = VerifyCommonParameters(traceLevel, source);
        if (SUCCEEDED(hr) && (work < CausalitySynchronousWork_CompletionNotification || work > CausalitySynchronousWork_Execution))
        {
            hr = E_INVALIDARG;
        }
    }
    // Is the synchronous work item "internal" to the operation?
    // We handle the keywords differently in this case.
    if (work == CausalitySynchronousWork_Execution)
    {
        if (SUCCEEDED(hr) && ShouldLog(traceLevel, source, ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_OperationWork))
        {
            // Write the event
            hr = WriteEventPayload(traceLevel, source, 0, nullptr, CompleteSynchronousOperationWorkItemEventDescriptors[traceLevel][source]);
        }
    }
    else
    {
        if (SUCCEEDED(hr) && ShouldLog(traceLevel, source, ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_SynchronousWorkItem))
        {
            // Write the event
            hr = WriteEventPayload(traceLevel, source, 0, nullptr, CompleteSynchronousWorkItemEventDescriptors[traceLevel][source]);
        }
    }
    return hr;
}
