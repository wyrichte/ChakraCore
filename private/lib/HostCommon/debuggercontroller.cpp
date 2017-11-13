//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "HostCommonPch.h"

#include "jscript9diag.h"

// The javascript portion of the controller is 'imported' by escaping the quote and 
// backslashes and wrapping each line as a string then #including it here
static const WCHAR controllerScript[] = {
    #include "dbgcontroller.js.encoded"
    _u('\0')
};

#define IfFlagSet(value ,flag) ((value & flag) == flag)

LPCWSTR DebuggerController::s_largeString = _u("\"<large string>\"");

// 200MB, expergen generates huge file sometimes.
#define MAX_BASELINE_SIZE       (1024*1024*200)

JsValueRef __stdcall ScriptEngineWrapper::EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    for (unsigned int i = 1; i < argumentCount; i++)
    {
        if (i > 1)
        {
            wprintf(_u(" "));
        }
        JsValueRef strValue;
        if (JScript9Interface::JsrtConvertValueToString(arguments[i], &strValue) == JsNoError)
        {
            LPCWSTR str = NULL;
            size_t length;
            if (JScript9Interface::JsrtStringToPointer(strValue, &str, &length) == JsNoError)
            {
                wprintf(_u("%s"), str);
            }
        }
    }

    wprintf(_u("\n"));

    JsValueRef undefinedValue;
    JScript9Interface::JsrtGetUndefinedValue(&undefinedValue);
    return undefinedValue;
}

ScriptEngineWrapper::ScriptEngineWrapper()
    : m_runtime(JS_INVALID_RUNTIME_HANDLE), m_context(nullptr)
{
    // TODO: better error handling.

    // Make sure chakra dll is loaded.
    if(GetModuleHandle(_u("chakratest.dll")) == NULL && GetModuleHandle(_u("chakra.dll")) == NULL)
    {
        JScript9Interface::ArgInfo args;
        if(JScript9Interface::LoadDll(false, nullptr, args) == NULL)
        {
            DebuggerController::LogError(_u("unable to load chakra dll"));
            return;
        }
    }

    if (JScript9Interface::JsrtCreateRuntime(JsRuntimeAttributeDisableBackgroundWork, NULL, &m_runtime) != JsNoError)
    {
        DebuggerController::LogError(_u("debugger controller script initialization"));
        return;
    }
    if (JScript9Interface::JsrtCreateContext(m_runtime, &m_context) != JsNoError)
    {
        DebuggerController::LogError(_u("debugger controller context creation"));
        return;
    }
    if (JScript9Interface::JsrtSetCurrentContext(m_context) != JsNoError)
    {
        DebuggerController::LogError(_u("debugger controller SetCurrentContext"));
    }

    // TODO: dependency on jsrt from common? 
    // Install WScript.Echo
    if (FAILED(InstallHostCallback(_u("Echo"), &EchoCallback, nullptr)))
    {
        DebuggerController::LogError(_u("installation of WScript.Echo"));
    }
    
    // Run the built-in controller script.
    JsValueRef globalFunc = JS_INVALID_REFERENCE;
    if (JScript9Interface::JsrtParseScriptWithAttributes(controllerScript, JS_SOURCE_CONTEXT_NONE, _u("dbgcontroller.js"), JsParseScriptAttributeLibraryCode, &globalFunc) != JsNoError)
    {
        DebuggerController::LogError(_u("parse controller script"));
    }

    JsValueRef undefinedValue = JS_INVALID_REFERENCE;
    if (JScript9Interface::JsrtGetUndefinedValue(&undefinedValue) != JsNoError)
    {
        DebuggerController::LogError(_u("Unable to get undefined value"));
    }

    JsValueRef args[] = { undefinedValue };
    JsValueRef result = JS_INVALID_REFERENCE;
    if (JScript9Interface::JsrtCallFunction(globalFunc, args, _countof(args), &result) != JsNoError)
    {
        DebuggerController::LogError(_u("run controller script"));
    }
}

