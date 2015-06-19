/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "StdAfx.h"
#include "WScriptJsrt.h"

WScriptJsrt::OnAttachCallback WScriptJsrt::onAttach;

MessageQueue* WScriptJsrt::s_messageQueue = NULL;

#define IfJsrtErrorFail(expr, ret) do { if ((expr) != JsNoError) return ret; } while (0)
#define IfJsrtError(expr) do { if((expr) != JsNoError) { hr = E_FAIL; goto Error; } } while(0)
#define IfJsrtErrorSetGo(expr) do { errorCode = (expr); if(errorCode != JsNoError) { hr = E_FAIL; goto Error; } } while(0)

JsValueRef __stdcall WScriptJsrt::EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    for (unsigned int i = 1; i < argumentCount; i++)
    {          
        if (i > 1)
        {
            wprintf(L" ");
        }
        JsValueRef strValue;
        if (JScript9Interface::JsrtConvertValueToString(arguments[i], &strValue) == JsNoError) 
        {
            LPCWSTR str = NULL;
            size_t length;
            if (JScript9Interface::JsrtStringToPointer(strValue, &str, &length) == JsNoError) 
            {
                wprintf(L"%s", str);
            }
        }
    }

    wprintf(L"\n");        

    JsValueRef undefinedValue;
    if (JScript9Interface::JsrtGetUndefinedValue(&undefinedValue) == JsNoError)
    {
        return undefinedValue;
    }
    else
    {
        return nullptr;
    }
}

bool WScriptJsrt::CreateArgsObject(JsValueRef *argsObject)
{
    LPWSTR *argv = HostConfigFlags::argsVal;
    JsValueRef retArr;

    Assert(argsObject);
    *argsObject = nullptr;

    IfJsrtErrorFail(JScript9Interface::JsrtCreateArray(HostConfigFlags::argsCount, &retArr), false);

    for (int i = 0; i < HostConfigFlags::argsCount; i++)
    {
        JsValueRef value;
        JsValueRef index;

        IfJsrtErrorFail(JScript9Interface::JsrtPointerToString(argv[i], wcslen(argv[i]), &value), false);
        IfJsrtErrorFail(JScript9Interface::JsrtDoubleToNumber(i, &index), false);
        IfJsrtErrorFail(JScript9Interface::JsrtSetIndexedProperty(retArr, index, value), false);
    }

    *argsObject = retArr;

    return true;
}

JsValueRef __stdcall WScriptJsrt::QuitCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    int exitCode = 0;

    if (argumentCount > 1)
    {
        double exitCodeDouble;
        IfJsrtErrorFail(JScript9Interface::JsrtNumberToDouble(arguments[1], &exitCodeDouble), JS_INVALID_REFERENCE);
        exitCode = (int)exitCodeDouble;
    }

    ExitProcess(exitCode);

    return NULL;
}

JsValueRef __stdcall WScriptJsrt::LoadScriptFileCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    LPWSTR errorMessage = L"Internal error.";
    JsValueRef returnValue = JS_INVALID_REFERENCE;
    JsErrorCode errorCode = JsNoError;

    if (argumentCount != 2)
    {
        errorCode = JsErrorInvalidArgument;
        errorMessage = L"Too many or too few arguments.";
    }
    else
    {
        const wchar_t *filename;
        size_t length;

        errorCode = JScript9Interface::JsrtStringToPointer(arguments[1], &filename, &length);
        if (errorCode == JsNoError)
        {
            LPCWSTR fileContents = NULL;
            HRESULT hr = JsHostLoadScriptFromFile(filename, fileContents);
            if (FAILED(hr))
            {
                errorCode = JsErrorInvalidArgument;
                errorMessage = L"Couldn't load file.";
            }
            else
            {
                errorCode = JScript9Interface::JsrtPointerToString(fileContents, wcslen(fileContents), &returnValue);
            }
        }
    }

    if (errorCode != JsNoError)
    {
        JsValueRef errorObject;
        JsValueRef errorMessageString;

        errorCode = JScript9Interface::JsrtPointerToString(errorMessage, wcslen(errorMessage), &errorMessageString);

        if (errorCode != JsNoError)
        {
            errorCode = JScript9Interface::JsrtCreateError(errorMessageString, &errorObject);

            if (errorCode != JsNoError)
            {
                JScript9Interface::JsrtSetException(errorObject);
            }
        }

        return JS_INVALID_REFERENCE;
    }

    return returnValue;
}

