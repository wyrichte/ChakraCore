/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "WscriptFastDom.h"
#include "Jscript9Interface.h"
#include "hostsysinfo.h"

#include <initguid.h>
#include <guids.h>
#include <fcntl.h>

std::multimap<Var, IJsHostScriptSite*> scriptEngineMap;

MessageQueue *WScriptFastDom::s_messageQueue = NULL;
JsHostActiveScriptSite* WScriptFastDom::s_mainScriptSite = nullptr;
NotifyCallback WScriptFastDom::s_keepaliveCallback = nullptr;
bool WScriptFastDom::s_enableEditTest = false;
bool WScriptFastDom::s_stdInAtEOF = false;
unsigned int MessageBase::s_messageCount = 0;

struct EngineThreadLocalData
{
    EngineThreadLocalData()
        :threadData(nullptr)
    {
    }

    ~EngineThreadLocalData()
    {
        if (threadData)
        {
            delete threadData;
            threadData = nullptr;
        }
    }
    EngineThreadData* threadData;
};

EngineThreadData::EngineThreadData(HANDLE readyEvent, HANDLE terminateHandle)
    :readyEvent(readyEvent), terminateHandle(terminateHandle)
{
    this->parent = GetEngineThreadData();
    InitializeCriticalSection(&this->csReportQ);
    leaving = false;
}

EngineThreadData::~EngineThreadData()
{
    DeleteCriticalSection(&this->csReportQ);
}

thread_local EngineThreadLocalData threadLocalData;

EngineThreadData* GetEngineThreadData()
{
    return threadLocalData.threadData;
}
void SetEngineThreadData(EngineThreadData* threadData)
{
    Assert(threadLocalData.threadData == nullptr);
    threadLocalData.threadData = threadData;
}

Var WScriptFastDom::Echo(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];

    return EchoToStream(stdout, /* newLine */ true, function, callInfo.Count, args);
}

Var WScriptFastDom::DispatchDOMMutationBreakpoint(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = E_FAIL;
    IActiveScriptDirect * activeScriptDirect = nullptr;
    hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (FAILED(hr))
    {
        return nullptr;
    }

    hr = activeScriptDirect->TriggerDOMMutationBreakpoint();
    CheckRecordedException(activeScriptDirect, hr);

    if (activeScriptDirect != nullptr)
    {
        activeScriptDirect->Release();
    }

    return nullptr;
}

Var WScriptFastDom::StdErrWriteLine(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];

    return EchoToStream(stderr, /* newLine */ true, function, callInfo.Count, args);
}

Var WScriptFastDom::StdErrWrite(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];

    return EchoToStream(stderr, /* newLine */ false, function, callInfo.Count, args);
}

Var WScriptFastDom::StdOutWriteLine(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];

    return EchoToStream(stdout, /* newLine */ true, function, callInfo.Count, args);
}

Var WScriptFastDom::StdOutWrite(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];

    return EchoToStream(stdout, /* newLine */ false, function, callInfo.Count, args);
}

Var WScriptFastDom::StdInReadLine(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr;
    Var result = nullptr;
    IActiveScriptDirect * activeScriptDirect = NULL;
    IJavascriptOperations * operations = NULL;
    bool readFailed = false;
    char *buf = NULL;
    IfFailedGo(JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect));
    IfFailedGo(activeScriptDirect->GetJavascriptOperations(&operations));

    buf = new char[StdInMaxLineLength];

    if (fgets(buf, StdInMaxLineLength, stdin) == nullptr)
    {
        if (s_stdInAtEOF)
        {
            // The stream is already known to be at EOF.  Raise an exception.
            readFailed = true;
        }
        else
        {
            if (feof(stdin))
            {
                // Return an empty string.
                s_stdInAtEOF = true;
                buf[0] = '\0';
            }
            else
            {
                // The read failed, but not because of EOF.  Raise an exception.
                readFailed = true;
            }
        }

    }
    else
    {
        // Strip off the terminating newline.
        char *ptr = strrchr(buf, '\n');
        if (ptr != nullptr)
        {
            *ptr = '\0';
        }
    }

    if (!readFailed)
    {
        // The string we read is UTF8, and needs to be converted.
        bool failed = false;
        char16 *wbuf = new char16[StdInMaxLineLength];
        wbuf[0] = _u('\0');
        size_t bufLen = strlen(buf);
        hr = S_OK;

        // bufLen will never be trucated as it is guarded by StdInMaxLineLength above.
        DWORD retVal = 0;
        if (bufLen > 0)
        {
            retVal = MultiByteToWideChar(CP_UTF8, 0, buf, static_cast<int>(bufLen + 1 /*for null terminator*/), wbuf, StdInMaxLineLength);
        }

        if (bufLen > 0 && retVal == 0)
        {
            failed = true;
            printf("ERR: %d\n", GetLastError());
        }
        else
        {
            hr = activeScriptDirect->StringToVar(wbuf, static_cast<int>(wcslen(wbuf)), &result);
        }

        delete[] wbuf;

        if (FAILED(hr))
        {
            IfFailedGo(hr);
        }
        else if (failed)
        {
            IfFailedGo(E_FAIL);
        }
    }

LReturn:

    if (buf != nullptr)
    {
        delete[] buf;
    }

    if (readFailed)
    {
        Var errorObject = NULL;
        hr = activeScriptDirect->CreateErrorObject(JavascriptError, hr, _u("Read from stdin failed."), &errorObject);
        if (SUCCEEDED(hr))
        {
            operations->Release();
            activeScriptDirect->Release();
            operations->ThrowException(activeScriptDirect, errorObject, FALSE);
        }
    }

    if (operations != nullptr)
    {
        operations->Release();
    }

    CheckRecordedException(activeScriptDirect, hr);
    if (activeScriptDirect != nullptr)
    {
        activeScriptDirect->Release();
    }

    return result;
}

Var WScriptFastDom::StdInEOF(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;
    IActiveScriptDirect * activeScriptDirect = nullptr;
    Var result = nullptr;
    IfFailedGo(JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect));

    if (feof(stdin) != 0)
    {
        s_stdInAtEOF = true;
    }

    IfFailedGo(activeScriptDirect->BOOLToVar(s_stdInAtEOF, &result));

LReturn:

    if (activeScriptDirect != nullptr)
    {
        activeScriptDirect->Release();
    }

    return result;
}


Var WScriptFastDom::EchoToStream(FILE * stream, bool newLine, Var function, unsigned int count, Var * args)
{
    HRESULT hr = S_OK;

    IActiveScriptDirect * activeScriptDirect = NULL;
    hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (FAILED(hr))
    {
        return NULL;
    }

    for (unsigned int i = 0; i < count - 1; i++)
    {
        BSTR argToString;
        hr = activeScriptDirect->VarToString(args[i], &argToString);
        if (SUCCEEDED(hr))
        {
            if (i > 0)
            {
                fwprintf(stream, _u(" "));
            }
            fwprintf(stream, _u("%ls"), argToString);
            SysFreeString(argToString);
        }
        else
        {
            CheckRecordedException(activeScriptDirect, hr);
        }
    }

    if (newLine)
    {
        fwprintf(stream, _u("\n"));
    }
    fflush(stream);

    Var undefined = NULL;
    activeScriptDirect->GetUndefined(&undefined);
    activeScriptDirect->Release();

    return undefined;
}
// LPCWSTR WorkingSetProc = _u("var ws = new Object(); ws.workingSet = arguments[0]; ws.maxWorkingSet = arguments[1]; ws.pageFault = arguments[2]; ws.privateUsage = arguments[3]; return ws;");  template <typename T>
HRESULT WScriptFastDom::GetWorkingSetFromActiveScript(IActiveScriptDirect* activeScriptDirect, VARIANT* varResult)
{
    HRESULT hr = NOERROR;
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX memoryCounter;
    memoryCounter.cb = sizeof(memoryCounter);

    if (!GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&memoryCounter, sizeof(memoryCounter)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    IActiveScriptGarbageCollector* gc;
    if (SUCCEEDED(activeScriptDirect->QueryInterface(IID_IActiveScriptGarbageCollector, (void**)&gc)))
    {
        gc->CollectGarbage(SCRIPTGCTYPE_NORMAL);
        gc->Release();
    }

    CComPtr<IDispatch> procDispatch;
    CComPtr<IActiveScriptParseProcedure> procedureParse;

#if _WIN64 || USE_32_OR_64_BIT
    hr = activeScriptDirect->QueryInterface(IID_IActiveScriptParseProcedure2_64, (void**)&procedureParse);
#endif
#if !_WIN64 || USE_32_OR_64_BIT
    hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScriptParseProcedure2_32), (void**)&procedureParse);
#endif
    if (SUCCEEDED(hr))
    {
        hr = procedureParse->ParseProcedureText(
            _u("var ws = new Object(); ws.workingSet = arguments[0]; ws.maxWorkingSet = arguments[1]; ws.pageFault = arguments[2]; ws.privateUsage = arguments[3]; return ws;"),
            NULL, NULL, NULL, NULL, NULL, (DWORD)(-1), 0, 0, &procDispatch);
    }

    IfFailedReturn(hr);

    VariantInit(varResult);
    CComPtr<IDispatchEx> dispEx;
    EXCEPINFO ei;
    VARIANT args[5]; // this & other properties
    IfFailedReturn(procDispatch->QueryInterface(__uuidof(IDispatchEx), (void**)&dispEx));
    memset(&ei, 0, sizeof(ei));
    DISPID dispIdNamed;
    DISPPARAMS dispParams;
    dispParams.cNamedArgs = 1;
    dispIdNamed = DISPID_THIS;
    dispParams.rgdispidNamedArgs = &dispIdNamed;
    dispParams.cArgs = 5;
    dispParams.rgvarg = args;
    args[0].vt = VT_DISPATCH;
    args[0].pdispVal = dispEx;
    args[1].vt = VT_R8;
    args[1].dblVal = (double)memoryCounter.PrivateUsage;
    args[2].vt = VT_R8;
    args[2].dblVal = (double)memoryCounter.PageFaultCount;
    args[3].vt = VT_R8;
    args[3].dblVal = (double)memoryCounter.PeakWorkingSetSize;
    args[4].vt = VT_R8;
    args[4].dblVal = (double)memoryCounter.WorkingSetSize;
    hr = dispEx->InvokeEx(0, 0x1, DISPATCH_METHOD | DISPATCH_PROPERTYGET, &dispParams, varResult, &ei, NULL);
    return hr;
}

