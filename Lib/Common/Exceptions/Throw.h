//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js {

    class ScriptContext;

    class Throw
    {
    public:
        static void __declspec(noreturn) OutOfMemory();
        static void __declspec(noreturn) StackOverflow(ScriptContext *scriptContext, PVOID returnAddress);
        static void __declspec(noreturn) NotImplemented();
        static void __declspec(noreturn) InternalError();
        static void __declspec(noreturn) FatalInternalError();
        static void __declspec(noreturn) FatalProjectionError();

        static bool IsTEProcess();
        static void GenerateDumpAndTerminateProcess(PEXCEPTION_POINTERS exceptInfo);

        static void CheckAndThrowOutOfMemory(BOOLEAN status);
#ifdef GENERATE_DUMP
        static bool ReportAssert(__in LPSTR fileName, uint lineNumber, __in LPSTR error, __in LPSTR message);
        static void LogAssert();
        static int GenerateDump(PEXCEPTION_POINTERS exceptInfo, LPCWSTR filePath, int ret = EXCEPTION_CONTINUE_SEARCH);
        static void GenerateDump(LPCWSTR filePath, bool terminate = false);
    private:
        static CriticalSection csGenereateDump;
        __declspec(thread) static  StackBackTrace * stackBackTrace;
        static const int StackToSkip = 2;
        static const int StackTraceDepth = 40;
#endif
    };

    // Info:        Verify the result or throw catastrophic
    // Parameters:  HRESULT
    inline void VerifyOkCatastrophic(__in HRESULT hr)
    {
        if (hr == E_OUTOFMEMORY)
        {
            Js::Throw::OutOfMemory();
        }
        else if (FAILED(hr))
        {
            Js::Throw::FatalProjectionError();
        }
    }

    // Info:        Verify the result or throw catastrophic
    // Parameters:  bool
    template<typename TCheck>
    inline void VerifyCatastrophic(__in TCheck result)
    {
        if (!result)
        {
            Assert(false);
            Js::Throw::FatalProjectionError();
        }
    }


} // namespace Js

#define BEGIN_TRANSLATE_TO_HRESULT(type) \
{\
    try \
    { \
        AUTO_HANDLED_EXCEPTION_TYPE(type);

#define BEGIN_TRANSLATE_TO_HRESULT_NESTED(type) \
{\
    try \
    { \
        AUTO_NESTED_HANDLED_EXCEPTION_TYPE(type);

#define BEGIN_TRANSLATE_OOM_TO_HRESULT BEGIN_TRANSLATE_TO_HRESULT(ExceptionType_OutOfMemory)
#define BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED BEGIN_TRANSLATE_TO_HRESULT_NESTED(ExceptionType_OutOfMemory)

#define END_TRANSLATE_OOM_TO_HRESULT(hr) \
    } \
    catch (Js::OutOfMemoryException) \
    {   \
        hr = E_OUTOFMEMORY; \
    }\
}

#define END_TRANSLATE_OOM_TO_HRESULT_AND_EXCEPTION_OBJECT(hr, scriptContext, exceptionObject) \
    } \
    catch(Js::OutOfMemoryException) \
    {   \
        hr = E_OUTOFMEMORY; \
        *exceptionObject = Js::JavascriptExceptionOperators::GetOutOfMemoryExceptionObject(scriptContext); \
    } \
    }

#define BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT BEGIN_TRANSLATE_TO_HRESULT((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow))
#define BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT_NESTED BEGIN_TRANSLATE_TO_HRESULT_NESTED((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow))
#define BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT BEGIN_TRANSLATE_TO_HRESULT((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow | ExceptionType_JavascriptException))
#define BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED BEGIN_TRANSLATE_TO_HRESULT_NESTED((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow | ExceptionType_JavascriptException))

#define END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
    } \
    catch (Js::InternalErrorException)   \
    {   \
        hr = E_FAIL;    \
    }   \
    catch (Js::OutOfMemoryException) \
    {   \
        hr = E_OUTOFMEMORY; \
    }   \
    catch (Js::StackOverflowException) \
    { \
        hr = VBSERR_OutOfStack; \
    } \
    catch (Js::NotImplementedException)  \
    {   \
        hr = E_NOTIMPL; \
    } \
    catch (Js::ScriptAbortException)  \
    {   \
        hr = E_ABORT; \
    } 

