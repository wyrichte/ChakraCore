//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

#include "Library\ES5Array.h"
#include "ActiveScriptProfilerHeapEnum.h"
#include "edgejsstatic.h"

Js::FunctionInfo DebugObject::EntryInfo::Write(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryWrite));
Js::FunctionInfo DebugObject::EntryInfo::WriteLine(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryWriteLine));
Js::FunctionInfo DebugObject::EntryInfo::GetterSetNonUserCodeExceptions(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetterSetNonUserCodeExceptions), Js::FunctionInfo::DoNotProfile);
Js::FunctionInfo DebugObject::EntryInfo::SetterSetNonUserCodeExceptions(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntrySetterSetNonUserCodeExceptions), Js::FunctionInfo::DoNotProfile);
Js::FunctionInfo DebugObject::EntryInfo::GetterDebuggerEnabled(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetterDebuggerEnabled), Js::FunctionInfo::DoNotProfile);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
Js::FunctionInfo DebugObject::EntryInfo::GetWorkingSet(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetWorkingSet));
Js::FunctionInfo DebugObject::EntryInfo::SourceDebugBreak(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntrySourceDebugBreak));
Js::FunctionInfo DebugObject::EntryInfo::InvokeFunction(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryInvokeFunction));
Js::FunctionInfo DebugObject::EntryInfo::GetHostInfo(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetHostInfo));
Js::FunctionInfo DebugObject::EntryInfo::GetMemoryInfo(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetMemoryInfo));
Js::FunctionInfo DebugObject::EntryInfo::GetTypeHandlerName(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetTypeHandlerName));
Js::FunctionInfo DebugObject::EntryInfo::GetArrayType(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetArrayType));
Js::FunctionInfo DebugObject::EntryInfo::DumpHeap(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::DumpHeapInternal));
Js::FunctionInfo DebugObject::EntryInfo::CreateDebugDisposableObject(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryCreateDebugDisposableObject));
Js::FunctionInfo DebugObject::EntryInfo::IsInJit(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryIsInJit));
Js::FunctionInfo DebugObject::EntryInfo::GetCurrentSourceInfo(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetCurrentSourceInfo));
Js::FunctionInfo DebugObject::EntryInfo::GetLineOfPosition(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetLineOfPosition));
Js::FunctionInfo DebugObject::EntryInfo::GetPositionOfLine(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetPositionOfLine));
Js::FunctionInfo DebugObject::EntryInfo::AddFTLProperty(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryAddFTLProperty));
Js::FunctionInfo DebugObject::EntryInfo::AddLazyFTLProperty(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryAddLazyFTLProperty));
Js::FunctionInfo DebugObject::EntryInfo::CreateTypedObject(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryCreateTypedObject));
Js::FunctionInfo DebugObject::EntryInfo::CreateProjectionArrayBuffer(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryCreateProjectionArrayBuffer));
Js::FunctionInfo DebugObject::EntryInfo::EmitStackTraceEvent(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryEmitStackTraceEvent));
Js::FunctionInfo DebugObject::EntryInfo::GetTypeInfo(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetTypeInfo));
Js::FunctionInfo DebugObject::EntryInfo::ParseFunction(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryParseFunction));
Js::FunctionInfo DebugObject::EntryInfo::SetAutoProxyName(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntrySetAutoProxyName));
Js::FunctionInfo DebugObject::EntryInfo::DisableAutoProxy(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryDisableAutoProxy));
Js::FunctionInfo DebugObject::EntryInfo::CreateDebugFuncExecutorInDisposeObject(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryCreateDebugFuncExecutorInDisposeObject)); 
Js::FunctionInfo DebugObject::EntryInfo::DetachAndFreeObject(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::DetachAndFreeObject));
Js::FunctionInfo DebugObject::EntryInfo::IsAsmJSModule(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryIsAsmJSModule));
Js::FunctionInfo DebugObject::EntryInfo::Enable(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryEnable));
Js::FunctionInfo DebugObject::EntryInfo::DisableImplicitCalls(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryDisableImplicitCalls));
Js::FunctionInfo DebugObject::EntryInfo::EnableImplicitCalls(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryEnableImplicitCalls));

#else
#ifdef ENABLE_HEAP_DUMPER
Js::FunctionInfo DebugObject::EntryInfo::DumpHeap(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::DumpHeap));
#endif
#endif
#if JS_PROFILE_DATA_INTERFACE
Js::FunctionInfo DebugObject::EntryInfo::GetProfileDataObject(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetProfileDataObject));
#endif

#if ENABLE_DEBUG_CONFIG_OPTIONS
DebugDisposableObject::DebugDisposableObject(Js::DynamicType* type, Js::ScriptContext* scriptContext,
    bool collectOnDispose, uint bytesToAllocateOnDispose, bool allocateLeaf, uint allocationCount):
RecyclableObject(type),
collectOnDispose(collectOnDispose),
bytesToAllocateOnDispose(bytesToAllocateOnDispose),
allocateLeaf(allocateLeaf),
allocationCount(allocationCount)
{
}

void DebugDisposableObject::Dispose(bool isShutdown) 
{ 
    ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

    if (!isShutdown && threadContext)
    {
        Recycler* recycler = threadContext->GetRecycler();

        if (collectOnDispose)
        {
            recycler->CollectNow<CollectNowForceInThread>();
        }

        if (bytesToAllocateOnDispose > 0)
        {
            HRESULT hr = S_OK;

            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                for (uint i = 0; i < allocationCount; i++)
                {
                    volatile byte* bytes = nullptr;
            
                    if (allocateLeaf)
                    {
                        bytes = RecyclerNewArrayLeaf(recycler, byte, bytesToAllocateOnDispose);
                    }
                    else
                    {
                        bytes = (byte*) RecyclerNewFinalizedPlus(recycler, this->bytesToAllocateOnDispose, DebugDisposableObject,
                            (Js::DynamicType*)PointerValue(this->type), this->GetScriptContext(), false, false, false, 0);
                    }

                    // Free it out right now, it'll get collected in the next cycle
                    bytes = nullptr;
                }
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr);
        }
    }
}

DebugFuncExecutorInDisposeObject::DebugFuncExecutorInDisposeObject(Js::DynamicType* type, 
    Js::JavascriptFunction * functionToCall, Js::Var* args, ushort numberOfArgs) :
DynamicObject(type),
functionToCall(functionToCall),
args(args),
numberOfArgs(numberOfArgs)
{
    if (functionToCall != nullptr)
    {
        Recycler *recycler = type->GetScriptContext()->GetRecycler();
        // Pin functionToCall and Args so they don't get collected till we dispose this object
        recycler->RootAddRef(functionToCall);
        AssertMsg(args != nullptr, "DebugFuncExecutorInDisposeObject's Dispose doesn't have valid arguments to pass to function.");
        recycler->RootAddRef(args);
        this->areAllParametersPinned = true;
    }
}

void DebugFuncExecutorInDisposeObject::Dispose(bool isShutdown)
{
    // Check if we have pinned valid function and args
    if (areAllParametersPinned)
    {
        Js::ScriptContext* scriptContext = functionToCall->GetScriptContext();

        // If script context is still alive, call the function
        if (!scriptContext->IsClosed())
        {
            AssertMsg(functionToCall != nullptr, "DebugFuncExecutorInDisposeObject's Dispose doesn't have valid function to execute.");
            AssertMsg(args != nullptr, "DebugFuncExecutorInDisposeObject's Dispose doesn't have valid arguments to pass to function.");

            Js::Arguments jsArguments(Js::CallInfo(Js::CallFlags_Value, numberOfArgs), args);

            HRESULT hr = NOERROR;
            BEGIN_TRANSLATE_OOM_TO_HRESULT
            for (unsigned int i = 0; i < jsArguments.Info.Count; i++)
            {
                jsArguments.Values[i] = Js::CrossSite::MarshalVar(scriptContext, jsArguments.Values[i]);
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr);

            Var result = nullptr;
            ScriptSite::CallRootFunction(functionToCall, jsArguments, nullptr, &result);
            
        }

        // unpin the functionCall and args that we pinned in this object's constructor
        Recycler *recycler = scriptContext->GetRecycler();
        recycler->RootRelease(functionToCall);
        recycler->RootRelease(args);
    }
    DynamicObject::Dispose(isShutdown);
}

#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
Js::FunctionInfo DebugObject::EntryInfo::GetterFaultInjectionCookie(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntryGetterFaultInjectionCookie), Js::FunctionInfo::DoNotProfile);
Js::FunctionInfo DebugObject::EntryInfo::SetterFaultInjectionCookie(FORCE_NO_WRITE_BARRIER_TAG(DebugObject::EntrySetterFaultInjectionCookie), Js::FunctionInfo::DoNotProfile);
#endif


Js::Var DebugObject::EntryWrite(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);
    return WriteHelper(function, args, false);
}

Js::Var DebugObject::EntryWriteLine(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);
    return WriteHelper(function, args, true);
}

Js::FunctionBody * DebugObject::GetCallerFunctionBody(Js::ScriptContext *scriptContext)
{
    // Find the current function scope.
    Js::JavascriptFunction* caller = nullptr;
    Js::FunctionBody *funcBody = NULL;
    if (Js::JavascriptStackWalker::GetCaller(&caller, scriptContext))
    {
        if (caller != NULL)
        {
            funcBody = caller->GetFunctionBody();
        }
    }

    return funcBody;
}

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
Js::Var DebugObject::EntryGetterFaultInjectionCookie(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    ARGUMENTS(args, callInfo);    

    int cookie = 0;

#ifdef FAULT_INJECTION // no action for free test build because faultinjection is not enabled in fretest build

    // for fault injection self testing
    // command: 'jshost -FaultInjection:1 -FaultInjectionType:6 xx.js' will show OOM
    // xx.js: 
    //  Debug.faultInjectionCookie = 12345;
    //  WScript.Echo(Debug.faultInjectionCookie);
    INJECT_FAULT(Js::FaultInjection::Global.FaultInjectionSelfTest, []()->bool{
        return Js::FaultInjection::Global.FaultInjectionCookie == 12345;
    }, {
        Output::Print(_u("Fault Injected!"));
        Output::Flush();
        Js::Throw::OutOfMemory();
    });

    cookie = Js::FaultInjection::Global.FaultInjectionCookie;

#endif 

    return Js::TaggedInt::ToVarUnchecked(cookie);
}