JsValueRef __stdcall WScriptJsrt::LoadScriptCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = E_FAIL;
    JsErrorCode errorCode = JsNoError;
    LPCWSTR errorMessage = L"Internal error.";
    size_t errorMessageLength = wcslen(errorMessage);
    JsValueRef returnValue = JS_INVALID_REFERENCE;
    JsValueRef innerException = JS_INVALID_REFERENCE;
    JsErrorCode innerErrorCode = JsNoError;

    if (argumentCount < 2 || argumentCount > 4)
    {
        errorCode = JsErrorInvalidArgument;
        errorMessage = L"Too many or too few arguments.";
    }
    else
    {
        const wchar_t *fileContent;
        const wchar_t *fileName;
        const wchar_t *scaType = L"self";
        size_t fileContentLength;
        size_t fileNameLength;
        size_t scaTypeLength;

        IfJsrtErrorSetGo(JScript9Interface::JsrtStringToPointer(arguments[1], &fileContent, &fileContentLength));

        if (argumentCount > 2)
        {
            IfJsrtErrorSetGo(JScript9Interface::JsrtStringToPointer(arguments[2], &scaType, &scaTypeLength));
        }

        fileName = L"YourScript.js";
        fileNameLength = wcslen(fileName);
        if (argumentCount > 3)
        {
            IfJsrtErrorSetGo(JScript9Interface::JsrtStringToPointer(arguments[3], &fileName, &fileNameLength));
        }
        wchar_t fullPath[_MAX_PATH];
        if (_wfullpath(fullPath, fileName, _MAX_PATH) == nullptr)
        {
            IfFailGo(E_FAIL);
        }
        // canonicalize that path name to lower case for the profile storage
        size_t len = wcslen(fullPath);
        for (size_t i = 0; i < len; i++)
        {
            fullPath[i] = towlower(fullPath[i]);
        }

        if (wcscmp(scaType, L"self") == 0)
        {
            IfJsrtErrorSetGo(JScript9Interface::JsrtRunScript(fileContent, 0, fullPath, &returnValue));
            if (errorCode != JsNoError)
            {
                JScript9Interface::JsrtGetAndClearException(&innerException);
            }
        }
        else if (wcscmp(scaType, L"samethread") == 0)
        {
            JsValueRef context = JS_INVALID_REFERENCE;
            JsValueRef newContext = JS_INVALID_REFERENCE;

            // Create a new context and set it as the current context
            IfJsrtErrorSetGo(JScript9Interface::JsrtGetCurrentContext(&context));
            JsRuntimeHandle runtime = JS_INVALID_RUNTIME_HANDLE;
            IfJsrtErrorSetGo(JScript9Interface::JsrtGetRuntime(context, &runtime));
                    
            IfJsrtErrorSetGo(JScript9Interface::JsrtCreateContext(runtime, &newContext));
            IfJsrtErrorSetGo(JScript9Interface::JsrtSetCurrentContext(newContext));

            // Initialize the host objects
            Initialize(nullptr);

            errorCode = JScript9Interface::JsrtRunScript(fileContent, 0, fullPath, &returnValue);
            if (errorCode != JsNoError)
            {
                errorMessage = L"Error";
                JScript9Interface::JsrtGetAndClearException(&innerException);
                if (innerException != nullptr)
                {
                    JsPropertyIdRef messagePropertyId = JS_INVALID_REFERENCE;
                    innerErrorCode = JScript9Interface::JsrtGetPropertyIdFromName(L"message", &messagePropertyId);

                    if (innerErrorCode == JsNoError)
                    {
                        bool hasMessageProperty = false;
                        JScript9Interface::JsrtHasProperty(innerException, messagePropertyId, &hasMessageProperty);
                        if (hasMessageProperty)
                        {
                            JsValueRef messageProperty = JS_INVALID_REFERENCE;
                            innerErrorCode = JScript9Interface::JsrtGetProperty(innerException, messagePropertyId, &messageProperty);
                            if (innerErrorCode == JsNoError)
                            {
                                innerErrorCode = JScript9Interface::JsrtStringToPointer(messageProperty, &errorMessage, &errorMessageLength);
                            }
                        }
                    }
                }
            }
            // Set the context back to the old one
            JScript9Interface::JsrtSetCurrentContext(context);
        }
        else
        {
            errorCode = JsErrorInvalidArgument;
            errorMessage = L"Unsupported argument type for SCA type.";
        }
    }