Var WScriptFastDom::GetWorkingSet(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];

    HRESULT hr;
    IActiveScriptDirect*  activeScriptDirect = NULL;
    hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    Var instance = NULL;
    if (SUCCEEDED(hr))
    {
        VARIANT varResult;
        VariantInit(&varResult);
        if (SUCCEEDED(GetWorkingSetFromActiveScript(activeScriptDirect, &varResult)))
        {
            if (varResult.vt == VT_DISPATCH)
            {
                CComPtr<IDispatchEx> wsDispatch;
                hr = varResult.pdispVal->QueryInterface(__uuidof(IDispatchEx), (void**)&wsDispatch);
                if (SUCCEEDED(hr))
                {
                    hr = activeScriptDirect->DispExToVar(wsDispatch, &instance);
                }
                CheckRecordedException(activeScriptDirect, hr);
            }
            else
            {
                hr = E_FAIL;
            }
            VariantClear(&varResult);
        }
        activeScriptDirect->Release();
    }
    if (SUCCEEDED(hr))
    {
        return instance;
    }
    return NULL;
}

Var WScriptFastDom::Quit(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;

    int exitCode = 0;

    if (callInfo.Count > 1)
    {
        Var* arg = &args[1];

        IActiveScriptDirect * activeScriptDirect = NULL;
        hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
        if (SUCCEEDED(hr))
        {
            hr = activeScriptDirect->VarToInt(*arg, &exitCode);
            if (FAILED(hr))
            {
                exitCode = 0;
                CheckRecordedException(activeScriptDirect, hr);
            }
        }
        activeScriptDirect->Release();
    }

    ExitProcess(exitCode);
}

Var WScriptFastDom::QuitHtmlHost(Var function, CallInfo callInfo, Var* args)
{
    // Post a custom quit message to delay shutdown gracefully. This is invoked from WScript.Quit,
    // thus we are in the middle of some dispatch call. If we cleanup right now, we may null out
    // some objects unexpected by the dispatch call path (e.g. HostDispatch::scriptSite).
    PostThreadMessage(GetCurrentThreadId(), WM_USER_QUIT, (WPARAM)0, (LPARAM)0);
    return nullptr;
}


bool WScriptFastDom::ParseRunInfoFromArgs(CComPtr<IActiveScriptDirect> activeScriptDirect, CallInfo callInfo, Var* args, RunInfo& runInfo, bool isSourceRaw)
{
    runInfo.hr = S_OK;
    runInfo.errorMessage = _u("");

    /*
        Arguments:
        1. Source string - Either the path to the source file or the actual source code, depending on the API used.
        2. Context string - Indicates whether to run in self (same engine as the current one) or samethread (run on a new engine instance) or crossthread context. Self is default.
        3. diagnostic string, if needed.
        4. true or false to indicate whether the egnine is primary or not.
    */
    if (callInfo.Count < 2 || callInfo.Count > 6)
    {
        runInfo.hr = E_INVALIDARG;
        runInfo.errorMessage = _u("Too many or too few arguments.");

        return false;
    }

    if (isSourceRaw)
    {
        unsigned int length;
        runInfo.hr = activeScriptDirect->VarToRawString(args[0], &runInfo.source, &length);
    }
    else
    {
        BSTR content;
        runInfo.hr = activeScriptDirect->VarToString(args[0], &content);
        runInfo.source = content ? content : _u("");
    }

    if (!SUCCEEDED(runInfo.hr))
    {
        runInfo.errorMessage = _u("Failed while reading the source");
        return false;
    }

    if (callInfo.Count > 2)
    {
        if (callInfo.Count > 3)
        {
            if (callInfo.Count > 4)
            {
                if (callInfo.Count > 5)
                {
                    int domainId;
                    runInfo.hr = activeScriptDirect->VarToInt(args[4], &domainId);
                    if (SUCCEEDED(runInfo.hr))
                    {
                        runInfo.domainId = (WORD)domainId;
                    }
                    else
                    {
                        runInfo.errorMessage = _u("Failed while reading the fifth arg (domainId)");
                        return false;
                    }
                }

                BOOL isPrimary;
                runInfo.hr = activeScriptDirect->VarToBOOL(args[3], &isPrimary);
                if (SUCCEEDED(runInfo.hr))
                {
                    runInfo.isPrimary = !!isPrimary;
                }
                else
                {
                    runInfo.errorMessage = _u("Failed while reading the fourth arg (primary flag)");
                    return false;
                }
            }

            CComBSTR diagnostics;
            runInfo.hr = activeScriptDirect->VarToString(args[2], &diagnostics);
            if (SUCCEEDED(runInfo.hr))
            {
                if (wcscmp(diagnostics, _u("diagnostics")) == 0)
                {
                    runInfo.isDiagnosticHost = true;
                }
            }
            else
            {
                runInfo.errorMessage = _u("Failed while reading the third arg (diagnostics flag)");
                return false;
            }
        }

        CComBSTR context;
        runInfo.hr = activeScriptDirect->VarToString(args[1], &context);
        if (SUCCEEDED(runInfo.hr))
        {
            runInfo.SetContext(context);
        }
        else
        {
            runInfo.errorMessage = _u("Failed while reading the second arg (context flag)");
            return false;
        }
    }

    return true;
}

Var WScriptFastDom::LoadTextFile(Var function, CallInfo callInfo, Var* args)
{
    ScriptDirect scriptDirect;
    RunInfo runInfo;
    IActiveScriptDirect * activeScriptDirect = NULL;
    Var returnVar = NULL;
    runInfo.errorMessage = _u("LoadTextFile call failed.");
    runInfo.hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (FAILED(runInfo.hr))
    {
        scriptDirect.ThrowIfFailed(runInfo.hr, runInfo.errorMessage);
        return returnVar;
    }

    if (callInfo.Count < 2)
    {
        runInfo.hr = activeScriptDirect->GetUndefined(&returnVar);
        goto Cleanup;
    }

    const char16 *fileName;
    uint fileNameLength;

    runInfo.hr = activeScriptDirect->VarToRawString(args[1], &fileName, &fileNameLength);
    if (FAILED(runInfo.hr))
    {
        goto Cleanup;
    }

    UINT lengthBytes = 0;
    bool isUtf8 = false;
    LPCOLESTR contentsRaw = nullptr;
    const char16 *fileContent;
    runInfo.hr = JsHostLoadScriptFromFile(fileName, fileContent, &isUtf8, &contentsRaw, &lengthBytes);
    if (FAILED(runInfo.hr))
    {
        goto Cleanup;
    }
    activeScriptDirect->StringToVar(fileContent, lengthBytes, &returnVar);

Cleanup:
    activeScriptDirect->Release();
    scriptDirect.ThrowIfFailed(runInfo.hr, runInfo.errorMessage);
    return returnVar;
}

Var WScriptFastDom::LoadBinaryFile(Var function, CallInfo callInfo, Var* args)
{
    ScriptDirect scriptDirect;
    RunInfo runInfo;
    IActiveScriptDirect * activeScriptDirect = NULL;
    Var returnVar = NULL;
    runInfo.errorMessage = _u("LoadBinaryFile call failed.");
    runInfo.hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    scriptDirect.ThrowIfFailed(runInfo.hr, runInfo.errorMessage);

    if (callInfo.Count < 2)
    {
        runInfo.hr = activeScriptDirect->GetUndefined(&returnVar);
        goto Cleanup;
    }

    const char16 *fileName;
    uint fileNameLength;

    runInfo.hr = activeScriptDirect->VarToRawString(args[1], &fileName, &fileNameLength);
    if (FAILED(runInfo.hr))
    {
        goto Cleanup;
    }

    const char16 *fileContent;
    UINT lengthBytes = 0;
    runInfo.hr = JsHostLoadBinaryFile(fileName, fileContent, lengthBytes);
    if (FAILED(runInfo.hr))
    {
        goto Cleanup;
    }

    runInfo.hr = activeScriptDirect->CreateArrayBufferFromBuffer((byte*)fileContent, lengthBytes, &returnVar);
    if (FAILED(runInfo.hr))
    {
        goto Cleanup;
    }

Cleanup:
    activeScriptDirect->Release();
    scriptDirect.ThrowIfFailed(runInfo.hr, runInfo.errorMessage);
    return returnVar;
}

Var WScriptFastDom::Flag(Var function, CallInfo callInfo, Var* args)
{
    ScriptDirect scriptDirect;
    RunInfo runInfo;
    IActiveScriptDirect * activeScriptDirect = NULL;
    Var returnVar = NULL;
    runInfo.errorMessage = _u("Flag call failed.");
    runInfo.hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    scriptDirect.ThrowIfFailed(runInfo.hr, runInfo.errorMessage);
    runInfo.hr = activeScriptDirect->GetUndefined(&returnVar);

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (callInfo.Count > 1)
    {
        const char16 *cmd;
        uint cmdLength;

        runInfo.hr = activeScriptDirect->VarToRawString(args[1], &cmd, &cmdLength);
        if (FAILED(runInfo.hr))
        {
            goto Cleanup;
        }

        const char16* argv[] = { nullptr, cmd };
        JScript9Interface::SetConfigFlags(2, (char16**)argv, nullptr);
    }
#endif

Cleanup:
    activeScriptDirect->Release();
    scriptDirect.ThrowIfFailed(runInfo.hr, runInfo.errorMessage);
    return returnVar;
}