#define CATCH_UNHANDLED_EXCEPTION(hr) \
    catch (...) \
    {   \
        AssertMsg(FALSE, "invalid exception thrown and didn't get handled");    \
        hr = E_FAIL;    \
    } \
    }

#define END_TRANSLATE_EXCEPTION_TO_HRESULT(hr) \
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr)\
    CATCH_UNHANDLED_EXCEPTION(hr)

#define END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr) \
    Assert(!JsUtil::ExternalApi::IsScriptActiveOnCurrentThreadContext()); \
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
    END_TRANSLATE_ERROROBJECT_TO_HRESULT(hr) \
    CATCH_UNHANDLED_EXCEPTION(hr)

// Use this version if execution is in script (use rarely)
#define END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_INSCRIPT(hr) \
    Assert(JsUtil::ExternalApi::IsScriptActiveOnCurrentThreadContext()); \
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
    END_TRANSLATE_ERROROBJECT_TO_HRESULT_INSCRIPT(hr) \
    CATCH_UNHANDLED_EXCEPTION(hr)

#define END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NOASSERT(hr) \
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
    END_TRANSLATE_ERROROBJECT_TO_HRESULT(hr) \
    CATCH_UNHANDLED_EXCEPTION(hr)

#define END_TRANSLATE_ERROROBJECT_TO_HRESULT_EX(hr, GetRuntimeErrorFunc) \
    catch(Js::JavascriptExceptionObject *  exceptionObject)  \
    {   \
        GET_RUNTIME_ERROR_IMPL(hr, GetRuntimeErrorFunc, exceptionObject); \
    }

#define GET_RUNTIME_ERROR_IMPL(hr, GetRuntimeErrorFunc, exceptionObject) \
    {   \
        Js::Var errorObject = exceptionObject->GetThrownObject(null);   \
        if (errorObject != NULL && (Js::JavascriptError::Is(errorObject) ||  \
            Js::JavascriptError::IsRemoteError(errorObject)))   \
        {       \
            hr = GetRuntimeErrorFunc(Js::RecyclableObject::FromVar(errorObject), NULL);   \
        }   \
        else \
        {  \
            AssertMsg(errorObject == NULL, "errorObject should be NULL");  \
            hr = E_OUTOFMEMORY;  \
        }  \
    }

#define GET_RUNTIME_ERROR(hr, exceptionObject) \
    GET_RUNTIME_ERROR_IMPL(hr, Js::JavascriptError::GetRuntimeErrorWithScriptEnter, exceptionObject)

#define END_TRANSLATE_ERROROBJECT_TO_HRESULT(hr) \
    END_TRANSLATE_ERROROBJECT_TO_HRESULT_EX(hr, Js::JavascriptError::GetRuntimeErrorWithScriptEnter)

#define END_RECORD_TRANSLATE_ERROROBJECT_TO_HRESULT(hr, scriptContext) \
    catch (Js::JavascriptExceptionObject *  exceptionObject)  \
    {   \
        if (!scriptContext->HasRecordedException())  \
        {  \
            BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED \
                exceptionObject = exceptionObject->CloneIfStaticExceptionObject(scriptContext);  \
                scriptContext->RecordException(exceptionObject);  \
            END_TRANSLATE_OOM_TO_HRESULT(hr) \
        }  \
    }

#define END_GET_ERROROBJECT(hr, scriptContext, exceptionObject) \
    catch (Js::JavascriptExceptionObject *  _exceptionObject)  \
    {   \
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED \
            exceptionObject = _exceptionObject; \
            exceptionObject = exceptionObject->CloneIfStaticExceptionObject(scriptContext);  \
        END_TRANSLATE_OOM_TO_HRESULT(hr) \
    }

// Use this version if execution is in script (use rarely)
#define END_TRANSLATE_ERROROBJECT_TO_HRESULT_INSCRIPT(hr) \
    END_TRANSLATE_ERROROBJECT_TO_HRESULT_EX(hr, Js::JavascriptError::GetRuntimeError)

#define TRANSLATE_EXCEPTION_TO_HRESULT_ENTRY(ex) \
    } \
    catch (ex) \
    {
