//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#define PARAM_NOT_NULL(p) \
    if (p == NULL) \
	    { \
    return JsErrorNullArgument; \
	    }

#define VALIDATE_INCOMING_RUNTIME_HANDLE(p) \
	    { \
        if (p == JS_INVALID_RUNTIME_HANDLE) \
		        { \
            return JsErrorInvalidArgument; \
		        } \
	    }

#define VALIDATE_INCOMING_PROPERTYID(p, scriptContext) \
	    { \
    if (p == JS_INVALID_REFERENCE || \
    Js::IsInternalPropertyId(((Js::PropertyRecord *)p)->GetPropertyId())) \
		        { \
        return JsErrorInvalidArgument; \
		        } \
	    }

#define VALIDATE_INCOMING_REFERENCE(p, scriptContext) \
	    { \
    if (p != JS_INVALID_REFERENCE && !Js::TaggedNumber::Is(p)) \
	        { \
        Js::RecyclableObject * obj = Js::RecyclableObject::FromVar(p); \
        if (obj->GetScriptContext() != scriptContext) \
		            { \
            return JsErrorInvalidArgument; \
		            } \
	        } \
	    }

#define VALIDATE_INCOMING_OBJECT(p, scriptContext) \
	    { \
    if (p != JS_INVALID_REFERENCE) \
	        { \
        if (!Js::JavascriptOperators::IsObject(p)) \
		            { \
            return JsErrorArgumentNotObject; \
		            } \
            Js::RecyclableObject * obj = Js::RecyclableObject::FromVar(p); \
            if (obj->GetScriptContext() != scriptContext) \
			            { \
            return JsErrorInvalidArgument; \
			            } \
	        } \
	    }

#define VALIDATE_INCOMING_OBJECT_OR_NULL(p, scriptContext) \
	    { \
    if (p != JS_INVALID_REFERENCE) \
	        { \
        if (!Js::JavascriptOperators::IsObjectOrNull(p)) \
		            { \
            return JsErrorArgumentNotObject; \
		            } \
            Js::RecyclableObject * obj = Js::RecyclableObject::FromVar(p); \
            if (obj->GetScriptContext() != scriptContext) \
			            { \
            return JsErrorInvalidArgument; \
			            } \
	        } \
	    }

#define VALIDATE_INCOMING_FUNCTION(p, scriptContext) \
	    { \
    if (p != JS_INVALID_REFERENCE) \
	        { \
        if (!Js::JavascriptFunction::Is(p)) \
		            { \
            return JsErrorInvalidArgument; \
		            } \
	        } \
	    }

#define VALIDATE_INCOMING_VALUE_CONTEXT(p, scriptContext) \
        { \
    if (Js::RecyclableObject::Is(p)) \
            { \
        Js::RecyclableObject* obj = Js::RecyclableObject::FromVar(p); \
        if (obj->GetScriptContext() != scriptContext) \
                    { \
            return JsErrorInvalidArgument; \
                    } \
            } \
        }

template <class Fn>
JsErrorCode GlobalAPIWrapper(Fn fn)
{
    JsErrorCode errCode = JsNoError;
    try
    {
        // For now, treat this like an out of memory; consider if we should do something else here.

        AUTO_NESTED_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

        errCode = fn();

        // These are error codes that should only be produced by the wrappers and should never
        // be produced by the internal calls.
        Assert(errCode != JsErrorFatal &&
            errCode != JsErrorNoCurrentContext &&
            errCode != JsErrorInExceptionState &&
            errCode != JsErrorInDisabledState &&
            errCode != JsErrorOutOfMemory &&
            errCode != JsErrorScriptException &&
            errCode != JsErrorScriptTerminated);
    }
    catch (Js::OutOfMemoryException)
    {
        return JsErrorOutOfMemory;
    }
    catch (Js::StackOverflowException)
    {
        return JsErrorOutOfMemory;
    }
    catch (JsrtComException e)
    {
        return e.GetJsErrorFromHResult();
    }
    catch (Js::ExceptionBase)
    {
        AssertMsg(false, "Unexpected engine exception.");
        return JsErrorFatal;
    }
    catch (...)
    {
        AssertMsg(false, "Unexpected non-engine exception.");
        return JsErrorFatal;
    }

    return errCode;
}

JsErrorCode CheckContext(JsrtContext *currentContext, bool verifyRuntimeState, bool allowInObjectBeforeCollectCallback = false);

