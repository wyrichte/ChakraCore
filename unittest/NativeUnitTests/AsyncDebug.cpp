// Copyright (C) Microsoft. All rights reserved.
//

#include "stdafx.h"
#include "activprof.h"
#include "ScriptDirectUnitTests.h"
#include "edgescriptDirect.h"
#include "Windows.Foundation.Diagnostics.h"

HRESULT TestTraceAsyncOperationStarting(IActiveScriptDirectAsyncCausality* asyncCausality, GUID* guid, LPCWSTR operationName, UINT64* operationId, AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;
    std::stringstream buf;

    hr = asyncCausality->TraceAsyncOperationStarting(guid, operationName, logLevel, operationId);

    if (FAILED(hr))
    {
        buf << "Failed to call IActiveScriptDirectAsyncCausality::TraceAsyncOperationStarting with hr = " << hex << hr;
        Print(buf.str());
    }

    return hr;
}

HRESULT TestTraceAsyncCallbackStarting(IActiveScriptDirectAsyncCausality* asyncCausality, GUID* guid, UINT64 operationId, AsyncCausality_CallbackType workType, AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;
    std::stringstream buf;

    hr = asyncCausality->TraceAsyncCallbackStarting(guid, operationId, workType, logLevel);

    if (FAILED(hr))
    {
        buf << "Failed to call IActiveScriptDirectAsyncCausality::TraceAsyncCallbackStarting with hr = " << hex << hr;
        Print(buf.str());
    }

    return hr;
}

HRESULT TestTraceAsyncCallbackCompleted(IActiveScriptDirectAsyncCausality* asyncCausality, AsyncCausality_CallbackType workType, AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;
    std::stringstream buf;

    hr = asyncCausality->TraceAsyncCallbackCompleted(workType, logLevel);

    if (FAILED(hr))
    {
        buf << "Failed to call IActiveScriptDirectAsyncCausality::TraceAsyncCallbackCompleted with hr = " << hex << hr;
        Print(buf.str());
    }

    return hr;
}

HRESULT TestUpdateAsyncCallbackRelation(IActiveScriptDirectAsyncCausality* asyncCausality, GUID* guid, UINT64 operationId, AsyncCausality_RelationType relation, AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;
    std::stringstream buf;

    hr = asyncCausality->UpdateAsyncCallbackRelation(guid, operationId, relation, logLevel);

    if (FAILED(hr))
    {
        buf << "Failed to call IActiveScriptDirectAsyncCausality::UpdateAsyncCallbackRelation with hr = " << hex << hr;
        Print(buf.str());
    }

    return hr;
}

HRESULT TestTraceAsyncOperationCompleted(IActiveScriptDirectAsyncCausality* asyncCausality, GUID* guid, UINT64 operationId, AsyncCausality_OperationStatus status, AsyncCausality_LogLevel logLevel)
{
    HRESULT hr;
    std::stringstream buf;

    hr = asyncCausality->TraceAsyncOperationCompleted(guid, operationId, status, logLevel);

    if (FAILED(hr))
    {
        buf << "Failed to call IActiveScriptDirectAsyncCausality::TraceAsyncOperationCompleted with hr = " << hex << hr;
        Print(buf.str());
    }

    return hr;
}

HRESULT TestAsyncDebugWithScriptClosed(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test IActiveScriptDirectAsyncCausality functionality is blocked when script site is closed...");

    HRESULT hr;
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();
    CComPtr<IActiveScriptDirectAsyncCausality> asyncCausality;
    CComPtr<IActiveScript> activeScript;

    GUID testGuid = { 0x12345678, 0xe251, 0x4588, { 0xae, 0x37, 0x9d, 0x6d, 0x74, 0x8a, 0xcd, 0x49 } };
    UINT64 operationId = (UINT64)-1;
        
    IfFailGoto(activeScriptDirect->QueryInterface(__uuidof(IActiveScriptDirectAsyncCausality), (void**)&asyncCausality), error);

    if (!asyncCausality)
    {
        hr = E_FAIL;
        goto error;
    }

    IfFailGoto(TestTraceAsyncOperationStarting(asyncCausality, &testGuid, L"SomeOtherOperationName", &operationId, AsyncCausality_LogLevel_Required), error);
    
    IfFailGoto(activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript), error);

    activeScript->Close();
    
    if(TestTraceAsyncOperationStarting(asyncCausality, &testGuid, L"SimpleTestOperationName", &operationId, AsyncCausality_LogLevel_Required) != E_ACCESSDENIED)
    {
        hr = E_FAIL;
        goto error;
    }

