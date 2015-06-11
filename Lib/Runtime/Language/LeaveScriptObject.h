//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once


//----------------------------------
// EnterScriptStart/EnterScriptEnd
//----------------------------------
#if defined(DBG) && defined(_M_IX86)
#define SAVE_FS0()  __entryExitRecord.scriptEntryFS0 = (void*)__readfsdword(0)
#else
#define SAVE_FS0()
#endif


#define BEGIN_ENTER_SCRIPT_EX(scriptContext, doCleanup, isCallRoot, hasCaller, isScript) \
        { \
            Js::ScriptContext* __localScriptContext = scriptContext; \
            Js::ScriptEntryExitRecord __entryExitRecord = {0}; \
            SAVE_FS0(); \
            Js::EnterScriptObject __enterScriptObject = Js::EnterScriptObject(__localScriptContext, &__entryExitRecord, \
                _ReturnAddress(), doCleanup, isCallRoot, hasCaller);     \
            __localScriptContext->OnScriptStart(isCallRoot, __localScriptContext->diagProbesContainer.isForcedToEnterScriptStart, isScript); \
            __enterScriptObject.VerifyEnterScript();

#define BEGIN_ENTER_SCRIPT(scriptContext, doCleanup, isCallRoot, hasCaller) \
        BEGIN_ENTER_SCRIPT_EX(scriptContext, doCleanup, isCallRoot, hasCaller, /*isScript*/true) \

#define END_ENTER_SCRIPT \
        }

//---------------------------------------------------------------------
// EnterScriptStart/EnterScriptEnd with javascript exception handling
//---------------------------------------------------------------------
#define BEGIN_JS_RUNTIME_CALL_EX(scriptContext, doCleanup) \
        BEGIN_ENTER_SCRIPT(scriptContext, doCleanup, /*isCallRoot*/ false, /*hasCaller*/false) \
        {

#define BEGIN_JS_RUNTIME_CALLROOT_EX(scriptContext, hasCaller) \
        BEGIN_ENTER_SCRIPT(scriptContext, /*doCleanup*/ true, /*isCallRoot*/ true, hasCaller) \
        {

#define BEGIN_JS_RUNTIME_CALL(scriptContext) \
        BEGIN_JS_RUNTIME_CALL_EX(scriptContext, /*doCleanup*/ true)

// Use _NOT_SCRIPT to indicate we are not really starting script, avoid certain risky/lengthy work.
#define BEGIN_JS_RUNTIME_CALL_NOT_SCRIPT(scriptContext) \
        BEGIN_ENTER_SCRIPT_EX(scriptContext, /*doCleanup*/false, /*isCallRoot*/false, /*hasCaller*/false, /*isScript*/false) \
        {

#define END_JS_RUNTIME_CALL(scriptContext) \
        } \
        END_ENTER_SCRIPT

#define BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, doCleanup) \
        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT \
        BEGIN_ENTER_SCRIPT(scriptContext, doCleanup, /*isCallRoot*/ false, /*hasCaller*/false) \
        { \
        IGNORE_STACKWALK_EXCEPTION(scriptContext); \

#define BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_AND_GET_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, doCleanup) \
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, doCleanup) \

#define BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED(scriptContext, doCleanup) \
        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED \
        BEGIN_ENTER_SCRIPT(scriptContext, doCleanup, /*isCallRoot*/ false, /*hasCaller*/false) \
        { \

#define END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr) \
        } \
        END_ENTER_SCRIPT \
        END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
        END_TRANSLATE_ERROROBJECT_TO_HRESULT(hr) \
        CATCH_UNHANDLED_EXCEPTION(hr)

#define END_JS_RUNTIME_CALL_AND_TRANSLATE_AND_GET_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr, scriptContext, exceptionObject) \
        } \
        END_ENTER_SCRIPT \
        END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
        END_GET_ERROROBJECT(hr, scriptContext, exceptionObject) \
        CATCH_UNHANDLED_EXCEPTION(hr)

#define BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_OOM_TO_HRESULT(scriptContext, doCleanup, hasCaller) \
        BEGIN_TRANSLATE_OOM_TO_HRESULT \
        BEGIN_ENTER_SCRIPT(scriptContext, doCleanup, /*isCallRoot*/ false, hasCaller) \
        {

#define END_JS_RUNTIME_CALL_AND_TRANSLATE_OOM_TO_HRESULT(hr) \
        } \
        END_ENTER_SCRIPT \
        END_TRANSLATE_OOM_TO_HRESULT(hr)

#define END_TRANSLATE_SO_OOM_JSEXCEPTION(hr) \
        } \
        catch (Js::JavascriptExceptionObject *) \
        { \
        } \
        catch (Js::OutOfMemoryException) \
        { \
        } \
        catch (Js::StackOverflowException) \
        { \
        } \
        catch (...) \
        { \
            AssertMsg(false, "this exception didn't get handled"); \
            hr = E_FAIL; \
        } \
    }

#ifdef CHECK_STACKWALK_EXCEPTION
#define IGNORE_STACKWALK_EXCEPTION(scriptContext) \
        scriptContext->SetIgnoreStackWalkException();