Js::Var DebugObject::EntrySetterFaultInjectionCookie(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count >= 2)
    {
#ifdef FAULT_INJECTION // no action for free test build because faultinjection is not enabled in fretest build
        Js::FaultInjection::Global.FaultInjectionCookie = Js::JavascriptConversion::ToInt32(args[1], scriptContext);
#endif 
        return args[1];
    }

    return scriptContext->GetLibrary()->GetUndefined();    
}
#endif //ENABLE_DEBUG_CONFIG_OPTIONS

Js::Var DebugObject::EntryGetterSetNonUserCodeExceptions(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (scriptSite)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
        if (scriptEngine && scriptEngine->IsDebuggerEnvironmentAvailable())
        {
            Js::FunctionBody * pBody = GetCallerFunctionBody(scriptContext);
            return Js::JavascriptBoolean::ToVar((pBody && pBody->IsNonUserCode() ? TRUE : FALSE), scriptContext);
        }
    }

    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var DebugObject::EntrySetterSetNonUserCodeExceptions(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (scriptSite)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
        if (scriptEngine && scriptEngine->IsDebuggerEnvironmentAvailable())
        {
            if (args.Info.Count >= 2)
            {
                if (Js::JavascriptConversion::ToBoolean(args[1], scriptContext) != FALSE)
                {
                    Js::FunctionBody * pBody = GetCallerFunctionBody(scriptContext);
                    if (pBody)
                    {
                        pBody->SetIsNonUserCode(true);
                    }
                }

                return args[1];
            }
        }
    }

    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var DebugObject::EntryGetterDebuggerEnabled(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    return Js::JavascriptBoolean::ToVar(scriptContext->IsScriptContextInDebugMode(), scriptContext);
}

Js::Var DebugObject::WriteHelper(Js::RecyclableObject* function, Js::Arguments args, bool newLine)
{
    Js::ScriptContext* scriptContext = function->GetScriptContext();
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (scriptSite)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();

        IDebugApplication *pda;
        if (!scriptEngine || FAILED(scriptEngine->GetDebugApplicationCoreNoRef(&pda)))
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }

        for (uint i=1; i<args.Info.Count; i++)
        {

            try
            {
                Js::JavascriptString *value = Js::JavascriptConversion::ToString(args[i], scriptContext);
                const char16 * str = value->GetSz(); // flatten it before leave script

                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    pda->DebugOutput(str);
                }
                END_LEAVE_SCRIPT(scriptContext)
            }
            catch (const Js::JavascriptException& err)
            {
                err.GetAndClear();  // discard exception object
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    pda->DebugOutput(_u("[error]"));
                }
                END_LEAVE_SCRIPT(scriptContext)
            }
        }

        if (newLine)
        {
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                pda->DebugOutput(_u("\n"));
            }
            END_LEAVE_SCRIPT(scriptContext)
        }
    }

    return scriptContext->GetLibrary()->GetUndefined();
}

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
Js::Var DebugObject::EntryGetWorkingSet(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New)); 

    HANDLE hProcess = GetCurrentProcess();
    Js::JavascriptLibrary* library = function->GetLibrary();
    Js::ScriptContext* scriptContext = function->GetScriptContext();
    PROCESS_MEMORY_COUNTERS_EX memoryCounter;
    memoryCounter.cb = sizeof(memoryCounter);

    PsapiLibrary psapiLib;
    if (!psapiLib.GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&memoryCounter,sizeof(memoryCounter)))
    {
        return library->GetNull();
    }
    Js::DynamicObject* result = library->CreateObject();

    Js::PropertyId workingSetId = scriptContext->GetOrAddPropertyIdTracked(_u("workingSet"), wcslen(_u("workingSet")));
    Js::PropertyId maxWorkingSetId = scriptContext->GetOrAddPropertyIdTracked(_u("maxWorkingSet"), wcslen(_u("maxWorkingSet")));
    Js::PropertyId pageFaultId = scriptContext->GetOrAddPropertyIdTracked(_u("pageFault"), wcslen(_u("pageFault")));
    Js::PropertyId privateUsageId = scriptContext->GetOrAddPropertyIdTracked(_u("privateUsage"), wcslen(_u("privateUsage")));

    result->SetProperty(workingSetId, Js::JavascriptNumber::New((double)memoryCounter.WorkingSetSize, scriptContext), Js::PropertyOperation_None, NULL);
    result->SetProperty(maxWorkingSetId, Js::JavascriptNumber::New((double)memoryCounter.PeakWorkingSetSize, scriptContext), Js::PropertyOperation_None, NULL);
    result->SetProperty(pageFaultId, Js::JavascriptNumber::New((double)memoryCounter.PageFaultCount, scriptContext), Js::PropertyOperation_None , NULL);
    result->SetProperty(privateUsageId, Js::JavascriptNumber::New((double)memoryCounter.PrivateUsage, scriptContext),Js::PropertyOperation_None , NULL);
    
    return result;
}

Js::Var DebugObject::EntrySourceDebugBreak(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New)); 

    BEGIN_LEAVE_SCRIPT(function->GetScriptContext())
    {
        DebugBreak();
    }
    END_LEAVE_SCRIPT(function->GetScriptContext())

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    return scriptContext->GetLibrary()->GetUndefined();
}

__declspec(noinline) Js::Var InvokeFunc(Js::JavascriptFunction* function, Js::Arguments args)
{
    Js::Var result;
    BEGIN_JS_RUNTIME_CALL(function->GetScriptContext())
    {
        result = function->CallFunction(args);
    }
    END_JS_RUNTIME_CALL(function->GetScriptContext())
    return result;
}

Js::Var DebugObject::EntryInvokeFunction(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New)); 
    ARGUMENTS(args, callInfo);
    Js::Var result = function->GetScriptContext()->GetLibrary()->GetUndefined();

    BEGIN_LEAVE_SCRIPT(function->GetScriptContext())
    {
        if(args.Info.Count >= 2)
        {
            Js::JavascriptFunction* invokeFunction = Js::JavascriptFunction::FromVar(args[1]);
            for (uint i=0; i<args.Info.Count-2; ++i)
            {
                args.Values[i] = args.Values[i+2];
            }
            args.Info.Count  = args.Info.Count - 1;
            InvokeFunc(invokeFunction, args);
        }
    }
    END_LEAVE_SCRIPT(function->GetScriptContext())
    return result;
}

Js::Var DebugObject::EntryGetCurrentSourceInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Assert(!(callInfo.Flags & CallFlags_New));

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    Js::Var result = scriptContext->GetLibrary()->GetUndefined();

    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (scriptSite)
    {
        ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
        if (scriptEngine)
        {
            IActiveScriptDirect* pScriptDirect = scriptEngine;
            HRESULT hr;
            LPCWSTR url = nullptr;
            ULONG line, column;

            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = pScriptDirect->GetCurrentSourceInfo(&url, &line, &column);
            }
            END_LEAVE_SCRIPT(scriptContext);

            if (FAILED(hr))
            {
                Js::JavascriptError::MapAndThrowError(scriptContext, hr);
            }

            Js::DynamicObject* obj = scriptContext->GetLibrary()->CreateObject();
            Js::Var urlVar = url ?
                Js::JavascriptString::NewCopyBuffer(url, wcslen(url), scriptContext) :
                scriptContext->GetLibrary()->GetNullString();

            Js::JavascriptOperators::SetProperty(obj, obj,
                scriptContext->GetOrAddPropertyIdTracked(_u("url"), wcslen(_u("url"))),
                urlVar,
                scriptContext);
            Js::JavascriptOperators::SetProperty(obj, obj,
                scriptContext->GetOrAddPropertyIdTracked(_u("line"), wcslen(_u("line"))),
                Js::JavascriptNumber::ToVar(static_cast<uint32>(line), scriptContext),
                scriptContext);
            Js::JavascriptOperators::SetProperty(obj, obj,
                scriptContext->GetOrAddPropertyIdTracked(_u("column"), wcslen(_u("column"))),
                Js::JavascriptNumber::ToVar(static_cast<uint32>(column), scriptContext),
                scriptContext);

            result = obj;
        }
    }

    return result;
}

Js::Var DebugObject::EntryGetLineOfPosition(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Assert(!(callInfo.Flags & CallFlags_New));

    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count != 2 || !Js::JavascriptFunction::Is(args[1]))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    Js::JavascriptFunction* javascriptFunction = Js::JavascriptFunction::FromVar(args[1]);

    CComPtr<IDebugDocumentContext> pDocumentContext = NULL;
    scriptContext->GetDocumentContext(javascriptFunction->GetFunctionBody(), &pDocumentContext);

    Js::Var result = scriptContext->GetLibrary()->GetUndefined();

    if (pDocumentContext != nullptr)
    {
        CComPtr<IDebugDocumentContext> pDebugDocumentContext;
        HRESULT hr = pDocumentContext->QueryInterface(__uuidof(IDebugDocumentContext), reinterpret_cast<void **>(&pDebugDocumentContext));
        if (SUCCEEDED(hr))
        {
            CComPtr<IDebugDocument> pDocument;
            hr = pDebugDocumentContext->GetDocument(&pDocument);
            if (SUCCEEDED(hr))
            {
                CComPtr<IDebugDocumentText> pDocumentText;
                hr = pDocument->QueryInterface(__uuidof(IDebugDocumentText), (void **)&pDocumentText);

                if (SUCCEEDED(hr))
                {
                    // Find the character position of this script function in the main source
                    ULONG cCharPosition = 0;
                    ULONG cNumChars = 0;
                    IfFailGo(pDocumentText->GetPositionOfContext(pDebugDocumentContext, &cCharPosition, &cNumChars));

                    // Ask PDM for the line/column offset.
                    ULONG line = 0;
                    ULONG column = 0;
                    IfFailGo(pDocumentText->GetLineOfPosition(cCharPosition, &line, &column));

                    // Create the object to return to the caller.
                    Js::DynamicObject* obj = scriptContext->GetLibrary()->CreateObject();

                    Js::JavascriptOperators::SetProperty(obj, obj,
                        scriptContext->GetOrAddPropertyIdTracked(_u("line"), wcslen(_u("line"))),
                        Js::JavascriptNumber::ToVar(static_cast<uint32>(line), scriptContext),
                        scriptContext);

                    Js::JavascriptOperators::SetProperty(obj, obj,
                        scriptContext->GetOrAddPropertyIdTracked(_u("column"), wcslen(_u("column"))),
                        Js::JavascriptNumber::ToVar(static_cast<uint32>(column), scriptContext),
                        scriptContext);

                    result = obj;
                }
            }
        }
    }

Error:
    return result;
}