ScriptEngineWrapper::~ScriptEngineWrapper()
{
    if(JScript9Interface::JsrtSetCurrentContext(NULL) != JsNoError)
    {
        DebuggerController::LogError(_u("debugger controller teardown")); 
    }
    if(JScript9Interface::JsrtDisposeRuntime(m_runtime) != JsNoError)
    {
        DebuggerController::LogError(_u("debugger controller runtime teardown"));
    }
}

HRESULT ScriptEngineWrapper::CallGlobalFunction(WCHAR const* function, JsValueRef* result, WCHAR const* arg1, WCHAR const *arg2, WCHAR const *arg3)
{
    JsValueRef globalObj = nullptr;
    JsValueRef targetFunc = nullptr;
    JsPropertyIdRef targetFuncId;

    // Save the current context
    JsContextRef prevContext = nullptr;
    if(JScript9Interface::JsrtGetCurrentContext(&prevContext) != JsNoError)
        return E_FAIL;

    // Set the current context
    if(JScript9Interface::JsrtSetCurrentContext(m_context) != JsNoError)
        return E_FAIL;

    // Get the global object
    if(JScript9Interface::JsrtGetGlobalObject(&globalObj) != JsNoError)
        return E_FAIL;

    // Get a script string for the function name
    if(JScript9Interface::JsrtGetPropertyIdFromName(function, &targetFuncId) != JsNoError)
        return E_FAIL;

    // Get the target function
    if(JScript9Interface::JsrtGetProperty(globalObj, targetFuncId, &targetFunc) != JsNoError)
        return E_FAIL;

    static const int MaxArgs = 3;
    int argCount = 1;
    WCHAR const* args[MaxArgs] = { arg1, arg2, arg3 };
    JsValueRef jsArgs[MaxArgs + 1];

    // Pass in undefined for 'this'
    if(JScript9Interface::JsrtGetUndefinedValue(&jsArgs[0]) != JsNoError)
        return E_FAIL;

    // Marshal the arguments
    for(int i = 0; i < MaxArgs; ++i,argCount++)
    {
        if(args[i] == nullptr)
            break;
        else
        {
            if(JScript9Interface::JsrtPointerToString(args[i], wcslen(args[i]), &jsArgs[i + 1]) != JsNoError)
                return E_FAIL;
        }
    }

    // Call the function
    if(JScript9Interface::JsrtCallFunction(targetFunc, jsArgs, (unsigned short)argCount, result) != JsNoError)
        return E_FAIL;

    // Restore the previous context
    if(JScript9Interface::JsrtSetCurrentContext(prevContext) != JsNoError)
        return E_FAIL;

    return S_OK;
}

HRESULT ScriptEngineWrapper::InstallHostCallback(LPCWSTR propName, JsNativeFunction function, void *data)
{
    HRESULT hr = S_OK;

    // Get the Global object
    JsValueRef globalObj;
    JsrtCheckError(JScript9Interface::JsrtGetGlobalObject(&globalObj));

    // Get the property ID for WScript
    JsPropertyIdRef wscriptId;
    JsrtCheckError(JScript9Interface::JsrtGetPropertyIdFromName(_u("WScript"), &wscriptId));

    // Check if WScript exists on global
    JsValueRef wscriptObject;
    bool result;
    JsrtCheckError(JScript9Interface::JsrtHasProperty(globalObj, wscriptId, &result));
    if(!result)
    {
        // Create the WScript object
        JsrtCheckError(JScript9Interface::JsrtCreateObject(&wscriptObject));

        // Store on global
        JsrtCheckError(JScript9Interface::JsrtSetProperty(globalObj, wscriptId, wscriptObject, true));
    }
    else
    {
        // Get the WScript object
        JsrtCheckError(JScript9Interface::JsrtGetProperty(globalObj, wscriptId, &wscriptObject));
    }

    // Create the callback function
    JsValueRef jsFunction;
    JsrtCheckError(JScript9Interface::JsrtCreateFunction(function, nullptr, &jsFunction));

    
    // Create an object to hold the callback's external data
    JsPropertyIdRef dataId;
    JsValueRef dataObj;
    JsrtCheckError(JScript9Interface::JsrtGetPropertyIdFromName(EXTERNAL_DATA_PROP, &dataId));
    JsrtCheckError(JScript9Interface::JsrtCreateExternalObject(data, nullptr, &dataObj));
    JsrtCheckError(JScript9Interface::JsrtSetProperty(jsFunction, dataId, dataObj, true));

    // Create the target property Id
    JsPropertyIdRef newPropId;
    JsrtCheckError(JScript9Interface::JsrtGetPropertyIdFromName(propName, &newPropId));

    // Store the callback function on WScript
    JsrtCheckError(JScript9Interface::JsrtSetProperty(wscriptObject, newPropId, jsFunction, true));


Error:
    return hr;
}