Var WScriptFastDom::LoadScriptFile(Var function, CallInfo callInfo, Var* args)
{
    RunInfo runInfo;
    CComPtr<IActiveScriptDirect> activeScriptDirect = NULL;
    CComPtr<IJavascriptOperations>  operations = NULL;
    runInfo.hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (SUCCEEDED(runInfo.hr))
    {
        runInfo.hr = activeScriptDirect->GetJavascriptOperations(&operations);
    }
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }

    Var returnValue = NULL;
    runInfo.hr = activeScriptDirect->GetUndefined(&returnValue);
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }
    args = &args[1];

    if (ParseRunInfoFromArgs(activeScriptDirect, callInfo, args, runInfo))
    {
        if (runInfo.context == RunInfo::ContextType::self)
        {
            CComPtr<IActiveScript> activeScript;
            runInfo.hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript);
            if (SUCCEEDED(runInfo.hr))
            {
                IJsHostScriptSite * jsHostScriptSite;
                runInfo.hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
                if (SUCCEEDED(runInfo.hr))
                {
                    runInfo.hr = jsHostScriptSite->LoadScriptFile(runInfo.source);
                    if (SUCCEEDED(runInfo.hr))
                    {
                        runInfo.hr = activeScriptDirect->GetGlobalObject(&returnValue);
                        if (SUCCEEDED(runInfo.hr))
                        {
                            runInfo.hr = AddToScriptEngineMapNoThrow(returnValue, jsHostScriptSite);
                        }
                    }
                    jsHostScriptSite->Release();
                }
            }
        }
        else if (runInfo.context == RunInfo::ContextType::sameThread)
        {
            JsHostActiveScriptSite * scriptSite;
            runInfo.hr = CreateNewEngine(GetCurrentThread(), &scriptSite, true, runInfo.isDiagnosticHost, runInfo.isPrimary /* not primary by default */, runInfo.domainId);
            if (SUCCEEDED(runInfo.hr))
            {
                runInfo.hr = scriptSite->LoadScriptFile(runInfo.source);
                if (SUCCEEDED(runInfo.hr))
                {
                    CComPtr<IActiveScript> newActiveScript = NULL;
                    runInfo.hr = scriptSite->GetActiveScript(&newActiveScript);
                    if (SUCCEEDED(runInfo.hr))
                    {
                        IActiveScriptDirect * newActiveScriptDirect = NULL;
                        runInfo.hr = newActiveScript->QueryInterface(__uuidof(IActiveScriptDirect), (void**)&newActiveScriptDirect);
                        if (SUCCEEDED(runInfo.hr))
                        {
                            runInfo.hr = newActiveScriptDirect->GetGlobalObject(&returnValue);
                            newActiveScriptDirect->Release();
                            if (SUCCEEDED(runInfo.hr))
                            {
                                IJsHostScriptSite * jsHostScriptSite;
                                runInfo.hr = newActiveScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
                                if (SUCCEEDED(runInfo.hr))
                                {
                                    runInfo.hr = AddToScriptEngineMapNoThrow(returnValue, jsHostScriptSite);
                                    jsHostScriptSite->Release();
                                }
                            }
                        }
                    }
                }
                scriptSite->Release();
            }
        }
        else if (runInfo.context == RunInfo::ContextType::crossThread)
        {
            if (HostSystemInfo::SupportsOnlyMultiThreadedCOM())
            {
                Assert(FALSE); // crossthread scenario is not supported for WP8.
                runInfo.hr = E_INVALIDARG;
            }
            else
            {
                HANDLE newThread;
                runInfo.hr = CreateEngineThread(&newThread);
                if (SUCCEEDED(runInfo.hr))
                {
                    JsHostActiveScriptSite * scriptSite;
                    runInfo.hr = CreateNewEngine(newThread, &scriptSite, true, runInfo.isDiagnosticHost, runInfo.isPrimary /* not primary */, runInfo.domainId);
                    if (SUCCEEDED(runInfo.hr))
                    {
                        runInfo.hr = scriptSite->LoadScriptFile(runInfo.source);
                        if (SUCCEEDED(runInfo.hr))
                        {
                            CComPtr<IDispatchEx> globalObjectDispatchEx = nullptr;
                            runInfo.hr = scriptSite->GetGlobalObjectDispatchEx(&globalObjectDispatchEx);
                            if (SUCCEEDED(runInfo.hr))
                            {
                                runInfo.hr = activeScriptDirect->DispExToVar(globalObjectDispatchEx, &returnValue);
                                if (SUCCEEDED(runInfo.hr))
                                {
                                    CComPtr<IActiveScript> newActiveScript = NULL;
                                    runInfo.hr = scriptSite->GetActiveScript(&newActiveScript);
                                    if (SUCCEEDED(runInfo.hr))
                                    {
                                        CComPtr<IJsHostScriptSite> jsHostScriptSite;
                                        runInfo.hr = newActiveScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
                                        if (SUCCEEDED(runInfo.hr))
                                        {
                                            runInfo.hr = AddToScriptEngineMapNoThrow(returnValue, jsHostScriptSite);
                                        }
                                    }
                                }
                                CheckRecordedException(activeScriptDirect, runInfo.hr, false);
                            }
                        }
                        scriptSite->Release();
                    }
                    CloseHandle(newThread);
                }
            }
        }
        else  // from  module
        {
            CComPtr<IActiveScript> activeScript;
            runInfo.hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript);
            if (SUCCEEDED(runInfo.hr))
            {
                CComPtr<IJsHostScriptSite> jsHostScriptSite;
                Var errorObject = nullptr;
                runInfo.hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
                if (SUCCEEDED(runInfo.hr))
                {
                    runInfo.hr = jsHostScriptSite->LoadModuleFile(runInfo.source, FALSE, (byte**)&errorObject, (DWORD_PTR)nullptr);
                }
                CheckRecordedException(activeScriptDirect, runInfo.hr, false);
                if (FAILED(runInfo.hr) && errorObject != nullptr)
                {
                    runInfo.hr = operations->ThrowException(activeScriptDirect, errorObject, FALSE);
                }
            }
        }
    }

    CheckRecordedException(activeScriptDirect, runInfo.hr, false);
    if (FAILED(runInfo.hr))
    {
        Var errorObject = nullptr;
        activeScriptDirect->CreateErrorObject(JsErrorType::JavascriptError, runInfo.hr, runInfo.errorMessage, &errorObject);
        operations->ThrowException(activeScriptDirect, errorObject, FALSE);
    }

    return returnValue;
}

Var WScriptFastDom::RegisterModuleSource(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;

    ScriptDirect scriptDirect;
    CComPtr<IActiveScriptDirect> activeScriptDirect = nullptr;
    CComPtr<IActiveScript> activeScript = nullptr;
    CComPtr<IJsHostScriptSite> jsHostScriptSite = nullptr;
    CComBSTR moduleIdentiferBstr;
    CComBSTR moduleSource;
    Var returnValue = nullptr;

    IfFailGo(scriptDirect.From(function));
    returnValue = scriptDirect.GetUndefined();

    IfFailGo(JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect));
    IfFailGo(activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript));
    IfFailGo(activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite));

    if (callInfo.Count < 3)
    {
        scriptDirect.ThrowIfFailed(E_INVALIDARG, _u("Too few arguments."));
        return returnValue;
    }

    IfFailGo(activeScriptDirect->VarToString(args[1], &moduleIdentiferBstr));
    IfFailGo(activeScriptDirect->VarToString(args[2], &moduleSource));

    hr = jsHostScriptSite->RegisterModuleSource(moduleIdentiferBstr, moduleSource);

Error:
    scriptDirect.ThrowIfFailed(hr);

    return returnValue;
}

Var WScriptFastDom::LoadModule(Var function, CallInfo callInfo, Var* args)
{
    RunInfo runInfo;
    CComPtr<IActiveScriptDirect> activeScriptDirect = NULL;
    CComPtr<IJavascriptOperations> operations = NULL;
    JsHostActiveScriptSite *scriptSite = nullptr;
    runInfo.hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }
    runInfo.hr = activeScriptDirect->GetJavascriptOperations(&operations);
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }
    CComPtr<IActiveScript> activeScript;
    Var errorObject = nullptr;
    runInfo.hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript);
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }

    args = &args[1];
    Var returnValue = NULL;
    runInfo.hr = activeScriptDirect->GetUndefined(&returnValue);
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }

    CComPtr<IJsHostScriptSite> jsHostScriptSite;
    runInfo.hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
    if (SUCCEEDED(runInfo.hr))
    {
        if (ParseRunInfoFromArgs(activeScriptDirect, callInfo, args, runInfo, true))
        {
            scriptSite = (JsHostActiveScriptSite*)(IJsHostScriptSite*)jsHostScriptSite;
            if (scriptSite != nullptr)
            {
                runInfo.hr = scriptSite->LoadModule(runInfo.source, (byte**)&errorObject);
            }
        }
    }
    CheckRecordedException(activeScriptDirect, runInfo.hr, false);
    if (FAILED(runInfo.hr))
    {
        if (!errorObject)
        {
            activeScriptDirect->CreateErrorObject(JsErrorType::JavascriptError, runInfo.hr, runInfo.errorMessage, &errorObject);
        }
        operations->ThrowException(activeScriptDirect, errorObject, FALSE);
    }
    return returnValue;
}

