#include "StdAfx.h"

#include "jscript9diag.h"

// The javascript portion of the controller is 'imported' by escaping the quote and 
// backslashes and wrapping each line as a string then #including it here
static const WCHAR controllerScript[] = {
    #include "dbgcontroller.js.encoded"
    L'\0'
};

#define IfFlagSet(value ,flag) ((value & flag) == flag)

LPCWSTR DebuggerController::s_largeString = L"\"<large string>\"";

// 200MB, expergen generates huge file sometimes.
#define MAX_BASELINE_SIZE       (1024*1024*200)

ScriptEngineWrapper::ScriptEngineWrapper()
    : m_runtime(JS_INVALID_RUNTIME_HANDLE), m_context(nullptr)
{
    // TODO: better error handling.

    // Make sure chakra dll is loaded.
    if(GetModuleHandle(L"chakratest.dll") == NULL && GetModuleHandle(L"chakra.dll") == NULL)
    {
        JScript9Interface::ArgInfo args;
        if(JScript9Interface::LoadDll(false, nullptr, args) == NULL)
        {
            DebuggerController::LogError(L"unable to load chakra dll");
            return;
        }
    }


    if (JScript9Interface::JsrtCreateRuntime(JsRuntimeAttributeDisableBackgroundWork, NULL, &m_runtime) != JsNoError)
    {
        DebuggerController::LogError(L"debugger controller script initialization");
        return;
    }
    if (JScript9Interface::JsrtCreateContext(m_runtime, &m_context) != JsNoError)
    {
        DebuggerController::LogError(L"debugger controller context creation");
        return;
    }
    if (JScript9Interface::JsrtSetCurrentContext(m_context) != JsNoError)
    {
        DebuggerController::LogError(L"debugger controller SetCurrentContext");
    }

    // Install WScript.Echo
    if (FAILED(InstallHostCallback(L"Echo", &EchoCallback, nullptr)))
    {
        DebuggerController::LogError(L"installation of WScript.Echo");
    }
    
    // Run the built-in controller script.
    if (JScript9Interface::JsrtRunScript(controllerScript, (DWORD_PTR)-3 /*A large number which can't be address*/, L"dbgcontroller.js", nullptr) != JsNoError)
    {
        DebuggerController::LogError(L"run controller script");
    }
}