DebuggerController::DebuggerController(LPCWSTR baselineFilename, LPCWSTR baselinePath)
    : m_scriptWrapper(new ScriptEngineWrapper),
    m_baselineFilename(baselineFilename ? baselineFilename : _u("")),
    m_baselinePath(baselinePath ? baselinePath : _u(""))
{
}

DebuggerController::~DebuggerController()
{
    delete m_scriptWrapper;
}

HRESULT DebuggerController::DumpLog()
{
    return m_scriptWrapper->CallGlobalFunction(_u("DumpLog"), NULL);
}

HRESULT DebuggerController::LogBreakpoint(__in __nullterminated WCHAR const* reason)
{
    return m_scriptWrapper->CallGlobalFunction(_u("RecordEvent"), NULL, reason);
}

HRESULT DebuggerController::LogLocals(__in __nullterminated WCHAR const* locals)
{
    return m_scriptWrapper->CallGlobalFunction(_u("RecordEvent"), NULL, locals);
}

HRESULT DebuggerController::LogCallstack(__in __nullterminated WCHAR const* callstack)
{
    return m_scriptWrapper->CallGlobalFunction(_u("RecordEvent"), NULL, callstack);
}

HRESULT DebuggerController::LogSetNextStatement(__in __nullterminated WCHAR const* description)
{
    return m_scriptWrapper->CallGlobalFunction(_u("RecordEvent"), NULL, description);
}

HRESULT DebuggerController::LogEvaluateExpression(__in __nullterminated WCHAR const* expression)
{
    return m_scriptWrapper->CallGlobalFunction(_u("RecordEvent"), NULL, expression);
}

HRESULT DebuggerController::LogMessage(__in __nullterminated WCHAR const* message)
{
    return m_scriptWrapper->CallGlobalFunction(_u("RecordEvent"), NULL, message);
}

HRESULT DebuggerController::LogJson(__in __nullterminated WCHAR const* logString)
{
    HRESULT hr = S_OK;

    std::wstring encodedLogString;
    DebuggerController::EncodeString(logString, encodedLogString);

    std::wstring message = _u("{\"log\" : \"");
    message += encodedLogString;
    message += _u("\"}");
    hr = this->LogMessage(message.c_str());

    return hr;
}

HRESULT DebuggerController::SetBaseline(std::wstring const& fullPath)
{
    LPSTR script = NULL;
    FILE *file = NULL;
    int numChars = 0;
    LPWSTR wideScript = NULL;
    HRESULT hr = S_OK;

    if(_wfopen_s(&file, fullPath.c_str(), _u("rb")) != 0)
    {
        DebuggerController::LogError(_u("opening baseline file '%s'"), fullPath.c_str());
    }
    else
    {
        int fileSize = _filelength(_fileno(file));
        if(fileSize <= MAX_BASELINE_SIZE)
        {
            script = new char[fileSize + 1];

            numChars = static_cast<int>(fread(script, sizeof(script[0]), fileSize, file));
            if(numChars == fileSize)
            {
                script[numChars] = '\0';

                // Convert to wide for sending to JScript.
                wideScript = new WCHAR[numChars + 2];
                if(MultiByteToWideChar(CP_UTF8, 0, script, static_cast<int>(strlen(script)) + 1, wideScript, numChars + 2) == 0)
                {
                    DebuggerController::LogError(_u("MultiByteToWideChar"));
                    IfFailGo(E_FAIL);
                }

                IfFailGo(m_scriptWrapper->CallGlobalFunction(_u("SetBaseline"), NULL, wideScript));
            }
            else
            {
                DebuggerController::LogError(_u("failed to read from baseline file"));
                IfFailGo(E_FAIL);
            }
        }
        else
        {
            DebuggerController::LogError(_u("baseline file too large"));
            IfFailGo(E_FAIL);
        }
    }
Error:
    if(script)
        delete[] script;
    if(wideScript)
        delete[] wideScript;
    if(file)
        fclose(file);
    return hr;
}