Var WScriptFastDom::LoadScript(Var function, CallInfo callInfo, Var* args)
{
    RunInfo runInfo;
    CComPtr<IActiveScriptDirect> activeScriptDirect = NULL;
    CComPtr<IJavascriptOperations> operations = NULL;
    JsHostActiveScriptSite *scriptSite = nullptr;
    runInfo.hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (SUCCEEDED(runInfo.hr))
    {
        runInfo.hr = activeScriptDirect->GetJavascriptOperations(&operations);
    }
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }

    Var returnValue = NULL;
    runInfo.hr = activeScriptDirect->GetUndefined(&returnValue);
    if (FAILED(runInfo.hr))
    {
        return NULL;
    }
    args = &args[1];

    if (ParseRunInfoFromArgs(activeScriptDirect, callInfo, args, runInfo, true))
    {
        if (runInfo.context == RunInfo::ContextType::self)
        {
            IActiveScript * activeScript;
            runInfo.hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript);
            if (SUCCEEDED(runInfo.hr))
            {
                IJsHostScriptSite * jsHostScriptSite;
                runInfo.hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
                if (SUCCEEDED(runInfo.hr))
                {
                    scriptSite = (JsHostActiveScriptSite*)jsHostScriptSite;
                    if (scriptSite != nullptr)
                    {
                        scriptSite->delegateErrorHandling = true;
                        scriptSite->lastException = new ExceptionData();

                        runInfo.hr = scriptSite->LoadScript(runInfo.source);
                        if (SUCCEEDED(runInfo.hr))
                        {
                            runInfo.hr = activeScriptDirect->GetGlobalObject(&returnValue);
                            if (SUCCEEDED(runInfo.hr))
                            {
                                runInfo.hr = AddToScriptEngineMapNoThrow(returnValue, jsHostScriptSite);
                            }
                        }

                        scriptSite->delegateErrorHandling = false;
                    }
                }

                activeScript->Release();
            }
        }
        else if (runInfo.context == RunInfo::ContextType::sameThread)
        {
            runInfo.hr = CreateNewEngine(GetCurrentThread(), &scriptSite, true, runInfo.isDiagnosticHost, runInfo.isPrimary /* By default not primary in LoadScript */, runInfo.domainId);
            if (SUCCEEDED(runInfo.hr))
            {
                Assert(scriptSite->delegateErrorHandling == false);
                Assert(scriptSite->lastException == nullptr);
                scriptSite->delegateErrorHandling = true;
                scriptSite->lastException = new ExceptionData();

                runInfo.hr = scriptSite->LoadScript(runInfo.source);

                if (SUCCEEDED(runInfo.hr))
                {
                    IActiveScript * newActiveScript = NULL;
                    runInfo.hr = scriptSite->GetActiveScript(&newActiveScript);
                    if (SUCCEEDED(runInfo.hr))
                    {
                        IActiveScriptDirect * newActiveScriptDirect = NULL;
                        runInfo.hr = newActiveScript->QueryInterface(__uuidof(IActiveScriptDirect), (void**)&newActiveScriptDirect);
                        if (SUCCEEDED(runInfo.hr))
                        {
                            runInfo.hr = newActiveScriptDirect->GetGlobalObject(&returnValue);
                            newActiveScriptDirect->Release();
                            if (SUCCEEDED(runInfo.hr))
                            {
                                IJsHostScriptSite * jsHostScriptSite;
                                runInfo.hr = newActiveScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
                                if (SUCCEEDED(runInfo.hr))
                                {
                                    runInfo.hr = AddToScriptEngineMapNoThrow(returnValue, jsHostScriptSite);
                                    jsHostScriptSite->Release();
                                }
                            }
                        }
                        newActiveScript->Release();
                    }
                }
                scriptSite->delegateErrorHandling = false;
            }
        }
        else if (runInfo.context == RunInfo::ContextType::crossThread)
        {
            if (HostSystemInfo::SupportsOnlyMultiThreadedCOM())
            {
                Assert(FALSE); // crossthread scenario is not supported for WP8.
                runInfo.hr = E_INVALIDARG;
            }
            else
            {
                HANDLE newThread;
                runInfo.hr = CreateEngineThread(&newThread);
                if (SUCCEEDED(runInfo.hr))
                {
                    GetEngineThreadData()->childrenThreadIds.push_back(GetThreadId(newThread));

                    runInfo.hr = CreateNewEngine(newThread, &scriptSite, true, runInfo.isDiagnosticHost, runInfo.isPrimary /* not primary */, runInfo.domainId);
                    if (SUCCEEDED(runInfo.hr))
                    {
                        runInfo.hr = scriptSite->LoadScript(runInfo.source);
                        if (SUCCEEDED(runInfo.hr))
                        {
                            CComPtr<IDispatchEx> globalObjectDispatchEx = nullptr;
                            runInfo.hr = scriptSite->GetGlobalObjectDispatchEx(&globalObjectDispatchEx);
                            if (SUCCEEDED(runInfo.hr))
                            {
                                runInfo.hr = activeScriptDirect->DispExToVar(globalObjectDispatchEx, &returnValue);
                                if (SUCCEEDED(runInfo.hr))
                                {
                                    CComPtr<IActiveScript> newActiveScript = NULL;
                                    runInfo.hr = scriptSite->GetActiveScript(&newActiveScript);
                                    if (SUCCEEDED(runInfo.hr))
                                    {
                                        CComPtr<IJsHostScriptSite> jsHostScriptSite;
                                        runInfo.hr = newActiveScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
                                        if (SUCCEEDED(runInfo.hr))
                                        {
                                            runInfo.hr = AddToScriptEngineMapNoThrow(returnValue, jsHostScriptSite);
                                        }
                                    }
                                }
                                CheckRecordedException(activeScriptDirect, runInfo.hr, false);
                            }
                        }
                    }
                    CloseHandle(newThread);
                }
            }
        }
    }

    CheckRecordedException(activeScriptDirect, runInfo.hr, false);
    if (FAILED(runInfo.hr))
    {
        Var errorObject = NULL;
        if (scriptSite != nullptr && scriptSite->lastException != nullptr)
        {
            switch (scriptSite->lastException->errorType)
            {
            case JsErrorType::JavascriptParseError:
                runInfo.hr = activeScriptDirect->CreateErrorObject(JsErrorType::JavascriptSyntaxError, runInfo.hr, scriptSite->lastException->description, &errorObject);
                break;
            case JsErrorType::JavascriptTypeError:
                runInfo.hr = activeScriptDirect->CreateErrorObject(JsErrorType::JavascriptTypeError, runInfo.hr, scriptSite->lastException->description, &errorObject);
                break;
            default:
                if (scriptSite->lastException->thrownObject != nullptr)
                {
                    errorObject = scriptSite->lastException->thrownObject;
                    runInfo.hr = S_OK;
                }
                else
                {
                    runInfo.hr = activeScriptDirect->CreateErrorObject(JsErrorType::JavascriptError, runInfo.hr, scriptSite->lastException->description, &errorObject);
                }
                break;
            }
            delete scriptSite->lastException;
            scriptSite->lastException = nullptr;
            scriptSite->Release();
        }
        else
        {
            runInfo.hr = activeScriptDirect->CreateErrorObject(JavascriptError, runInfo.hr, runInfo.errorMessage, &errorObject);
        }

        if (SUCCEEDED(runInfo.hr))
        {
            runInfo.hr = operations->ThrowException(activeScriptDirect, errorObject, FALSE);
        }
    }
    else if (scriptSite != nullptr)
    {
        scriptSite->Release();
    }

    return returnValue;
}

HRESULT WScriptFastDom::AddToScriptEngineMapNoThrow(Var globalObject, IJsHostScriptSite* jsHostScriptSite)
{
    HRESULT hr = S_OK;
    AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);
    try
    {
        scriptEngineMap.insert(std::pair<Var, IJsHostScriptSite*>(globalObject, jsHostScriptSite));
    }
    catch(const exception &)
    {
        hr = E_FAIL;
    }
    jsHostScriptSite->AddRef();

    return hr;
}

void WScriptFastDom::ShutdownAll()
{
    for (auto i = scriptEngineMap.begin(); i != scriptEngineMap.end(); i++)
    {
        i->second->Release();
    }
    scriptEngineMap.clear();

    s_messageQueue = NULL;
}

Var WScriptFastDom::PerformSourceRundown(Var function, CallInfo callInfo, Var* args)
{
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
    diagnosticsHelper->m_shouldPerformSourceRundown = true;

    ::PerformSourceRundown(); // Queue a source rundown message

    return NULL;
}

Var WScriptFastDom::DebugDynamicAttach(Var function, CallInfo callInfo, Var* args)
{
    if (callInfo.Count > 1)
    {
        args = &args[1];

        FastDomDebugAttach(args[0]);
    }
    return NULL;
}

// Sets the debugger into a mode to perform dynamic detachment (no refresh detach).
Var WScriptFastDom::DebugDynamicDetach(Var function, CallInfo callInfo, Var* args)
{
    if (callInfo.Count > 1)
    {
        args = &args[1];

        FastDomDebugDetach(args[0]);
    }
    return NULL;
}