Error:
    JsValueRef value = JS_INVALID_REFERENCE;
    if (errorCode != JsNoError)
    {
        if (innerErrorCode != JsNoError)
        {
            // Failed to retrieve the inner error message, so set a custom error string
            errorMessage = ConvertErrorCodeToMessage(errorCode);
        }

        JsValueRef error = JS_INVALID_REFERENCE;
        JsValueRef messageProperty = JS_INVALID_REFERENCE;
        errorMessageLength = wcslen(errorMessage);
        innerErrorCode = JScript9Interface::JsrtPointerToString(errorMessage, errorMessageLength, &messageProperty);
        if (innerErrorCode == JsNoError)
        {
            innerErrorCode = JScript9Interface::JsrtCreateError(messageProperty, &error);
            if (innerErrorCode == JsNoError)
            {
                innerErrorCode = JScript9Interface::JsrtSetException(error);
            }
        }
    }

    JScript9Interface::JsrtDoubleToNumber(errorCode, &value);
    _flushall();

    return value;
}

JsValueRef __stdcall WScriptJsrt::GetWorkingSetCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    LPWSTR errorMessage = L"Internal error.";
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX memoryCounter;
    memoryCounter.cb = sizeof(memoryCounter);

    if (!GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&memoryCounter,sizeof(memoryCounter)))
    {
        goto error;
    }

    JsContextRef context;
    JsRuntimeHandle runtime;

    if (JScript9Interface::JsrtGetCurrentContext(&context) != JsNoError ||
        JScript9Interface::JsrtGetRuntime(context, &runtime) != JsNoError ||
        JScript9Interface::JsrtCollectGarbage(runtime) != JsNoError)
    {
        goto error;
    }

    JsValueRef object;

    if (JScript9Interface::JsrtCreateObject(&object) != JsNoError)
    {
        goto error;
    }

    JsPropertyIdRef prop;
    JsValueRef value;

    if (JScript9Interface::JsrtGetPropertyIdFromName(L"workingSet", &prop) != JsNoError ||
        JScript9Interface::JsrtDoubleToNumber((double)memoryCounter.WorkingSetSize, &value) != JsNoError ||
        JScript9Interface::JsrtSetProperty(object, prop, value, true) != JsNoError)
    {
        goto error;
    }

    if (JScript9Interface::JsrtGetPropertyIdFromName(L"maxWorkingSet", &prop) != JsNoError ||
        JScript9Interface::JsrtDoubleToNumber((double)memoryCounter.PeakWorkingSetSize, &value) != JsNoError ||
        JScript9Interface::JsrtSetProperty(object, prop, value, true) != JsNoError)
    {
        goto error;
    }

    if (JScript9Interface::JsrtGetPropertyIdFromName(L"pageFault", &prop) != JsNoError ||
        JScript9Interface::JsrtDoubleToNumber((double)memoryCounter.PageFaultCount, &value) != JsNoError ||
        JScript9Interface::JsrtSetProperty(object, prop, value, true) != JsNoError)
    {
        goto error;
    }

    if (JScript9Interface::JsrtGetPropertyIdFromName(L"privateUsage", &prop) != JsNoError ||
        JScript9Interface::JsrtDoubleToNumber((double)memoryCounter.PrivateUsage, &value) != JsNoError ||
        JScript9Interface::JsrtSetProperty(object, prop, value, true) != JsNoError)
    {
        goto error;
    }

    return object;