HRESULT DebuggerController::VerifyAndWriteNewBaselineFile(std::wstring const& filename)
{
    JsValueRef result = NULL;
    HRESULT hr = S_OK;
    std::wstring baselineFilename;
    std::wstring rebaseFilename;
    FILE *file = NULL;
    LPSTR baselineDataANSI = NULL;

    if (filename == _u(""))
    {
        DebuggerController::LogError(_u("Debugger did not receive .js file load notification."));
        goto Error;
    }

    // If a baseline file was passed in, use that; otherwise, use the filename of
    // the script file as the baseline;

    if (!m_baselinePath.empty())
    {
        baselineFilename.append(m_baselinePath);
        baselineFilename.append(_u("\\"));
    }

    bool createNewBaseline = false;

    if (m_baselineFilename.empty())
    {
        // test.js.baseline
        baselineFilename.append(filename);
        baselineFilename.append(_u(".dbg.baseline"));

        // Check if we have read permission, if succeed then treat this as baseline file to compare against
        if (_waccess_s(baselineFilename.c_str(), 4) == 0)
        {
            SetBaseline(baselineFilename);
        }
        else
        {
            DebuggerController::LogError(_u("Failed opening baseline file, baseline will be created at '%s'"), baselineFilename.c_str());
            createNewBaseline = true;
        }
    }
    else
    {
        baselineFilename.append(m_baselineFilename);
        SetBaseline(baselineFilename);
    }


    LPCWSTR baselineData;
    size_t baselineDataLength;

    // Call the verification function.
    IfFailGo(m_scriptWrapper->CallGlobalFunction(_u("Verify"), &result));

    double val;
    JsValueRef numberVal;
    bool passed = false;
    // Convert the result to a number, and check whether the test passed/vailed;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(result, &numberVal));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numberVal, &val));
    passed = (val == 0);

    // If the test failed, or a new baseline was requested, write out a baseline
    if (!passed || createNewBaseline)
    {
        JsrtCheckError(m_scriptWrapper->CallGlobalFunction(_u("GetOutputJson"), &result));
        IfFailGo(JScript9Interface::JsrtStringToPointer(result, &baselineData, &baselineDataLength));

#pragma warning(disable: 38021) // From MSDN: For the code page UTF-8 dwFlags must be set to either 0 or WC_ERR_INVALID_CHARS. Otherwise, the function fails with ERROR_INVALID_FLAGS.
        int multiByteDataLength = WideCharToMultiByte(CP_UTF8, 0, baselineData, -1, NULL, 0, NULL, NULL);
        baselineDataANSI = new char[multiByteDataLength];
        if (WideCharToMultiByte(CP_UTF8, 0, baselineData, -1, baselineDataANSI, multiByteDataLength, NULL, NULL) == 0)
        {
#pragma warning(default: 38021) // From MSDN: For the code page UTF-8 dwFlags must be set to either 0 or WC_ERR_INVALID_CHARS. Otherwise, the function fails with ERROR_INVALID_FLAGS.
            DebuggerController::LogError(_u("WideCharToMultiByte"));
            goto Error;
        }

        std::wstring newFile = baselineFilename;
        if (!passed && !createNewBaseline)
        {
            newFile = baselineFilename + _u(".rebase");
        }
        if (_wfopen_s(&file, newFile.c_str(), _u("wt")) != 0)
        {
            DebuggerController::LogError(_u("DumpLogToFile failed to open rebase file '%s'"), newFile.c_str());
            goto Error;
        }

        int countWritten = static_cast<int>(fwrite(baselineDataANSI, sizeof(baselineDataANSI[0]), strlen(baselineDataANSI), file));
        if (countWritten != (int)strlen(baselineDataANSI))
        {
            DebuggerController::LogError(_u("DumpLogToFile failed to write data to file"));
            goto Error;
        }

        fclose(file);
    }

Error:
    if (baselineDataANSI)
        delete[] baselineDataANSI;

    return hr;
}