HRESULT WScriptFastDom::GetHtmlDebugFunctionHelper(Var function, CallInfo callInfo, Var* args, IDispatchEx** dispFunction)
{
    HRESULT hr = NOERROR;
    CComPtr<IActiveScriptDirect> activeScriptDirect;
    hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (FAILED(hr))
    {
        return E_FAIL;
    }
    if (callInfo.Count < 2)
    {
        return E_FAIL;
    }
    BOOL isCallable;
    hr = activeScriptDirect->IsObjectCallable(args[1], &isCallable);
    if (FAILED(hr) || !isCallable)
    {
        return E_FAIL;
    }
    hr = activeScriptDirect->VarToDispEx(args[1], dispFunction);

    return hr;
}

void WScriptFastDom::CheckRecordedException(IActiveScriptDirect* activeScript, HRESULT hr, bool release /*= true*/)
{
    if (hr == SCRIPT_E_RECORDED && activeScript)
    {
        if (!release)
        {
            // The only available API is "Release"AndRethrowException
            // So add a ref here to mimic a no release version of the API
            activeScript->AddRef();
        }
        activeScript->ReleaseAndRethrowException(hr);
    }

}

Var WScriptFastDom::HtmlPerformSourceRundown(Var function, CallInfo callInfo, Var* args)
{
    CComPtr<IDispatchEx> functionDispatch;
    HRESULT hr = GetHtmlDebugFunctionHelper(function, callInfo, args, &functionDispatch);
    if (FAILED(hr))
    {
        return nullptr;
    }
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
    CreateDebugCallbackMessage(functionDispatch, [=]()
    {
        return diagnosticsHelper->HtmlDynamicAttach(IDM_DEBUGGERDYNAMICATTACHSOURCERUNDOWN);
    });

    return nullptr;
}

Var WScriptFastDom::HtmlDebugDynamicAttach(Var function, CallInfo callInfo, Var* args)
{
    CComPtr<IDispatchEx> functionDispatch;
    HRESULT hr = GetHtmlDebugFunctionHelper(function, callInfo, args, &functionDispatch);
    if (FAILED(hr))
    {
        return nullptr;
    }
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
    CreateDebugCallbackMessage(functionDispatch, [=]()
    {
        return diagnosticsHelper->HtmlDynamicAttach(IDM_DEBUGGERDYNAMICATTACH);
    });

    return NULL;
}

Var WScriptFastDom::HtmlDebugDynamicDetach(Var function, CallInfo callInfo, Var* args)
{
    CComPtr<IDispatchEx> functionDispatch;
    HRESULT hr = GetHtmlDebugFunctionHelper(function, callInfo, args, &functionDispatch);
    if (FAILED(hr))
    {
        return nullptr;
    }
    DiagnosticsHelper *diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
    CreateDebugCallbackMessage(functionDispatch, [=]()
    {
        return diagnosticsHelper->HtmlDynamicDetach();
    });

    return NULL;
}

Var WScriptFastDom::Edit(Var function, CallInfo callInfo, Var* args)
{
    if (callInfo.Count == 3)
    {
        args = &args[1];

        HRESULT hr = S_OK;

        CComPtr<IActiveScriptDirect> activeScriptDirect;
        hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
        if (FAILED(hr))
        {
            return nullptr;
        }

        CComBSTR editLabel;
        hr = activeScriptDirect->VarToString(args[0], &editLabel);
        if (SUCCEEDED(hr))
        {
            CComPtr<IDebugDocumentText> spDebugDocumentText;
            ULONG startOffset;
            ULONG length;
            PCWSTR editContent;
            ULONG newLength;
            DiagnosticsHelper* diagnosticsHelper = DiagnosticsHelper::GetDiagnosticsHelper();
            bool editRangeAndContentFound = diagnosticsHelper->GetEditRangeAndContent(editLabel, &spDebugDocumentText, &startOffset, &length, &editContent, &newLength);
            if (editRangeAndContentFound)
            {
                FastDomEdit(args[1], spDebugDocumentText, startOffset, length, editContent, newLength);
            }
            else
            {
                // Ooops, label not found
                LPCWSTR errorMessage = _u("WScript.Edit() label not found.");
                CComPtr<IJavascriptOperations> operations = nullptr;
                hr = activeScriptDirect->GetJavascriptOperations(&operations);
                Var errorObject = nullptr;
                if (SUCCEEDED(hr))
                {
                    hr = activeScriptDirect->CreateErrorObject(JavascriptError, hr, errorMessage, &errorObject);
                    if (SUCCEEDED(hr))
                    {
                        hr = operations->ThrowException(activeScriptDirect, errorObject, FALSE);
                    }
                }

                return nullptr;
            }
        }
        else
        {
            CheckRecordedException(activeScriptDirect, hr, false);
        }

        Var undefined = nullptr;
        activeScriptDirect->GetUndefined(&undefined);
        return undefined;
    }

    // TODO: Error reporting - WScript.Edit() is called with wrong number of arguments!

    return nullptr;
}

Var WScriptFastDom::StartScriptProfiler(Var function, CallInfo callInfo, Var* args)
{
    if (callInfo.Count > 1)
    {
        args = &args[1];

        FastDomStartProfiler(args[0]);
    }
    return nullptr;
}

Var WScriptFastDom::StopScriptProfiler(Var function, CallInfo callInfo, Var* args)
{
    Var targetFunction = nullptr; // WScript.StopProfiling([optional]func)
    if (callInfo.Count > 1)
    {
        args = &args[1];
        targetFunction = args[0];
    }

    FastDomStopProfiler(targetFunction);
    return nullptr;
}

Var WScriptFastDom::InitializeProjection(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;

    IActiveScriptDirect * activeScriptDirect = NULL;
    hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (SUCCEEDED(hr))
    {
        IActiveScript * activeScript;
        hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript);
        if (SUCCEEDED(hr))
        {
            IJsHostScriptSite * jsHostScriptSite;
            hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
            if (SUCCEEDED(hr))
            {
                hr = jsHostScriptSite->InitializeProjection();
                jsHostScriptSite->Release();
            }
            activeScript->Release();
        }
        activeScriptDirect->Release();
    }

    return NULL;
}

Var WScriptFastDom::RegisterCrossThreadInterfacePS(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;

    IActiveScriptDirect * activeScriptDirect = NULL;
    hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
    if (SUCCEEDED(hr))
    {
        IActiveScript * activeScript;
        hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript);
        if (SUCCEEDED(hr))
        {
            IJsHostScriptSite * jsHostScriptSite;
            hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
            if (SUCCEEDED(hr))
            {
                hr = jsHostScriptSite->RegisterCrossThreadInterface();
                jsHostScriptSite->Release();
            }
            activeScript->Release();
        }
        activeScriptDirect->Release();
    }

    return NULL;
}

Var WScriptFastDom::CreateCanvasPixelArray(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();

    if(callInfo.Count != 2)
    {
        goto Error;
    }

    int length;
    IfFailGo(scriptDirect.GetProperty(args[0], _u("length"), &length));

    Var pixelArray;
    IfFailGo(scriptDirect->CreatePixelArray(static_cast<UINT>(length), &pixelArray));

    BYTE* pBuffer;
    UINT bufferLen;
    IfFailGo(scriptDirect->GetPixelArrayBuffer(pixelArray, &pBuffer, &bufferLen));

    // Read data into pixel array buffer
    {
        ByteBufferContainer byteBuffer(pBuffer, bufferLen);
        IfFailGo(scriptDirect.ReadArray(args[0], &byteBuffer));
    }

    result = pixelArray;

Error:
    scriptDirect.ThrowIfFailed(hr);
    return result;
}

Var WScriptFastDom::SetTimeout(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;
    args = &args[1];
    Var result = nullptr;
    ScriptDirect scriptDirect;

    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();

    // SetTimeout(function, time)
    if(callInfo.Count != 3)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    // Retrieve the function
    Var callbackFunction = args[0];

    // Retrieve and convert the time
    int time;
    IfFailGo(scriptDirect->VarToInt(args[1], &time));

    // Push the callback
    CallbackMessage *msg = new CallbackMessage(time, callbackFunction);
    s_messageQueue->Push(msg);

    // Build a return value that can be used as the key for ClearTimeout.
    IfFailGo(scriptDirect->IntToVar(msg->GetId(), &result));

Error:
    scriptDirect.ThrowIfFailed(hr);
    return result;
}

Var WScriptFastDom::ClearTimeout(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;
    args = &args[1];
    Var result = nullptr;
    ScriptDirect scriptDirect;

    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();

    // ClearTimeout(id)
    if(callInfo.Count != 2)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    // Retrieve and convert the id
    int id;
    IfFailGo(scriptDirect->VarToInt(args[0], &id));

    s_messageQueue->RemoveById(id);

Error:
    scriptDirect.ThrowIfFailed(hr);
    return result;
}

Var WScriptFastDom::EmitStackTraceEvent(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;
    args = &args[1];
    ScriptDirect scriptDirect;
    USHORT maxFrame = JSCRIPT_FULL_STACKTRACE;
    IfFailGo(scriptDirect.From(function));

    if(callInfo.Count > 1)
    {
        int value;
        IfFailGo(scriptDirect->VarToInt(args[0], &value));
        maxFrame = (USHORT)value;
    }

    UINT64 operationId = 201; // random operation id
    scriptDirect->EmitStackTraceEvent(operationId, maxFrame);

Error:
    return scriptDirect.GetUndefined();
}

Var WScriptFastDom::CallFunction(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    ScriptDirect scriptDirect;

    if (scriptDirect.From(function) == S_OK)
    {
        // CallFunciton(function)
        if (callInfo.Count == 2)
        {
            // Retrieve the function
            Var functionToCall = args[0];

            // For simplicity use the callback pattern but call directly.
            CallbackMessage msg(0/*timeout*/, functionToCall);
            msg.CallJavascriptFunction();
        }
    }

    return scriptDirect.GetUndefined();
}