#else
#define IGNORE_STACKWALK_EXCEPTION(scriptContext)
#endif

// This is used in the debugging scenario where the execution will go to the PDM and the PDM makes call to the engine again.
// In that scenario we need to enforce the current EER to have 'hasCaller' property set, which will enable the stack walking across frames.
#define ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext) \
        scriptContext->EnforceEERHasCaller();

#define BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT2(scriptContext, doCleanup, hasCaller) \
        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT \
        BEGIN_ENTER_SCRIPT(scriptContext, doCleanup, /*isCallRoot*/ false, /*hasCaller*/hasCaller) \
        { \

// Same as above but allows custom handling of exception object.
#define END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT2(hr, exceptionObject) \
        } \
        END_ENTER_SCRIPT \
        END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
        catch(Js::JavascriptExceptionObject *  exceptionObject)

namespace Js
{
    class EnterScriptObject
    {
    private:
        ScriptEntryExitRecord* entryExitRecord;
        bool doCleanup;
        bool isCallRoot;
        bool hasForcedEnter; // due to debugging.
        ScriptContext* scriptContext;
        HRESULT hr; // we need to throw outside of constructor
        JavascriptLibrary* library;  // stack pin the library.
    public:
        EnterScriptObject(ScriptContext* scriptContext, ScriptEntryExitRecord* entryExitRecord, 
            void * returnAddress, bool doCleanup, bool isCallRoot, bool hasCaller)
        {
#ifdef PROFILE_EXEC
            scriptContext->ProfileBegin(Js::RunPhase);    
#endif
            
            // Keep a copy locally so the optimizer can just copy prop it to the dtor
            this->scriptContext = scriptContext;
            this->entryExitRecord = entryExitRecord;
            this->doCleanup = doCleanup;
            this->isCallRoot = isCallRoot;
            this->hr = NOERROR;
            this->hasForcedEnter = scriptContext->diagProbesContainer.isForcedToEnterScriptStart;

            // Initialize the entry exit record
            entryExitRecord->returnAddrOfScriptEntryFunction = returnAddress;
            entryExitRecord->hasCaller = hasCaller;
            entryExitRecord->scriptContext = scriptContext;
#ifdef EXCEPTION_CHECK
            entryExitRecord->handledExceptionType = ExceptionCheck::ClearHandledExceptionType();
#endif
#if DBG_DUMP
            entryExitRecord->isCallRoot = isCallRoot;            
#endif
            if (!scriptContext->IsClosed())
            {
                library = scriptContext->GetLibrary();
            }
            try
            {
                AUTO_NESTED_HANDLED_EXCEPTION_TYPE(ExceptionType_OutOfMemory);
                scriptContext->GetThreadContext()->PushHostScriptContext(scriptContext->GetHostScriptContext());
            }
            catch (Js::OutOfMemoryException)
            {
                this->hr = E_OUTOFMEMORY;
            }
            BEGIN_NO_EXCEPTION
            {
                // We can not have any exception in the constructor, otherwise the destructor will 
                // not run and we might be in an inconsistent state

                // Put any code that may raise an exception in OnScriptStart
                scriptContext->GetThreadContext()->EnterScriptStart(entryExitRecord, doCleanup);
            }
            END_NO_EXCEPTION
        }

        void VerifyEnterScript()
        {
            if (FAILED(hr))
            {
                Assert(hr == E_OUTOFMEMORY);
                throw Js::OutOfMemoryException();
            }
        }

        ~EnterScriptObject()
        {
            scriptContext->OnScriptEnd(isCallRoot, hasForcedEnter);
            if (SUCCEEDED(hr))
            {
                scriptContext->GetThreadContext()->PopHostScriptContext();
            }
            scriptContext->GetThreadContext()->EnterScriptEnd(entryExitRecord, doCleanup); 
#ifdef PROFILE_EXEC
            scriptContext->ProfileEnd(Js::RunPhase);    
#endif
        }
    };

    template<bool stackProbe, bool leaveForHost, bool isFPUControlRestoreNeeded>
    class LeaveScriptObject
    {
    private:
        ScriptContext *const scriptContext;
        void *const frameAddress;
        bool leftScript;
        SmartFPUControl savedFPUControl;
        DECLARE_EXCEPTION_CHECK_DATA;

    public:
        LeaveScriptObject(ScriptContext *const scriptContext, void *const frameAddress)
            : scriptContext(scriptContext),
            frameAddress(frameAddress),
            savedFPUControl(isFPUControlRestoreNeeded)
        {

            leftScript = scriptContext->LeaveScriptStart<stackProbe, leaveForHost>(frameAddress);

            // We should be in script when we leave
            Assert(leftScript);
            SAVE_EXCEPTION_CHECK;
        }

        ~LeaveScriptObject()
        {
            // We should be in script when we leave
            Assert(leftScript);
            RESTORE_EXCEPTION_CHECK;
            if(leftScript)
            {
                scriptContext->LeaveScriptEnd<leaveForHost>(frameAddress);
            }
        }
    };
}