error:
    JsValueRef errorObject;
    JsValueRef errorMessageString;

    JsErrorCode errorCode = JScript9Interface::JsrtPointerToString(errorMessage, wcslen(errorMessage), &errorMessageString);

    if (errorCode != JsNoError)
    {
        errorCode = JScript9Interface::JsrtCreateError(errorMessageString, &errorObject);

        if (errorCode != JsNoError)
        {
            JScript9Interface::JsrtSetException(errorObject);
        }
    }

    return JS_INVALID_REFERENCE;
}

JsValueRef __stdcall WScriptJsrt::AttachCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    if (argumentCount == 2 && onAttach != nullptr)
    {
        onAttach(arguments[1]);
    }

    return JS_INVALID_REFERENCE;
}

JsValueRef WScriptJsrt::SetTimeoutCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    LPWSTR errorMessage = L"invalid call to WScript.SetTimeout";

    if(argumentCount != 3)
        goto Error;

    JsValueRef function = arguments[1];
    JsValueRef timerId;
    unsigned int time;
    double tmp;

    if(JScript9Interface::JsrtNumberToDouble(arguments[2], &tmp) != JsNoError)
        goto Error;

    time = static_cast<int>(tmp);

    CallbackMessage *msg = new CallbackMessage(time, function);

    s_messageQueue->Push(msg);

    if(JScript9Interface::JsrtDoubleToNumber(static_cast<double>(msg->GetId()), &timerId) != JsNoError)
        goto Error;

    return timerId;

Error:
    JsValueRef errorObject;
    JsValueRef errorMessageString;

    JsErrorCode errorCode = JScript9Interface::JsrtPointerToString(errorMessage, wcslen(errorMessage), &errorMessageString);

    if (errorCode != JsNoError)
    {
        errorCode = JScript9Interface::JsrtCreateError(errorMessageString, &errorObject);

        if (errorCode != JsNoError)
        {
            JScript9Interface::JsrtSetException(errorObject);
        }
    }

    return JS_INVALID_REFERENCE;
}

JsValueRef WScriptJsrt::ClearTimeoutCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    LPWSTR errorMessage = L"invalid call to WScript.ClearTimeout";

    if(argumentCount != 2)
        goto Error;

    unsigned int timerId;
    double tmp;
    JsValueRef undef;
    JsValueRef global;

    if(JScript9Interface::JsrtNumberToDouble(arguments[1], &tmp) != JsNoError)
        goto Error;

    timerId = static_cast<int>(tmp);

    s_messageQueue->RemoveById(timerId);

    if(JScript9Interface::JsrtGetGlobalObject(&global) != JsNoError
        || JScript9Interface::JsrtGetUndefinedValue(&undef) != JsNoError)
        goto Error;

    return undef;

Error:
    JsValueRef errorObject;
    JsValueRef errorMessageString;

    JsErrorCode errorCode = JScript9Interface::JsrtPointerToString(errorMessage, wcslen(errorMessage), &errorMessageString);

    if (errorCode != JsNoError)
    {
        errorCode = JScript9Interface::JsrtCreateError(errorMessageString, &errorObject);

        if (errorCode != JsNoError)
        {
            JScript9Interface::JsrtSetException(errorObject);
        }
    }

    return JS_INVALID_REFERENCE;
}