Js::Var DebugObject::EntryGetPositionOfLine(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Assert(!(callInfo.Flags & CallFlags_New));

    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count != 3 || !Js::JavascriptFunction::Is(args[1]) || !Js::JavascriptNumber::Is(args[2]))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    Js::JavascriptFunction* javascriptFunction = Js::JavascriptFunction::FromVar(args[1]);

    IDebugDocumentContext *pDocumentContext = NULL;
    scriptContext->GetDocumentContext(javascriptFunction->GetFunctionBody(), &pDocumentContext);

    Js::Var result = scriptContext->GetLibrary()->GetUndefined();
    if (pDocumentContext != nullptr)
    {
        IDebugDocumentContext *pDebugDocumentContext = nullptr;
        HRESULT hr = pDocumentContext->QueryInterface(__uuidof(IDebugDocumentContext), reinterpret_cast<void **>(&pDebugDocumentContext));
        if (SUCCEEDED(hr))
        {
            IDebugDocument *pDocument = NULL;
            hr = pDebugDocumentContext->GetDocument(&pDocument);
            if (SUCCEEDED(hr))
            {
                IDebugDocumentText *pDocumentText = NULL;
                hr = pDocument->QueryInterface(__uuidof(IDebugDocumentText), (void **)&pDocumentText);

                if (SUCCEEDED(hr))
                {
                    // Ask PDM for the character position of a line.
                    ULONG line = (ULONG)Js::JavascriptConversion::ToNumber(args[2], scriptContext);
                    ULONG characterPosition;
                    pDocumentText->GetPositionOfLine(line, &characterPosition);

                    result = Js::JavascriptNumber::ToVar(static_cast<uint32>(characterPosition), scriptContext);

                    pDocumentText->Release();
                }
                pDocument->Release();
            }
            pDebugDocumentContext->Release();
        }
        pDocumentContext->Release();
    }

    return result;
}

Js::Var DebugObject::EntryIsInJit(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    bool isTopFrameNative;
    bool isTopFrameJavaScript = Js::JavascriptStackWalker::TryIsTopJavaScriptFrameNative(function->GetScriptContext(), &isTopFrameNative);
    if (isTopFrameJavaScript)
    {
        return isTopFrameNative ?
            function->GetLibrary()->GetTrue() :
            function->GetLibrary()->GetFalse();
    }
    Js::Throw::FatalInternalError();
}

Js::Var DebugObject::EntryGetHostInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New)); 
    Js::JavascriptLibrary* library = function->GetLibrary();
    Js::ScriptContext* scriptContext = function->GetScriptContext();
    scriptContext->GetRecycler()->CollectNow<CollectNowForceInThread>();

    Js::DynamicObject* result = library->CreateObject();

    Js::PropertyId activeSiteCountId = scriptContext->GetOrAddPropertyIdTracked(_u("activeSiteCount"), wcslen(_u("activeSiteCount")));
    result->SetProperty(activeSiteCountId, Js::JavascriptNumber::New((double)ThreadContext::GetScriptSiteHolderCount(), scriptContext), Js::PropertyOperation_None, NULL);

    Js::ScriptContext* contextList = scriptContext->GetThreadContext()->GetScriptContextList();
    uint index = 0;
    LPCWSTR url;
    while (contextList != NULL)
    {
        url = contextList->GetUrl();
        result->SetItem(index, Js::JavascriptString::NewWithArenaSz(url, scriptContext), Js::PropertyOperation_None);
        index++;
        contextList = contextList->next;
    }
    result->SetProperty(Js::PropertyIds::length, Js::JavascriptNumber::New(index, scriptContext), Js::PropertyOperation_None, NULL);
    return result;
}

Js::Var DebugObject::EntryGetMemoryInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New)); 
    Js::ScriptContext* scriptContext = function->GetScriptContext();
    scriptContext->GetRecycler()->CollectNow<CollectNowForceInThread>();

    return scriptContext->GetThreadContext()->GetMemoryStat(scriptContext);
}

Js::Var DebugObject::EntryGetTypeHandlerName(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New));

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count != 2)
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    if (Js::JavascriptOperators::IsObject(args[1]) && Js::RecyclableObject::FromVar(args[1])->GetTypeId() != TypeIds_HostDispatch)
    {
        Js::DynamicObject* obj = Js::DynamicObject::FromVar(args[1]);
        char16 name[256];
        size_t size;
        if (mbstowcs_s(&size, name, obj->GetDynamicType()->GetTypeHandler()->GetCppName(), _TRUNCATE) != 0)
        {
            Js::Throw::FatalInternalError();
        }
        charcount_t length = size - 1; // size includes null character
        return Js::JavascriptString::NewCopyBuffer(name, length, scriptContext);
    }
    else
    {
        return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("The argument does not have a type handler"));
    }
}

Js::Var DebugObject::EntryGetArrayType(Js::RecyclableObject* function, Js::CallInfo callInfo, ...) 
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New));

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count != 2)
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    if (Js::JavascriptOperators::IsObject(args[1]))
    {
        Js::TypeId typeIdOfObject = Js::JavascriptOperators::GetTypeId(args[1]);
        if (typeIdOfObject == TypeIds_HostDispatch) {
            if (Js::RecyclableObject::FromVar(args[1])->GetRemoteTypeId(&typeIdOfObject) == FALSE)
                return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("Notarray"));
        }

        switch (typeIdOfObject)
        {
        case Js::TypeIds_Array:
            return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("var[]"));
            break;
        case Js::TypeIds_NativeIntArray:
        case Js::TypeIds_CopyOnAccessNativeIntArray:
            return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("int[]"));
            break;
        case Js::TypeIds_NativeFloatArray:
            return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("float[]"));
            break;
        case Js::TypeIds_ES5Array:
            return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("ES5[]"));
            break;
        case Js::TypeIds_Uint8Array:
        case Js::TypeIds_Int16Array:
        case Js::TypeIds_Uint16Array:
        case Js::TypeIds_Int32Array:
        case Js::TypeIds_Uint32Array:
        case Js::TypeIds_Float32Array:
        case Js::TypeIds_Float64Array:
        case Js::TypeIds_Int64Array:
        case Js::TypeIds_Uint64Array:
        case Js::TypeIds_CharArray:
        case Js::TypeIds_BoolArray:
            return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("Typed[]"));
        default:
            return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("Notarray"));
            break;
        }

    }
    else
    {
        return scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("The argument is not a valid object"));
    }
}

IASDDebugObjectHelper* DebugObject::EnsureDebugObjectHelper(Js::ScriptContext* scriptContext)
{
    ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
    IASDDebugObjectHelper* helper = scriptSite->GetDebugObjectHelper();
    if (helper == nullptr)
    {
        helper = HeapNew(IASDDebugObjectHelper, scriptContext->GetActiveScriptDirect());
        scriptSite->SetDebugObjectHelper(helper);
    }
    return helper;
}

//  createTypedObject(typeId, className, extension #, useDefaultTypedOperations)
Js::Var DebugObject::EntryCreateTypedObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...) 
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New));

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    if (args.Info.Count < 4)
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    if (scriptContext->IsClosed())
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
    if (!Js::TaggedInt::Is(args[1]) || !Js::JavascriptString::Is(args[2]) || !Js::TaggedInt::Is(args[3]))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    JavascriptTypeId typeId = (JavascriptTypeId)(Js::TaggedInt::ToInt32(args[1]));
    Js::JavascriptString* className = Js::JavascriptString::FromVar(args[2]);
    int extensionSize = Js::TaggedInt::ToInt32(args[3]);
    bool useDefaultTypeOperations =false;
    if (args.Info.Count > 4)
    {
        useDefaultTypeOperations = Js::JavascriptConversion::ToBool(args[4], scriptContext);
    }
    IActiveScriptDirect* scriptDirect = scriptContext->GetActiveScriptDirect();
    Js::PropertyId nameId = scriptContext->GetOrAddPropertyIdTracked(className->GetSz(), className->GetLength());
    IASDDebugObjectHelper* helper = DebugObject::EnsureDebugObjectHelper(scriptContext);
    HTYPE htype = helper->EnsureType(typeId, nameId, useDefaultTypeOperations);

    HRESULT hr;
    Var result;
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = scriptDirect->CreateTypedObject(htype, extensionSize, true /*bindReference*/, &result);
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (SUCCEEDED(hr))
    {
        return result;
    }
    else
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
}


//  createProjectionArrayBuffer(buffer, length)
Js::Var DebugObject::EntryCreateProjectionArrayBuffer(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count < 2)
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    
    if (scriptContext->IsClosed())
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    uint32 length = Js::TaggedInt::ToUInt32(args[1]);
    
    return scriptContext->GetLibrary()->CreateProjectionArraybuffer(length);
}

//  Initiate the EmitStackTraceEvent call
//  emitStackTraceEvent(operationId, maxFrameCount), both params are optional
Js::Var DebugObject::EntryEmitStackTraceEvent(Js::RecyclableObject* function, Js::CallInfo callInfo, ...) 
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New));

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    if (scriptContext->IsClosed())
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    UINT64 operationId = 54321; // Some random ID
    USHORT maxFrameCount = JSCRIPT_FULL_STACKTRACE;
    if (args.Info.Count > 1)
    {
        if (!Js::TaggedInt::Is(args[1]))
        {
            Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
        }
        operationId = Js::TaggedInt::ToInt64(args[1]);
    }

    if (args.Info.Count > 2)
    {
        if (!Js::TaggedInt::Is(args[2]))
        {
            Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
        }
        maxFrameCount = Js::TaggedInt::ToUInt16(args[2]);
    }

    IActiveScriptDirect* scriptDirect = scriptContext->GetActiveScriptDirect();
    HRESULT hr;
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = scriptDirect->EmitStackTraceEvent(operationId, maxFrameCount);
        hr; // Omitting result
    }
    END_LEAVE_SCRIPT(scriptContext);

    return scriptContext->GetLibrary()->GetUndefined();
}


