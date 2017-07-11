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
                if (errorCode == JsNoError)
                {
                    errorCode = JScript9Interface::JsrtRunScript(fileContents, 0, filename, &returnValue);
                }
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

JsValueRef __stdcall WScriptJsrt::LoadTextFileCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = E_FAIL;
    JsValueRef returnValue = JS_INVALID_REFERENCE;
    JsErrorCode errorCode = JsNoError;

    if (argumentCount < 2)
    {
        IfJsrtErrorSetGo(JScript9Interface::JsrtGetUndefinedValue(&returnValue));
    }
    else
    {
        const char16 *fileContent;
        const char16 *fileName;
        size_t fileNameLength;

        IfJsrtErrorSetGo(JScript9Interface::JsrtStringToPointer(arguments[1], &fileName, &fileNameLength));

        if (errorCode == JsNoError)
        {
            UINT lengthBytes = 0;
            bool isUtf8 = false;
            LPCOLESTR contentsRaw = nullptr;
            hr = JsHostLoadScriptFromFile(fileName, fileContent, &isUtf8, &contentsRaw, &lengthBytes);

            if (FAILED(hr))
            {
                fwprintf(stderr, L"Couldn't load file.\n");
            }
            else
            {
                JsValueRef stringObject;
                IfJsrtErrorSetGo(JScript9Interface::JsrtPointerToString(fileContent, lengthBytes, &stringObject));
                return stringObject;
            }
        }
    }

Error:
    return returnValue;
}

JsValueRef __stdcall WScriptJsrt::LoadBinaryFileCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = E_FAIL;
    JsValueRef returnValue = JS_INVALID_REFERENCE;
    JsErrorCode errorCode = JsNoError;

    if (argumentCount < 2)
    {
        IfJsrtErrorSetGo(JScript9Interface::JsrtGetUndefinedValue(&returnValue));
    }
    else
    {
        const char16 *fileContent;
        const char16 *fileName;
        size_t fileNameLength;

        IfJsrtErrorSetGo(JScript9Interface::JsrtStringToPointer(arguments[1], &fileName, &fileNameLength));

        if (errorCode == JsNoError)
        {
            UINT lengthBytes = 0;

            hr = JsHostLoadBinaryFile(fileName, fileContent, lengthBytes);
            if (FAILED(hr))
            {
                fwprintf(stderr, L"Couldn't load file.\n");
            }
            else
            {
                JsValueRef arrayBuffer;
                IfJsrtErrorSetGo(JScript9Interface::JsrtCreateArrayBuffer(lengthBytes, &arrayBuffer));
                BYTE* buffer;
                unsigned int bufferLength;
                IfJsrtErrorSetGo(JScript9Interface::JsrtGetArrayBufferStorage(arrayBuffer, &buffer, &bufferLength));
                if (bufferLength < lengthBytes)
                {
                    fwprintf(stderr, L"Array buffer size is insufficient to store the binary file.\n");
                }
                else
                {
                    if (memcpy_s(buffer, bufferLength, (BYTE*)fileContent, lengthBytes) == 0)
                    {
                        returnValue = arrayBuffer;
                    }
                }
            }
        }
    }

Error:
    return returnValue;
}

JsValueRef __stdcall WScriptJsrt::FlagCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = E_FAIL;
    JsValueRef returnValue = JS_INVALID_REFERENCE;
    JsErrorCode errorCode = JsNoError;

    IfJsrtErrorSetGo(JScript9Interface::JsrtGetUndefinedValue(&returnValue));

#if ENABLE_DEBUG_CONFIG_OPTIONS
    if (argumentCount > 1)
    {
        const char16 *cmd;
        size_t cmdLength;

        IfJsrtErrorSetGo(JScript9Interface::JsrtStringToPointer(arguments[1], &cmd, &cmdLength));
        const char16* argv[] = { nullptr, cmd };
        JScript9Interface::SetConfigFlags(2, (char16**)argv, nullptr);
    }
#endif