void DebuggerController::Log(__in __nullterminated char16 *msg, ...)
{
    BOOL debuggerPresent = IsDebuggerPresent();
    if (HostConfigFlags::flags.DebugDumpText || debuggerPresent)
    {
        va_list args;
        va_start(args, msg);
        char16 buf[2048];

        const char16* prefix = _u("LOG: ");
        const int prefixLength = static_cast<int>(wcslen(prefix));
        StringCchCopyW(buf, _countof(buf), prefix);
        size_t usableLength = _countof(buf) - prefixLength - 2; // accounting for \n\0 at the end
        _vsnwprintf_s(buf + prefixLength, usableLength, _TRUNCATE, msg, args);
        StringCchCatW(buf, _countof(buf), _u("\n\0"));
        va_end(args);
    
        if (HostConfigFlags::flags.DebugDumpText)
        {
            wprintf(_u("%s"), buf);
            fflush(stdout);
        }

        if (debuggerPresent)
        {
            OutputDebugStringW(buf);
        }
    }
}

void DebuggerController::LogError(__in __nullterminated char16 *msg, ...)
{
    if (!HostConfigFlags::flags.MuteHostErrorMsgIsEnabled)
    {
        va_list args;
        va_start(args, msg);
        wprintf(_u("ERROR: "));
        vwprintf(msg, args);
        wprintf(_u("\n"));
        fflush(stdout);
        va_end(args);
    }
}

HRESULT DebuggerController::InstallHostCallback(LPCWSTR propName, JsNativeFunction function, void* data)
{
    return m_scriptWrapper->InstallHostCallback(propName, function, data);
}

HRESULT DebuggerController::AddSourceFile(LPCWSTR text, ULONG srcId)
{
    WCHAR buf[64];
    _itow_s(srcId, buf, 10);
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(_u("AddSourceFile"), &result, text, buf);
}

HRESULT DebuggerController::HandleBreakpoint(LONG bpId)
{
    JsValueRef result;
    WCHAR buf[64];
    _itow_s(bpId, buf, 10);
    return m_scriptWrapper->CallGlobalFunction(_u("HandleBreakpoint"), &result, buf);
}

HRESULT DebuggerController::HandleException()
{
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(_u("HandleException"), &result);
}

HRESULT DebuggerController::ResetBpMap()
{
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(_u("ResetBpMap"), &result);
}

HRESULT DebuggerController::HandleMutationBreakpoint()
{
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(_u("HandleMutationBreakpoint"), &result);
}

HRESULT DebuggerController::HandleDOMMutationBreakpoint()
{
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(_u("HandleDOMMutationBreakpoint"), &result);
}

HRESULT ScriptEngineWrapper::GetExternalData(JsValueRef func, void **data)
{
    HRESULT hr = S_OK;

    JsPropertyIdRef dataId;
    JsValueRef dataProp;

    JsrtCheckError(JScript9Interface::JsrtGetPropertyIdFromName(EXTERNAL_DATA_PROP, &dataId));

    JsrtCheckError(JScript9Interface::JsrtGetProperty(func, dataId, &dataProp));

    JsrtCheckError(JScript9Interface::JsrtGetExternalData(dataProp, data));

Error:
    return hr;
}

JsValueRef DebuggerController::GetJavascriptUndefined()
{
    JsValueRef result;
    JScript9Interface::JsrtGetUndefinedValue(&result);
    return result;
}

JsValueRef DebuggerController::ConvertDoubleToNumber(double val)
{
    JsValueRef result;
    JsErrorCode ret = JScript9Interface::JsrtDoubleToNumber(val, &result);

    return ret == JsNoError ? result : nullptr;
}