Var WScriptFastDom::SetRestrictedMode(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    ScriptDirect scriptDirect;

    if (scriptDirect.From(function) == S_OK)
    {
        BOOL result = false;
        // CallFunction(function)
        if (callInfo.Count == 2
            && (scriptDirect->VarToBOOL(args[0], &result) == S_OK))
        {
            CComPtr<IActiveScriptDirect> activeScriptDirect(nullptr);
            CComPtr<IActiveScriptProperty> activeScriptProperty(nullptr);

            HRESULT hr = JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect);
            Assert(hr == S_OK);
            hr = activeScriptDirect->QueryInterface(__uuidof(IActiveScriptProperty), (LPVOID*)&activeScriptProperty);
            Assert(hr == S_OK);

            if (s_mainScriptSite != nullptr)
            {
                VARIANT varValue;
                varValue.vt = VT_BOOL;
                varValue.boolVal = result ? VARIANT_TRUE : VARIANT_FALSE;
                hr = activeScriptProperty->SetProperty(SCRIPTPROP_EVAL_RESTRICTION, nullptr, &varValue);
                Assert(hr == S_OK);
            }
            else
            {
                Assert(false);
            }
        }
    }

    return scriptDirect.GetUndefined();
}

Var WScriptFastDom::SetEvalEnabled(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    ScriptDirect scriptDirect;

    if (scriptDirect.From(function) == S_OK)
    {
        BOOL result = false;
        // CallFunction(function)
        if (callInfo.Count == 2
            && (scriptDirect->VarToBOOL(args[0], &result) == S_OK))
        {
            if (s_mainScriptSite != nullptr)
            {
                s_mainScriptSite->SetEvalAllowed(result);
            }
            else
            {
                Assert(false);
            }
        }
    }

    return scriptDirect.GetUndefined();
}

Var WScriptFastDom::TestConstructor(Var function, CallInfo callInfo, Var* args)
{
    Var obj;
    ScriptDirect scriptDirect;

    if (scriptDirect.From(function) == S_OK)
    {
        if (scriptDirect->CreateObject(&obj) == S_OK)
        {
            return obj;
        }
    }
    return scriptDirect.GetUndefined();
}

HRESULT WScriptFastDom::TestConstructorInitMethod(Var instance)
{
    return S_OK;
}

Var WScriptFastDom::Shutdown(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    std::multimap<Var, IJsHostScriptSite*>::iterator iter;
    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();

    if (callInfo.Count != 2)
    {
        // Implicitly assume the main script site needs to be closed
        if (s_mainScriptSite != nullptr)
        {
            hr = ShutdownEngine(s_mainScriptSite);
            if (SUCCEEDED(hr))
            {
                // Release this reference
                s_mainScriptSite->Release();
                s_mainScriptSite = nullptr;
            }
        }
        return result;
    }

    Var globalObject = args[0];
    {
        AutoCriticalSection autoHostThreadCS(&hostThreadMapCs);
        iter = scriptEngineMap.find(globalObject);
        if (iter != scriptEngineMap.end())
        {
            ShutdownEngine((JsHostActiveScriptSite*)iter->second);
            iter->second->Release();
            scriptEngineMap.erase(iter);
        }
    }

Error:
    return result;
}

Var WScriptFastDom::SetKeepAlive(Var function, CallInfo callInfo, Var* args)
{
    s_keepaliveCallback();
    return NOERROR;
}

Var WScriptFastDom::Broadcast(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();
    if (callInfo.Count > 1)
    {
        void* sharedContent = nullptr;
        JScript9Interface::GetContentOfSharedArrayBuffer(args[0], &sharedContent);

        int id = 1;
        for each(auto tid in GetEngineThreadData()->childrenThreadIds)
        {
            PostThreadMessage(tid, WM_BROADCAST_SAB, (WPARAM)sharedContent, id);
        }
    }

Error:
    return result;
}
Var WScriptFastDom::ReceiveBroadcast(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();
    if (callInfo.Count > 1)
    {
        if (GetEngineThreadData()->cbReceiveBroadcast)
        {
            IfFailGo(scriptDirect.JsVarRelease(GetEngineThreadData()->cbReceiveBroadcast));
        }

        IfFailGo(scriptDirect.JsVarAddRef(args[0]));
        GetEngineThreadData()->cbReceiveBroadcast = args[0];
    }

Error:
    return result;
}
Var WScriptFastDom::Report(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();
    if (callInfo.Count > 1)
    {
        BSTR str;
        IfFailGo(scriptDirect.VarToString(args[0], &str));
        
        EnterCriticalSection(&GetEngineThreadData()->parent->csReportQ);
        GetEngineThreadData()->parent->reportQ.push_back(std::wstring(str));
        LeaveCriticalSection(&GetEngineThreadData()->parent->csReportQ);
    }

Error:
    return result;
}
Var WScriptFastDom::GetReport(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    std::wstring wstr;
    bool hasReport = false;

    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetNull();

    
    EnterCriticalSection(&GetEngineThreadData()->csReportQ);
    if (!GetEngineThreadData()->reportQ.empty())
    {
        wstr =  GetEngineThreadData()->reportQ.front();
        GetEngineThreadData()->reportQ.pop_front();
        hasReport = true;
    }
    LeaveCriticalSection(&GetEngineThreadData()->csReportQ);

    if (hasReport)
    {
        IfFailGo(scriptDirect.StringToVar(wstr.c_str(), &result));
    }

Error:
    return result;
}
Var WScriptFastDom::Leaving(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();

    GetEngineThreadData()->leaving = true;

    if (GetEngineThreadData()->cbReceiveBroadcast)
    {
        IfFailGo(scriptDirect.JsVarRelease(GetEngineThreadData()->cbReceiveBroadcast));
        GetEngineThreadData()->cbReceiveBroadcast = nullptr;
    }

Error:
    return result;
}
Var WScriptFastDom::Sleep(Var function, CallInfo callInfo, Var* args)
{
    args = &args[1];
    HRESULT hr = S_OK;
    ScriptDirect scriptDirect;
    Var result = nullptr;
    IfFailGo(scriptDirect.From(function));
    result = scriptDirect.GetUndefined();
    if (callInfo.Count > 1)
    {
        int time;
        IfFailGo(scriptDirect->VarToInt(args[0], &time));
        ::Sleep(time);
    }

Error:
    return result;
}


void WScriptFastDom::ReceiveBroadcastCallBack(void* sharedContent, int id)
{
    Var function = GetEngineThreadData()->cbReceiveBroadcast;
    if (function)
    {
        HRESULT hr;
        ScriptDirect scriptDirect;
        CComPtr<IActiveScriptDirect> activeScriptDirect(nullptr);
        Var globalObject = NULL;
        Var sab = NULL;
        Var varId = NULL;
        Var result = NULL;

        IfFailGo(scriptDirect.From(function));
        IfFailGo(JScript9Interface::JsVarToScriptDirect(function, &activeScriptDirect));

        IfFailGo(scriptDirect->GetGlobalObject(&globalObject));

        JScript9Interface::CreateSharedArrayBufferFromContent(activeScriptDirect, sharedContent, &sab);
        IfFailGo(scriptDirect->DoubleToVar(id, &varId));

        Var args[] = { globalObject, sab,  };
        CallInfo callInfo = { _countof(args), CallFlags_None };

        IfFailGo(scriptDirect->Execute(function, callInfo, args, NULL, &result));

    }
Error:
    return;
}