// arguments
//   addFTLProperty(obj, propertyName, index, initValue);
Js::Var DebugObject::EntryAddFTLProperty(Js::RecyclableObject* function, Js::CallInfo callInfo, ...) 
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New));

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    if (args.Info.Count < 4)
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    Js::RecyclableObject* obj = Js::RecyclableObject::FromVar(args[1]);
    if (Js::StaticType::Is(Js::JavascriptOperators::GetTypeId(obj)))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    Js::Var propertyName = args[2];

    if (!Js::JavascriptString::Is(propertyName))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    Js::Var indexVar = args[3];
    Js::Var value;
    int index = 0;
    if (!Js::TaggedInt::Is(indexVar))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    index = Js::TaggedInt::ToInt32(indexVar);

    if (args.Info.Count > 4)
    {
        value = args[4];
    }
    else
    {
        value = Js::TaggedInt::ToVarUnchecked(0);
    }
    IActiveScriptDirect* scriptDirect = scriptContext->GetActiveScriptDirect();
    if (scriptContext->IsClosed())
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
    Js::PropertyId propertyId;
    Js::Var getter = scriptContext->GetLibrary()->GetUndefined();
    Js::Var setter = scriptContext->GetLibrary()->GetUndefined();
    Js::JavascriptString* propertyString = Js::JavascriptString::FromVar(propertyName);
    propertyId = scriptContext->GetOrAddPropertyIdTracked(propertyString->GetSz(), propertyString->GetLength());
    HRESULT hr;

    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = scriptDirect->GetTypedObjectSlotAccessor(Js::JavascriptOperators::GetTypeId(obj), propertyId, index, &getter, &setter);
    }
    END_LEAVE_SCRIPT(scriptContext);

    switch (hr)
    {
    case S_OK:
        break;
    case E_OUTOFMEMORY:
        Js::JavascriptError::ThrowOutOfMemoryError(scriptContext);
    case E_INVALIDARG:
        return scriptContext->GetLibrary()->GetUndefined();
    default:
        AssertMsg(UNREACHED, "GetTypedObjectSlotAccessor returned unexpected HRESULT");
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Js::RecyclableObject* protoObj = Js::JavascriptOperators::GetPrototype(obj);
    if (!Js::StaticType::Is(Js::JavascriptOperators::GetTypeId(protoObj)))
    {
        Js::JavascriptOperators::SetAccessors(Js::DynamicObject::FromVar(protoObj), propertyId, getter, setter, Js::PropertyOperation_None);
    }
    else
    {
        Js::JavascriptOperators::SetAccessors(Js::DynamicObject::FromVar(obj), propertyId, getter, setter, Js::PropertyOperation_None);
    }
    Js::Var outArgs[2];
    outArgs[0] = obj;
    outArgs[1] = value;
    Js::Arguments jsArguments(0, nullptr);
    jsArguments.Info.Count = callInfo.Count;
    jsArguments.Info.Flags = (Js::CallFlags)callInfo.Flags;
    jsArguments.Values = (Js::Var*)outArgs;

    Js::JavascriptFunction::CallFunction<true>(Js::JavascriptFunction::FromVar(setter), (Js::JavascriptFunction::FromVar(setter))->GetEntryPoint(), jsArguments);

    return getter;
}

static Var getterTrampoline(Var function, CallInfo callInfo, Var *args)
{
    return Js::TaggedInt::ToVarUnchecked(1);
}
static Var setterTrampoline(Var function, CallInfo callInfo, Var *args)
{
    return function;
}

// arguments
//   addLazyFTLProperty(obj, propertyName, index);
Js::Var DebugObject::EntryAddLazyFTLProperty(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New));

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    if (args.Info.Count < 4)
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    Js::RecyclableObject* obj = Js::RecyclableObject::FromVar(args[1]);
    if (Js::StaticType::Is(Js::JavascriptOperators::GetTypeId(obj)))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    Js::Var propertyName = args[2];

    if (!Js::JavascriptString::Is(propertyName))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    Js::Var indexVar = args[3];
    int index = 0;
    if (!Js::TaggedInt::Is(indexVar))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }
    index = Js::TaggedInt::ToInt32(indexVar);

    if (scriptContext->IsClosed())
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
    Js::PropertyId propertyId;
    Js::Var getter = scriptContext->GetLibrary()->GetUndefined();
    Js::Var setter = scriptContext->GetLibrary()->GetUndefined();
    Js::JavascriptString* propertyString = Js::JavascriptString::FromVar(propertyName);
    propertyId = scriptContext->GetOrAddPropertyIdTracked(propertyString->GetSz(), propertyString->GetLength());
    HRESULT hr;

    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        hr = scriptEngine->GetObjectSlotAccessor(Js::JavascriptOperators::GetTypeId(obj), propertyId, index, &getterTrampoline, &setterTrampoline, &getter, &setter);
    }
    END_LEAVE_SCRIPT(scriptContext);

    switch (hr)
    {
    case S_OK:
        break;
    case E_OUTOFMEMORY:
        Js::JavascriptError::ThrowOutOfMemoryError(scriptContext);
    case E_INVALIDARG:
        return scriptContext->GetLibrary()->GetUndefined();
    default:
        AssertMsg(UNREACHED, "GetTypedObjectSlotAccessor returned unexpected HRESULT");
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Js::RecyclableObject* protoObj = Js::JavascriptOperators::GetPrototype(obj);
    if (!Js::StaticType::Is(Js::JavascriptOperators::GetTypeId(protoObj)))
    {
        Js::JavascriptOperators::SetAccessors(Js::DynamicObject::FromVar(protoObj), propertyId, getter, setter, Js::PropertyOperation_None);
    }
    else
    {
        Js::JavascriptOperators::SetAccessors(Js::DynamicObject::FromVar(obj), propertyId, getter, setter, Js::PropertyOperation_None);
    }
    Js::Var outArgs[2];
    outArgs[0] = obj;
    Js::Arguments jsArguments(0, nullptr);
    jsArguments.Info.Count = callInfo.Count;
    jsArguments.Info.Flags = (Js::CallFlags)callInfo.Flags;
    jsArguments.Values = (Js::Var*)outArgs;

    Js::JavascriptFunction::CallFunction<true>(Js::JavascriptFunction::FromVar(setter), (Js::JavascriptFunction::FromVar(setter))->GetEntryPoint(), jsArguments);

    return getter;
}

Js::Var DebugObject::EntryDisableImplicitCalls(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ThreadContext* threadContext = function->GetScriptContext()->GetThreadContext();

    threadContext->DisableImplicitCall();

    return function;
}

Js::Var DebugObject::EntryEnableImplicitCalls(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    ThreadContext* threadContext = function->GetScriptContext()->GetThreadContext();

    threadContext->ClearDisableImplicitFlags();

    return function;
}

Js::Var DebugObject::EntryGetTypeInfo(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    HRESULT hr = NOERROR;
    ARGUMENTS(args, callInfo);
    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count <= 1)
    {
        Js::JavascriptError::ThrowError(scriptContext, VBSERR_TypeMismatch);
    }

    if (args.Info.Count < 2)
    {
        return function;
    }

    Var obj = args.Values[1];
    if (!Js::DynamicType::Is(Js::JavascriptOperators::GetTypeId(obj)))
    {
        return function;
    }
    CComPtr<IDispatchEx> dispPtr = nullptr;
    dispPtr = (IDispatchEx*)JavascriptDispatch::Create<true>(Js::DynamicObject::FromVar(obj));
    if (dispPtr != nullptr)
    {
        CComPtr<ITypeInfo> typeInfo;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = dispPtr->GetTypeInfo(0, 0x409, &typeInfo);
        }
        END_LEAVE_SCRIPT(scriptContext);
        if (SUCCEEDED(hr))
        {
            UINT count;
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = dispPtr->GetTypeInfoCount(&count);
            }
            END_LEAVE_SCRIPT(scriptContext);
            if (count != 1)
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_InternalError, _u("invalid typeinfo count"));
            }
        }
    }
    if (FAILED(hr))
    {
        HostDispatch::HandleDispatchError(scriptContext, hr, nullptr);
    }
    return function;
}

Js::Var DebugObject::EntryParseFunction(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    HRESULT hr = NOERROR;
    ARGUMENTS(args, callInfo);
    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count <= 1)
    {
        Js::JavascriptError::ThrowError(scriptContext, VBSERR_TypeMismatch);
    }

    LPCWSTR sourceString = Js::JavascriptConversion::ToString(args.Values[1], scriptContext)->GetSz();
    Var resultFunc;
    BEGIN_LEAVE_SCRIPT(scriptContext)
    {
        LoadScriptFlag loadScriptFlag = LoadScriptFlag_isFunction;
        ScriptEngine* scriptEngine = static_cast<ScriptEngine*>(scriptContext->GetActiveScriptDirect());
        hr = scriptEngine->ParseInternal((LPWSTR)sourceString, &resultFunc, &loadScriptFlag);
        if (hr == NO_ERROR)
        {
            Assert(Js::ScriptFunction::Is(resultFunc));
            Js::ScriptFunction::FromVar(resultFunc)->SetIsActiveScript(true);
        }
    }
    END_LEAVE_SCRIPT(scriptContext);
    if (FAILED(hr))
    {
        HostDispatch::HandleDispatchError(scriptContext, hr,nullptr);
    }
    return resultFunc;
}

// Enable an extra feature/testhook on Debug object at runtime
Js::Var DebugObject::EntryEnable(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    HRESULT hr = NOERROR;
    ARGUMENTS(args, callInfo);
    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count <= 1)
    {
        Js::JavascriptError::ThrowError(scriptContext, VBSERR_TypeMismatch);
    }

    Js::JavascriptString* featureString = Js::JavascriptConversion::ToString(args.Values[1], scriptContext);
    PCWSTR feature = featureString->GetSz();
    if (_wcsicmp(feature, _u("TestDeferredConstructor")) == 0)
    {
        Js::PropertyId nameId = scriptContext->GetOrAddPropertyIdTracked(_u("TestDeferredConstructor"));
        Var resultFunc;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {            
            hr = scriptContext->GetActiveScriptDirect()->CreateDeferredConstructor(
                &DummyScriptMethod,
                nameId,
                &DummyInitializeMethod,
                0,
                FALSE,
                FALSE,
                &resultFunc);
        }
        END_LEAVE_SCRIPT(scriptContext);

        if (SUCCEEDED(hr))
        {
            Js::RecyclableObject::FromVar(args.Values[0])->SetProperty(
                scriptContext->GetLibrary()->CreateStringFromCppLiteral(_u("TestDeferredConstructor")),
                resultFunc,
                Js::PropertyOperationFlags::PropertyOperation_None,
                nullptr);
        }
    }

    if (FAILED(hr))
    {
        HostDispatch::HandleDispatchError(scriptContext, hr, nullptr);
    }
    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var DebugObject::EntryIsAsmJSModule(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Js::ScriptContext* scriptContext = function->GetScriptContext();
    if (args.Info.Count < 2)
    {
        Js::JavascriptError::ThrowError(scriptContext, VBSERR_IllegalFuncCall);
    }
#ifdef ASMJS_PLAT
    Var asmFuncVar = args[1] ;
    if (Js::ScriptFunction::Is(asmFuncVar))
    {
        Js::JavascriptFunction* asmFunction = Js::JavascriptFunction::FromVar(asmFuncVar);
        Js::FunctionBody* asmFunctionBody = asmFunction->GetFunctionBody();
        bool isASMJS = asmFunctionBody->IsAsmJSModule();
        if (isASMJS)
        {
            return scriptContext->GetLibrary()->GetTrue();
        }
    }
#endif
    return scriptContext->GetLibrary()->GetFalse();
}