LPCWSTR DebuggerController::EncodeString(const std::wstring& str, std::wstring& encodedStr)
{
    return EncodeString(str.c_str(), str.size(), encodedStr);
}

LPCWSTR DebuggerController::EncodeString(BSTR str, std::wstring& encodedStr)
{
    return EncodeString(str, SysStringLen(str), encodedStr);
}

LPCWSTR DebuggerController::EncodeString(_In_reads_z_(len) LPCWSTR str, _In_ size_t len, std::wstring& encodedStr)
{
    encodedStr.reserve(len);

    for(size_t i = 0; i < len; i++)
    {
        switch(str[i])
        {
        case '\"':
            encodedStr += _u("\\\"");
            break;
        case '\\':
            encodedStr += _u("\\\\");
            break;
        case '\r':
            //swallow \r
            break;
        case '\n':
            encodedStr += _u("\\n");
            break;
        case '\t':
            encodedStr += _u("\\t");
            break;
        default:
            {
                WCHAR charStr[2] = { str[i], _u('\0') };
                encodedStr += charStr;
                break;
            }
        }
    }
    return encodedStr.c_str();
}

UINT DebuggerController::GetRadix(DebugPropertyFlags flags)
{
    UINT radix = flags & 0xFF;
    return radix ? radix : 10;
}

LPCWSTR DebuggerController::GetBreakpointReason(BREAKREASON reason)
{
    BREAKREASONEX reasonEx = (BREAKREASONEX)reason;
    switch(reasonEx)
    {
    case BREAKREASON_ERROR_EX:                     // script error
        return _u("exception");

    case BREAKREASON_BREAKPOINT_EX:                // Script breakpoint
        return _u("breakpoint");

    case BREAKREASON_STEP_EX:                      // Caused by the stepping mode
        return _u("step");

    case BREAKREASON_DEBUGGER_BLOCK_EX:            // Caused by another thread breaking
        return _u("debugger_block");

    case BREAKREASON_HOST_INITIATED_EX:            // Caused by host requested break
        return _u("host_initiated");

    case BREAKREASON_LANGUAGE_INITIATED_EX:        // Caused by a scripted break
        return _u("language_initiated");

    case BREAKREASON_DEBUGGER_HALT_EX:             // Caused by debugger IDE requested break
        return _u("debugger_halt");

    case BREAKREASON_MUTATION_BREAKPOINT_EX:       // Caused by mutation breakpoint
        return _u("mutation_breakpoint");

    case BREAKREASON_DOMMUTATION_BREAKPOINT_EX:    // Caused by DOM mutation break
        return _u("DOM mutation breakpoint");

    case BREAKREASON_JIT_EX:                   // Caused by JIT Debugging startup
    default:
        return _u("unknown");
    }
}

LPCWSTR DebuggerController::GetBreakResumeAction(BREAKRESUMEACTION resumeAction)
{
    switch (resumeAction)
    {
    case BREAKRESUMEACTION_ABORT:
        return _u("abort");

    case BREAKRESUMEACTION_CONTINUE:
        return _u("continue");

    case BREAKRESUMEACTION_STEP_INTO:
        return _u("step_into");

    case BREAKRESUMEACTION_STEP_OVER:
        return _u("step_over");

    case BREAKRESUMEACTION_STEP_OUT:
        return _u("step_out");

    case BREAKRESUMEACTION_IGNORE:
        return _u("ignore");

    case BREAKRESUMEACTION_STEP_DOCUMENT:
        return _u("step_document");

    default:
        return _u("unknown");
    }
}

LPCWSTR DebuggerController::GetErrorResumeAction(ERRORRESUMEACTION errorAction)
{
    switch (errorAction)
    {
    case ERRORRESUMEACTION_ReexecuteErrorStatement:
        return _u("ReexecuteErrorStatement");

    case ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller:
        return _u("AbortCallAndReturnErrorToCaller");

    case ERRORRESUMEACTION_SkipErrorStatement:
        return _u("SkipErrorStatement");

    default:
        return _u("unknown");
    }
}