/* static */
HRESULT WScriptFastDom::CreateArgsObject(IActiveScriptDirect *const activeScriptDirect, __out Var *argsObject)
{
    Assert(argsObject);

    HRESULT hr   = S_OK;
    LPWSTR *argv = HostConfigFlags::argsVal;
    Var retArr   = NULL;
    Var value    = NULL;
    Var index    = NULL;
    *argsObject   = NULL;
    CComPtr<IJavascriptOperations> jsOp;

    Assert(activeScriptDirect);

    Var undefined = NULL;
    activeScriptDirect->GetUndefined(&undefined);

    hr = activeScriptDirect->CreateArrayObject(HostConfigFlags::argsCount, &retArr);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = activeScriptDirect->GetJavascriptOperations(&jsOp);
    if (FAILED(hr))
    {
        return hr;
    }

    for (int i = 0; i < HostConfigFlags::argsCount; i++)
    {
        // Ok to trucate cArgs as it is guarded by overflow check inside StringToVar
        hr = activeScriptDirect->StringToVar((LPCWSTR)argv[i], static_cast<int>(wcslen((LPCWSTR)argv[i])), &value);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = activeScriptDirect->IntToVar(i, &index);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = jsOp->SetItem(activeScriptDirect, retArr, index, value);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    *argsObject = retArr;

    return hr;
}

HRESULT WScriptFastDom::AddMethodToObject(__in LPWSTR propertyName, __in IActiveScriptDirect* scriptDirect, __inout Var wscript, __in ScriptMethod signature)
{
    HRESULT hr;
    ITypeOperations * operations = nullptr;
    hr = scriptDirect->GetDefaultTypeOperations(&operations);
    IfFailedGo(hr);

    PropertyId propertyId;
    hr = scriptDirect->GetOrAddPropertyId(propertyName, &propertyId);
    IfFailedGo(hr);
    Var funcVar;
    hr = scriptDirect->BuildDOMDirectFunction(NULL, signature, propertyId, -1, 0, &funcVar);
    IfFailedGo(hr);
    BOOL result;
    hr =  operations->SetProperty(scriptDirect, wscript, propertyId, funcVar, &result);
    IfFailedGo(hr);

LReturn:
    if(operations)
    {
        operations->Release();
    }
    return hr;
}

void WScriptFastDom::AddMessageQueue(MessageQueue *messageQueue)
{
    Assert(s_messageQueue == NULL);
    s_messageQueue = messageQueue;
}

void WScriptFastDom::SetMainScriptSite(JsHostActiveScriptSite* activeScriptSite)
{
    Assert(s_mainScriptSite == nullptr);
    s_mainScriptSite = activeScriptSite;
    s_mainScriptSite->AddRef();
}

void WScriptFastDom::ClearMainScriptSite()
{
    if (s_mainScriptSite != nullptr)
    {
        s_mainScriptSite->Release();
        s_mainScriptSite = nullptr;
    }

}
HRESULT WScriptFastDom::InitializeProperty(IActiveScriptDirect *activeScriptDirect, __in LPCWSTR propName, __out Var * obj, __out PropertyId *propId)
{
    HRESULT hr = S_OK;
    HTYPE type;

    IfFailedGo(activeScriptDirect->GetOrAddPropertyId(propName, propId));
    IfFailedGo(activeScriptDirect->CreateType(TypeId_Unspecified, NULL, 0, NULL, NULL, NULL, FALSE, *propId, TRUE, &type));
    IfFailedGo(activeScriptDirect->CreateTypedObject(type, 0, TRUE, obj));

LReturn:
    return hr;
}

HRESULT WScriptFastDom::InitializeStreams(IActiveScriptDirect * activeScriptDirect, Var wscript)
{
    HRESULT hr = S_OK;
    ITypeOperations * operations = nullptr;
    BOOL result;

    IfFailedGo(activeScriptDirect->GetDefaultTypeOperations(&operations));


    // Create the properties on WScript
    Var stdErr, stdOut, stdIn;
    PropertyId stdErrPropId, stdOutPropId, stdInPropId;
    IfFailedGo(InitializeProperty(activeScriptDirect, _u("StdErr"), &stdErr, &stdErrPropId));
    IfFailedGo(operations->SetProperty(activeScriptDirect, wscript, stdErrPropId, stdErr, &result));
    IfFailedGo(InitializeProperty(activeScriptDirect, _u("StdOut"), &stdOut, &stdOutPropId));
    IfFailedGo(operations->SetProperty(activeScriptDirect, wscript, stdOutPropId, stdOut, &result));
    IfFailedGo(InitializeProperty(activeScriptDirect, _u("StdIn"), &stdIn, &stdInPropId));
    IfFailedGo(operations->SetProperty(activeScriptDirect, wscript, stdInPropId, stdIn, &result));


    // Add methods to the streams
    IfFailedGo(AddMethodToObject(_u("WriteLine"), activeScriptDirect, stdErr, WScriptFastDom::StdErrWriteLine));
    IfFailedGo(AddMethodToObject(_u("Write"), activeScriptDirect, stdErr, WScriptFastDom::StdErrWrite));
    IfFailedGo(AddMethodToObject(_u("WriteLine"), activeScriptDirect, stdOut, WScriptFastDom::StdOutWriteLine));
    IfFailedGo(AddMethodToObject(_u("Write"), activeScriptDirect, stdOut, WScriptFastDom::StdOutWrite));
    IfFailedGo(AddMethodToObject(_u("ReadLine"), activeScriptDirect, stdIn, WScriptFastDom::StdInReadLine));
    IfFailedGo(AddMethodToObject(_u("EOF"), activeScriptDirect, stdIn, WScriptFastDom::StdInEOF));

LReturn:
    if (operations != nullptr)
    {
        operations->Release();
    }
    return hr;
}

HRESULT WScriptFastDom::Initialize(IActiveScript * activeScript, BOOL isHTMLHost, NotifyCallback keepaliveCallback)
{
    HRESULT hr = S_OK;
    ITypeOperations * operations = NULL;
    IActiveScriptDirect * activeScriptDirect = NULL;
    BOOL result;

    hr = activeScript->QueryInterface(__uuidof(IActiveScriptDirect), (void **)&activeScriptDirect);
    IfFailedGo(hr);

    hr = activeScriptDirect->GetDefaultTypeOperations(&operations);
    IfFailedGo(hr);

    Var globalObject;
    hr = activeScriptDirect->GetGlobalObject(&globalObject);
    IfFailedGo(hr);

    // Create the WScript object
    PropertyId wscriptPropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("WScript"), &wscriptPropertyId);
    IfFailedGo(hr);
    Var wscript;
    hr = activeScriptDirect->CreateObject(&wscript);
    IfFailedGo(hr);
    hr = operations->SetProperty(activeScriptDirect, globalObject, wscriptPropertyId, wscript, &result);
    IfFailedGo(hr);

    // Create the print method of the global object
    hr = AddMethodToObject(_u("print"), activeScriptDirect, globalObject, WScriptFastDom::Echo);
    IfFailedGo(hr);

    // Create the read method of the global object
    hr = AddMethodToObject(_u("read"), activeScriptDirect, globalObject, WScriptFastDom::LoadTextFile);
    IfFailedGo(hr);

    // Create the readbuffer method of the global object
    hr = AddMethodToObject(_u("readbuffer"), activeScriptDirect, globalObject, WScriptFastDom::LoadBinaryFile);
    IfFailedGo(hr);

    // Create the LoadTextFile method
    hr = AddMethodToObject(_u("LoadTextFile"), activeScriptDirect, wscript, WScriptFastDom::LoadTextFile);
    IfFailedGo(hr);

    // Create the LoadBinaryFile method
    hr = AddMethodToObject(_u("LoadBinaryFile"), activeScriptDirect, wscript, WScriptFastDom::LoadBinaryFile);
    IfFailedGo(hr);

    // Create the LoadBinaryFile method
    hr = AddMethodToObject(_u("Flag"), activeScriptDirect, wscript, WScriptFastDom::Flag);
    IfFailedGo(hr);

    // Create the Echo method
    hr = AddMethodToObject(_u("Echo"), activeScriptDirect, wscript, WScriptFastDom::Echo);
    IfFailedGo(hr);

    // Triggers a DOM mutation breakpoint
    hr = AddMethodToObject(_u("ChangeDOMElement"), activeScriptDirect, wscript, WScriptFastDom::DispatchDOMMutationBreakpoint);
    IfFailedGo(hr);

    // Create the console object
    PropertyId consolePropertyId;
    hr = activeScriptDirect->GetOrAddPropertyId(_u("console"), &consolePropertyId);
    IfFailedGo(hr);
    Var console;
    hr = activeScriptDirect->CreateObject(&console);
    IfFailedGo(hr);
    hr = operations->SetProperty(activeScriptDirect, globalObject, consolePropertyId, console, &result);

    // Create the console.log method
    hr = AddMethodToObject(_u("log"), activeScriptDirect, console, WScriptFastDom::Echo);
    IfFailedGo(hr);

    if (!isHTMLHost)
    {
        // Create the Args method
        PropertyId argsPropertyId;
        hr = activeScriptDirect->GetOrAddPropertyId(_u("Arguments"), &argsPropertyId);
        IfFailedGo(hr);
        Var args;
        hr = CreateArgsObject(activeScriptDirect, &args);
        IfFailedGo(hr);
        Assert(args);
        hr = operations->SetProperty(activeScriptDirect, wscript, argsPropertyId, args, &result);
        IfFailedGo(hr);
    }

    // Initialize StdErr, StdOut, StdIn
    IfFailedGo(InitializeStreams(activeScriptDirect, wscript));

    // Create the Quit method
    if (!isHTMLHost)
    {
        hr = AddMethodToObject(_u("Quit"), activeScriptDirect, wscript, WScriptFastDom::Quit);
    }
    else
    {
        hr = AddMethodToObject(_u("Quit"), activeScriptDirect, wscript, WScriptFastDom::QuitHtmlHost);
    }
    IfFailedGo(hr);

    if (isHTMLHost)
    {
        s_keepaliveCallback = keepaliveCallback;
        hr = AddMethodToObject(_u("SetKeepAlive"), activeScriptDirect, wscript, WScriptFastDom::SetKeepAlive);
    }
    IfFailedGo(hr);

    if (!isHTMLHost)
    {
        // Create the LoadScriptFile method
        hr = AddMethodToObject(_u("LoadScriptFile"), activeScriptDirect, wscript, WScriptFastDom::LoadScriptFile);
        IfFailedGo(hr);

        // Create the LoadScript method
        hr = AddMethodToObject(_u("LoadScript"), activeScriptDirect, wscript, WScriptFastDom::LoadScript);
        IfFailedGo(hr);

        // Create the LoadScript method
        hr = AddMethodToObject(_u("LoadModule"), activeScriptDirect, wscript, WScriptFastDom::LoadModule);
        IfFailedGo(hr);

        // Create the InitializeProjection method
        hr = AddMethodToObject(_u("InitializeProjection"), activeScriptDirect, wscript, WScriptFastDom::InitializeProjection);
        IfFailedGo(hr);

        // Create the RegisterModuleSource method
        hr = AddMethodToObject(_u("RegisterModuleSource"), activeScriptDirect, wscript, WScriptFastDom::RegisterModuleSource);
        IfFailedGo(hr);
    }

    if (!isHTMLHost)
    {
        // Create the perform source rundown method
        hr = AddMethodToObject(_u("PerformSourceRundown"), activeScriptDirect, wscript, WScriptFastDom::PerformSourceRundown);
        IfFailedGo(hr);

        // Create the Dynamic attach method
        hr = AddMethodToObject(_u("Attach"), activeScriptDirect, wscript, WScriptFastDom::DebugDynamicAttach);
        IfFailedGo(hr);

        // Create the Dynamic detach method
        hr = AddMethodToObject(_u("Detach"), activeScriptDirect, wscript, WScriptFastDom::DebugDynamicDetach);
        IfFailedGo(hr);
    }
    else
    {
        // Create the perform source rundown method
        hr = AddMethodToObject(_u("PerformSourceRundown"), activeScriptDirect, wscript, WScriptFastDom::HtmlPerformSourceRundown);
        IfFailedGo(hr);

        // Create the Dynamic attach method
        hr = AddMethodToObject(_u("Attach"), activeScriptDirect, wscript, WScriptFastDom::HtmlDebugDynamicAttach);
        IfFailedGo(hr);

        // Create the Dynamic detach method
        hr = AddMethodToObject(_u("Detach"), activeScriptDirect, wscript, WScriptFastDom::HtmlDebugDynamicDetach);
        IfFailedGo(hr);
    }

#ifdef EDIT_AND_CONTINUE
    if (s_enableEditTest) // Only add method when command line explicitly requests, to avoid affecting other unrelated baseline.
    {
        // Create the Dynamic edit method
        hr = AddMethodToObject(_u("Edit"), activeScriptDirect, wscript, WScriptFastDom::Edit);
        IfFailedGo(hr);
    }
#endif

    // Create the Script Profile method
    hr = AddMethodToObject(_u("StartProfiling"), activeScriptDirect, wscript, WScriptFastDom::StartScriptProfiler);
    IfFailedGo(hr);

    // Create the Script Profile method
    hr = AddMethodToObject(_u("StopProfiling"), activeScriptDirect, wscript, WScriptFastDom::StopScriptProfiler);
    IfFailedGo(hr);

    // Create the RegisterCrossThreadInterfacePS method
    hr = AddMethodToObject(_u("RegisterCrossThreadInterfacePS"), activeScriptDirect, wscript, WScriptFastDom::RegisterCrossThreadInterfacePS);
    IfFailedGo(hr);

    // Create the GetWorkingSet method
    hr = AddMethodToObject(_u("GetWorkingSet"), activeScriptDirect, wscript, WScriptFastDom::GetWorkingSet);
    IfFailedGo(hr);

    // Create the CreateCanvasPixelArray method
    hr = AddMethodToObject(_u("CreateCanvasPixelArray"), activeScriptDirect, wscript, WScriptFastDom::CreateCanvasPixelArray);
    IfFailedGo(hr);

    // Create the Shutdown method
    hr = AddMethodToObject(_u("Shutdown"), activeScriptDirect, wscript, WScriptFastDom::Shutdown);
    IfFailedGo(hr);

    if (!isHTMLHost)
    {
        // Create the SetTimeout method
        hr = AddMethodToObject(_u("SetTimeout"), activeScriptDirect, wscript, WScriptFastDom::SetTimeout);
        IfFailedGo(hr);

        // Create the ClearTimeout method
        hr = AddMethodToObject(_u("ClearTimeout"), activeScriptDirect, wscript, WScriptFastDom::ClearTimeout);
        IfFailedGo(hr);
    }

    hr = AddMethodToObject(_u("EmitStackTraceEvent"), activeScriptDirect, wscript, WScriptFastDom::EmitStackTraceEvent);
    IfFailedGo(hr);

    if (HostConfigFlags::flags.EnableMiscWScriptFunctions)
    {
        hr = AddMethodToObject(_u("CallFunction"), activeScriptDirect, wscript, WScriptFastDom::CallFunction);
        IfFailedGo(hr);
    }

    hr = AddMethodToObject(_u("SetEvalEnabled"), activeScriptDirect, wscript, WScriptFastDom::SetEvalEnabled);
    IfFailedGo(hr);

    hr = AddMethodToObject(_u("SetRestrictedMode"), activeScriptDirect, wscript, WScriptFastDom::SetRestrictedMode);
    IfFailedGo(hr);

    if (!isHTMLHost && HostConfigFlags::flags.$262)
    {
        IfFailedGo(hr = AddMethodToObject(_u("Broadcast"), activeScriptDirect, wscript, WScriptFastDom::Broadcast));
        IfFailedGo(hr = AddMethodToObject(_u("ReceiveBroadcast"), activeScriptDirect, wscript, WScriptFastDom::ReceiveBroadcast));
        IfFailedGo(hr = AddMethodToObject(_u("Report"), activeScriptDirect, wscript, WScriptFastDom::Report));
        IfFailedGo(hr = AddMethodToObject(_u("GetReport"), activeScriptDirect, wscript, WScriptFastDom::GetReport));
        IfFailedGo(hr = AddMethodToObject(_u("Leaving"), activeScriptDirect, wscript, WScriptFastDom::Leaving));
        IfFailedGo(hr = AddMethodToObject(_u("Sleep"), activeScriptDirect, wscript, WScriptFastDom::Sleep));

        const wchar_t $262[] =
            #include "..\..\bin\ch\262.js"
            ;

        IJsHostScriptSite * jsHostScriptSite;
        hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
        if (SUCCEEDED(hr))
        {
            JsHostActiveScriptSite* scriptSite = (JsHostActiveScriptSite*)jsHostScriptSite;
            if (scriptSite != nullptr)
            {
                scriptSite->delegateErrorHandling = true;
                scriptSite->lastException = new ExceptionData();

                hr = scriptSite->LoadScript($262);

                scriptSite->delegateErrorHandling = false;
            }
            jsHostScriptSite->Release();
        }
    }

    // Add constructor ImageData
    PropertyId namePropertyId;
    Var testConstructor;
    IfFailedGo(activeScriptDirect->GetOrAddPropertyId(_u("TestConstructor"), &namePropertyId));
    IfFailedGo(activeScriptDirect->CreateDeferredConstructor(&WScriptFastDom::TestConstructor, namePropertyId, TestConstructorInitMethod, 5, FALSE, FALSE,  &testConstructor));
    IfFailedGo(operations->SetProperty(activeScriptDirect, wscript, namePropertyId, testConstructor, &result));

LReturn:
    if (operations)
    {
        operations->Release();
    }

    if (activeScriptDirect)
    {
        activeScriptDirect->Release();
    }

    return hr;
}