Js::Var DebugObject::EntrySetAutoProxyName(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{    
    ARGUMENTS(args, callInfo);
    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count < 1)
    {
        Js::JavascriptError::ThrowError(scriptContext, VBSERR_TypeMismatch);
    }
    Var stateVar = args.Info.Count > 1 ? args[1] : scriptContext->GetLibrary()->GetUndefined();
    Js::Configuration::Global.flags.Enable(Js::autoProxyFlag);
    if (Js::JavascriptOperators::GetTypeId(stateVar) != TypeIds_Undefined)
    {
        Js::JavascriptString* objectName = Js::JavascriptConversion::ToString(stateVar, scriptContext);
        scriptContext->GetThreadContext()->SetAutoProxyName(objectName->GetSz());
    }
    return scriptContext->GetLibrary()->GetUndefined();
}

Js::Var DebugObject::EntryDisableAutoProxy(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{    
    ARGUMENTS(args, callInfo);
    Js::ScriptContext* scriptContext = function->GetScriptContext();
    scriptContext->GetThreadContext()->SetAutoProxyName(nullptr);
    Js::Configuration::Global.flags.Disable(Js::autoProxyFlag);
    return scriptContext->GetLibrary()->GetUndefined();
}

extern IDebugApplication *s_pda;

Js::Var DebugObject::DumpHeapInternal(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (! scriptSite)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (! scriptEngine)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // ignore failure and use alternate dump methods
    scriptEngine->GetDebugApplicationCoreNoRef(&s_pda);
    Js::Var argArray[10] = { NULL };
    Assert(args.Info.Count <= _countof(argArray));
    UINT i = 0;
    for ( ; i < args.Info.Count && i < _countof(argArray); i++)
    {
        argArray[i] = args[i];
    }
    HeapDumper heapDumper(*scriptEngine, argArray, i);
    return heapDumper.DumpHeap();
}

Js::Var DebugObject::EntryCreateDebugDisposableObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    ThreadContext* threadContext = scriptContext->GetThreadContext();

#define USING_PROPERTY_RECORD(propertyName)  \
    const Js::PropertyRecord* propertyName##PropertyRecord = NULL; \
    LPCWSTR propertyName##PropertyName = _u("") _u(#propertyName); \
    threadContext->GetOrAddPropertyId(propertyName##PropertyName, (int) Js::JavascriptString::GetBufferLength(propertyName##PropertyName), &propertyName##PropertyRecord);

    USING_PROPERTY_RECORD(collectOnDispose);
    USING_PROPERTY_RECORD(allocateLeaf);
    USING_PROPERTY_RECORD(bytesToAllocate);
    USING_PROPERTY_RECORD(disposableObjectSize);
    USING_PROPERTY_RECORD(allocationCount);

    bool collectOnDispose = false;
    uint bytesToAllocateOnDispose = 16;
    bool allocateLeaf = true;
    uint disposableObjectSize = 8192;
    uint allocationCount = 256;

    if (args.Info.Count >= 2)
    {
        Js::DynamicObject* parameterObject = Js::DynamicObject::FromVar(args[1]);
        Js::Var value = NULL;

        if (Js::JavascriptOperators::GetProperty(parameterObject, collectOnDisposePropertyRecord->GetPropertyId(), &value, scriptContext))
        {
            collectOnDispose = Js::JavascriptConversion::ToBool(value, scriptContext);
        }

        if (Js::JavascriptOperators::GetProperty(parameterObject, bytesToAllocatePropertyRecord->GetPropertyId(), &value, scriptContext))
        {
            bytesToAllocateOnDispose = Js::JavascriptConversion::ToUInt32(value, scriptContext);
        }

        if (Js::JavascriptOperators::GetProperty(parameterObject, allocateLeafPropertyRecord->GetPropertyId(), &value, scriptContext))
        {
            allocateLeaf = Js::JavascriptConversion::ToBool(value, scriptContext);
        }

        if (Js::JavascriptOperators::GetProperty(parameterObject, disposableObjectSizePropertyRecord->GetPropertyId(), &value, scriptContext))
        {
            disposableObjectSize = Js::JavascriptConversion::ToUInt32(value, scriptContext);
        }

        if (Js::JavascriptOperators::GetProperty(parameterObject, allocationCountPropertyRecord->GetPropertyId(), &value, scriptContext))
        {
            allocationCount = Js::JavascriptConversion::ToUInt32(value, scriptContext);
        }
    }

    Js::Var disposableObject = RecyclerNewFinalizedPlus(threadContext->GetRecycler(), disposableObjectSize + sizeof(DebugDisposableObject), DebugDisposableObject, 
        scriptContext->GetLibrary()->GetDebugDisposableObjectType(), scriptContext,
        collectOnDispose, bytesToAllocateOnDispose, allocateLeaf, allocationCount);
    return disposableObject;
}

Js::Var DebugObject::EntryCreateDebugFuncExecutorInDisposeObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    Assert(!(callInfo.Flags & CallFlags_New));

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    
    if (args.Info.Count < 2)
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    };

    Js::JavascriptFunction* functionToCall = 0;
    Var* jsArguments = 0;

    // Process only if we have valid function
    if (Js::JavascriptOperators::GetTypeId(args[1]) == TypeIds_Function)
    {
        functionToCall = Js::JavascriptFunction::FromVar(args[1]);
        jsArguments = RecyclerNewArray(scriptContext->GetRecycler(), Var, args.Info.Count - 1);

        // Set args[0] = this
        js_memcpy_s(jsArguments, sizeof(Var), args.Values, sizeof(Var));

        // Set arguments to be passed to the function
        if (args.Info.Count > 2)
        {
            js_memcpy_s(jsArguments + 1, sizeof(Var)* (args.Info.Count - 2), args.Values + 2, sizeof(Var)* (args.Info.Count - 2));
        }

        Js::Var disposableObject = RecyclerNewFinalized(scriptContext->GetRecycler(),
            DebugFuncExecutorInDisposeObject,
            scriptContext->GetLibrary()->GetDebugFuncExecutorInDisposeObjectType(),
            functionToCall,
            jsArguments,
            (ushort)args.Info.Count - 1
            );

        return disposableObject;
    }
    
    Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_Invalid);
}

Js::Var DebugObject::DetachAndFreeObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    ARGUMENTS(args, callInfo);
    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if (args.Info.Count != 2)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (!scriptSite)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (!scriptEngine)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    BYTE* outBuffer = nullptr;
    UINT outBufferLength = 0;
    TypedArrayBufferAllocationType outAllocationType;
    HRESULT hr;

    BEGIN_LEAVE_SCRIPT(scriptContext) 
    {
        hr = scriptEngine->DetachTypedArrayBuffer(args.Values[1], &outBuffer, &outBufferLength, &outAllocationType, nullptr, nullptr);
    }
    END_LEAVE_SCRIPT(scriptContext);

    if (FAILED(hr))
    {
        return Js::JavascriptNumber::New(hr, scriptContext);
    }

    hr = scriptEngine->FreeDetachedTypedArrayBuffer(outBuffer, outBufferLength, outAllocationType);

    return Js::JavascriptNumber::New(hr, scriptContext);
}
#endif

#ifdef ENABLE_HEAP_DUMPER
IDebugApplication *s_pda = NULL;

#ifndef ENABLE_DEBUG_CONFIG_OPTIONS
Js::Var DebugObject::DumpHeap(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

    ARGUMENTS(args, callInfo);

    Js::ScriptContext* scriptContext = function->GetScriptContext();
    ScriptSite * scriptSite = ScriptSite::FromScriptContext(scriptContext);
    if (! scriptSite)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
    ScriptEngine* scriptEngine = scriptSite->GetScriptEngine();
    if (! scriptEngine)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }

    int argc = args.Info.Count;
    
    double arg1 = (argc <= 1 ? 0.0 : Js::JavascriptConversion::ToNumber(args[1], scriptContext));
    bool printBaselineComparison = false;
    if (arg1 != HeapDumperObjectToDumpFlag::HeapDumperDumpNew
        && arg1 != HeapDumperObjectToDumpFlag::HeapDumperDumpOld
        && arg1 != HeapDumperObjectToDumpFlag::HeapDumperDumpAll)
    {
        return scriptContext->GetLibrary()->GetUndefined();
    }
    if (argc >= 3)
    {
        printBaselineComparison = (Js::JavascriptConversion::ToBoolean(args[2], scriptContext) != 0);
    }
    HeapDumper heapDumper(*scriptEngine, (HeapDumperObjectToDumpFlag)((UINT)arg1), printBaselineComparison);
    heapDumper.DumpHeap();
    return scriptContext->GetLibrary()->GetUndefined();
}
#endif

void OutputDump(Js::ScriptContext* scriptContext, const char16 *form, ...)
{
    va_list argptr;
    va_start(argptr, form);
    char16 buf[2048];
    _vsnwprintf_s(buf, _countof(buf), _TRUNCATE, form, argptr);

    if (s_pda)
    {
        s_pda->DebugOutput(buf);
    }
    else
    {
        Output::Print(buf);
        Output::Flush();
    }
}

UINT HeapDumper::IndentBuffer::currIndent = 0;
WCHAR HeapDumper::IndentBuffer::buffer[HeapDumper::IndentBuffer::maxIndent];

HRESULT HeapDumper::IndentBuffer::Append(LPCWSTR appendStr, UINT& prevIndent)
{
    UINT appendLen = wcslen(appendStr);
    if ((maxIndent - currIndent) < appendLen)
    {        
        Output::Print(_u("*** Error: maxindent exceeded ***\n"));
        return E_FAIL;
    }
    wcscat_s(buffer, maxIndent, appendStr);
    prevIndent = currIndent;
    currIndent += appendLen; 
    return S_OK;
}

HRESULT HeapDumper::IndentBuffer::Append(LPCWSTR appendStr)
{
    UINT prevIndent = 0;
    return Append(appendStr, prevIndent);
}

void HeapDumper::IndentBuffer::SetIndent(UINT indentAmount)
{
    if (indentAmount < currIndent)
    {
        currIndent = indentAmount;
        buffer[currIndent] = '\0';
    }
}