LPCWSTR DebuggerController::GetBreakpointState(BREAKPOINT_STATE state)
{
    switch (state)
    {
    case BREAKPOINT_DELETED:
        return _u("deleted");

    case BREAKPOINT_DISABLED:
        return _u("disabled");

    case BREAKPOINT_ENABLED:
        return _u("enabled");

    default:
        return _u("unknown");
    }
}

LPCWSTR Location::ToString(LocationToStringFlags flags)
{
    WCHAR buf[20];

    stringRep = _u("{\"start\" : ");
    _itow_s(startChar,buf,_countof(buf),10);
    stringRep += buf;

    stringRep += _u(", \"length\" : ");
    _itow_s(length,buf,_countof(buf),10);
    stringRep += buf;

    stringRep += _u(", \"text\" : \"");
    stringRep += DebuggerController::EncodeString(text, encodedText);
    stringRep += _u("\"");

    if (frameDescription.size())
    {
        stringRep += _u(", \"frameDescription\" : \"");

        std::wstring frameEncodedText;
        stringRep += DebuggerController::EncodeString(frameDescription, frameEncodedText);
        stringRep += _u("\"");
    }

    if ((flags & LTSF_IncludeDocId) != 0)
    {
        // really output the source id (ever incrementing source file index)
        stringRep += _u(", \"documentId\" : ");
        _itow_s((int)(this->srcId), buf, _countof(buf), 10);
        stringRep += buf;
    }

    if ((flags & LTSF_IncludeDebugPropertyFlag) != 0)
    {
        DebuggerController::AppendDebugPropertyAttributesToString(stringRep, this->debugPropertyAttributes);
    }

    if ((flags & LTSF_IncludeLineCol) != 0)
    {
        // line number
        stringRep += _u(", \"line\" : ");
        _itow_s((int)(this->lineNumber), buf, _countof(buf), 10);
        stringRep += buf;

        stringRep += _u(", \"column\" : ");
        _itow_s((int)(this->columnNumber), buf, _countof(buf), 10);
        stringRep += buf;
    }

    stringRep += _u("}");

    return stringRep.c_str();
}

HRESULT JsrtValueConverter::InternalConvert(ULONG *val)
{
    HRESULT hr = S_OK;

    if(m_currArg >= m_argCount)
    {
        hr = E_FAIL;
    }
    else
    {
        double tmp;
        JsValueRef numValue;
        JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(m_args[m_currArg++], &numValue));
        JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numValue, &tmp));
        *val = (ULONG)tmp;
    }

Error:
    return hr;
}

HRESULT JsrtValueConverter::InternalConvert(LPCWSTR *val)
{
    HRESULT hr = S_OK;

    if(m_currArg >= m_argCount)
    {
        hr = E_FAIL;
    }
    else
    {
        size_t length;
        JsValueRef strValue;
        JsrtCheckError(JScript9Interface::JsrtConvertValueToString(m_args[m_currArg++], &strValue));
        JsrtCheckError(JScript9Interface::JsrtStringToPointer(strValue, val, &length));
    }

Error:
    return hr;
}

HRESULT JsrtValueConverter::InternalConvert(bool *val)
{
    HRESULT hr = S_OK;

    if(m_currArg >= m_argCount)
    {
        hr = E_FAIL;
    }
    else
    {
        JsValueRef boolValue;
        JsrtCheckError(JScript9Interface::JsrtConvertValueToBoolean(m_args[m_currArg++], &boolValue));
        JsrtCheckError(JScript9Interface::JsrtBooleanToBool(boolValue, val));
    }

Error:
    return hr;
}

void SourceMap::Initialize(LPCWSTR text)
{
    // First line starts at the 0th character.
    m_lineOffsets.push_back(0);

    LPCWSTR ptr = text;
    while(*ptr)
    {
        switch(*ptr++) 
        {
        case _u('\r'):
            if(*ptr == _u('\n'))
                ptr++;
            // fall through
        case _u('\n'):
        case _u('\u2028'):
        case _u('\u2029'):
            // The next line starts at the next character.  If the buffer ends here,
            // then don't count the line.
            if(*ptr != _u('\0'))
            {
                m_lineOffsets.push_back(static_cast<int>(ptr-text));
            }
            break;
        }
    }
}