bool WScriptJsrt::Initialize(OnAttachCallback onAttach)
{
    WScriptJsrt::onAttach = onAttach;

    JsValueRef wscript;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateObject(&wscript), false);

    JsValueRef echo;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(EchoCallback, nullptr, &echo), false);
    JsPropertyIdRef echoName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"Echo", &echoName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, echoName, echo, true), false);

    JsValueRef argsObject;

    if (!CreateArgsObject(&argsObject))
    {
        return false;
    }
    JsPropertyIdRef argsName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"Arguments", &argsName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, argsName, argsObject, true), false);

    JsValueRef quit;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(QuitCallback, nullptr, &quit), false);
    JsPropertyIdRef quitName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"Quit", &quitName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, quitName, quit, true), false);

    JsValueRef loadScriptFile;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(LoadScriptFileCallback, nullptr, &loadScriptFile), false);
    JsPropertyIdRef loadScriptFileName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"LoadScriptFile", &loadScriptFileName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, loadScriptFileName, loadScriptFile, true), false);

    JsValueRef loadScript;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(LoadScriptCallback, nullptr, &loadScript), false);
    JsPropertyIdRef loadScriptName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"LoadScript", &loadScriptName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, loadScriptName, loadScript, true), false);

    JsValueRef getWorkingSet;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(GetWorkingSetCallback, nullptr, &getWorkingSet), false);
    JsPropertyIdRef getWorkingSetName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"GetWorkingSet", &getWorkingSetName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, getWorkingSetName, getWorkingSet, true), false);

    JsValueRef attach;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(AttachCallback, nullptr, &attach), false);
    JsPropertyIdRef attachName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"Attach", &attachName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, attachName, attach, true), false);

    JsValueRef setTimeout;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(SetTimeoutCallback, nullptr, &setTimeout), false);
    JsPropertyIdRef setTimeoutName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"SetTimeout", &setTimeoutName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, setTimeoutName, setTimeout, true), false);

    JsValueRef clearTimeout;
    IfJsrtErrorFail(JScript9Interface::JsrtCreateFunction(ClearTimeoutCallback, nullptr, &clearTimeout), false);
    JsPropertyIdRef clearTimeoutName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"ClearTimeout", &clearTimeoutName), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(wscript, clearTimeoutName, clearTimeout, true), false);

    JsPropertyIdRef wscriptName;
    IfJsrtErrorFail(JScript9Interface::JsrtGetPropertyIdFromName(L"WScript", &wscriptName), false);
    JsValueRef global;
    IfJsrtErrorFail(JScript9Interface::JsrtGetGlobalObject(&global), false);
    IfJsrtErrorFail(JScript9Interface::JsrtSetProperty(global, wscriptName, wscript, true), false);

    return true;
}

void WScriptJsrt::AddMessageQueue(MessageQueue *messageQueue)
{
    Assert(s_messageQueue == NULL);

    s_messageQueue = messageQueue;
}

WScriptJsrt::CallbackMessage::CallbackMessage(unsigned int time, JsValueRef function) : MessageBase(time), m_function(function)
{
    JScript9Interface::JsrtAddRef(m_function, NULL);
}

WScriptJsrt::CallbackMessage::~CallbackMessage()
{
    JScript9Interface::JsrtRelease(m_function, NULL);
    m_function = NULL;
}

HRESULT WScriptJsrt::CallbackMessage::Call()
{
    HRESULT hr = S_OK;

    JsValueRef global;
    JsValueRef result;
    JsValueRef stringValue;
    JsValueType type;

    IfJsrtError(JScript9Interface::JsrtGetGlobalObject(&global));
    IfJsrtError(JScript9Interface::JsrtGetValueType(m_function, &type));

    if (type == JsString)
    {
        LPCWSTR script = NULL;
        size_t length = 0;

        IfJsrtError(JScript9Interface::JsrtConvertValueToString(m_function, &stringValue));
        IfJsrtError(JScript9Interface::JsrtStringToPointer(stringValue, &script, &length));

        // Run the code
        IfJsrtError(JScript9Interface::JsrtRunScript(script, JS_SOURCE_CONTEXT_NONE, L"" /*sourceUrl*/, nullptr /*no result needed*/));
    }
    else
    {
        IfJsrtError(JScript9Interface::JsrtCallFunction(m_function, &global, 1, &result));
    }

Error:
    if(FAILED(hr))
    {
        JsValueRef exception;
        JsValueRef strExcep;
        LPCWSTR msg;
        size_t length;
        IfJsrtErrorFail(JScript9Interface::JsrtGetAndClearException(&exception), E_FAIL);
        IfJsrtErrorFail(JScript9Interface::JsrtConvertValueToString(exception, &strExcep), E_FAIL);
        IfJsrtErrorFail(JScript9Interface::JsrtStringToPointer(strExcep, &msg, &length), E_FAIL);

        wprintf(L"Script Error: %s\n", msg);        
    }
    return hr;
}