Error:
    return returnValue;
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

    JsValueRef global, wscript, argsObject, echo, loadTextFile, loadBinaryFile, console;
    IfJsrtErrorFail(JScript9Interface::JsrtGetGlobalObject(&global), false);
    IfJsrtErrorFail(JScript9Interface::JsrtCreateObject(&wscript), false);
    IfJsrtErrorFail(JScript9Interface::JsrtCreateObject(&console), false);
    if (!CreateArgsObject(&argsObject)) return false;

    // WScript functions
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("Echo"), EchoCallback, &echo), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("Quit"), QuitCallback), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("LoadScriptFile"), LoadScriptFileCallback), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("LoadScript"), LoadScriptCallback), false);
    //IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("LoadModule"), LoadModuleCallback), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("SetTimeout"), SetTimeoutCallback), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("ClearTimeout"), ClearTimeoutCallback), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("Attach"), AttachCallback), false);
    //IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("Detach"), DetachCallback), false);
    //IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("DumpFunctionPosition"), DumpFunctionPositionCallback), false);
    //IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("RequestAsyncBreak"), RequestAsyncBreakCallback), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("LoadBinaryFile"), LoadBinaryFileCallback, &loadBinaryFile), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("LoadTextFile"), LoadTextFileCallback, &loadTextFile), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("GetWorkingSet"), GetWorkingSetCallback), false);
    IfJsrtErrorFail(InstallFunctionOnObject(wscript, _u("Flag"), FlagCallback), false);
    IfJsrtErrorFail(InstallPropOnObject(wscript, _u("Arguments"), argsObject), false);

    // console functions
    IfJsrtErrorFail(InstallPropOnObject(console, _u("log"), echo), false);

    // Global properties
    IfJsrtErrorFail(InstallPropOnObject(global, _u("WScript"), wscript), false);
    IfJsrtErrorFail(InstallPropOnObject(global, _u("print"), echo), false);
    IfJsrtErrorFail(InstallPropOnObject(global, _u("read"), loadTextFile), false);
    IfJsrtErrorFail(InstallPropOnObject(global, _u("readbuffer"), loadBinaryFile), false);
    IfJsrtErrorFail(InstallPropOnObject(global, _u("console"), console), false);

    return true;
}

JsErrorCode WScriptJsrt::InstallFunctionOnObject(JsValueRef object, const char16* name, JsNativeFunction nativeFunction, JsValueRef* outFunc)
{
    JsErrorCode ret;
    JsValueRef func;
    ret = JScript9Interface::JsrtCreateFunction(nativeFunction, nullptr, &func);
    if (ret != JsNoError) return ret;
    if (outFunc)
    {
        *outFunc = func;
    }

    JsPropertyIdRef funcName;
    ret = JScript9Interface::JsrtGetPropertyIdFromName(name, &funcName);
    if (ret != JsNoError) return ret;
    ret = JScript9Interface::JsrtSetProperty(object, funcName, func, true);

    return ret;
}

JsErrorCode WScriptJsrt::InstallPropOnObject(JsValueRef object, const char16* name, JsValueRef var)
{
    JsErrorCode ret;

    JsPropertyIdRef propName;
    ret = JScript9Interface::JsrtGetPropertyIdFromName(name, &propName);
    if (ret != JsNoError) return ret;
    ret = JScript9Interface::JsrtSetProperty(object, propName, var, true);

    return ret;
}

void WScriptJsrt::AddMessageQueue(MessageQueue *messageQueue)
{
    Assert(s_messageQueue == NULL);

    s_messageQueue = messageQueue;
}

WScriptJsrt::CallbackMessage::CallbackMessage(unsigned int time, JsValueRef function) : MessageBase(time), m_function(function)
{
    JsErrorCode error = JScript9Interface::JsrtAddRef(m_function, NULL);
    if (error != JsNoError)
    {
        // Simply report a fatal error and exit because continuing from this point would result in inconsistent state
        // and FailFast telemetry would not be useful.
        wprintf(_u("FATAL ERROR: JScript9Interface::JsrtAddRef failed in WScriptJsrt::CallbackMessage::`ctor`. error=0x%x\n"), error);
        exit(1);
    }
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