int SourceMap::GetOffset(int line)
{
    if(line < 0 || line > (int)m_lineOffsets.size())
        return -1;
    else
        return m_lineOffsets[line];
}

int SourceMap::GetLine(int offset)
{
    for(int i = 0; i < (int)m_lineOffsets.size(); ++i)
    {
        if(offset < m_lineOffsets[i])
            return i - 1;
    }
    return static_cast<int>(m_lineOffsets.size()) - 1;
}

int SourceMap::GetNumLines()
{
    return static_cast<int>(m_lineOffsets.size());
}

/*static*/
void DebuggerController::AppendDebugPropertyAttributesToString(std::wstring& stringRep, DWORD debugPropertyAttributes, bool prefixSeparator /*= true*/)
{
    if (prefixSeparator)
    {
        stringRep += _u(", ");
    }

    stringRep += _u("\"flags\" : [");

    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_INVALID) ? _u("\"DBGPROP_ATTRIB_VALUE_IS_INVALID\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE) ? _u("\"DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_FAKE) ? _u("\"DBGPROP_ATTRIB_VALUE_IS_FAKE\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_METHOD) ? _u("\"DBGPROP_ATTRIB_VALUE_IS_METHOD\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_EVENT) ? _u("\"DBGPROP_ATTRIB_VALUE_IS_EVENT\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_RAW_STRING) ? _u("\"DBGPROP_ATTRIB_VALUE_IS_RAW_STRING\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_READONLY) ? _u("\"DBGPROP_ATTRIB_VALUE_READONLY\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_PUBLIC) ? _u("\"DBGPROP_ATTRIB_ACCESS_PUBLIC\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_PRIVATE) ? _u("\"DBGPROP_ATTRIB_ACCESS_PRIVATE\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_PROTECTED) ? _u("\"DBGPROP_ATTRIB_ACCESS_PROTECTED\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_FINAL) ? _u("\"DBGPROP_ATTRIB_ACCESS_FINAL\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_GLOBAL) ? _u("\"DBGPROP_ATTRIB_STORAGE_GLOBAL\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_STATIC) ? _u("\"DBGPROP_ATTRIB_STORAGE_STATIC\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_FIELD) ? _u("\"DBGPROP_ATTRIB_STORAGE_FIELD\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_VIRTUAL) ? _u("\"DBGPROP_ATTRIB_STORAGE_VIRTUAL\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_TYPE_IS_CONSTANT) ? _u("\"DBGPROP_ATTRIB_TYPE_IS_CONSTANT\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_TYPE_IS_SYNCHRONIZED) ? _u("\"DBGPROP_ATTRIB_TYPE_IS_SYNCHRONIZED\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_TYPE_IS_VOLATILE) ? _u("\"DBGPROP_ATTRIB_TYPE_IS_VOLATILE\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_HAS_EXTENDED_ATTRIBS) ? _u("\"DBGPROP_ATTRIB_HAS_EXTENDED_ATTRIBS\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_FRAME_INTRYBLOCK) ? _u("\"DBGPROP_ATTRIB_FRAME_INTRYBLOCK\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_FRAME_INCATCHBLOCK) ? _u("\"DBGPROP_ATTRIB_FRAME_INCATCHBLOCK\", ") : _u("");
    stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_FRAME_INFINALLYBLOCK) ? _u("\"DBGPROP_ATTRIB_FRAME_INFINALLYBLOCK\", ") : _u("");

    if (debugPropertyAttributes != 0)
    {
        // remove the trailing ", "
        stringRep = stringRep.substr(0, stringRep.length() - 2);
    }

    WCHAR buf[20];
    stringRep += _u("], \"flagsAsValue\" : ");
    _itow_s(debugPropertyAttributes, buf, _countof(buf), 10);
    stringRep += buf;
}