error:
    FinishTest(myTests, "Done.", hr);

    return hr;
}

HRESULT TestBasicActiveScriptDirectAsyncCausality(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test basic IActiveScriptDirectAsyncCausality functionality...");

    HRESULT hr;
    CComPtr<IActiveScriptDirect> activeScriptDirect = myTests->GetScriptDirectNoRef();
    CComPtr<IActiveScriptDirectAsyncCausality> asyncCausality;

    GUID testGuid = { 0x12345678, 0xe251, 0x4588, { 0xae, 0x37, 0x9d, 0x6d, 0x74, 0x8a, 0xcd, 0x49 } };
    UINT64 operationId = (UINT64)-1;
    UINT64 operationId2 = (UINT64)-1;
        
    IfFailGoto(activeScriptDirect->QueryInterface(__uuidof(IActiveScriptDirectAsyncCausality), (void**)&asyncCausality), error);

    if (!asyncCausality)
    {
        hr = E_FAIL;
        goto error;
    }

    IfFailGoto(TestTraceAsyncOperationStarting(asyncCausality, &testGuid, L"SimpleTestOperationName", &operationId, AsyncCausality_LogLevel_Required), error);
    IfFailGoto(TestTraceAsyncOperationCompleted(asyncCausality, &testGuid, operationId, AsyncCausality_OperationStatus_Completed, AsyncCausality_LogLevel_Required), error);
    IfFailGoto(TestTraceAsyncCallbackStarting(asyncCausality, &testGuid, operationId, AsyncCausality_CallbackType_Completion, AsyncCausality_LogLevel_Verbose), error);
    IfFailGoto(TestTraceAsyncCallbackCompleted(asyncCausality, AsyncCausality_CallbackType_Completion, AsyncCausality_LogLevel_Verbose), error);

    IfFailGoto(TestTraceAsyncOperationStarting(asyncCausality, NULL, L"SomeOtherOperationName", &operationId, AsyncCausality_LogLevel_Required), error);
    IfFailGoto(TestTraceAsyncOperationStarting(asyncCausality, NULL, NULL, &operationId2, AsyncCausality_LogLevel_Required), error);
    IfFailGoto(TestTraceAsyncOperationCompleted(asyncCausality, NULL, operationId, AsyncCausality_OperationStatus_Error, AsyncCausality_LogLevel_Required), error);
    IfFailGoto(TestTraceAsyncCallbackStarting(asyncCausality, NULL, operationId, AsyncCausality_CallbackType_Completion, AsyncCausality_LogLevel_Verbose), error);
    IfFailGoto(TestTraceAsyncCallbackCompleted(asyncCausality, AsyncCausality_CallbackType_Completion, AsyncCausality_LogLevel_Verbose), error);
    IfFailGoto(TestTraceAsyncOperationCompleted(asyncCausality, NULL, operationId2, AsyncCausality_OperationStatus_Completed, AsyncCausality_LogLevel_Required), error);
    IfFailGoto(TestTraceAsyncCallbackStarting(asyncCausality, NULL, operationId2, AsyncCausality_CallbackType_Completion, AsyncCausality_LogLevel_Verbose), error);
    IfFailGoto(TestUpdateAsyncCallbackRelation(asyncCausality, NULL, operationId2, AsyncCausality_RelationType_AssignDelegate, AsyncCausality_LogLevel_Verbose), error);
    IfFailGoto(TestTraceAsyncCallbackCompleted(asyncCausality, AsyncCausality_CallbackType_Completion, AsyncCausality_LogLevel_Verbose), error);

error:
    FinishTest(myTests, "Done.", hr);

    return hr;
}

