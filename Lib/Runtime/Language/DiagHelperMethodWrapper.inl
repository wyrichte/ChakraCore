//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

namespace Js
{
    // try-catch wrapper used to wrap helper calls or library/built-in calls.
    // Template parameters:
    // - Fn: the type of function to wrap
    // - doCheckParentInterpreterFrame: whether we should check for parent frame being interpreter frame, 
    //   needed in case of library call and not needed in case of helper.
    template <bool doCheckParentInterpreterFrame, typename Fn>
    Var HelperOrLibraryMethodWrapper(ScriptContext* scriptContext, Fn fn)
    {
        Assert(scriptContext);
        JavascriptExceptionObject* exceptionObject = nullptr;
        try
        {
            return fn();
        }
        catch (JavascriptExceptionObject* _exceptionObject)
        {
            exceptionObject = _exceptionObject;
        }

        if (exceptionObject != nullptr)
        {
            // Note: there also could be plain OutOfMemoryException and StackOverflowException, no special handling for these.
            if (!exceptionObject->IsDebuggerSkip() ||
                exceptionObject == scriptContext->GetThreadContext()->GetPendingOOMErrorObject() ||
                exceptionObject == scriptContext->GetThreadContext()->GetPendingSOErrorObject() ||
                !scriptContext)
            {
                throw exceptionObject->CloneIfStaticExceptionObject(scriptContext);
            }

            if (doCheckParentInterpreterFrame)
            {
                // Note: JavascriptStackWalker is slow, but this is not hot path at all.
                // Note: we treat internal script code (such as Intl) as library code, thus
                //       ignore isLibraryCode=true callers.
                bool isTopUserFrameNative;
                bool isTopUserFrameJavaScript = Js::JavascriptStackWalker::TryIsTopJavaScriptFrameNative(
                    scriptContext, &isTopUserFrameNative, /* ignoreLibraryCode = */ true);
                AssertMsg(isTopUserFrameJavaScript, "How could we get non-javascript frame on exception?");

                if (isTopUserFrameJavaScript && !isTopUserFrameNative)
                {
                    // If parent frame is interpreter frame, it already has try-catch around all calls, 
                    // so that we don't need any special handling here.
                    throw exceptionObject->CloneIfStaticExceptionObject(scriptContext);
                }
            }

            Assert(exceptionObject->IsDebuggerSkip());
            int nextStatementOffset;
            int offsetFromDebugger = exceptionObject->GetByteCodeOffsetAfterDebuggerSkip();
            if (offsetFromDebugger != DebuggingFlags::InvalidByteCodeOffset)
            {
                // The offset is already set for us by debugger (such as by set next statement).
                nextStatementOffset = offsetFromDebugger;
            }
            else 
            {
                ByteCodeReader reader;
                reader.Create(exceptionObject->GetFunctionBody(), exceptionObject->GetByteCodeOffset());
                // Determine offset for next statement here.
                if (!scriptContext->diagProbesContainer.GetNextUserStatementOffsetForAdvance(
                    exceptionObject->GetFunctionBody(), &reader, exceptionObject->GetByteCodeOffset(), &nextStatementOffset)) 
                {
                    // Can't advance.
                    throw exceptionObject->CloneIfStaticExceptionObject(scriptContext);
                }
            }

            // Continue after exception.
            // Note: for this scenario InterpreterStackFrame::DebugProcess resets its state, 
            // looks like we don't need to that because we start with brand new interpreter frame.

            // Indicate to bailout check that we should bail out for/into debugger and set the byte code offset to one of next statement.
            scriptContext->GetThreadContext()->GetDebuggingFlags()->SetByteCodeOffsetAndFuncAfterIgnoreException(
                nextStatementOffset, exceptionObject->GetFunctionBody()->GetFunctionNumber());

        }
        return scriptContext->GetLibrary()->GetUndefined();
    }
}