void HeapDumper::IndentBuffer::Print()
{
    Output::Print(_u("%s"), buffer);
}

ULONG HeapDumper::FindObjectInSnapshot(DWORD_PTR obj)
{
    for (ULONG i = 0; i < numSnapshotElements; i++)
    {
        if (pSnapshot[i]->objectId == obj)
        {
            return i;
        }
    }
    return ULONG_MAX;
}

void HeapDumper::DumpAddress(ULONG objIndex, bool newLine=true)
{
    if (! dumpArgs.printBaselineComparison)
    {
        Output::Print(_u(" index: %u"), objIndex);
        Output::Print(_u(" id: %u"), pSnapshot[objIndex]->objectId);
        Output::Print(_u(" address: %p"), pSnapshot[objIndex]->objectId);
    }
    if (newLine)
    {
        Output::Print(_u("\n"));
    }
}

HRESULT HeapDumper::DumpSnapshotObject(DWORD_PTR address)
{
    ULONG objIndex = FindObjectInSnapshot(address);
    if (objIndex == ULONG_MAX)
    {
        Output::Print(_u("*** Error: Object missing from snapshot ***\n"));
        return E_FAIL;
    }
    PROFILER_HEAP_OBJECT& obj = *pSnapshot[objIndex];
    if ((obj.flags & profilerHeapObjectFlagsDumped) != 0)
    {
        Output::Print(_u(" type: %s") , obj.typeNameId == PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE ? _u("unspecified") : GetNameFromId(obj.typeNameId));
        DumpAddress(objIndex, false);
        Output::Print(_u(" (seen)\n"));
    }
    else
    {
        obj.flags = obj.flags | profilerHeapObjectFlagsDumped;
        DumpObjectInfo(objIndex);
    }
    return S_OK;
}

LPCWSTR HeapDumper::GetStringValue(LPCWSTR propertyValue)
{
    if (!propertyValue)
    {
        return _u("(null)");
    }
    else if (wcslen(propertyValue) == 0)
    {
        return _u("(empty string)");
    }
    else if (this->dumpArgs.trimDirectoryNameFromFullPath)
    {
        // If the string represents full path (not a relative path), trim directory part/leave only file part.
        bool isFullPath =
            PathGetDriveNumber(propertyValue) != -1 ||                            // Local path (d:\a.txt).
            PathIsUNC(propertyValue) ||                                           // UNC path (\\server\share\a.txt).
            PathIsLFNFileSpec(propertyValue) && !PathIsFileSpec(propertyValue);   // LFN but not just file part (\\?\.\server\share\a.txt).
        if (isFullPath)
        {
            return PathFindFileNameW(propertyValue);
        }
    }

    return propertyValue;
}

HRESULT HeapDumper::DumpProperty(PROFILER_HEAP_OBJECT_RELATIONSHIP& elem)
{
    HRESULT hr = S_OK;
    PROFILER_RELATIONSHIP_INFO propertyType = ActiveScriptProfilerHeapEnum::GetRelationshipInfo(elem);
    UINT prevIndent = UINT_MAX;
    switch (propertyType)
    {
        case PROFILER_PROPERTY_TYPE_HEAP_OBJECT:
        case PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT:
        {
            IfFailGo(IndentBuffer::Append(_u("  "), prevIndent));
            IfFailGo(DumpSnapshotObject(elem.objectId));
            break;
        }
        case PROFILER_PROPERTY_TYPE_NUMBER:
        {
            Output::Print(_u(" value: %f\n"), elem.numberValue);
            break;
        }
        case PROFILER_PROPERTY_TYPE_STRING:
        {
            Output::Print(_u(" value: %.200s\n"), GetStringValue(elem.stringValue));
            break;
        }
        case PROFILER_PROPERTY_TYPE_SUBSTRING:
        {
            if (this->dumpArgs.enumFlags & PROFILER_HEAP_ENUM_FLAGS_SUBSTRINGS) 
            {
                BSTR bstrStringValue = SysAllocStringLen(elem.subString->value, elem.subString->length);
                if (bstrStringValue == NULL)
                {
                    return E_OUTOFMEMORY;
                }
                Output::Print(_u(" value: %.200s\n"), GetStringValue(bstrStringValue));
                SysFreeString(bstrStringValue);
            }
            else 
            {
                Assert("HeapEnum property type is substring but PROFILER_HEAP_ENUM_FLAGS_SUBSTRINGS enum flag is not set");
            }
            break;
        }
        case PROFILER_PROPERTY_TYPE_BSTR:
        {
            // length of string must be shorter than buffer length in Output::Print()
            Output::Print(_u(" value: %.2000s\n"), GetStringValue(elem.bstrValue));
            break;
        }
        default:
        {
            IndentBuffer::Print();
            Output::Print(_u("*** Error: Unexpected property type ***\n"));
            return E_FAIL;
        }

    }
Error:
    if (prevIndent != UINT_MAX)
    {
        IndentBuffer::SetIndent(prevIndent);
    }
    return hr;
}


HRESULT HeapDumper::DumpObjectInfo(ULONG objIndex)
{
    PROFILER_HEAP_OBJECT& obj = *pSnapshot[objIndex];
    HRESULT hr = S_OK;
    Output::Print(_u(" type: %s") , obj.typeNameId == PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE ? _u("unspecified") : GetNameFromId(obj.typeNameId));
    Output::Print(_u(" flags: ("));
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_NEW_STATE_UNAVAILABLE)
    {
        Output::Print(_u("new_state_unavailable"));
    }
    else
    {
        Output::Print(_u("%s"), (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_NEW_OBJECT) == 0 ? _u("old") : _u("new"));
    }
