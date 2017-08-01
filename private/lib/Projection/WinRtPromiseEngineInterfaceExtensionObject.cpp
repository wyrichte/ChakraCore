//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------
#include "ProjectionPch.h"

#include "Types\DeferredTypeHandler.h"
#include "Library\EngineInterfaceObject.h"

#include "WinRTPromiseEngineInterfaceExtensionObject.h"
#include "AsyncDebug.h"

#include "ByteCode\ByteCodeSerializer.h"
#include "errstr.h"
#include "ByteCode\ByteCodeDumper.h"

#pragma warning(push)
#pragma warning(disable:4309) // truncation of constant value
#pragma warning(disable:4838) // conversion from 'int' to 'const char' requires a narrowing conversion	
#if _M_AMD64 
#include "InJavascript\Promise.js.bc.64b.h"
#else
#include "InJavascript\Promise.js.bc.32b.h"
#endif
#pragma warning(pop)

#define IfFailThrowHr(op) \
    if (FAILED(hr=(op))) \
    { \
    Js::JavascriptError::MapAndThrowError(scriptContext, hr); \
    } \

namespace Projection
{
    Js::NoProfileFunctionInfo WinRTPromiseEngineInterfaceExtensionObject::EntryInfo::Promise_EnqueueTask(FORCE_NO_WRITE_BARRIER_TAG(WinRTPromiseEngineInterfaceExtensionObject::EntryPromise_EnqueueTask));

    WinRTPromiseEngineInterfaceExtensionObject::WinRTPromiseEngineInterfaceExtensionObject(Js::ScriptContext* scriptContext) :
        Js::EngineExtensionObjectBase(Js::EngineInterfaceExtensionKind::EngineInterfaceExtensionKind_WinRTPromise, scriptContext),
        promiseByteCode(nullptr),
        promiseNativeInterfaces(nullptr)
    {
    }

    void WinRTPromiseEngineInterfaceExtensionObject::Initialize()
    {
        Js::JavascriptLibrary* library = scriptContext->GetLibrary();
        this->scriptContext = scriptContext;
        if (scriptContext->GetConfig()->IsWinRTEnabled())
        {
            Js::DynamicObject * commonNativeInterfaces = library->GetEngineInterfaceObject()->GetCommonNativeInterfaces();
            this->promiseNativeInterfaces = Js::DynamicObject::New(library->GetRecycler(),
                Js::DynamicType::New(scriptContext, Js::TypeIds_Object, commonNativeInterfaces, nullptr,
                    Js::DeferredTypeHandler<InitializePromiseNativeInterfaces>::GetDefaultInstance()));
            library->AddMember(library->GetEngineInterfaceObject(), Js::PropertyIds::Promise, this->promiseNativeInterfaces);
        }
    }