template <bool verifyRuntimeState, class Fn>
JsErrorCode ContextAPIWrapper(Fn fn)
{
    JsrtContext *currentContext = JsrtContext::GetCurrent();
    JsErrorCode errCode = CheckContext(currentContext, verifyRuntimeState);

    if (errCode != JsNoError)
    {
        return errCode;
    }

    Js::ScriptContext *scriptContext = currentContext->GetScriptContext();

    try
    {
        AUTO_NESTED_HANDLED_EXCEPTION_TYPE((ExceptionType)ExceptionType_JavascriptException);

        // Enter script
        BEGIN_ENTER_SCRIPT(scriptContext, true, true, true)
        {
            errCode = fn(scriptContext);
        }
        END_ENTER_SCRIPT

            // These are error codes that should only be produced by the wrappers and should never
            // be produced by the internal calls.
            Assert(errCode != JsErrorFatal &&
            errCode != JsErrorNoCurrentContext &&
            errCode != JsErrorInExceptionState &&
            errCode != JsErrorInDisabledState &&
            errCode != JsErrorOutOfMemory &&
            errCode != JsErrorScriptException &&
            errCode != JsErrorScriptTerminated);
    }
    catch (Js::JavascriptExceptionObject *exceptionObject)
    {
        Assert(scriptContext->GetThreadContext()->GetRecordedException() == NULL);

        if (exceptionObject == scriptContext->GetThreadContext()->GetPendingOOMErrorObject())
        {
            return JsErrorOutOfMemory;
        }
        else if (exceptionObject == scriptContext->GetThreadContext()->GetPendingSOErrorObject())
        {
            return JsErrorOutOfMemory;
        }
        else
        {
            scriptContext->GetThreadContext()->SetRecordedException(exceptionObject);

            return JsErrorScriptException;
        }
    }
    catch (Js::ScriptAbortException)
    {
        Assert(scriptContext->GetThreadContext()->GetRecordedException() == NULL);
        scriptContext->GetThreadContext()->SetRecordedException(scriptContext->GetThreadContext()->GetPendingTerminatedErrorObject());
        return JsErrorScriptTerminated;
    }
    catch (Js::EvalDisabledException)
    {
        return JsErrorScriptEvalDisabled;
    }
    catch (JsrtComException e)
    {
        return e.GetJsErrorFromHResult();
    }
    catch (Js::ExceptionBase)
    {
        AssertMsg(false, "Unexpected engine exception.");
        return JsErrorFatal;
    }
    catch (...)
    {
        AssertMsg(false, "Unexpected non-engine exception.");
        return JsErrorFatal;
    }

    return errCode;
}

template <class Fn>
JsErrorCode ContextAPINoScriptWrapper(Fn fn)
{
    JsrtContext *currentContext = JsrtContext::GetCurrent();
    JsErrorCode errCode = CheckContext(currentContext, /*verifyRuntimeState*/true, /*allowInObjectBeforeCollectCallback*/true);

    if (errCode != JsNoError)
    {
        return errCode;
    }

    Js::ScriptContext *scriptContext = currentContext->GetScriptContext();

    try
    {
        // For now, treat this like an out of memory; consider if we should do something else here.

        AUTO_NESTED_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

        errCode = fn(scriptContext);

        // These are error codes that should only be produced by the wrappers and should never
        // be produced by the internal calls.
        Assert(errCode != JsErrorFatal &&
            errCode != JsErrorNoCurrentContext &&
            errCode != JsErrorInExceptionState &&
            errCode != JsErrorInDisabledState &&
            errCode != JsErrorOutOfMemory &&
            errCode != JsErrorScriptException &&
            errCode != JsErrorScriptTerminated);
    }
    catch (Js::ScriptAbortException)
    {
        Assert(scriptContext->GetThreadContext()->GetRecordedException() == NULL);
        scriptContext->GetThreadContext()->SetRecordedException(scriptContext->GetThreadContext()->GetPendingTerminatedErrorObject());
        return JsErrorScriptTerminated;
    }
    catch (Js::OutOfMemoryException)
    {
        return JsErrorOutOfMemory;
    }
    catch (Js::StackOverflowException)
    {
        return JsErrorOutOfMemory;
    }
    catch (JsrtComException e)
    {
        return e.GetJsErrorFromHResult();
    }
    catch (Js::ExceptionBase)
    {
        AssertMsg(false, "Unexpected engine exception.");
        return JsErrorFatal;
    }
    catch (...)
    {
        AssertMsg(false, "Unexpected non-engine exception.");
        return JsErrorFatal;
    }

    return errCode;
}

void HandleScriptCompileError(Js::ScriptContext * scriptContext, CompileScriptException * se);