#ifdef HEAP_ENUMERATION_VALIDATION
    // If change this, must also update ActiveScriptProfilerHeapEnum::PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_OBJECT
    ULONG PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_OBJECT_LIBRARY = 0x20000000;
    ULONG PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_OBJECT_USER = 0x10000000;
    if (obj.flags & PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_OBJECT_LIBRARY)
    {
        Output::Print(_u(" *** unreported library object ***"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_OBJECT_USER)
    {
        Output::Print(_u(" *** unreported user object ***"));
    }
#endif
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_IS_ROOT)
    {
        Output::Print(_u(" root"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_SITE_CLOSED)
    {
        Output::Print(_u(" closed"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE)
    {
        Output::Print(_u(" WinRT_Instance"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_RUNTIMECLASS)
    {
        Output::Print(_u(" WinRT_RuntimeClass"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_DELEGATE)
    {
        Output::Print(_u(" WinRT_Delegate"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_NAMESPACE)
    {
        Output::Print(_u(" WinRT_Namespace"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL)
    {
        Output::Print(_u(" external"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_UNKNOWN)
    {
        Output::Print(_u(" external_unknown"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_DISPATCH)
    {
        Output::Print(_u(" external_dispatch"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_SIZE_APPROXIMATE)
    {
        Output::Print(_u(" size_approximate"));
    }
    if (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_SIZE_UNAVAILABLE)
    {
        Output::Print(_u(" size_unavailable"));
    }
    Output::Print(_u(")"));
    DumpAddress(objIndex, false);
    if (! dumpArgs.printBaselineComparison)
    {
        Output::Print(_u(" size: %u"), obj.size);
    }
    Output::Print(_u(" optional info records (%u)\n"), obj.optionalInfoCount);
    UINT prevIndent = UINT_MAX;
    IfFailGo(IndentBuffer::Append(_u("  "), prevIndent));
    UINT currIndent = IndentBuffer::CurrentIndent();

    if (currIndent <= dumpArgs.maxDumpIndent)
    {
        PROFILER_HEAP_OBJECT_OPTIONAL_INFO optionalInfoArray[PROFILER_HEAP_OBJECT_OPTIONAL_INFO_MAX_VALUE];

        IfFailGo(pEnum->GetOptionalInfo(&obj, obj.optionalInfoCount, optionalInfoArray));
        for (UINT j = 0; j < obj.optionalInfoCount; j++)
        {
            PROFILER_HEAP_OBJECT_OPTIONAL_INFO* optionalInfo = &optionalInfoArray[j];
            IndentBuffer::SetIndent(currIndent);
            IndentBuffer::Print();
            switch (optionalInfo->infoType)
            {
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INTERNAL_PROPERTY:
                {
                    Output::Print(_u("[Internal property]"));
                    IfFailGo(IndentBuffer::Append(_u("|")));
                    IfFailGo(DumpProperty(*(optionalInfo->internalProperty)));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_PROTOTYPE:
                {
                    Output::Print(_u("[Prototype]"));
                    IfFailGo(IndentBuffer::Append(_u("|")));
                    IfFailGo(DumpSnapshotObject(optionalInfo->prototype));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_FUNCTION_NAME:
                {
                    Output::Print(_u("[Function name] %s\n"), optionalInfo->functionName);
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SCOPE_LIST:
                {
                    PROFILER_HEAP_OBJECT_SCOPE_LIST& scopeList = *optionalInfo->scopeList;
                    Output::Print(_u("[Scopes] count: %u\n"), scopeList.count);
                    IfFailGo(IndentBuffer::Append(_u("|")));
                    for (UINT k = 0; k < scopeList.count; k++)
                    {
                        IndentBuffer::Print();
                        Output::Print(_u(" scope[%u]:"), k);
                        IfFailGo(DumpSnapshotObject(scopeList.scopes[k]));
                    }
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_NAME_PROPERTIES:
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& properties = *optionalInfo->namePropertyList;
                    Output::Print(_u("[Properties] count: %u\n"), properties.count);
                    IfFailGo(DumpRelationshipList(properties, /*isIndexPropertyList*/ false, /*includeId*/!dumpArgs.printBaselineComparison));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_INDEX_PROPERTIES:
                {
                    _PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& arrayElements = *optionalInfo->indexPropertyList;
                    Output::Print(_u("[Array elements] count: %u\n"), arrayElements.count);
                    IfFailGo(DumpRelationshipList(arrayElements, /*isIndexPropertyList*/ true, /*includeId*/false));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_RELATIONSHIPS:
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& relationships = *optionalInfo->relationshipList;
                    Output::Print(_u("[Relationships] count: %u\n"), relationships.count);
                    IfFailGo(DumpRelationshipList(relationships, /*isIndexPropertyList*/ false, /*includeId*/!dumpArgs.printBaselineComparison));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_ATTRIBUTES_SIZE:
                {
                    Output::Print(_u("[Element attributes size] %u\n"), optionalInfo->elementAttributesSize);
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_ELEMENT_TEXT_CHILDREN_SIZE:
                {
                    Output::Print(_u("[Element text children size] %u\n"), optionalInfo->elementTextChildrenSize);
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WINRTEVENTS:
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& events = *optionalInfo->eventList;
                    Output::Print(_u("[Events] count: %u\n"), events.count);
                    IfFailGo(DumpRelationshipList(events, /*isIndexPropertyList*/ false, /*includeId*/!dumpArgs.printBaselineComparison));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_MAP_COLLECTION_LIST:
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list = *optionalInfo->mapCollectionList;
                    Output::Print(_u("[Map key-value pairs] count: %u\n"), list.count);
                    IfFailGo(DumpCollectionList(list));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_SET_COLLECTION_LIST:
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list = *optionalInfo->setCollectionList;
                    if (wcscmp(_u("WeakSetObject"), GetNameFromId(obj.typeNameId)) == 0)
                    {
                        Output::Print(_u("[WeakSet values] count: %u \n"), list.count);
                    }
                    else
                    {
                        Output::Print(_u("[Set values] count: %u\n"), list.count);
                    }
                    IfFailGo(DumpCollectionList(list));
                    break;
                }
                case PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WEAKMAP_COLLECTION_LIST:
                {
                    PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list = *optionalInfo->weakMapCollectionList;
                    Output::Print(_u("[WeakMap key-value pairs] count: %u\n"), list.count);
                    IfFailGo(DumpCollectionList(list));
                    break;
                }
                default:
                {
                    Assert(false);
                    IndentBuffer::Print();
                    Output::Print(_u("*** Error: Unexpected optional info type ***\n"));
                    return E_FAIL;
                }
            } // end switch
        } // end for
    }
Error:
    if (prevIndent != UINT_MAX)
    {
        IndentBuffer::SetIndent(prevIndent);
    }
    return hr;
}

HRESULT HeapDumper::DumpRelationshipList(PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list, bool isIndexPropertyList, bool includeId)
{
    HRESULT hr = S_OK;
    IfFailGo(IndentBuffer::Append(_u("|")));
    for (UINT k = 0; k < list.count; k++)
    {
        PROFILER_HEAP_OBJECT_RELATIONSHIP& elem = list.elements[k];
        IndentBuffer::Print();
        if (elem.relationshipId == PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE)
        {
            Output::Print(_u(" [ID unavailable]:"));
        }
        else
        {
            if (isIndexPropertyList)
            {
                Output::Print(_u(" %u:"), elem.relationshipId);
            }
            else
            {
                Output::Print(_u(" %u %s:"), k, GetNameFromId(elem.relationshipId));

                DumpRelationshipFlags(elem);

                if (includeId)
                {
                    Output::Print(_u(" pid: %u "), elem.relationshipId);
                }
            }
        }
        DumpProperty(elem);
    }
Error:
    return hr;
}

HRESULT HeapDumper::DumpCollectionList(PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list)
{
    HRESULT hr = S_OK;
    IfFailGo(IndentBuffer::Append(_u("|")));
    for (UINT k = 0; k < list.count; k++)
    {
        PROFILER_HEAP_OBJECT_RELATIONSHIP& elem = list.elements[k];
        Assert(elem.relationshipId == PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE);
        IndentBuffer::Print();
        Output::Print(_u(" %u:"), k);
        DumpProperty(elem);
    }
Error:
    return hr;
}

void HeapDumper::DumpRelationshipFlags(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship)
{
    if (!ActiveScriptProfilerHeapEnum::AreAnyRelationshipFlagsSet(relationship))
    {
        return;
    }

    Output::Print(_u(" rsFlags:"));

    bool wasDumped = false;
    if (ActiveScriptProfilerHeapEnum::IsRelationshipFlagSet(relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_IS_GET_ACCESSOR))
    {
        Output::Print(_u(" getter"));
        wasDumped = true;
    }
    if (ActiveScriptProfilerHeapEnum::IsRelationshipFlagSet(relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_IS_SET_ACCESSOR))
    {
        Output::Print(_u(" setter"));
        wasDumped = true;
    }
    if (ActiveScriptProfilerHeapEnum::IsRelationshipFlagSet(relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_LET_VARIABLE))
    {
        Output::Print(_u(" let"));
        wasDumped = true;
    }
    if (ActiveScriptProfilerHeapEnum::IsRelationshipFlagSet(relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS_CONST_VARIABLE))
    {
        Output::Print(_u(" const"));
        wasDumped = true;
    }

    AssertMsg(wasDumped, "Unhandled PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS encountered.  Update DumpRelationshipFlags().");
    if (!wasDumped)
    {
        Output::Print(_u(" ** Error ***: unknown relationship flag"));
    }
}

LPCWSTR HeapDumper::GetNameFromId(PROFILER_HEAP_OBJECT_NAME_ID nameId)
{
    if (nameId > maxPropertyId)
    {
        return _u("** Error ***: invalid nameId");
    }
    if (nameId == 0)
    {
        return _u("** Error ***: property ID is 0");
    }
    if (nameId == Js::PropertyIds::_this)
    {
        return _u("this");
    }
    if (nameId == Js::PropertyIds::_super|| nameId == Js::PropertyIds::_superConstructor)
    {
        return _u("super");
    }
    if (nameId == Js::PropertyIds::_newTarget)
    {
        return _u("new.target");
    }
    return pPropertyIdMap[nameId];
}

bool HeapDumper::IsNewStateAvailable(PROFILER_HEAP_OBJECT& obj)
{
    return (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_NEW_STATE_UNAVAILABLE) == 0;
}

bool HeapDumper::IsExternalObject(PROFILER_HEAP_OBJECT& obj)
{
    return (obj.flags & (PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL | PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_UNKNOWN | PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_DISPATCH)) != 0;
}

Js::Var HeapDumper::DumpHeap()
{
    HRESULT hr = S_OK;
    Js::ScriptContext* scriptContext = scriptEngine.GetScriptContext();
    Js::RecyclableObject* result = scriptContext->GetLibrary()->GetUndefined();

#ifdef ENABLE_TEST_HOOKS
    // mshtmlhost support to avoid allocation at enumeration time.
    scriptContext->GetOrAddPropertyIdTracked(_u("dummyFastDomVar"), wcslen(_u("dummyFastDomVar")));
    scriptContext->GetOrAddPropertyIdTracked(_u("DOMObject"), wcslen(_u("DOMObject")));
#endif

    Js::PropertyId typePid = scriptContext->GetOrAddPropertyIdTracked(_u("type"), wcslen(_u("type")));
    Js::PropertyId sizePid = scriptContext->GetOrAddPropertyIdTracked(_u("size"), wcslen(_u("size")));
    Js::PropertyId newPid = scriptContext->GetOrAddPropertyIdTracked(_u("new"), wcslen(_u("new")));    
    Js::PropertyId objectIdPid = scriptContext->GetOrAddPropertyIdTracked(_u("id"), wcslen(_u("id")));
    Js::PropertyId objectPid = scriptContext->GetOrAddPropertyIdTracked(_u("object"), wcslen(_u("object")));

    scriptContext->GetOrAddPropertyIdTracked(_u("pinned"), wcslen(_u("pinned")));
    scriptContext->GetOrAddPropertyIdTracked(_u("sub object"), wcslen(_u("sub object")));

    BEGIN_LEAVE_SCRIPT(scriptContext);
    hr = scriptEngine.EnumHeap2(dumpArgs.enumFlags, &pEnum);
    if (hr != S_OK)
    {
        OutputDump(scriptContext, _u("[error : DumpHeap failed on EnumHeap]\n"));
        goto Error;
    }

    const size_t snapshotChunkSize = 1000;
    PROFILER_HEAP_OBJECT* snapshotChunk[snapshotChunkSize];
    
    numSnapshotElements = 0;
    pSnapshot = (PROFILER_HEAP_OBJECT**)malloc(sizeof(PROFILER_HEAP_OBJECT*));
    if (! pSnapshot)
    {
        OutputDump(scriptContext, _u("[error : DumpHeap failed on malloc]\n"));
        goto Error;
    }
    
    do {
        ULONG numFetched;
        hr = pEnum->Next(snapshotChunkSize, snapshotChunk, &numFetched);
        if (numFetched == 0 || FAILED(hr)) break;
        void* newSnapshot = realloc(pSnapshot, sizeof(PROFILER_HEAP_OBJECT*) * (numSnapshotElements + numFetched));
        if (! newSnapshot)
        {
            OutputDump(scriptContext, _u("[error : DumpHeap failed on malloc]\n"));
            goto Error;
        }
        pSnapshot = (PROFILER_HEAP_OBJECT**)newSnapshot;
        UINT copySize = sizeof(PROFILER_HEAP_OBJECT*) * numFetched;
        __analysis_assume(numFetched <= snapshotChunkSize); //  OACR doesn't know that numFetched is constrained to max snapshotChunkSize in IActiveScriptHeapEnum::Next above
        js_memcpy_s(pSnapshot + numSnapshotElements, copySize, snapshotChunk, copySize);
        numSnapshotElements += numFetched;
    } while (TRUE);

    if (SUCCEEDED(hr))
    {
        hr = pEnum->GetNameIdMap(&pPropertyIdMap, &maxPropertyId);
    }
    if (FAILED(hr))
    {
        OutputDump(scriptContext, _u("[error : DumpHeap failed on Next()]\n"));
        goto Error;
    }
    END_LEAVE_SCRIPT(scriptContext);

    Js::JavascriptLibrary* library = scriptContext->GetLibrary();
    if (dumpArgs.returnArray)
    {
        result = library->CreateArray(0, 1);
    }
    if (dumpArgs.printHeapEnum)
    {
        Output::Print(_u("\n\n===========>Starting Heap Dump for %s<===============\n"), 
            dumpArgs.objectToDump ? _u("specific object") :
            dumpArgs.dumpType==HeapDumperDumpNew ? _u("new objects only") : 
            dumpArgs.dumpType==HeapDumperDumpOld ? _u("old objects only") : 
            _u("all objects"));
    }
    ULONG elemCount = 0;
    for (ULONG i=0; i < numSnapshotElements; i++)
    {
        PROFILER_HEAP_OBJECT& obj = *pSnapshot[i];
        bool isNewObject = (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_NEW_OBJECT) != 0;
        if (dumpArgs.objectToDump != NULL)
        {
            if (obj.objectId != (DWORD_PTR)(dumpArgs.objectToDump))
            {
                continue;
            }
        }
        else if (IsNewStateAvailable(obj)  &&
                (dumpArgs.dumpType==HeapDumperDumpOld && isNewObject ||
                dumpArgs.dumpType==HeapDumperDumpNew && ! isNewObject))
        {
            continue;
        }
        else if (dumpArgs.dumpRootsOnly && (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_IS_ROOT) == 0)
        {
            continue;            
        }
        if (dumpArgs.returnArray)
        {
            Js::DynamicObject* element = library->CreateObject();

            Js::PropertyId rootPid = scriptContext->GetOrAddPropertyIdTracked(_u("root"), wcslen(_u("root")));            
            Js::PropertyId externalAddressPid = scriptContext->GetOrAddPropertyIdTracked(_u("externalAddress"), wcslen(_u("externalAddress")));            
            Js::PropertyId winrtInstancePid = scriptContext->GetOrAddPropertyIdTracked(_u("winrtInstance"), wcslen(_u("winrtInstance")));
            Js::PropertyId winrtRuntimeClassPid = scriptContext->GetOrAddPropertyIdTracked(_u("winrtRuntimeClass"), wcslen(_u("winrtRuntimeClass")));
            Js::PropertyId winrtDelegatePid = scriptContext->GetOrAddPropertyIdTracked(_u("winrtDelegate"), wcslen(_u("winrtDelegate")));
            Js::PropertyId winrtNamespacePid = scriptContext->GetOrAddPropertyIdTracked(_u("winrtNamespace"), wcslen(_u("winrtNamespace")));

            if (obj.typeNameId != PROFILER_HEAP_OBJECT_NAME_ID_UNAVAILABLE)
            {
                LPCWSTR typeName = GetNameFromId(obj.typeNameId);
                element->SetProperty(typePid, library->CreateStringObject(Js::JavascriptString::NewCopySz(typeName, scriptContext)), Js::PropertyOperation_None, NULL);  
            }
            element->SetProperty(sizePid, Js::JavascriptNumber::New((double)obj.size, scriptContext), Js::PropertyOperation_None, NULL);
            element->SetProperty(newPid, isNewObject ? library->GetTrue() : library->GetFalse(), Js::PropertyOperation_None, NULL);
            element->SetProperty(rootPid, (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_IS_ROOT) != 0 ? library->GetTrue() : library->GetFalse(), Js::PropertyOperation_None, NULL);
            element->SetProperty(objectIdPid, Js::JavascriptNumber::New((double)obj.objectId, scriptContext), Js::PropertyOperation_None, NULL);
            if (IsExternalObject(obj))
            {
                element->SetProperty(externalAddressPid, Js::JavascriptNumber::New((double)obj.objectId, scriptContext), Js::PropertyOperation_None, NULL);
            }
            else
            {
                element->SetProperty(objectPid, (Js::Var)obj.objectId, Js::PropertyOperation_None, NULL);
            }

            element->SetProperty(winrtInstancePid, (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE) != 0 ? library->GetTrue() : library->GetFalse(), Js::PropertyOperation_None, NULL);
            element->SetProperty(winrtRuntimeClassPid, (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_RUNTIMECLASS) != 0 ? library->GetTrue() : library->GetFalse(), Js::PropertyOperation_None, NULL);
            element->SetProperty(winrtDelegatePid, (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_DELEGATE) != 0 ? library->GetTrue() : library->GetFalse(), Js::PropertyOperation_None, NULL);
            element->SetProperty(winrtNamespacePid, (obj.flags & PROFILER_HEAP_OBJECT_FLAGS_WINRT_NAMESPACE) != 0 ? library->GetTrue() : library->GetFalse(), Js::PropertyOperation_None, NULL);

            result->SetItem(elemCount++, element, Js::PropertyOperation_None);
        }
        if (dumpArgs.printHeapEnum && (obj.flags & profilerHeapObjectFlagsDumped) == 0)
        {
            Output::Print(_u("\n"));
            IndentBuffer::SetIndent(0);
            IndentBuffer::Append(_u(" |"));
            DumpSnapshotObject(obj.objectId);
        }
        if (dumpArgs.objectToDump != NULL)
        {
            break;
        }

    }
    if (dumpArgs.printHeapEnum)
    {
        Output::Print(_u("\n==========>End Heap Dump<===============\n\n\n"));
        Output::Flush();
    }

Error:
    return result;
}

HeapDumper::HeapDumper(ScriptEngine& scriptEngineIn, HeapDumperObjectToDumpFlag objectsToDump, BOOL minimalDump) :
      numSnapshotElements(0), pSnapshot(NULL), maxPropertyId(0), pPropertyIdMap(NULL), scriptEngine(scriptEngineIn)
{
    dumpArgs.dumpType = objectsToDump;
    dumpArgs.printBaselineComparison = (minimalDump != FALSE);
    dumpArgs.printHeapEnum = true;
    dumpArgs.enumFlags = PROFILER_HEAP_ENUM_FLAGS::PROFILER_HEAP_ENUM_FLAGS_RELATIONSHIP_SUBSTRINGS;
    dumpArgs.trimDirectoryNameFromFullPath = false; // Set it to dump full paths when called via ScriptEngine.
}

HeapDumper::HeapDumper(ScriptEngine& scriptEngineIn, Js::Var args[], int argc) :
      numSnapshotElements(0), pSnapshot(NULL), maxPropertyId(0), pPropertyIdMap(NULL), scriptEngine(scriptEngineIn)
{
    if (argc <= 1)
    {
        return;
    }
    Js::ScriptContext* scriptContext = scriptEngine.GetScriptContext();
    double arg1 = Js::JavascriptConversion::ToNumber(args[1], scriptContext);
    if (arg1 == HeapDumperDumpNew || arg1 == HeapDumperDumpOld || arg1 == HeapDumperDumpAll)
    {
        dumpArgs.dumpType = (HeapDumperObjectToDumpFlag)((UINT)arg1);
    }
    else
    {
        dumpArgs.objectToDump = args[1];
        dumpArgs.printBaselineComparison = true;
    }

    // Note: if you pass undefined, the arg will use default value.
    if (HasArg(args, argc, 2))
    {
        dumpArgs.printHeapEnum = (Js::JavascriptConversion::ToBoolean(args[2], scriptContext) != 0);
    }
    if (HasArg(args, argc, 3))
    {
        dumpArgs.printBaselineComparison = (Js::JavascriptConversion::ToBoolean(args[3], scriptContext) != 0);
    }
    if (HasArg(args, argc, 4))
    {
        dumpArgs.dumpRootsOnly = (Js::JavascriptConversion::ToBoolean(args[4], scriptContext) != 0);
    }
    if (HasArg(args, argc, 5))
    {
        dumpArgs.returnArray = (Js::JavascriptConversion::ToBoolean(args[5], scriptContext) != 0);
    }
    if (HasArg(args, argc, 6))
    {
        dumpArgs.enumFlags = static_cast<PROFILER_HEAP_ENUM_FLAGS>(Js::JavascriptConversion::ToInt32(args[6], scriptContext));
    }
    if (HasArg(args, argc, 7))
    {
        dumpArgs.maxDumpIndent = Js::JavascriptConversion::ToUInt32(args[7], scriptContext);
    }
    if (HasArg(args, argc, 8))
    {
        dumpArgs.trimDirectoryNameFromFullPath = Js::JavascriptConversion::ToBoolean(args[8], scriptContext) != 0;
    }
}

bool HeapDumper::HasArg(Js::Var args[], int argc, int i)
{
    return i < argc && !Js::JavascriptOperators::IsUndefinedObject(args[i]);
}

HeapDumper::~HeapDumper()
{
    if (pSnapshot) 
    { 
        pEnum->FreeObjectAndOptionalInfo(numSnapshotElements, pSnapshot);
        free(pSnapshot); 
        pSnapshot = NULL; 
    }
    if (pPropertyIdMap)
    {
        CoTaskMemFree(pPropertyIdMap);
        pPropertyIdMap = NULL;
    }
}
#endif

#if JS_PROFILE_DATA_INTERFACE
Js::Var DebugObject::EntryGetProfileDataObject(Js::RecyclableObject* function, Js::CallInfo callInfo, ...)
{
    PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
    ARGUMENTS(args, callInfo);
    Assert(!(callInfo.Flags & CallFlags_New));
    Js::ScriptContext* scriptContext = function->GetScriptContext();

    if(args.Info.Count != 2 || !Js::JavascriptFunction::Is(args[1]))
    {
        Js::JavascriptError::ThrowError(scriptContext, JSERR_FunctionArgument_Invalid);
    }

    Js::JavascriptFunction *func = Js::JavascriptFunction::FromVar(args[1]);
    //To be safe; check if it is deserialized. This is a Debug object anyways
    func->GetFunctionProxy()->EnsureDeserialized();
    Js::FunctionBody *funcBody = func->GetFunctionBody();

    return RecyclerNew(scriptContext->GetRecycler(), Js::ProfileDataObject, 
        scriptContext->GetLibrary()->GetObjectType(), funcBody);

}
#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

HTYPE IASDDebugObjectHelper::EnsureType(JavascriptTypeId typeId, Js::PropertyId nameId, bool useDefaultTypeOperations)
{
    ScriptSite* scriptSite = ScriptSite::FromScriptDirect(scriptDirect);
    Js::ScriptContext* scriptContext = scriptSite->GetScriptSiteContext();
    if (nullptr == externalTypeMap) 
    {
        Recycler* recycler = scriptContext->GetRecycler();
        externalTypeMap = RecyclerNew(recycler, ExternalTypeMap, recycler, 32);
        scriptContext->BindReference(externalTypeMap);
    }
    HTYPE returnType;
    if (externalTypeMap->TryGetValue(typeId, &returnType))
    {
        return returnType;
    }
    HRESULT hr = NOERROR;
    CComPtr<ITypeOperations> defaultTypeOperations = nullptr;
    if (useDefaultTypeOperations) 
    {
        hr = scriptDirect->GetDefaultTypeOperations(&defaultTypeOperations);
    }
    if (SUCCEEDED(hr))
    {
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = scriptDirect->CreateType(typeId, nullptr/*inheritedTypeIds*/, 0/*inheritedTypeIdsCount*/, nullptr /*prototype*/, nullptr /*entryPoint*/, defaultTypeOperations, false /*isDefered*/, nameId, true/*bindReference*/, &returnType);
        }
        END_LEAVE_SCRIPT(scriptContext);
    }
    if (FAILED(hr))
    {
        HostDispatch::HandleDispatchError(scriptContext, hr, nullptr);
    }
    externalTypeMap->Add(typeId, returnType);
    return returnType;
}

#endif 