ScriptEngineWrapper::~ScriptEngineWrapper()
{
    if(JScript9Interface::JsrtSetCurrentContext(NULL) != JsNoError)
    {
        DebuggerController::LogError(L"debugger controller teardown"); 
    }
    if(JScript9Interface::JsrtDisposeRuntime(m_runtime) != JsNoError)
    {
        DebuggerController::LogError(L"debugger controller runtime teardown");
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
    JsrtCheckError(JScript9Interface::JsrtGetPropertyIdFromName(L"WScript", &wscriptId));

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

DebuggerController::DebuggerController(LPCWSTR baselineFilename)
    : m_scriptWrapper(new ScriptEngineWrapper), 
    m_baselineFilename(baselineFilename ? baselineFilename : L"")
{
}

DebuggerController::~DebuggerController()
{
    delete m_scriptWrapper;
}

HRESULT DebuggerController::DumpLog()
{
    return m_scriptWrapper->CallGlobalFunction(L"DumpLog", NULL);
}

HRESULT DebuggerController::LogBreakpoint(__in __nullterminated WCHAR const* reason)
{
    return m_scriptWrapper->CallGlobalFunction(L"RecordEvent", NULL, reason);
}

HRESULT DebuggerController::LogLocals(__in __nullterminated WCHAR const* locals)
{
    return m_scriptWrapper->CallGlobalFunction(L"RecordEvent", NULL, locals);
}

HRESULT DebuggerController::LogCallstack(__in __nullterminated WCHAR const* callstack)
{
    return m_scriptWrapper->CallGlobalFunction(L"RecordEvent", NULL, callstack);
}

HRESULT DebuggerController::LogSetNextStatement(__in __nullterminated WCHAR const* description)
{
    return m_scriptWrapper->CallGlobalFunction(L"RecordEvent", NULL, description);
}

HRESULT DebuggerController::LogEvaluateExpression(__in __nullterminated WCHAR const* expression)
{
    return m_scriptWrapper->CallGlobalFunction(L"RecordEvent", NULL, expression);
}

HRESULT DebuggerController::LogMessage(__in __nullterminated WCHAR const* message)
{
    return m_scriptWrapper->CallGlobalFunction(L"RecordEvent", NULL, message);
}

HRESULT DebuggerController::LogJson(__in __nullterminated WCHAR const* logString)
{
    HRESULT hr = S_OK;

    std::wstring encodedLogString;
    DebuggerController::EncodeString(logString, encodedLogString);

    std::wstring message = L"{\"log\" : \"";
    message += encodedLogString;
    message += L"\"}";
    hr = this->LogMessage(message.c_str());

    return hr;
}

HRESULT DebuggerController::SetBaseline()
{
    LPSTR script = NULL;
    FILE *file = NULL;
    int numChars = 0;
    LPWSTR wideScript = NULL;
    HRESULT hr = S_OK;

    if(_wfopen_s(&file, m_baselineFilename.c_str(), L"rb") != 0)
    {
        DebuggerController::LogError(L"opening baseline file '%s'", m_baselineFilename.c_str());
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
                    DebuggerController::LogError(L"MultiByteToWideChar");
                    IfFailGo(E_FAIL);
                }

                IfFailGo(m_scriptWrapper->CallGlobalFunction(L"SetBaseline", NULL, wideScript));
            }
            else
            {
                DebuggerController::LogError(L"failed to read from baseline file");
                IfFailGo(E_FAIL);
            }
        }
        else
        {
            DebuggerController::LogError(L"baseline file too large");
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

    if (filename == L"")
    {
        DebuggerController::LogError(L"Debugger did not receive .js file load notification.");
        goto Error;
    }

    // If a baseline file was passed in, use that; otherwise, use the filename of
    // the script file as the baseline;
    if (m_baselineFilename.empty())
    {
        // test.js.baseline
        baselineFilename = filename + L".dbg.baseline";
        // Check if we have read permission, if succeed then treat this as baseline file to compare against
        if (_waccess_s(baselineFilename.c_str(), 4) == 0)
        {
            m_baselineFilename = baselineFilename;
            SetBaseline();
        }
        else
        {
            DebuggerController::LogError(L"Failed opening baseline file, baseline will be created at '%s'", baselineFilename.c_str());
        }
    }
    else
    {
        baselineFilename = m_baselineFilename;
        SetBaseline();
    }


    LPCWSTR baselineData;
    size_t baselineDataLength;

    // Call the verification function.
    IfFailGo(m_scriptWrapper->CallGlobalFunction(L"Verify", &result));

    double val;
    JsValueRef numberVal;
    bool passed = false;
    // Convert the result to a number, and check whether the test passed/vailed;
    JsrtCheckError(JScript9Interface::JsrtConvertValueToNumber(result, &numberVal));
    JsrtCheckError(JScript9Interface::JsrtNumberToDouble(numberVal, &val));
    passed = (val == 0);

    // If the test failed, or a new baseline was requested, write out a baseline
    if (!passed || m_baselineFilename.empty())
    {
        JsrtCheckError(m_scriptWrapper->CallGlobalFunction(L"GetOutputJson", &result));
        IfFailGo(JScript9Interface::JsrtStringToPointer(result, &baselineData, &baselineDataLength));

#pragma warning(disable: 38021) // From MSDN: For the code page UTF-8 dwFlags must be set to either 0 or WC_ERR_INVALID_CHARS. Otherwise, the function fails with ERROR_INVALID_FLAGS.
        int multiByteDataLength = WideCharToMultiByte(CP_UTF8, 0, baselineData, -1, NULL, 0, NULL, NULL);
        baselineDataANSI = new char[multiByteDataLength];
        if (WideCharToMultiByte(CP_UTF8, 0, baselineData, -1, baselineDataANSI, multiByteDataLength, NULL, NULL) == 0)
        {
#pragma warning(default: 38021) // From MSDN: For the code page UTF-8 dwFlags must be set to either 0 or WC_ERR_INVALID_CHARS. Otherwise, the function fails with ERROR_INVALID_FLAGS.
            DebuggerController::LogError(L"WideCharToMultiByte");
            goto Error;
        }
        std::wstring newFile = baselineFilename;
        if (!passed && !m_baselineFilename.empty())
        {
            newFile = m_baselineFilename + L".rebase";
        }
        if (_wfopen_s(&file, newFile.c_str(), L"wt") != 0)
        {
            DebuggerController::LogError(L"DumpLogToFile failed to open rebase file '%s'", newFile.c_str());
            goto Error;
        }

        int countWritten = static_cast<int>(fwrite(baselineDataANSI, sizeof(baselineDataANSI[0]), strlen(baselineDataANSI), file));
        if (countWritten != (int)strlen(baselineDataANSI))
        {
            DebuggerController::LogError(L"DumpLogToFile failed to write data to file");
            goto Error;
        }

        fclose(file);
    }

Error:
    if (baselineDataANSI)
        delete[] baselineDataANSI;

    return hr;
}

void DebuggerController::Log(__in __nullterminated wchar_t *msg, ...)
{
    BOOL debuggerPresent = IsDebuggerPresent();
    if (HostConfigFlags::flags.DebugDumpText || debuggerPresent)
    {
        va_list args;
        va_start(args, msg);
        wchar_t buf[2048];

        const wchar_t* prefix = L"LOG: ";
        const int prefixLength = static_cast<int>(wcslen(prefix));
        StringCchCopyW(buf, _countof(buf), prefix);
        size_t usableLength = _countof(buf) - prefixLength - 2; // accounting for \n\0 at the end
        _vsnwprintf_s(buf + prefixLength, usableLength, _TRUNCATE, msg, args);
        StringCchCatW(buf, _countof(buf), L"\n\0");
        va_end(args);
    
        if (HostConfigFlags::flags.DebugDumpText)
        {
            wprintf(L"%s", buf);
            fflush(stdout);
        }

        if (debuggerPresent)
        {
            OutputDebugStringW(buf);
        }
    }
}

void DebuggerController::LogError(__in __nullterminated wchar_t *msg, ...)
{
    va_list args;
    va_start(args, msg);
    wprintf(L"ERROR: ");
    vwprintf(msg, args);
    wprintf(L"\n");
    fflush(stdout);
    va_end(args);
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
    return m_scriptWrapper->CallGlobalFunction(L"AddSourceFile", &result, text, buf);
}

HRESULT DebuggerController::HandleBreakpoint(LONG bpId)
{
    JsValueRef result;
    WCHAR buf[64];
    _itow_s(bpId, buf, 10);
    return m_scriptWrapper->CallGlobalFunction(L"HandleBreakpoint", &result, buf);
}

HRESULT DebuggerController::HandleException()
{
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(L"HandleException", &result);
}

HRESULT DebuggerController::ResetBpMap()
{
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(L"ResetBpMap", &result);
}

HRESULT DebuggerController::HandleMutationBreakpoint()
{
    JsValueRef result;
    return m_scriptWrapper->CallGlobalFunction(L"HandleMutationBreakpoint", &result);
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
            encodedStr += L"\\\"";
            break;
        case '\\':
            encodedStr += L"\\\\";
            break;
        case '\r':
            //swallow \r
            break;
        case '\n':
            encodedStr += L"\\n";
            break;
        case '\t':
            encodedStr += L"\\t";
            break;
        default:
            {
                WCHAR charStr[2] = { str[i], L'\0' };
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
    switch(reason)
    {
    case BREAKREASON_ERROR:                     // script error
        return L"exception";

    case BREAKREASON_BREAKPOINT:                // Script breakpoint
        return L"breakpoint";

    case BREAKREASON_STEP:                      // Caused by the stepping mode
        return L"step";

    case BREAKREASON_DEBUGGER_BLOCK:            // Caused by another thread breaking
        return L"debugger_block";

    case BREAKREASON_HOST_INITIATED:            // Caused by host requested break
        return L"host_initiated";

    case BREAKREASON_LANGUAGE_INITIATED:        // Caused by a scripted break
        return L"language_initiated";

    case BREAKREASON_DEBUGGER_HALT:             // Caused by debugger IDE requested break
        return L"debugger_halt";
    case BREAKREASON_MUTATION_BREAKPOINT:       // Caused by mutation breakpoint
        return L"mutation_breakpoint";
    case BREAKREASON_JIT:                   // Caused by JIT Debugging startup
    default:
        return L"unknown";
    }
}

LPCWSTR DebuggerController::GetBreakResumeAction(BREAKRESUMEACTION resumeAction)
{
    switch (resumeAction)
    {
    case BREAKRESUMEACTION_ABORT:
        return L"abort";

    case BREAKRESUMEACTION_CONTINUE:
        return L"continue";

    case BREAKRESUMEACTION_STEP_INTO:
        return L"step_into";

    case BREAKRESUMEACTION_STEP_OVER:
        return L"step_over";

    case BREAKRESUMEACTION_STEP_OUT:
        return L"step_out";

    case BREAKRESUMEACTION_IGNORE:
        return L"ignore";

    case BREAKRESUMEACTION_STEP_DOCUMENT:
        return L"step_document";

    default:
        return L"unknown";
    }
}

LPCWSTR DebuggerController::GetErrorResumeAction(ERRORRESUMEACTION errorAction)
{
    switch (errorAction)
    {
    case ERRORRESUMEACTION_ReexecuteErrorStatement:
        return L"ReexecuteErrorStatement";

    case ERRORRESUMEACTION_AbortCallAndReturnErrorToCaller:
        return L"AbortCallAndReturnErrorToCaller";

    case ERRORRESUMEACTION_SkipErrorStatement:
        return L"SkipErrorStatement";

    default:
        return L"unknown";
    }
}

LPCWSTR DebuggerController::GetBreakpointState(BREAKPOINT_STATE state)
{
    switch (state)
    {
    case BREAKPOINT_DELETED:
        return L"deleted";

    case BREAKPOINT_DISABLED:
        return L"disabled";

    case BREAKPOINT_ENABLED:
        return L"enabled";

    default:
        return L"unknown";
    }
}

LPCWSTR Location::ToString(LocationToStringFlags flags, bool isHybridDebugger)
{
    WCHAR buf[20];

    stringRep = L"{\"start\" : ";
    _itow_s(startChar,buf,_countof(buf),10);
    stringRep += buf;

    stringRep += L", \"length\" : ";
    _itow_s(length,buf,_countof(buf),10);
    stringRep += buf;

    stringRep += L", \"text\" : \"";
    stringRep += DebuggerController::EncodeString(text, encodedText);
    stringRep += L"\"";

    if (frameDescription.size())
    {
        stringRep += L", \"frameDescription\" : \"";

        std::wstring frameEncodedText;
        stringRep += DebuggerController::EncodeString(frameDescription, frameEncodedText);
        stringRep += L"\"";
    }

    if ((flags & LTSF_IncludeDocId) != 0)
    {
        // really output the source id (ever incrementing source file index)
        stringRep += L", \"documentId\" : ";
        _itow_s((int)(this->srcId), buf, _countof(buf), 10);
        stringRep += buf;
    }

    if ((flags & LTSF_IncludeDebugPropertyFlag) != 0)
    {
        DebuggerController::AppendDebugPropertyAttributesToString(stringRep, this->debugPropertyAttributes, isHybridDebugger);
    }

    if ((flags & LTSF_IncludeLineCol) != 0)
    {
        // line number
        stringRep += L", \"line\" : ";
        _itow_s((int)(this->lineNumber), buf, _countof(buf), 10);
        stringRep += buf;

        stringRep += L", \"column\" : ";
        _itow_s((int)(this->columnNumber), buf, _countof(buf), 10);
        stringRep += buf;
    }

    stringRep += L"}";

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
        case L'\r':
            if(*ptr == L'\n')
                ptr++;
            // fall through
        case L'\n':
        case L'\u2028':
        case L'\u2029':
            // The next line starts at the next character.  If the buffer ends here,
            // then don't count the line.
            if(*ptr != L'\0')
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
void DebuggerController::AppendDebugPropertyAttributesToString(std::wstring& stringRep, DWORD debugPropertyAttributes, bool isHybridDebugger, bool prefixSeparator /*= true*/)
{
    if (prefixSeparator)
    {
        stringRep += L", ";
    }

    stringRep += L"\"flags\" : [";

    if (!isHybridDebugger)
    {
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_INVALID) ? L"\"DBGPROP_ATTRIB_VALUE_IS_INVALID\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE) ? L"\"DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_FAKE) ? L"\"DBGPROP_ATTRIB_VALUE_IS_FAKE\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_METHOD) ? L"\"DBGPROP_ATTRIB_VALUE_IS_METHOD\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_EVENT) ? L"\"DBGPROP_ATTRIB_VALUE_IS_EVENT\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_IS_RAW_STRING) ? L"\"DBGPROP_ATTRIB_VALUE_IS_RAW_STRING\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_VALUE_READONLY) ? L"\"DBGPROP_ATTRIB_VALUE_READONLY\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_PUBLIC) ? L"\"DBGPROP_ATTRIB_ACCESS_PUBLIC\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_PRIVATE) ? L"\"DBGPROP_ATTRIB_ACCESS_PRIVATE\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_PROTECTED) ? L"\"DBGPROP_ATTRIB_ACCESS_PROTECTED\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_ACCESS_FINAL) ? L"\"DBGPROP_ATTRIB_ACCESS_FINAL\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_GLOBAL) ? L"\"DBGPROP_ATTRIB_STORAGE_GLOBAL\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_STATIC) ? L"\"DBGPROP_ATTRIB_STORAGE_STATIC\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_FIELD) ? L"\"DBGPROP_ATTRIB_STORAGE_FIELD\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_STORAGE_VIRTUAL) ? L"\"DBGPROP_ATTRIB_STORAGE_VIRTUAL\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_TYPE_IS_CONSTANT) ? L"\"DBGPROP_ATTRIB_TYPE_IS_CONSTANT\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_TYPE_IS_SYNCHRONIZED) ? L"\"DBGPROP_ATTRIB_TYPE_IS_SYNCHRONIZED\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_TYPE_IS_VOLATILE) ? L"\"DBGPROP_ATTRIB_TYPE_IS_VOLATILE\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_HAS_EXTENDED_ATTRIBS) ? L"\"DBGPROP_ATTRIB_HAS_EXTENDED_ATTRIBS\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_FRAME_INTRYBLOCK) ? L"\"DBGPROP_ATTRIB_FRAME_INTRYBLOCK\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_FRAME_INCATCHBLOCK) ? L"\"DBGPROP_ATTRIB_FRAME_INCATCHBLOCK\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, DBGPROP_ATTRIB_FRAME_INFINALLYBLOCK) ? L"\"DBGPROP_ATTRIB_FRAME_INFINALLYBLOCK\", " : L"";
    }
    else
    {
        // hybrid properties
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_HAS_CHILDREN) ? L"\"JS_PROPERTY_HAS_CHILDREN\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_FAKE) ? L"\"JS_PROPERTY_FAKE\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_METHOD) ? L"\"JS_PROPERTY_METHOD\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_READONLY) ? L"\"JS_PROPERTY_READONLY\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_NATIVE_WINRT_POINTER) ? L"\"JS_PROPERTY_NATIVE_WINRT_POINTER\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_FRAME_INTRYBLOCK) ? L"\"JS_PROPERTY_FRAME_INTRYBLOCK\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_FRAME_INCATCHBLOCK) ? L"\"JS_PROPERTY_FRAME_INCATCHBLOCK\", " : L"";
        stringRep += IfFlagSet(debugPropertyAttributes, JS_PROPERTY_FRAME_INFINALLYBLOCK) ? L"\"JS_PROPERTY_FRAME_INFINALLYBLOCK\", " : L"";
    }

    if (debugPropertyAttributes != 0)
    {
        // remove the trailing ", "
        stringRep = stringRep.substr(0, stringRep.length() - 2);
    }

    WCHAR buf[20];
    stringRep += L"], \"flagsAsValue\" : ";
    _itow_s(debugPropertyAttributes, buf, _countof(buf), 10);
    stringRep += buf;
}