WScriptFastDom::CallbackMessage::CallbackMessage(unsigned int time, Var function) : MessageBase(time), m_function(function)
{
    HRESULT hr = ScriptDirect::JsVarAddRef(m_function);
    if (FAILED(hr))
    {
        // Simply report a fatal error and exit because continuing from this point would result in inconsistent state
        // and FailFast telemetry would not be useful.
        wprintf(_u("FATAL ERROR: ScriptDirect::JsVarAddRef failed in WScriptFastDom::CallbackMessage::`ctor`. hr=0x%x\n"), hr);
        exit(1);
    }
}

WScriptFastDom::CallbackMessage::~CallbackMessage()
{
    ScriptDirect::JsVarRelease(m_function);
    m_function = NULL;
}

HRESULT WScriptFastDom::CallbackMessage::CallJavascriptFunction()
{
    HRESULT hr = S_OK;
    if (m_function)
    {
        CComPtr<IServiceProvider> serviceProvider;
        CallInfo callInfo = { 1, CallFlags_None };
        ScriptDirect scriptDirect;
        Var globalObject = NULL;
        Var result = NULL;
        IfFailGo(scriptDirect.From(m_function));

        BOOL isCalable = FALSE;
        IfFailGo(scriptDirect->IsObjectCallable(m_function, &isCalable));

        if (!isCalable)
        {
            PWSTR scriptCode = NULL;
            unsigned int length = 0;
            Var varFunction = NULL;
            IfFailGo(scriptDirect->VarToRawString(m_function, (LPCWSTR*)&scriptCode, &length));
            IfFailGo(scriptDirect->Parse(scriptCode, &varFunction));

            // varFunction is constructed now (store it back, this can be used for setInterval)
            ScriptDirect::JsVarRelease(m_function);
            m_function = varFunction;
            ScriptDirect::JsVarAddRef(m_function);
        }

        IfFailGo(scriptDirect->GetGlobalObject(&globalObject));
        IfFailGo(scriptDirect->GetServiceProvider(&serviceProvider));
        hr = scriptDirect->Execute(
            m_function,
            callInfo,
            &globalObject,
            NULL,
            &result
            );
    }

Error:
    return hr;
}

HRESULT WScriptFastDom::CallbackMessage::Call()
{
    return CallJavascriptFunction();
}

WScriptFastDom::ModuleMessage::ModuleMessage(ModuleRecord module, LPCWSTR specifier, IActiveScriptDirect* activeScriptDirect)
    : MessageBase(0), moduleRecord(module), specifier(specifier), scriptDirect(activeScriptDirect)
{
    // TODO: we might need to JsVarAddRef if chakra doesn't
}

WScriptFastDom::ModuleMessage::~ModuleMessage()
{
}

HRESULT WScriptFastDom::ModuleMessage::Call()
{
    HRESULT hr;
    Var result;
    if (specifier == (LPCWSTR)nullptr)
    {
        hr = scriptDirect->ModuleEvaluation(moduleRecord, &result);
    }
    else
    {
        Var errorObject;
        // fetch the module.
        CComPtr<IJsHostScriptSite> jsHostScriptSite = nullptr;
        CComPtr<IActiveScript> activeScript;
        IfFailGo(scriptDirect->QueryInterface(__uuidof(IActiveScript), (void**)&activeScript));

        hr = activeScript->GetScriptSite(IID_IJsHostScriptSite, (void**)&jsHostScriptSite);
        if (SUCCEEDED(hr))
        {
            hr = jsHostScriptSite->LoadModuleFile(specifier, TRUE, (byte**)&errorObject, (DWORD_PTR)this->moduleRecord);
        }
    }
Error:
    return hr;
}


void WScriptFastDom::EnableEditTests()
{
    s_enableEditTest = true;
}