    void WinRTPromiseEngineInterfaceExtensionObject::EnsurePromiseByteCode(_In_ Js::ScriptContext * scriptContext)
    {
        if (this->promiseByteCode == nullptr)
        {
            SourceContextInfo* sourceContextInfo = scriptContext->GetSourceContextInfo(Js::Constants::NoHostSourceContext, nullptr);

            Assert(sourceContextInfo != nullptr);

            SRCINFO si;
            memset(&si, 0, sizeof(si));
            si.sourceContextInfo = sourceContextInfo;
            SRCINFO* hsi = scriptContext->AddHostSrcInfo(&si);

            // Mark the Promise bytecode as internal library code - the real stack frames will be ignored during a stackwalk.
            // If we aren't profiling and function proxies are enabled, allow the bytecode to be built into a FunctionProxy instead of a FunctionBody.
            ulong flags = fscrIsLibraryCode | (CONFIG_FLAG(CreateFunctionProxy) && !scriptContext->IsProfiling() ? fscrAllowFunctionProxy : 0);
            HRESULT hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, (LPCUTF8)nullptr, hsi, (byte*)Js::Library_Bytecode_Promise, nullptr, &this->promiseByteCode);

            AssertMsg(SUCCEEDED(hr), "Failed to deserialize Promise.js bytecode - very probably the bytecode needs to be rebuilt.");
            IfFailThrowHr(hr);
        }
    }

    bool WinRTPromiseEngineInterfaceExtensionObject::InitializePromiseNativeInterfaces(Js::DynamicObject* promiseNativeInterfaces, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode)
    {
        typeHandler->Convert(promiseNativeInterfaces, mode, 9);

        Js::ScriptContext* scriptContext = promiseNativeInterfaces->GetScriptContext();
        Js::JavascriptLibrary* library = scriptContext->GetLibrary();

        // Promise has a dependency on the Debug object type being constructed so
        // undefer it here.
        Js::DynamicObject* debugObject = library->GetDebugObject();
        debugObject->GetDynamicType()->GetTypeHandler()->EnsureObjectReady(debugObject);
        // msTraceAsyncCallbackStarting([asyncOperationId: number=-1], [workType: number=1], [logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msTraceAsyncCallbackStarting, &AsyncDebug::EntryInfo::BeginAsyncCallback, 3);
        // msTraceAsyncCallbackCompleted([logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msTraceAsyncCallbackCompleted, &AsyncDebug::EntryInfo::CompleteAsyncCallback, 1);
        // msTraceAsyncOperationCompleted([asyncOperationID: number=-1], [status: number=1], [logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msTraceAsyncOperationCompleted, &AsyncDebug::EntryInfo::CompleteAsyncOperation, 3);
        // msUpdateAsyncCallbackRelation([relatedAsyncOperationID: number=-1], [relationType: number=5], [logLevel: number=1]):undefined.
        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::msUpdateAsyncCallbackRelation, &AsyncDebug::EntryInfo::UpdateAsyncCallbackStatus, 3);

        library->AddFunctionToLibraryObject(promiseNativeInterfaces, Js::PropertyIds::enqueueTask, &WinRTPromiseEngineInterfaceExtensionObject::EntryInfo::Promise_EnqueueTask, 2);

        // MS_ASYNC_OP_STATUS_SUCCESS: number=1.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_OP_STATUS_SUCCESS, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Completed, scriptContext), PropertyNone);
        // MS_ASYNC_OP_STATUS_CANCELED: number=2.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_OP_STATUS_CANCELED, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Canceled, scriptContext), PropertyNone);
        // MS_ASYNC_OP_STATUS_ERROR: number=3.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_OP_STATUS_ERROR, Js::JavascriptNumber::New(AsyncDebug::AsyncOperationStatus_Error, scriptContext), PropertyNone);
        // MS_ASYNC_CALLBACK_STATUS_ERROR: number=4.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::MS_ASYNC_CALLBACK_STATUS_ERROR, Js::JavascriptNumber::New(AsyncDebug::AsyncCallbackStatus_Error, scriptContext), PropertyNone);
        // setNonUserCodeExceptions(enableNonUserCodeExceptions: boolean):undefined. This is a setter.
        library->AddMember(promiseNativeInterfaces, Js::PropertyIds::setNonUserCodeExceptions, library->GetDebugObjectNonUserSetterFunction(), PropertyNone);

        promiseNativeInterfaces->SetHasNoEnumerableProperties(true);

        return true;
    }

#if DBG
    void WinRTPromiseEngineInterfaceExtensionObject::DumpByteCode()
    {
        Output::Print(_u("Dumping Promise Byte Code:"));
        this->EnsurePromiseByteCode(GetScriptContext());
        Js::ByteCodeDumper::DumpRecursively(promiseByteCode);
    }
#endif

    Js::Var WinRTPromiseEngineInterfaceExtensionObject::GetPromiseConstructor(_In_ Js::ScriptContext * scriptContext)
    {
        if (!scriptContext->VerifyAlive()) // Can't initialize if scriptContext closed, will need to run script
        {
            return nullptr;
        }

        Js::JavascriptLibrary* library = scriptContext->GetLibrary();
        this->EnsurePromiseByteCode(scriptContext);

        Assert(promiseByteCode != nullptr);

        Js::ScriptFunction* function = library->CreateScriptFunction(promiseByteCode->GetNestedFunctionForExecution(0));

        // If we are profiling, we need to register the script to the profiler callback, so the script compiled event will be sent.
        if (scriptContext->IsProfiling())
        {
            scriptContext->RegisterScript(function->GetFunctionProxy());
        }

        // Mark we are profiling library code already, so that any initialization library code called here won't be reported to profiler.
        // Also tell the debugger not to record events during intialization so that we don't leak information about initialization.
        Js::AutoInitLibraryCodeScope autoInitLibraryCodeScope(scriptContext);

        Js::Var args[] = { scriptContext->GetLibrary()->GetUndefined(), library->GetEngineInterfaceObject() };
        Js::CallInfo callInfo(Js::CallFlags_Value, _countof(args));
        Js::Var value = Js::JavascriptFunction::CallRootFunctionInScript(function, Js::Arguments(callInfo, args));

        return value;
    }

    Js::Var WinRTPromiseEngineInterfaceExtensionObject::EntryPromise_EnqueueTask(Js::RecyclableObject *function, Js::CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        RUNTIME_ARGUMENTS(args, callInfo);
        Assert(!(callInfo.Flags & CallFlags_New));
        unsigned argCount = args.Info.Count;
        Js::ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(argCount > 0, "Should always have implicit 'this'");

        if (callInfo.Count >= 2 && Js::JavascriptFunction::Is(args.Values[1]))
        {
            Js::JavascriptFunction* taskVar = Js::JavascriptFunction::FromVar(args.Values[1]);
            scriptContext->GetLibrary()->EnqueueTask(taskVar);
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

}