HRESULT TestOneEnum(UINT left, UINT right, const std::string leftName, const std::string rightName)
{
    std::stringstream buf;
    buf << "Verify enum (" << leftName << " (" << left << ") == " << rightName << " (" << right << "))";
    Print(buf.str());

    if (left != right)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT TestCausalityEnums(MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test causality enum mapping to chakra...");

    HRESULT hr;

    IfFailGoto(TestOneEnum(AsyncCausality_LogLevel_Required, Windows::Foundation::Diagnostics::CausalityTraceLevel_Required, "AsyncCausality_LogLevel_Required", "Windows::Foundation::Diagnostics::CausalityTraceLevel_Required"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_LogLevel_Important, Windows::Foundation::Diagnostics::CausalityTraceLevel_Important, "AsyncCausality_LogLevel_Important", "Windows::Foundation::Diagnostics::CausalityTraceLevel_Important"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_LogLevel_Verbose, Windows::Foundation::Diagnostics::CausalityTraceLevel_Verbose, "AsyncCausality_LogLevel_Verbose", "Windows::Foundation::Diagnostics::CausalityTraceLevel_Verbose"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_RelationType_AssignDelegate, Windows::Foundation::Diagnostics::CausalityRelation_AssignDelegate, "AsyncCausality_RelationType_AssignDelegate", "Windows::Foundation::Diagnostics::CausalityRelation_AssignDelegate"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_RelationType_Join, Windows::Foundation::Diagnostics::CausalityRelation_Join, "AsyncCausality_RelationType_Join", "Windows::Foundation::Diagnostics::CausalityRelation_Join"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_RelationType_ChooseAny, Windows::Foundation::Diagnostics::CausalityRelation_Choice, "AsyncCausality_RelationType_ChooseAny", "Windows::Foundation::Diagnostics::CausalityRelation_Choice"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_RelationType_Cancel, Windows::Foundation::Diagnostics::CausalityRelation_Cancel, "AsyncCausality_RelationType_Cancel", "Windows::Foundation::Diagnostics::CausalityRelation_Cancel"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_RelationType_Error, Windows::Foundation::Diagnostics::CausalityRelation_Error, "AsyncCausality_RelationType_Error", "Windows::Foundation::Diagnostics::CausalityRelation_Error"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_CallbackType_Completion, Windows::Foundation::Diagnostics::CausalitySynchronousWork_CompletionNotification, "AsyncCausality_CallbackType_Completion", "Windows::Foundation::Diagnostics::CausalitySynchronousWork_CompletionNotification"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_CallbackType_Progress, Windows::Foundation::Diagnostics::CausalitySynchronousWork_ProgressNotification, "AsyncCausality_CallbackType_Progress", "Windows::Foundation::Diagnostics::CausalitySynchronousWork_ProgressNotification"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_CallbackType_Execution, Windows::Foundation::Diagnostics::CausalitySynchronousWork_Execution, "AsyncCausality_CallbackType_Execution", "Windows::Foundation::Diagnostics::CausalitySynchronousWork_Execution"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_OperationStatus_Started, (UINT)Windows::Foundation::AsyncStatus::Started, "AsyncCausality_OperationStatus_Started", "Windows::Foundation::AsyncStatus::Started"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_OperationStatus_Completed, (UINT)Windows::Foundation::AsyncStatus::Completed, "AsyncCausality_OperationStatus_Completed", "Windows::Foundation::AsyncStatus::Completed"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_OperationStatus_Canceled, (UINT)Windows::Foundation::AsyncStatus::Canceled, "AsyncCausality_OperationStatus_Canceled", "Windows::Foundation::AsyncStatus::Canceled"), error);
    IfFailGoto(TestOneEnum(AsyncCausality_OperationStatus_Error, (UINT)Windows::Foundation::AsyncStatus::Error, "AsyncCausality_OperationStatus_Error", "Windows::Foundation::AsyncStatus::Error"), error);
   
error:

    FinishTest(myTests, "Done.", hr);

    return hr;
}

void RunAsyncDebugTest(MyScriptDirectTests* myTests)
{
    int flagCount = 0;
    LPWSTR* flags = NULL;
    myTests->GetFlags(&flagCount, &flags);
    LPCWSTR srcFileNameFullPath = flags[flagCount-1];

    WCHAR srcFileDrive[_MAX_DRIVE];
    WCHAR srcFileDir[_MAX_DIR];
    WCHAR srcFileName[_MAX_FNAME];
    if (srcFileNameFullPath)
    {
        _wsplitpath_s(srcFileNameFullPath, srcFileDrive, _countof(srcFileDrive), srcFileDir, _countof(srcFileDir), srcFileName, _countof(srcFileName), NULL, 0);
    }

    HRESULT hr = S_OK;

    try
    {
        if (srcFileNameFullPath && _wcsicmp(srcFileName, L"TestCausalityEnums") == 0)
        {
            hr = TestCausalityEnums(myTests);
        }
        else if(srcFileNameFullPath && _wcsicmp(srcFileName, L"TestBasicActiveScriptDirectAsyncCausality") == 0)
        {
            hr = TestBasicActiveScriptDirectAsyncCausality(myTests);
        }
        else if(srcFileNameFullPath && _wcsicmp(srcFileName, L"TestAsyncDebugWithScriptClosed") == 0)
        {
            hr = TestAsyncDebugWithScriptClosed(myTests);
        }

        if (FAILED(hr))
        {
            Print("Test failed", false);
        }
    }
    catch(std::string message)
    {
        Print(message, false);
    }
    catch(exception ex)
    {
        Print(ex.what(), false);
    }
 }
