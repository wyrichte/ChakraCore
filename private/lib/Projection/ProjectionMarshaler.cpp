//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#include "ProjectionPch.h"
#include "Library\JavascriptBooleanObject.h"
#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"
#include "JavascriptWinRTDate.h"

#ifdef PROJECTION_METADATA_TRACE
#define TRACE_METADATA(...) { ProjectionMarshaler::Trace(__VA_ARGS__); }
#else
#define TRACE_METADATA(...)
#endif

namespace Projection
{
    using namespace ProjectionModel;

    // Info:        Helper converts a name into an id
    // Parameters:  scriptContext - the script context
    //              propertyName - the property name
    PropertyId IdOfString(Js::ScriptContext *scriptContext, LPCWSTR propertyName)
    {
        // Bind the string to the ThreadContext
        return scriptContext->GetThreadContext()->GetOrAddPropertyRecordBind(
            JsUtil::CharacterBuffer<WCHAR>(propertyName, Js::JavascriptString::GetBufferLength(propertyName)))->GetPropertyId();
    }

    // Info:        Helper converts an id into a name
    // Parameters:  scriptContext - the script context
    //              id - the property id
    LPCWSTR StringOfId(Js::ScriptContext *scriptContext, PropertyId id)
    {
        Js::PropertyRecord const * nameStr = scriptContext->GetPropertyName(id);
        Js::VerifyCatastrophic(nameStr != nullptr);
#pragma warning(suppress: 6011) // nameStr != nullptr by VerifyCatastrophic()
        return nameStr->GetBuffer();
    }

    // Info:        Helper returns the approximate size indicated by the given gcPressure
    // Parameters:  gcPressure - the GC Pressure enum value (0=low, 1=medium, 2=high)
    size_t GetApproximateSizeForGCPressure(INT32 gcPressure)
    {
        switch (gcPressure)
        {
        case /*low*/0:
            return 10*1024; //10kB
        case /*medium*/1:
            return 100*1024; //100kB
        case /*high*/2:
            return 1024*1024; //1MB
        default:
            return 0;
        }
    }

    // Info:        Get a projection object instance as an extension to the given CustomExternalObject
    // Parameters:  scriptContext - the script context
    //              instance - the object instance
    ProjectionObjectInstance * GetProjectionObjectInstanceFromVar(Js::ScriptContext * scriptContext, Var instance)
    {
        if(!ProjectionObjectInstance::Is(instance))
        {
            // For example, a protoype method or property is called directly
            // var x = Animals.Dino.prototype.Height
            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch); 

        }
        return (ProjectionObjectInstance*)instance;
    }


    // Info:        Get a projection object instance as an extension to the given CustomExternalObject
    // Parameters:  instance - the object instance
    ProjectionObjectInstance * GetProjectionObjectInstanceFromVarNoThrow(Var instance)
    {
        if(ProjectionObjectInstance::Is(instance))
        {
            return (ProjectionObjectInstance*)instance;
        }

        return nullptr;
    }

    // Info:        Get an IUnknown from the given object encoded as a var extension
    // Parameters:  scriptContext - the script context
    //              instance - the object instance
    //              iid - the IID of the object to query for
    //              pp - receives the unknown
    //              isDefaultInterface - out value indicating whether the iid requested is the default interface of the expected 'this' object
    //              addRefDefault - flag indicating whether the interface should be addRef'd if it is the default interface
    //              methodName - name of the method, if we are verifying a 'this' pointer
    //              expectedTypeId - the expected type of the 'this' object
    void GetUnknownOfVarExtension(Js::ScriptContext * scriptContext, Var instance, const IID & iid, void ** pp, bool * isDefaultInterface, bool addRefDefault, MetadataStringId methodNameId, MetadataStringId expectedTypeId)
    {
        auto ceo = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(instance);
        if (ceo)
        {
            if ((expectedTypeId != MetadataStringIdNil) && (ceo->GetTypeNameId() != expectedTypeId))
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedWinRTType, StringOfId(scriptContext, expectedTypeId)); 
            }
        }

        ProjectionObjectInstance *thisObject = GetProjectionObjectInstanceFromVarNoThrow(instance);
        if(thisObject == nullptr)
        {
            // For example, a protoype method or property is called directly
            // var x = Animals.Dino.prototype.Height

            // Should only have a methodName if we are verifing 'this'
            if (methodNameId != MetadataStringIdNil)
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedInspectableObject, StringOfId(scriptContext, methodNameId)); 
            }
            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch); 
        }

        if (thisObject->GetUnknown() == nullptr)
        {
            if (methodNameId != MetadataStringIdNil)
            {
                Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject, StringOfId(scriptContext, methodNameId));
            }
            Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject);
        }

        *pp = (LPVOID)(thisObject->GetInterfaceOfNativeABI(iid, scriptContext, isDefaultInterface, addRefDefault));
    }

    HRESULT QueryInterfaceAfterLeaveScript(Js::ScriptContext *scriptContext, IUnknown *unknown, const IID & iid, void ** pp)
    {
        Assert(unknown != nullptr);
        HRESULT hr = S_OK;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = unknown->QueryInterface(iid, pp);
        }
        END_LEAVE_SCRIPT(scriptContext)
        return hr;
    }

    void VerifyDeprecatedAttributeOnce(RtABIMETHODSIGNATURE rtmethod, Js::ScriptContext* scriptContext, DeprecatedInvocationType methodType)
    {
        if (!scriptContext->GetThreadContext()->IsScriptActive())
        {
            // We can be called at close time when we tried to cleanup the projectionContext. At that time we are not in script, and we
            // don't really need to show the console warning either.
            return;
        }
        if (rtmethod->deprecatedAttributes != nullptr)
        {
            auto localDeprecatedAttributes = rtmethod->deprecatedAttributes;
            ((ProjectionModel::AbiMethodSignature*)rtmethod)->deprecatedAttributes = nullptr;
            IActiveScriptDirect* scriptDirect = scriptContext->GetActiveScriptDirect();
            LPWSTR methodStr = nullptr;
            localDeprecatedAttributes->Iterate([&](DeprecatedAttribute deprecatedAttribute){
                Js::StringBuilder<Recycler>* stringBuilder = Js::StringBuilder<Recycler>::New(scriptContext->GetRecycler(), 256);
                const Js::PropertyRecord* methodName = scriptContext->GetPropertyName(rtmethod->nameId);
                const Js::PropertyRecord* qualifyingName;
                if (deprecatedAttribute.rtcNameId != MetadataStringIdNil)
                {
                    // if runtimeclass name is specified, the method is defined from an interface with exclusiveto attribute. the method
                    // is not published, and we'll output the runtimeclass name from the exclusiveto attribute instead.
                    qualifyingName = scriptContext->GetPropertyName(deprecatedAttribute.rtcNameId);
                    TRACE_METADATA(_u("Deprecated from rtcName: %s\n"), qualifyingName);
                }
                else
                {
                    qualifyingName = scriptContext->GetPropertyName(deprecatedAttribute.classId);
                    TRACE_METADATA(_u("Deprecated from interface: %s\n"), qualifyingName);
                }
                // In IE10 and above, console.log is available by default.
                stringBuilder->AppendCppLiteral(_u("try { "));
                stringBuilder->AppendCppLiteral(_u("console.warn(\""));
                stringBuilder->AppendCppLiteral(_u("The "));
                switch (methodType)
                {
                case DeprecatedInvocation_Method:
                    if (rtmethod->methodKind == MethodKind_Getter ||
                        rtmethod->methodKind == MethodKind_Setter )
                    {
                        stringBuilder->AppendCppLiteral(_u("property"));
                    }
                    else
                    {
                        stringBuilder->AppendCppLiteral(_u("method"));
                    }
                    break;
                case DeprecatedInvocation_Class:
                    stringBuilder->AppendCppLiteral(_u("class"));
                    break;
                case DeprecatedInvocation_Delegate:
                    stringBuilder->AppendCppLiteral(_u("delegate"));
                    break;
                case DeprecatedInvocation_Event:
                    stringBuilder->AppendCppLiteral(_u("event"));
                    break;
                }
                stringBuilder->Append(_u(' '));
                stringBuilder->Append(qualifyingName->GetBuffer(), qualifyingName->GetLength());
                size_t methodNameOffset = 0;
                if (rtmethod->methodKind == MethodKind_Getter) 
                {
                    methodNameOffset = wcslen(_u("get_"));
                    Assert(wcslen(methodName->GetBuffer()) > methodNameOffset);
                }
                if (rtmethod->methodKind == MethodKind_Setter) 
                {
                    methodNameOffset = wcslen(_u("put_"));
                    Assert(wcslen(methodName->GetBuffer()) > methodNameOffset);
                }

                // methodType is set at runtime, while methodKind is set at parse time. 
                // we don't have an easy way to have the methodType information at parse time. 
                if (methodType == DeprecatedInvocation_Method ||
                    methodType == DeprecatedInvocation_Event)
                {
                    stringBuilder->Append(_u('.'));
                    AssertMsg(methodNameOffset < UINT32_MAX, "Invalid metadata: Method name sizes constrain to 2gb.");
                    stringBuilder->Append(methodName->GetBuffer() + methodNameOffset, methodName->GetLength() - (charcount_t)methodNameOffset);
                }
                stringBuilder->AppendCppLiteral(_u(" has been deprecated. "));
                stringBuilder->AppendSz(deprecatedAttribute.infoString);
                stringBuilder->AppendCppLiteral(_u("\");} catch(e) {};"));
                methodStr = stringBuilder->Detach();
                TRACE_METADATA(_u("Deprecated attribute message:%s\n"), methodStr);
                Var scriptFunc;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {            
                    HRESULT hr = scriptDirect->Parse(methodStr, &scriptFunc);
                    if (SUCCEEDED(hr))
                    {
                        Var varResult;
                        CallInfo callInfo = {1, CallFlags_None};
                        Var args[1];
                        args[0] = scriptFunc;
                        hr = scriptDirect->Execute(scriptFunc, callInfo, args, NULL, &varResult);
                    }
                }
                END_LEAVE_SCRIPT(scriptContext)
            });
        };
    }

    Var DoInvoke(ThisInfo *thisInfo, RtABIMETHODSIGNATURE signature, bool boundsToUndefined, Var * args, ushort argCount, ProjectionContext *projectionContext)
    {
        Assert(argCount > 0);
        Assert(projectionContext->GetScriptContext()->GetThreadContext()->IsScriptActive());
        Js::CallInfo callInfo(Js::CallFlags_Value, argCount);
        VerifyDeprecatedAttributeOnce(signature, projectionContext->GetScriptContext(), DeprecatedInvocation_Method);
        return InvokeMethodByThisInfo(thisInfo, signature, boundsToUndefined, args[0], Js::Arguments(callInfo, args), projectionContext);
    }

    // Info:        Helper which dispatches a method base on the type of 'this'
    // Parameters:  thisInfo - description of 'this'
    //              rtmethod - method to dispatch on this
    //              _this - the var which holds this
    //              args - function arguments
    //              projectionContext - projection Context
    Var InvokeMethodByThisInfo(ThisInfo * thisInfo, RtABIMETHODSIGNATURE rtmethod, bool boundsToUndefined, Var _this, Js::Arguments args, ProjectionContext *projectionContext)
    {
#if DBG
        ProjectionModel::AllowHeavyOperation allow;
#endif
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        // in + this
        if (size_t(args.Info.Count) < size_t(rtmethod->inParameterCount + 1))
        {
            Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, StringOfId(scriptContext, rtmethod->nameId));
        }

        bool isMethodAsync = false;
        RtABIPARAMETER asyncParameter = nullptr;
        AsyncDebug::AsyncOperationId asyncOperationId = AsyncDebug::InvalidAsyncOperationId;
        RuntimeClassThis * runtimeClassThis = nullptr;
        HRESULT hr;

        if (AsyncDebug::IsAsyncDebuggingEnabled(scriptContext))
        {
            isMethodAsync = projectionContext->GetProjectionAsyncDebug()->InstrumentAsyncMethodCallByScanningParameters(thisInfo, rtmethod, &asyncParameter, &asyncOperationId);
        }

        ProjectionMethodInvoker invoker(rtmethod, projectionContext);
        if (thisInfo->GetSpecialization() != nullptr && 
            thisInfo->GetSpecialization()->specializationType == specPromiseSpecialization &&
            (0 == wcscmp(StringOfId(scriptContext, rtmethod->nameId), _u("put_Completed"))))
        {
            // When the containing interface is derived from IAsyncInfo, the delegate created from the interface is treated as "async", and we support
            // priority delegate that host can prioritize the work items as needed. We need to set this out at method invocation time such that we 
            // have the information in parameter marshalling time.
            invoker.SetIsInAsyncInterface();
        }

        if (thisInfo->GetSpecialization() != nullptr && 
            thisInfo->GetSpecialization()->specializationType == specPromiseSpecialization &&
            (0 == wcscmp(StringOfId(scriptContext, rtmethod->nameId), _u("put_Progress"))))
        {
            // N.B. For IAsyncInfoWithProgress put_Progress supplies a delegate that accepts (IInspectable* operation, T progress).
            // Our existing delegate wrappers do not understand how to marshal type T, which is passed by value and not
            // as an IInspectable pointer.  STA threads can implicitly marshal this data through COM.  MTA threads do not
            // know how to marshal this data back to the correct Js thread.  Therfore we disable the progress call until a solution
            // can be implemented.
            APTTYPE aptType = APTTYPE_CURRENT;
            APTTYPEQUALIFIER aptQualifier = APTTYPEQUALIFIER_NONE;

            if (CoGetApartmentType(&aptType, &aptQualifier) != S_OK ||
                (aptType != APTTYPE_STA &&
                 aptType != APTTYPE_MAINSTA))
            {
                return scriptContext->GetLibrary()->GetUndefined();
            }
        }

        switch(thisInfo->thisType)
        {
        case thisDelegate:
            {
                DelegateThis * delegateThis = reinterpret_cast<DelegateThis*>(thisInfo);

#ifdef ENABLE_JS_ETW
                if (EventEnabledJSCRIPT_PROJECTION_INVOKENATIVEDELEGATE_START())
                {
                    LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                    LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                    EventWriteJSCRIPT_PROJECTION_INVOKENATIVEDELEGATE_START(runtimeClassName, methodName);
                }
#endif

                // Query interface for the iid
                IUnknown *unknown = nullptr;
                hr = QueryInterfaceAfterLeaveScript(scriptContext, delegateThis->_this, rtmethod->iid->instantiated, (void **)&unknown);
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                // InvokeUnknown takes ownership of this unknown and would release it.
                hr = invoker.InvokeUnknown(unknown, rtmethod->vtableIndex+2, args, false, true);
            }
            break;
        case thisRuntimeClass:
            {
                runtimeClassThis = reinterpret_cast<RuntimeClassThis*>(thisInfo);
                IUnknown * factory;

                if (runtimeClassThis->factory == nullptr)
                {
                    FastPathPopulateRuntimeClassThis(runtimeClassThis, projectionContext);
                }

                // This is a QI so we do not want to mark the callout for debug-stepping (via MarkerForExternalDebugStep).
                Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);

                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    // InvokeUnknown takes ownership of this unknown and would release it.
#pragma warning(suppress: 6011) // runtimeClassThis->factory set by FastPathPopulateRuntimeClassThis()
                    hr = runtimeClassThis->factory->QueryInterface(rtmethod->iid->instantiated, (void **)&factory);
                }
                END_LEAVE_SCRIPT(scriptContext)

                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                JS_ETW(EventWriteJSCRIPT_PROJECTION_CONSTRUCTRUNTIMECLASS_START(projectionContext->StringOfId(runtimeClassThis->typeId)));
                hr = invoker.InvokeUnknown(factory, rtmethod->vtableIndex+6, args);
            }
            break;
        case thisUnknownEventHandling:
        case thisUnknown: 
            {
                IUnknown *unknown = nullptr;
#ifdef ENABLE_JS_ETW
                if (EventEnabledJSCRIPT_PROJECTION_METHODCALL_START())
                {
                    LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                    LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                    EventWriteJSCRIPT_PROJECTION_METHODCALL_START(runtimeClassName, methodName);
                }
#endif

                bool isDefaultInterface = false;
                GetUnknownOfVarExtension(scriptContext, _this, rtmethod->iid->instantiated, (void**)&unknown, &isDefaultInterface, false, rtmethod->nameId, thisInfo->GetTypeId());

                // InvokeUnknown takes ownership of this unknown and would release it.
                hr = invoker.InvokeUnknown(unknown, rtmethod->vtableIndex+6, args, isDefaultInterface);
            }
            break;
        default:
            Js::Throw::FatalProjectionError();
        }

        Var result;

        if (isMethodAsync)
        {
            AsyncParameterMarker marker(asyncParameter, asyncOperationId);

            result = invoker.ReadOutOrThrow(hr, boundsToUndefined, args, &marker);
        }
        else
        {
            result = invoker.ReadOutOrThrow(hr, boundsToUndefined, args);
        }

        switch (thisInfo->thisType)
        {
        case thisUnknownEventHandling:
        case thisUnknown: 
#ifdef ENABLE_JS_ETW
            if (EventEnabledJSCRIPT_PROJECTION_METHODCALL_STOP())
            {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                EventWriteJSCRIPT_PROJECTION_METHODCALL_STOP(runtimeClassName, methodName);
            }
#endif
            break;
        case thisRuntimeClass:
            JS_ETW(EventWriteJSCRIPT_PROJECTION_CONSTRUCTRUNTIMECLASS_STOP(projectionContext->StringOfId(runtimeClassThis->typeId)));
            break;
        case thisDelegate:
#ifdef ENABLE_JS_ETW
            if (EventEnabledJSCRIPT_PROJECTION_INVOKENATIVEDELEGATE_STOP())
            {
                LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                EventWriteJSCRIPT_PROJECTION_INVOKENATIVEDELEGATE_STOP(runtimeClassName, methodName);
            }
#endif
            break;
        default:
            Js::Throw::FatalProjectionError();
        }        

        return result;
    }

    // Info:        Dynamic (non-fast-path) projection call
    Var DynamicCall(Signature * signature, Var method, Js::Arguments & args, Js::CallInfo & callInfo)
    {
        Assert(callInfo.Count > 0);
        Assert(Js::JavascriptWinRTFunction::FromVar(method)->GetScriptContext()->GetThreadContext()->IsScriptActive());
        Var result = NULL;
        VerifyDeprecatedAttributeOnce(signature->method, signature->projectionContext->GetScriptContext(), 
            (callInfo.Flags & CallFlags_New) ? DeprecatedInvocation_Class : DeprecatedInvocation_Method);
        result = InvokeMethodByThisInfo(signature->thisInfo, signature->method, signature->boundsToUndefined, args[0], args, signature->projectionContext);
        return result;
    }


    // Info:        Called to dispatch a single (non-overload) function
    // Parameters:  standard thunk parameters
    Var MethodSignatureThunk(Var method, Js::CallInfo callInfo, ...)
    {
        Var result;
#if DBG
        auto func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
        ProjectionModel::AllowHeavyOperation allow;
#endif
        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptWinRTFunction::FromVar(method);
        Signature * signature = reinterpret_cast<Signature*>(function->GetSignature());
        result = DynamicCall(signature, method, args, callInfo);
        return result;
    }

    bool ThisInfo::IsArray()
    {
        return (specialization != nullptr && (specialization->specializationType == specVectorSpecialization || specialization->specializationType == specVectorViewSpecialization)); 
    }


    // Info:        Construct
    // Parameters:  resourceCleanup - model to use for resource cleanup
    //              projectionContext - the projection context
    ProjectionMarshaler::ProjectionMarshaler(ResourceCleanup resourceCleanup, ProjectionContext * projectionContext, bool fReleaseExistingResource)
        : projectionContext(projectionContext), hstrings(nullptr), addrefs(nullptr), recyclerVars(nullptr), resourceCleanup(resourceCleanup), fReleaseExistingResource(fReleaseExistingResource),
        fReleaseOutResources(false), outHstrings(nullptr), outUnknowns(nullptr), fReleaseDelegateOutResources(false), delegateOutHstrings(nullptr), delegateOutUnknowns(nullptr),
        delegateOutMemory(nullptr), delegateOutArrayContents(nullptr), fInAsyncInterface(false)
    {
        allocatorObject = projectionContext->GetScriptContext()->GetTemporaryAllocator(_u("ProjectionMarshaler")); 
        alloc = allocatorObject->GetAllocator();

        // Save a unknownsReleaser for this instance of projection marshaler
#if DBG
        unknownsReleaser = 
#endif
            projectionContext->GetProjectionWriter()->GetUnknownsReleaser();
    }

    // Info:        Destruct. Release any held addrefs or hstrings
    ProjectionMarshaler::~ProjectionMarshaler()
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        recyclerVars->Iterate([&](Var recyclerVar) {
            scriptContext->GetRecycler()->RootRelease(recyclerVar);
        });

        auto winRtStringLibrary = projectionContext->GetThreadContext()->GetWinRTStringLibrary();
        
        hstrings->Iterate([&](HSTRING hs) {
            winRtStringLibrary->WindowsDeleteString(hs);
        });

        if (fReleaseOutResources)
        {
            outHstrings->Iterate([&](HSTRING *hs) {
                winRtStringLibrary->WindowsDeleteString(*hs);
            });
        }

        if (fReleaseDelegateOutResources)
        {
            delegateOutMemory->Iterate([&](MemoryCleanupRecord record) {
                memset(record.mem, 0, record.memsize);
            });

            delegateOutHstrings->Iterate([&](HSTRING hs) {
                winRtStringLibrary->WindowsDeleteString(hs);
            });
        }

        Assert(unknownsReleaser != nullptr);

        // BEGIN_LEAVE_SCRIPT would probe stack for Js::Constants::MinStackCallout and if it isnt available it would throw stack exception.
        // We dont want destructor throwing exception since it could have been called while unwinding stack after exception and would result in process terminate if not handled.
        // So we check for Js::Constants::MinStackCallout + 0xF0 
        // The additional bytes are rounded off to upperlimit of the stack bytes that would be used during Leave_Script but before the probe stack
        if ((addrefs->IsEmpty() 
            && (!fReleaseOutResources || outUnknowns->IsEmpty())
            && (!fReleaseDelegateOutResources || delegateOutUnknowns->IsEmpty())) 
            || scriptContext->GetThreadContext()->IsStackAvailableNoThrow(Js::Constants::MinStackCallout + 0xF0))
        {
            if (!addrefs->IsEmpty())
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    addrefs->Iterate([&](IUnknown * unknown) {
                        unknown->Release();
                    });
                }
                END_LEAVE_SCRIPT(scriptContext);
            }

            if (fReleaseOutResources && !outUnknowns->IsEmpty())
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    outUnknowns->Iterate([&](IUnknown **unknown) {
                        if (*unknown)
                        {
                            (*unknown)->Release();
                        }
                    });
                }
                END_LEAVE_SCRIPT(scriptContext);
            }

            if (fReleaseDelegateOutResources && !delegateOutUnknowns->IsEmpty())
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    delegateOutUnknowns->Iterate([&](IUnknown *unknown) {
                        unknown->Release();
                    });
                }
                END_LEAVE_SCRIPT(scriptContext);
            }

            projectionContext->GetProjectionWriter()->SetUnknownsReleaserAsUnused(
#if DBG
                unknownsReleaser
#endif
                );
        }
        else
        {
            // Prevent the leak by adding all these unknowns into the ProjectionWriter's finalizable var
            projectionContext->GetProjectionWriter()->SetUnknownsListForDisposingLater(
#if DBG
                unknownsReleaser,
#endif
                (fReleaseDelegateOutResources) ? delegateOutUnknowns->AppendListToCurrentList(addrefs) : addrefs, 
                (fReleaseOutResources) ? outUnknowns : nullptr,
                alloc);
        }

#if DBG
        unknownsReleaser = nullptr;
#endif
        scriptContext->ReleaseTemporaryAllocator(allocatorObject);
    }

    void ProjectionMarshaler::TransferOwnershipOfDelegateOutTypes()
    { 
        fReleaseDelegateOutResources = false; 
        delegateOutArrayContents->Iterate([&](FinalizableTypedArrayContents * contents) {
            contents->typedArrayBuffer = nullptr;
        });
    }

    void ProjectionMarshaler::RecordToReleaseExistingString(HSTRING hs)
    {
        if (fReleaseExistingResource)
        {
            Assert(resourceCleanup == CalleeRetainsOwnership);
            hstrings = hstrings->Prepend(hs, alloc);
        }
    }

    // Info:        Record an HSTRING to undo after the marshal
    // Parameters:  hr - the HSTRING
    void ProjectionMarshaler::RecordToUndo(HSTRING hs, bool forcedRecord)
    {
        if (resourceCleanup == CalleeTransfersOwnership || forcedRecord)
        {
            hstrings = hstrings->Prepend(hs,alloc);
        }
        else if (fReleaseDelegateOutResources)
        {
            delegateOutHstrings = delegateOutHstrings->Prepend(hs,alloc);
        }
    }

    void ProjectionMarshaler::RecordToReleaseExistingUnknown(IUnknown * unknown)
    {
        if (unknown && fReleaseExistingResource)
        {
            Assert(resourceCleanup == CalleeRetainsOwnership);
            addrefs = addrefs->Prepend(unknown, alloc);
        }
    }

    // Info:        Record an addref to undo after the marshal
    // Parameters:  unknown - the unknown which has an addref
    void ProjectionMarshaler::RecordToUndo(IUnknown * unknown, bool forceRelease)
    {
        if (unknown)
        {
            if (forceRelease || resourceCleanup == CalleeTransfersOwnership)
            {
                addrefs = addrefs->Prepend(unknown,alloc);
            }
            else if (fReleaseDelegateOutResources)
            {
                delegateOutUnknowns = delegateOutUnknowns->Prepend(unknown,alloc);
            }
        }
    }

    void ProjectionMarshaler::RecordRecyclerVar(Var recyclerVar, bool forcedRecord)
    {
        if (resourceCleanup == CalleeRetainsOwnership || forcedRecord)
        {
            Recycler *recycler = projectionContext->GetScriptContext()->GetRecycler();
#if DBG
            if (recycler->IsValidObject(recyclerVar))
#endif
            {
                recycler->RootAddRef(recyclerVar);
                recyclerVars = recyclerVars->Prepend(recyclerVar, alloc);
            }
        }
    }

    // Info:        Write an in pointer
    // Parameters:  pointer - a raw pointer to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInPointer(__in void * pointer, __in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        if (memSize<sizeof(LPVOID))
        {
            Js::Throw::FatalProjectionError();
        }
        *(void**)mem = pointer;
        return mem + sizeof(LPVOID);
    }


    // Info:        Write an in unknown - record to undo and release existing pointer before writing it.
    // Parameters:  pointer - a raw pointer to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInUnknown(__in IUnknown * unknown, __in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        RecordToUndo(unknown);
        RecordToReleaseExistingUnknown(*(IUnknown **)(mem));

        return WriteInPointer(unknown, mem, memSize);
    }

    // Info:        write a basic type
    // Parameters:  arg - the arg to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInBasicType(__in Var arg, __in RtBASICTYPE type, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        return WriteInCorElementType(arg, type->typeCor, mem, memSize);
    }

    // Info:        write a CorElementType type
    // Parameters:  arg - the arg to write
    //              typeCor - the type
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInCorElementType(__in Var arg, __in CorElementType typeCor, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        switch(typeCor)
        {
        case ELEMENT_TYPE_CHAR:
            {
                if (memSize<sizeof(char16))
                    Js::Throw::FatalProjectionError();
                LPCWSTR asString = (Js::JavascriptConversion::ToString(arg, scriptContext))->GetSz(); 
                if(asString != NULL && ::wcslen(asString) == 1)
                {
                    *(char16*)mem = asString[0];
                    return mem + sizeof(char16);
                }
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedWinRTChar); 
            }
        case ELEMENT_TYPE_BOOLEAN:
            return WriteInBoolean(arg, mem, memSize);

        case ELEMENT_TYPE_U1:
            {
                if (memSize<sizeof(unsigned __int8))
                    Js::Throw::FatalProjectionError();
                auto result = Js::JavascriptConversion::ToInt32(arg, scriptContext);
                *(unsigned __int8*)mem = (unsigned __int8)result;
                return mem + sizeof(unsigned __int8);
            }
        case ELEMENT_TYPE_I2:
            if (memSize<sizeof(__int16))
                Js::Throw::FatalProjectionError();
            *(__int16*)mem = (__int16)Js::JavascriptConversion::ToInt32(arg, scriptContext);
            return mem + sizeof(__int16);
        case ELEMENT_TYPE_U2:
            if (memSize<sizeof(unsigned __int16))
                Js::Throw::FatalProjectionError();
            *(unsigned __int16*)mem = (unsigned __int16)Js::JavascriptConversion::ToInt32(arg, scriptContext);
            return mem + sizeof(unsigned __int16);
        case ELEMENT_TYPE_I4:
            if (memSize<sizeof(__int32))
                Js::Throw::FatalProjectionError();
            *(__int32*)mem = (__int32)Js::JavascriptConversion::ToInt32(arg, scriptContext);
            return mem + sizeof(__int32);
        case ELEMENT_TYPE_U4:
            if (memSize<sizeof(unsigned __int32))
                Js::Throw::FatalProjectionError();
            *(unsigned __int32*)mem = (unsigned __int32)Js::JavascriptConversion::ToInt32(arg, scriptContext);
            return mem + sizeof(unsigned __int32);
        case ELEMENT_TYPE_I8:
            if (memSize<sizeof(__int64))
                Js::Throw::FatalProjectionError();
            *(__int64*)mem = Js::JavascriptConversion::ToInt64(arg, scriptContext);
            return mem + sizeof(__int64);
        case ELEMENT_TYPE_U8:
            if (memSize<sizeof(unsigned __int64))
                Js::Throw::FatalProjectionError();
            *(unsigned __int64*)mem = Js::JavascriptConversion::ToUInt64(arg, scriptContext);
            return mem + sizeof(unsigned __int64);
        case ELEMENT_TYPE_R4:
            if (memSize<sizeof(float))
                Js::Throw::FatalProjectionError();
            *(float*)mem = (float)Js::JavascriptConversion::ToNumber(arg, scriptContext);
            return mem + sizeof(float);
        case ELEMENT_TYPE_R8:
            if (memSize<sizeof(double))
                Js::Throw::FatalProjectionError();
            *(double*)mem = Js::JavascriptConversion::ToNumber(arg, scriptContext);
            return mem + sizeof(double);
        case ELEMENT_TYPE_STRING:
            {
                return WriteInString(arg, mem, memSize);
            }
        case ELEMENT_TYPE_OBJECT:
            return WriteInspectableObject(arg, mem, memSize);
        }
        Js::Throw::FatalProjectionError();
    }

    byte * ProjectionMarshaler::WriteInBoolean(Var varInput, __in_bcount(memSize) byte *mem, __in size_t memSize)
    {
        if (memSize<sizeof(bool))
        {
            Js::Throw::FatalProjectionError();
        }

        if (Js::JavascriptOperators::GetTypeId(varInput) == Js::TypeIds_BooleanObject)
        {
            *(bool*)mem = Js::JavascriptBooleanObject::FromVar(varInput)->GetValue() ? true: false;
        }
        else
        {
            *(bool*)mem = Js::JavascriptConversion::ToBoolean(varInput, projectionContext->GetScriptContext()) ? true: false;
        }

        C_ASSERT(sizeof(bool) == sizeof(boolean));
        return mem + sizeof(boolean);
    }

    byte * ProjectionMarshaler::WriteInString(Var varInput, __in_bcount(memSize) byte *mem, __in size_t memSize)
    {
        if (memSize<sizeof(HSTRING))
        {
            Js::Throw::FatalProjectionError();
        }

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        Js::JavascriptString * str = Js::JavascriptConversion::ToString(varInput, scriptContext);
        HSTRING hs;
        HRESULT hr = projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsCreateString(str->GetSz(), str->GetLength(),&hs);
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

        RecordToReleaseExistingString(*(HSTRING*)mem);
        *(HSTRING*)mem = hs;
        RecordToUndo(hs);

        return mem + sizeof(HANDLE);
    }

    byte * ProjectionMarshaler::WriteIReference(Var varInput, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        Assert(constructor != nullptr);
        Assert(constructor->iid->piid == IID_IReference1);
        Assert(!constructor->genericParameters->IsEmpty());

        RtCONCRETETYPE type = ProjectionModel::ConcreteType::From(constructor->genericParameters->First());
        RtBASICTYPE basicType = (type->typeCode == tcBasicType) ? BasicType::From(type) : nullptr;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        // The types T are not allowed as IReference<T> if it is HSTRING, interface, runtimeclass, delegates or object
        if (type->typeCode == tcInterfaceType 
            || type->typeCode == tcClassType 
            || type->typeCode == tcDelegateType 
            || (basicType != nullptr && (basicType->typeCor == ELEMENT_TYPE_OBJECT || basicType->typeCor == ELEMENT_TYPE_STRING)))
        {
            Assert(0);
            Js::JavascriptError::MapAndThrowError(scriptContext, VBSERR_ActionNotSupported);
        }

        Js::TypeId typeIdInput = Js::JavascriptOperators::GetTypeId(varInput);

        if (typeIdInput == Js::TypeIds_Null || typeIdInput == Js::TypeIds_Undefined)
        {
            return WriteInUnknown(nullptr, mem, memSize);
        }

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_WRITEIREFERENCE_START())
        {
            EventWriteJSCRIPT_PROJECTION_WRITEIREFERENCE_START(StringOfId(type->fullTypeNameId));
        }
#endif

        byte *typeStorage = AnewArrayZ(alloc, byte, type->storageSize);

        // Marshal Value into the type
        ProjectionMarshaler marshal(CalleeTransfersOwnership, projectionContext, false);
        marshal.WriteInType(varInput, type, typeStorage, type->storageSize, true); 

        // Create the propertyValue
        IInspectable *propertyValue = nullptr;
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        HRESULT hr = GetNonArrayTypeAsPropertyValue(type->storageSize, typeStorage, type, &propertyValue);
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
        RecordToUndo(propertyValue, true);

        IInspectable *inspectable = nullptr;
        hr = QueryInterfaceAfterLeaveScript(scriptContext, propertyValue, constructor->iid->instantiated, (LPVOID *)&inspectable);
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_WRITEIREFERENCE_STOP())
        {
            EventWriteJSCRIPT_PROJECTION_WRITEIREFERENCE_STOP(StringOfId(type->fullTypeNameId));
        }
#endif

        return WriteInUnknown(inspectable, mem, memSize);
    }

    byte * ProjectionMarshaler::WriteIReferenceArray(Var varInput, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        Assert(constructor != nullptr);
        Assert(constructor->iid->piid == IID_IReferenceArray1);
        Assert(!constructor->genericParameters->IsEmpty());

        // The types T are not allowed as IReferenceArray<T> cannot be parameter to method
        Assert(0);
        Js::JavascriptError::MapAndThrowError(projectionContext->GetScriptContext(), VBSERR_ActionNotSupported);
    }

    byte * ProjectionMarshaler::WriteInspectableObject(Var varInput, __in_bcount(memSize) byte *mem, __in size_t memSize, bool boxInterface)
    {
        JS_ETW(EventWriteJSCRIPT_PROJECTION_WRITEINSPECTABLE_START(boxInterface));

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);

        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
        Windows::Foundation::IPropertyValueStatics *propertyValueFactory = projectionWriter->GetPropertyValueFactory();
        IInspectable *propertyValue = nullptr;
        HRESULT hr = S_OK;
        if (varInput != nullptr)
        {
            switch(Js::JavascriptOperators::GetTypeId(varInput))
            {
                // Undefined or null
            case Js::TypeIds_Null:
            case Js::TypeIds_Undefined:
                {
                    propertyValue = nullptr;
                }
                break;

                // Booleans
            case Js::TypeIds_Boolean:
            case Js::TypeIds_BooleanObject:
                {
                    boolean boolFromVar = false;
                    WriteInBoolean(varInput, &boolFromVar, sizeof(boolFromVar));
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = propertyValueFactory->CreateBoolean(boolFromVar, &propertyValue);
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                }
                break;

                // String
            case Js::TypeIds_String:
            case Js::TypeIds_StringObject:
                {
                    HSTRING hs = nullptr;
                    ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
                    marshal.WriteInString(varInput, (byte *)&hs, sizeof(HSTRING));
                    RecordToUndo(hs, true);
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = propertyValueFactory->CreateString(hs, &propertyValue);
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                }
                break;

                // Number
            case Js::TypeIds_Int64Number:
                {
                    __int64 int64Value = Js::JavascriptConversion::ToInt64(varInput, scriptContext);
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = propertyValueFactory->CreateInt64(int64Value, &propertyValue);
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                }
                break;

            case Js::TypeIds_UInt64Number:
                {
                    unsigned __int64 uint64Value = Js::JavascriptConversion::ToUInt64(varInput, scriptContext);
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = propertyValueFactory->CreateUInt64(uint64Value, &propertyValue);
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                }
                break;

            case Js::TypeIds_Integer:
            case Js::TypeIds_Number:
            case Js::TypeIds_NumberObject:
                {
                    double doubleValue = Js::JavascriptConversion::ToNumber(varInput, scriptContext);
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = propertyValueFactory->CreateDouble(doubleValue, &propertyValue);
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                }
                break;

                // Date
            case Js::TypeIds_Date:
            case Js::TypeIds_WinRTDate:
                {
                    Windows::Foundation::DateTime dateFromVar;
                    WriteInDate(varInput, (byte *)&dateFromVar, sizeof(dateFromVar));
                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = propertyValueFactory->CreateDateTime(dateFromVar, &propertyValue);
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                }
                break;

                // Js Array
            case Js::TypeIds_Array:     
            case Js::TypeIds_NativeIntArray:
            case Js::TypeIds_NativeFloatArray:
            case Js::TypeIds_ES5Array:
                {
                    uint arrayLength = ArrayAsCollection::GetLength(static_cast<Js::JavascriptArray *>(varInput));
                    size_t sizeOfArray = sizeof(IInspectable*) * arrayLength;
                    IInspectable **inspectableValues = AnewArrayZ(alloc, IInspectable *, arrayLength);
                    ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);

                    byte *currentPointer = (byte *)inspectableValues;
                    for (uint index = 0; index < arrayLength; index++)
                    {
                        Var varIndex = Js::JavascriptNumber::ToVar(index, scriptContext);
                        Var varElement = Js::JavascriptOperators::OP_GetElementI(varInput, varIndex, scriptContext);

                        byte *nextPointer = marshal.WriteInspectableObject(varElement, currentPointer, sizeOfArray);
                        RecordToUndo((*(IUnknown **)currentPointer), true);

                        currentPointer = nextPointer;
                        Assert(static_cast<uint>(currentPointer - (byte *)inspectableValues) == (index + 1) * sizeof(LPVOID));
                        sizeOfArray = sizeOfArray - sizeof(LPVOID);
                    }

                    BEGIN_LEAVE_SCRIPT(scriptContext)
                    {
                        hr = propertyValueFactory->CreateInspectableArray(arrayLength, inspectableValues, &propertyValue);
                    }
                    END_LEAVE_SCRIPT(scriptContext)
                }
                break;

                // Typed Arrays
            case Js::TypeIds_Int8Array:
                // Win8 doesn't have Int8 arrays
                if (boxInterface)
                {
                    Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidPropertyValue, _u("Int8Array"));
                }
                else
                {
                    Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidIInspectable, _u("Int8Array"));
                }

            case Js::TypeIds_CopyOnAccessNativeIntArray:
                Assert(false);
                // fall-through

            default:
                {
                    if (Js::TypedArrayBase::Is(varInput))
                    {
                        hr = GetBuiltinTypedArrayAsPropertyValue(varInput, &propertyValue);
                    }
                    else if (ArrayProjection::Is(varInput))
                    {
                        // winrt array projected
                        ArrayObjectInstance *arrayObject = ArrayProjection::GetArrayObjectInstance(varInput);
                        ArrayProjection *pArrayProjection = projectionWriter->GetArrayProjection(arrayObject->GetPropertyId());
                        hr = GetTypedArrayAsPropertyValue(arrayObject->finalizableTypedArrayContents->numberOfElements * pArrayProjection->elementType->storageSize, arrayObject->finalizableTypedArrayContents->typedArrayBuffer, pArrayProjection->elementType, &propertyValue);
                    }
                    else
                    {
                        if (boxInterface)
                        {
                            // This isnt allowed any more so throw error
                            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
                        }
                        else
                        {
                            // Either inspectable or unsupported type. 
                            // The GetUnknownOfVar extension would throw for unsupported type.
                            // GetUnknownOfVarExtension should addRef the interface, even if it is the default
                            GetUnknownOfVarExtension(scriptContext, varInput, __uuidof(IInspectable), (LPVOID *)&propertyValue, nullptr, true);
                        }
                    }
                }
                break;
            }
        }

        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

        JS_ETW(EventWriteJSCRIPT_PROJECTION_WRITEINSPECTABLE_STOP(boxInterface));
        return WriteInUnknown(propertyValue, mem, memSize);
    }

    HRESULT ProjectionMarshaler::GetNonArrayTypeAsPropertyValue(
        __in size_t arraySize,
        __in_bcount(arraySize) byte * typeBuffer, 
        __in RtCONCRETETYPE type, 
        __out IInspectable **propertyValue)
    {
        Assert(CanMarshalType(type));

        HRESULT hr;
#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETNONARRAYTYPEASPROPERTYVALUE_START())
        {
            EventWriteJSCRIPT_PROJECTION_GETNONARRAYTYPEASPROPERTYVALUE_START(StringOfId(type->fullTypeNameId));
        }
#endif
        Windows::Foundation::IPropertyValueStatics *propertyValueFactory = projectionContext->GetProjectionWriter()->GetPropertyValueFactory();
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        switch(type->typeCode)
        {
            // Guid
        case tcSystemGuidType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = propertyValueFactory->CreateGuid(*((GUID *)typeBuffer), propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

            // Date Time
        case tcWindowsFoundationDateTimeType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = propertyValueFactory->CreateDateTime(*((Windows::Foundation::DateTime *)typeBuffer), propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

            // Time Span
        case tcWindowsFoundationTimeSpanType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = propertyValueFactory->CreateTimeSpan(*((Windows::Foundation::TimeSpan *)typeBuffer), propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

            // Basic Types
        case tcBasicType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                RtBASICTYPE basicType = BasicType::From(type);
                hr = GetNonArrayTypeAsPropertyValue(arraySize, typeBuffer, basicType->typeCor, propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

        case tcWindowsFoundationEventRegistrationTokenType:
        case tcWindowsFoundationHResultType:
        case tcEnumType:
        case tcStructType:
            {
                ObjectAsIReference *objectAsIReference = nullptr;
                hr = ObjectAsIReference::CreateIReference(projectionContext, arraySize, typeBuffer, type, &objectAsIReference);
                if (SUCCEEDED(hr))
                {
                    IUnknown *pUnknown = objectAsIReference->GetUnknown();
                    hr = pUnknown->QueryInterface(propertyValue);
                    pUnknown->Release();
                }
            }
            break;

            // Unexpected types
        case tcMissingGenericInstantiationType:
        case tcMissingNamedType:
        case tcInterfaceType:
        case tcClassType:
        case tcDelegateType:
        case tcByRefType:
        case tcArrayType:
            Assert(false);
            Js::JavascriptError::MapAndThrowError(scriptContext, JSERR_InvalidPropertyValue);
            break;

        default:
            Js::Throw::FatalProjectionError();
        }

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETNONARRAYTYPEASPROPERTYVALUE_STOP())
        {
            EventWriteJSCRIPT_PROJECTION_GETNONARRAYTYPEASPROPERTYVALUE_STOP(StringOfId(type->fullTypeNameId));
        }
#endif
        return hr;
    }

    HRESULT ProjectionMarshaler::GetNonArrayTypeAsPropertyValue(
        __in size_t arraySize,
        __in_bcount(arraySize) byte * typeBuffer, 
        __in CorElementType type, 
        __out IInspectable **propertyValue)
    {
#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETNONARRAYBASICTYPEASPROPERTYVALUE_START())
        {
            EventWriteJSCRIPT_PROJECTION_GETNONARRAYBASICTYPEASPROPERTYVALUE_START(StringOfId(projectionContext->GetProjectionBuilder()->GetBasicType(type)->fullTypeNameId));
        }
#endif
        Windows::Foundation::IPropertyValueStatics *propertyValueFactory = projectionContext->GetProjectionWriter()->GetPropertyValueFactory();
        HRESULT hr;
        switch(type)
        {
            // Char
        case ELEMENT_TYPE_CHAR:
            AnalysisAssert(arraySize >= sizeof(WCHAR));
            hr = propertyValueFactory->CreateChar16(*((WCHAR *)typeBuffer), propertyValue);
            break;

            // Boolean
        case ELEMENT_TYPE_BOOLEAN:
            AnalysisAssert(arraySize >= sizeof(boolean));
            hr = propertyValueFactory->CreateBoolean(*((boolean *)typeBuffer), propertyValue);
            break;

            // UInt8
        case ELEMENT_TYPE_U1:
            AnalysisAssert(arraySize >= sizeof(byte));
            hr = propertyValueFactory->CreateUInt8(*((byte *)typeBuffer), propertyValue);
            break;

            // Int16
        case ELEMENT_TYPE_I2:
            AnalysisAssert(arraySize >= sizeof(__int16));
            hr = propertyValueFactory->CreateInt16(*((__int16 *)typeBuffer), propertyValue);
            break;

            // UInt16
        case ELEMENT_TYPE_U2:
            AnalysisAssert(arraySize >= sizeof(unsigned __int16));
            hr = propertyValueFactory->CreateUInt16(*((unsigned __int16 *)typeBuffer), propertyValue);
            break;

            // Int32
        case ELEMENT_TYPE_I4:
            AnalysisAssert(arraySize >= sizeof(__int32));
            hr = propertyValueFactory->CreateInt32(*((__int32 *)typeBuffer), propertyValue);
            break;

            // UInt32
        case ELEMENT_TYPE_U4:
            AnalysisAssert(arraySize >= sizeof(unsigned __int32));
            hr = propertyValueFactory->CreateUInt32(*((unsigned __int32 *)typeBuffer), propertyValue);
            break;

            // Int64
        case ELEMENT_TYPE_I8:
            AnalysisAssert(arraySize >= sizeof(__int64));
            hr = propertyValueFactory->CreateInt64(*((__int64 *)typeBuffer), propertyValue);
            break;

            // Int64
        case ELEMENT_TYPE_U8:
            AnalysisAssert(arraySize >= sizeof(unsigned __int64));
            hr = propertyValueFactory->CreateUInt64(*((unsigned __int64 *)typeBuffer), propertyValue);
            break;

            // Float
        case ELEMENT_TYPE_R4:
            AnalysisAssert(arraySize >= sizeof(float));
            hr = propertyValueFactory->CreateSingle(*((float *)typeBuffer), propertyValue);
            break;

            // Double
        case ELEMENT_TYPE_R8:
            AnalysisAssert(arraySize >= sizeof(double));
            hr = propertyValueFactory->CreateDouble(*((double *)typeBuffer), propertyValue);
            break;

            // String
        case ELEMENT_TYPE_STRING:
            AnalysisAssert(arraySize >= sizeof(HSTRING));
            hr = propertyValueFactory->CreateString(*((HSTRING *)typeBuffer), propertyValue);
            break;

            // Inspectable
        case ELEMENT_TYPE_OBJECT:
            AnalysisAssert(arraySize >= sizeof(IInspectable *));
            hr = propertyValueFactory->CreateInspectable(*((IInspectable **)typeBuffer), propertyValue);
            break;

        default:
            Js::Throw::FatalProjectionError();
        }

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETNONARRAYBASICTYPEASPROPERTYVALUE_STOP())
        {
            EventWriteJSCRIPT_PROJECTION_GETNONARRAYBASICTYPEASPROPERTYVALUE_STOP(StringOfId(projectionContext->GetProjectionBuilder()->GetBasicType(type)->fullTypeNameId));
        }
#endif
        return hr;
    }

    HRESULT ProjectionMarshaler::GetBuiltinTypedArrayAsPropertyValue(
        __in Var varValue,
        __out IInspectable **propertyValue)
    {
        HRESULT hr = NOERROR;
        CorElementType typedArrayElementType = ELEMENT_TYPE_END;

        switch (Js::JavascriptOperators::GetTypeId(varValue))
        {
        case Js::TypeIds_Uint8Array:
        case Js::TypeIds_Uint8ClampedArray:
            typedArrayElementType = ELEMENT_TYPE_U1;
            break;
        case Js::TypeIds_Int16Array:
            typedArrayElementType = ELEMENT_TYPE_I2;
            break;
        case Js::TypeIds_Uint16Array:
            typedArrayElementType = ELEMENT_TYPE_U2;
            break;
        case Js::TypeIds_Int32Array:
            typedArrayElementType = ELEMENT_TYPE_I4;
            break;
        case Js::TypeIds_Uint32Array:
            typedArrayElementType = ELEMENT_TYPE_U4;
            break;
        case Js::TypeIds_Float32Array:
            typedArrayElementType = ELEMENT_TYPE_R4;
            break;
        case Js::TypeIds_Float64Array:
            typedArrayElementType = ELEMENT_TYPE_R8;
            break;
        case Js::TypeIds_Int64Array:
            typedArrayElementType = ELEMENT_TYPE_I8;
            break;
        case Js::TypeIds_Uint64Array:
            typedArrayElementType = ELEMENT_TYPE_U8;
            break;
        case Js::TypeIds_BoolArray:
            typedArrayElementType = ELEMENT_TYPE_BOOLEAN;
            break;
        case Js::TypeIds_CharArray:
            typedArrayElementType = ELEMENT_TYPE_CHAR;
            break;
        }
        AssertMsg(typedArrayElementType != ELEMENT_TYPE_END, "invalid base type for typed array");
        Js::TypedArrayBase* typedArrayBase = Js::TypedArrayBase::FromVar(varValue);
        byte* byteBuffer = typedArrayBase->GetByteBuffer();
        uint32 length = typedArrayBase->GetLength();
        hr = GetTypedArrayAsPropertyValue(length * Metadata::Assembly::GetBasicTypeSize(typedArrayElementType), byteBuffer, typedArrayElementType, propertyValue);
        return hr;
    }

    HRESULT ProjectionMarshaler::GetTypedArrayAsPropertyValue(
        __in size_t arraySize, 
        __in_bcount(arraySize) byte * typedArrayBuffer, 
        __in RtCONCRETETYPE type, 
        __out IInspectable **propertyValue)
    {
        Assert(CanMarshalType(type));

        HRESULT hr;
#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETTYPEDARRAYASPROPERTYVALUE_START())
        {
            EventWriteJSCRIPT_PROJECTION_GETTYPEDARRAYASPROPERTYVALUE_START(StringOfId(type->fullTypeNameId));
        }
#endif
        UINT32 length = (UINT32)(arraySize / type->storageSize); // Typed array originates from JS, therefore is constrained to INT_MAX length
        Windows::Foundation::IPropertyValueStatics *propertyValueFactory = projectionContext->GetProjectionWriter()->GetPropertyValueFactory();
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        switch(type->typeCode)
        {
            // Guid Array
        case tcSystemGuidType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = propertyValueFactory->CreateGuidArray(length, (GUID *)typedArrayBuffer, propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

            // Date Time Array
        case tcWindowsFoundationDateTimeType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = propertyValueFactory->CreateDateTimeArray(length, (Windows::Foundation::DateTime *)typedArrayBuffer, propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

            // Time Span
        case tcWindowsFoundationTimeSpanType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = propertyValueFactory->CreateTimeSpanArray(length, (Windows::Foundation::TimeSpan *)typedArrayBuffer, propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

            // Inspectable Array
        case tcInterfaceType:
        case tcClassType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {            
                hr = propertyValueFactory->CreateInspectableArray(length, (IInspectable **)typedArrayBuffer, propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

            // Basic Types
        case tcBasicType:
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                RtBASICTYPE basicType = BasicType::From(type);
                hr = GetTypedArrayAsPropertyValue(arraySize, typedArrayBuffer, basicType->typeCor, propertyValue);
            }
            END_LEAVE_SCRIPT(scriptContext)
            break;

        // Enum Array
        case tcWindowsFoundationEventRegistrationTokenType:
        case tcWindowsFoundationHResultType:
        case tcEnumType:
        case tcStructType:
        case tcDelegateType:
            {
                ObjectAsIReference *objectAsIReference = nullptr;
                hr = ObjectAsIReference::CreateIReferenceArray(projectionContext, arraySize, typedArrayBuffer, type, &objectAsIReference);
                if (SUCCEEDED(hr))
                {
                    IUnknown *pUnknown = objectAsIReference->GetUnknown();
                    hr = pUnknown->QueryInterface(propertyValue);
                    pUnknown->Release();
                }
            }
            break;

            // Unexpected types
        case tcMissingGenericInstantiationType:
        case tcMissingNamedType:
        case tcByRefType:
        case tcArrayType:
            Assert(false);
            Js::JavascriptError::MapAndThrowError(scriptContext, JSERR_InvalidPropertyValue);

        default:
            Js::Throw::FatalProjectionError();
        }
#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETTYPEDARRAYASPROPERTYVALUE_STOP())
        {
            EventWriteJSCRIPT_PROJECTION_GETTYPEDARRAYASPROPERTYVALUE_STOP(StringOfId(type->fullTypeNameId));
        }
#endif
        return hr;
    }

    HRESULT ProjectionMarshaler::GetTypedArrayAsPropertyValue(
        __in size_t arraySize, 
        __in_bcount(arraySize) byte * typedArrayBuffer, 
        __in CorElementType type, 
        __out IInspectable **propertyValue)
    {
#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETBASICTYPEDARRAYASPROPERTYVALUE_START())
        {
            EventWriteJSCRIPT_PROJECTION_GETBASICTYPEDARRAYASPROPERTYVALUE_START(StringOfId(projectionContext->GetProjectionBuilder()->GetBasicType(type)->fullTypeNameId));
        }
#endif
        Windows::Foundation::IPropertyValueStatics *propertyValueFactory = projectionContext->GetProjectionWriter()->GetPropertyValueFactory();
        UINT32 length = (UINT32)(arraySize / Metadata::Assembly::GetBasicTypeSize(type)); // Typed array originates from JS, therefore is constrained to INT_MAX length
        HRESULT hr;
        switch(type)
        {
            // Char array
        case ELEMENT_TYPE_CHAR:
            hr = propertyValueFactory->CreateChar16Array(length, (WCHAR *)typedArrayBuffer, propertyValue);
            break;

            // Boolean Array
        case ELEMENT_TYPE_BOOLEAN:
            hr = propertyValueFactory->CreateBooleanArray(length, (boolean *)typedArrayBuffer, propertyValue);
            break;

            // UInt8Array
        case ELEMENT_TYPE_U1:
            hr = propertyValueFactory->CreateUInt8Array(length, (byte *)typedArrayBuffer, propertyValue);
            break;

            // Int16 array
        case ELEMENT_TYPE_I2:
            hr = propertyValueFactory->CreateInt16Array(length, (__int16 *)typedArrayBuffer, propertyValue);
            break;

            // UInt16 array
        case ELEMENT_TYPE_U2:
            hr = propertyValueFactory->CreateUInt16Array(length, (unsigned __int16 *)typedArrayBuffer, propertyValue);
            break;

            // Int32 Array
        case ELEMENT_TYPE_I4:
            hr = propertyValueFactory->CreateInt32Array(length, (__int32 *)typedArrayBuffer, propertyValue);
            break;

            // UInt32 Array
        case ELEMENT_TYPE_U4:
            hr = propertyValueFactory->CreateUInt32Array(length, (unsigned __int32 *)typedArrayBuffer, propertyValue);
            break;

            // Int64 Array
        case ELEMENT_TYPE_I8:
            hr = propertyValueFactory->CreateInt64Array(length, (__int64 *)typedArrayBuffer, propertyValue);
            break;

            // Int64 Array
        case ELEMENT_TYPE_U8:
            hr = propertyValueFactory->CreateUInt64Array(length, (unsigned __int64 *)typedArrayBuffer, propertyValue);
            break;

            // Float Array
        case ELEMENT_TYPE_R4:
            hr = propertyValueFactory->CreateSingleArray(length, (float *)typedArrayBuffer, propertyValue);
            break;

            // Double Array
        case ELEMENT_TYPE_R8:
            hr = propertyValueFactory->CreateDoubleArray(length, (double *)typedArrayBuffer, propertyValue);
            break;

            // String Array
        case ELEMENT_TYPE_STRING:
            hr = propertyValueFactory->CreateStringArray(length, (HSTRING *)typedArrayBuffer, propertyValue);
            break;

            // Inspectables
        case ELEMENT_TYPE_OBJECT:
            hr = propertyValueFactory->CreateInspectableArray(length, (IInspectable **)typedArrayBuffer, propertyValue);
            break;

        default:
            hr = E_INVALIDARG;
        }
#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_GETBASICTYPEDARRAYASPROPERTYVALUE_STOP())
        {
            EventWriteJSCRIPT_PROJECTION_GETBASICTYPEDARRAYASPROPERTYVALUE_STOP(StringOfId(projectionContext->GetProjectionBuilder()->GetBasicType(type)->fullTypeNameId));
        }
#endif
        return hr;
    }

    // Info:        Write a guid
    // Parameters:  varInput - holds the guid to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInGuid(Var varInput, bool structsByValue, __in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        // From ABIMarshalingContext.cpp(316)------------------------------------------------------------------------------------------------
        Var baseValue;
        size_t sizeOfBytesRead = sizeof(GUID);

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (!Js::DynamicObject::Is(varInput))
        {
            baseValue = varInput;
        }
        else
        {
            Js::DynamicObject* varSource = Js::DynamicObject::FromVar(varInput);
            baseValue = Js::JavascriptConversion::ToPrimitive(varSource, Js::JavascriptHint::HintString, scriptContext);
        }

        Js::JavascriptString * str = Js::JavascriptConversion::ToString(baseValue, scriptContext);
        LPCWSTR guidStr = str->GetSz();
        byte * writeTo = mem;
#if _M_X64
        Assert(sizeof(GUID) > sizeof(LPVOID));
        if (!structsByValue)
        {
            // Since structByValue be false only for In parameter of the method we can safely assert for calleeTransfersOwnership
            Assert (resourceCleanup == CalleeTransfersOwnership);

            if (memSize<sizeof(LPVOID))
            {
                Js::Throw::FatalProjectionError();
            }
            sizeOfBytesRead = sizeof(LPVOID);
            byte *guidSpace = AnewArrayZ(alloc, byte, sizeof(GUID));

            *(GUID **)writeTo = (GUID *)guidSpace;
            writeTo = guidSpace;
        }
#endif

        HRESULT hr = GUIDParser::TryParseGUID(guidStr, (GUID*)writeTo);

        if(FAILED(hr))
        {
            // Reinterpret HRESULT for better mapping
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedWinRTGUID);
        }
        return mem + sizeOfBytesRead;
    }

    // Info:        Write a JavascriptDateTime out
    // Parameters:  varInput - holds the date to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInDate(Var varInput, __in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        size_t sizeOfDateTime = sizeof(Windows::Foundation::DateTime); 

        if (memSize < sizeOfDateTime)
        {
            Js::Throw::FatalProjectionError();
        }

        Assert(projectionContext->GetProjectionBuilder()->GetWindowsFoundationDateTimeUnderlyingType()->typeCor == ELEMENT_TYPE_I8);


        // We do a straight cast to INT64* because Windows::Foundation::DateTime is a struct with just
        // a single int64 member, and that can't change without a new type getting created based on ABI rules
        *(INT64*)mem = GetDateTimeOfJavascriptDate(varInput, projectionContext);

        return mem + sizeOfDateTime;
    }

    //
    // Version 6 Change:
    // Previously we would round the TimeSpan to ms precision in order to avoid having the double contain decimal digits.
    // Now we allow for the timespan to be represented with integers and digits (no truncation).
    //
    HRESULT ProjectionMarshaler::WinRTTimeSpanToNumberV6(INT64 ticks, __out double* pRet)
    {
        Assert(pRet != NULL);

        if (pRet == NULL)
        {
            return E_INVALIDARG;
        }

        // We want to preserve precision as best we could, and for low enough timespan values
        // Hence perform a division of doubles, to convert from ticks to milliseconds.
        double result = (double)ticks / Js::DateUtilities::ticksPerMillisecondDouble;
        *pRet = result;

        return S_OK;
    }

    //
    // Version 6 Change:
    // Same as for WinRTTimeSpanToNumberV6, remove truncation when converting between Number and WinRT TimeSpan.
    //
    HRESULT ProjectionMarshaler::NumberToWinRTTimeSpanV6(double span, __out INT64* pRet)
    {
        Assert(pRet != NULL);

        if (pRet == NULL)
        {
            return E_INVALIDARG;
        }

        //Otherwise the double multiplication might overflow
        if (span > INT64_MAX / Js::DateUtilities::ticksPerMillisecond)
        {
            return INTSAFE_E_ARITHMETIC_OVERFLOW;
        }

        //Multiply before converting to Int64, in order to get the 100-nanosecond precision which will get truncated
        INT64 spanAsInt64 = Js::NumberUtilities::TryToInt64(span * Js::DateUtilities::ticksPerMillisecondDouble);

        if (!Js::NumberUtilities::IsValidTryToInt64(spanAsInt64))
        {
            return INTSAFE_E_ARITHMETIC_OVERFLOW;
        }

        (*pRet) = spanAsInt64;
        return S_OK;
    }

    // Info:        Write a TimeSpan out
    // Parameters:  varInput - holds the number to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInTimeSpan(Var varInput, __in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        size_t sizeOfTimeSpan = sizeof(Windows::Foundation::TimeSpan); 

        if (memSize < sizeOfTimeSpan)
        {
            Js::Throw::FatalProjectionError();
        }

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        double span = Js::JavascriptConversion::ToNumber(varInput, scriptContext);
        if (Js::JavascriptNumber::IsNan(span))
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedNumber);
        }
        INT64 ticks;
        HRESULT hr = NumberToWinRTTimeSpanV6(span, &ticks);       
        if (FAILED(hr))
        {
            // If conversion failed, double value is outside the range of WinRT TimeSpan
            Js::JavascriptError::ThrowRangeError(scriptContext, JSERR_OutOfTimeSpanRange);
        }

        *(INT64*)mem = ticks;

        return mem + sizeOfTimeSpan;
    }

    // Info:        Write a struct
    // Parameters:  varInput - holds the struct
    //              constructor - model information about the struct
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteStructConstructorTypeInParameter(Var varInput, RtSTRUCTCONSTRUCTOR constructor, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, __in bool padHFA)
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (Js::TaggedNumber::Is(varInput))
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
        }
        Js::RecyclableObject * structObject = Js::RecyclableObject::FromVar(varInput);
        if (nullptr == structObject)
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedObject);
        }

        byte * storage = mem;
        AssertMsg(constructor->structType->storageSize < INT_MAX, "Invalid metadata: Max size of struct limited to 2gb");
        int bytesReadFromMem = (int)constructor->structType->storageSize;
#if _M_X64
        if (!structsByValue && constructor->structType->isPassByReference)
        {
            // Since structByValue be false only for In parameter of the method we can safely assert for calleeTransfersOwnership
            Assert (resourceCleanup == CalleeTransfersOwnership);
            if (memSize<sizeof(LPVOID))
            {
                Js::Throw::FatalProjectionError();
            }

            // Allocate space for the structure
            byte *structSpace = AnewArrayZ(alloc, byte, constructor->structType->storageSize);

            // Assign the allocated ptr to the memory and use allocated space to fill in the structure
            *(byte **)storage = structSpace;
            storage = structSpace;

            // we are reading only LPVOID from the mem so update only that
            Assert(constructor->structType->sizeOnStack == sizeof(LPVOID));
            bytesReadFromMem = (int)constructor->structType->sizeOnStack; 
        }
#endif
        ImmutableList<RtABIFIELDPROPERTY> * properties = constructor->structType->fields;
        size_t remainingStorage = constructor->structType->storageSize;
#if _M_ARM64
        // hfa padding only used on ARM64.
        int hfaPadding = 0;
#endif
        while(properties)
        {
            RtABIFIELDPROPERTY prop = properties->First();
            PropertyId id = prop->identifier; 
            Var fieldObject = nullptr;

            if (!Js::JavascriptOperators::GetProperty(structObject, id, &fieldObject, scriptContext))
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_MissingStructProperty, StringOfId(id));
            }

            byte * fieldMem = storage + prop->fieldOffset;
#if _M_ARM64

            if (padHFA)
            {
                fieldMem = storage + prop->fieldOffset + hfaPadding;
                hfaPadding = hfaPadding + 16 - prop->type->storageSize;
            }
#endif
            WriteInType(fieldObject, prop->type, fieldMem, remainingStorage, true);
            remainingStorage -= prop->type->storageSize;
            properties = properties->GetTail();
        }
        Assert(remainingStorage<constructor->structType->storageSize); // remainingStorage > 0 when per-struct alignment rounded up        
        return mem + bytesReadFromMem;
    }  

    byte * ProjectionMarshaler::WriteDelegateConstructorTypeInParameter(Var varInput, RtDELEGATECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        // From ABIMarshalingContext::MarshalDelegate ------------------------------------------------------------------------------------------------
        Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(varInput);
        if (typeId == Js::TypeIds_Null || typeId == Js::TypeIds_Undefined)
        {
            return WriteInUnknown(nullptr, mem, memSize);
        }

        // Validate that we're provided with a JavaScript function for callback
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (!Js::JavascriptFunction::Is(varInput))
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedFunction);
        }

        Js::JavascriptFunction * delegateFunction = Js::JavascriptFunction::FromVar(varInput);
        if(Js::JavascriptOperators::GetTypeId(delegateFunction) != Js::TypeIds_Function)
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_FunctionArgument_NeedFunction);
        }

        // Native delegates shouldnt be marshaled into the eventHandler
        RtRUNTIMEINTERFACECONSTRUCTOR invokeInterface = RuntimeInterfaceConstructor::From(constructor->invokeInterface);
        if (delegateFunction->IsWinRTFunction())
        {
            Js::JavascriptWinRTFunction * externalFunction = Js::JavascriptWinRTFunction::FromVar(delegateFunction);
            if (externalFunction->GetFunctionInfo()->GetOriginalEntryPoint() != reinterpret_cast<Js::JavascriptMethod>(MethodSignatureThunk))
            {
                // This WinRT function does not have entry point MethodSignatureThunk, and cannot be a delegate
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
            }
            Signature * signature = (Signature*)externalFunction->GetSignature();
            if (signature->thisInfo->thisType != thisDelegate)
            {
                // This WinRT function is not a delegate
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
            }

            // This is a native delegate - we can just get the IDelegate and put it out there.
            DelegateThis * delegateThis = reinterpret_cast<DelegateThis*>(signature->thisInfo);
            IUnknown *unknown = nullptr;
            IID iidDelegate = invokeInterface->iid->instantiated;

            HRESULT hr = QueryInterfaceAfterLeaveScript(scriptContext, delegateThis->_this, iidDelegate, (void**)&unknown);
            if (FAILED(hr) || !unknown)
            {
                // IIDs dont match so we cannot convert this delegate into one that we are suppose to
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
            }

            return WriteInUnknown(unknown, mem, memSize);
        }

        // Create the delegate object
        RtPROPERTY invokeProperty = invokeInterface->prototype->fields->First();
        RtABIMETHODPROPERTY invokeAbiProperty = AbiMethodProperty::From(invokeProperty);

        Delegate* delegateObject = NULL;
        HRESULT hr = Delegate::Create(projectionContext, StringOfId(constructor->typeId), invokeAbiProperty->body->signature, delegateFunction, nullptr, IsInAsyncInterface(), &delegateObject);
        IfFailedMapAndThrowHr(scriptContext, hr);

        IUnknown *pDelegate = delegateObject->GetUnknown();
        return WriteInUnknown(pDelegate, mem, memSize);
    }


    // Info:        Write an RuntimeClass constructor
    // Parameters:  varInput - the input
    //              constructor - the rtc constructor to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteRuntimeClassConstructor(Var varInput, RtRUNTIMECLASSCONSTRUCTOR rtc, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        RtRUNTIMEINTERFACECONSTRUCTOR iface = RuntimeInterfaceConstructor::From(rtc->defaultInterface.GetValue());
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        // If the rtc has default interface as IPropertyValue then we can marshal the rtc Windows.Foundation.PropertyValue only, rest of them should throw error
        if (iface->iid->instantiated == Windows::Foundation::IID_IPropertyValue)
        {
            if (rtc->typeDef->id != IdOfString(_u("Windows.Foundation.PropertyValue")))
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_InvalidRTCPropertyValueIn, StringOfId(rtc->typeDef->id));
            }

            // Create the propertyValue
            return WriteInspectableObject(varInput, mem, memSize, true);
        }

        // TODO: to define behaviour for rtc having IReference or IReferenceArray as in param or IPropertyValue which is not default interface

        // Get default unknown from runtime class, TryGetInterface would throw error if the varInput is not instance of runtimeClass
        IUnknown *unknown = TryGetInterface<true>(varInput, rtc->typeId, iface->iid->instantiated, MetadataStringIdNil, scriptContext, nullptr);

        // Since TryGetInterface doesnot addref the unknown we need to addref it as we might be called here because of delegate out param.
        if (unknown)
        {
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                unknown->AddRef();
            }
            END_LEAVE_SCRIPT(scriptContext);
        }
        return WriteInUnknown(unknown, mem, memSize);
    }

    // Info:        Write a particular interface
    // Parameters:  varInput - the input
    //              iid - of the interface to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInterfaceInParameter(Var varInput, const IID & iid, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (Js::TaggedNumber::Is(varInput))
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
        }

        Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(varInput);
        if (typeId == Js::TypeIds_Null || typeId == Js::TypeIds_Undefined)
        {
            return WriteInUnknown(nullptr, mem, memSize);
        }

        IID useIID = iid;
        if (useIID == GUID_NULL)
        {
            useIID = IID_IUnknown;
        }

        IUnknown *unknown = nullptr;
        GetUnknownOfVarExtension(scriptContext, varInput, useIID, (void**)&unknown, nullptr, true);
        return WriteInUnknown(unknown, mem, memSize);
    }

    // Info:        Write an interface constructor
    // Parameters:  varInput - the input
    //              constructor - the interface constructor to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteRuntimeInterfaceConstructor(Var varInput, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        Assert(constructor != nullptr);

        // From ABIMarshalingContext::MarshalInterface----------------------------------------------------------------------------------------------------------
        if (ArrayAsCollection::IsArrayInstance(varInput))
        {
            Js::JavascriptArray * asArray = static_cast<Js::JavascriptArray *>(varInput);
            if (constructor->iid->piid==IID_IIterable1)
            {
                ArrayAsIterable *pIIterable = NULL;
                HRESULT hr = ArrayAsIterable::Create(projectionContext, asArray, constructor, &pIIterable);
                IfFailedMapAndThrowHr(projectionContext->GetScriptContext(), hr);

                // Place the Iterable ptr into the target buffer
                IUnknown *pUnknown = pIIterable->GetUnknown();
                return WriteInUnknown(pUnknown, mem, memSize);
            } 
            else if (constructor->iid->piid==IID_IIterator1)
            {
                ArrayAsIterator *pIIterator = NULL;
                HRESULT hr = ArrayAsIterator::Create(projectionContext, asArray, constructor, &pIIterator);
                IfFailedMapAndThrowHr(projectionContext->GetScriptContext(), hr);

                // Place the IIterator ptr into the target buffer
                IUnknown *pUnknown = pIIterator->GetUnknown();
                return WriteInUnknown(pUnknown, mem, memSize);
            } 
            else if (constructor->iid->piid==IID_IVectorView1 || constructor->iid->piid==IID_IVector1)
            {
                ArrayAsVector *pIVector = nullptr;
                HRESULT hr = ArrayAsVector::Create(projectionContext, asArray, constructor, constructor->iid->piid == IID_IVectorView1, &pIVector);
                IfFailedMapAndThrowHr(projectionContext->GetScriptContext(), hr);

                // Place the IVector/View ptr into the target buffer
                IUnknown *pUnknown = pIVector->GetUnknown();
                return WriteInUnknown(pUnknown, mem, memSize);
            }
        }

        if (constructor->iid->piid == Windows::Foundation::IID_IPropertyValue)
        {
            // Create the propertyValue
            return WriteInspectableObject(varInput, mem, memSize, true);
        }
        else if (constructor->iid->piid == IID_IReference1)
        {
            return WriteIReference(varInput, constructor, mem, memSize);
        }
        else if (constructor->iid->piid == IID_IReferenceArray1)
        {
            return WriteIReferenceArray(varInput, constructor, mem, memSize);
        }

        return WriteInterfaceInParameter(varInput, constructor->iid->instantiated, mem, memSize);
    }

    // Info:        Write a function type
    // Parameters:  varInput - the input
    //              function - the function to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteFunctionTypeInParameter(Var varInput, RtFUNCTION function, __in_bcount(memSize) byte * mem,__in size_t memSize, bool structsByValue)
    {
        switch(function->functionType)
        {
        case functionDelegateConstructor:
            return WriteDelegateConstructorTypeInParameter(varInput, DelegateConstructor::From(function), mem, memSize);
        case functionStructConstructor: 
            return WriteStructConstructorTypeInParameter(varInput, StructConstructor::From(function), structsByValue, mem, memSize, false);
        case functionInterfaceConstructor:
            return WriteRuntimeInterfaceConstructor(varInput, RuntimeInterfaceConstructor::From(function), mem, memSize);
        case functionRuntimeClassConstructor: 
            return WriteRuntimeClassConstructor(varInput, RuntimeClassConstructor::From(function), mem, memSize);
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Write a given model expression
    // Parameters:  varInput - the input
    //              expr - the model expression
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteExprTypeInParameter(Var varInput, RtEXPR expr, __in_bcount(memSize) byte * mem,__in size_t memSize, bool structsByValue)
    {
        switch(expr->type)
        {
        case exprFunction: 
            return WriteFunctionTypeInParameter(varInput, Function::From(expr), mem, memSize, structsByValue);
        case exprEnum: 
            return WriteInCorElementType(varInput, Enum::From(expr)->baseTypeCode, mem, memSize);
        }
        Js::Throw::FatalProjectionError();
    }


    // Info:        Write by named type
    // Parameters:  varInput - the input
    //              typeName - the named type
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteTypeNameTypeInParameter(Var varInput, MetadataStringId typeId, MetadataStringId typeNameId, ImmutableList<RtTYPE> * genericParameters, __in_bcount(memSize) byte * mem,__in size_t memSize, bool structsByValue)
    {
        RtEXPR expr = nullptr;
        HRESULT hr = projectionContext->GetExpr(typeId, typeNameId, nullptr, genericParameters, &expr);
        if (FAILED(hr))
        {
            Js::VerifyOkCatastrophic(hr);
        }
        return WriteExprTypeInParameter(varInput, expr, mem, memSize, structsByValue);
    }

    // Info:        Write an array including the length
    // Parameters:  arg - the input
    //              type - the array type
    //              isByRef - whether the array is byref
    //              copyElements - whether elements should be copied from 'arg'
    //              lengthPointer - pointer to the length parameter
    //              arrayPointer - pointer to the array
    //              methodName - the name of the calling method
    void ProjectionMarshaler::WriteInArrayTypeIndividual(Var arg, RtARRAYTYPE type, bool isByRef, __in_bcount(sizeof(LPVOID)) byte * lengthPointer, __in_bcount(sizeof(LPVOID)) byte * arrayPointer, bool isOut, bool hasLength, uint32 lengthValue)
    {
        RecordRecyclerVar(arg);
        Assert(CanMarshalType(type));

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            // Identify Pattern and print the info
            Output::Print(_u("MemoryTrace: Writing Array for: %s\n"), (resourceCleanup == CalleeTransfersOwnership) ? _u("Method Call") : _u("Delegate Invoke"));
            Output::Print(_u("    Array Pattern: %s\n"), isByRef ? _u("ReceiveArray") : isOut ? _u("FillArray") : _u("PassArray"));
            Output::Print(_u("    Considering length attribute: %s\n"), hasLength ? _u("true") : _u("false"));
            Output::Flush();
        }
#endif

        // Validate that we're provided with an array 
        Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(arg);
        BOOL isNullArray = (typeId == Js::TypeIds_Null || typeId == Js::TypeIds_Undefined);
        BOOL isArray = ArrayAsCollection::IsArrayInstance(arg);
        BOOL isArrayProjection = ArrayProjection::Is(arg); 
        BOOL isTypedArray = Js::TypedArrayBase::Is(arg);
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        if (!isNullArray && !isArray && !isArrayProjection && !isTypedArray)
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedArrayObject);
        }

        // Get the length of the array from the preceding parameter
        uint arrayLength = 0;
        if (isArray)
        {
            arrayLength = ArrayAsCollection::GetLength(static_cast<Js::JavascriptArray *>(arg));
        } 
        else if (isTypedArray)
        {
            arrayLength = Js::TypedArrayBase::FromVar(arg)->GetLength();
        }
        else if (isArrayProjection)
        {
            ArrayObjectInstance *pArrayProjectionInstance = ArrayProjection::GetArrayObjectInstance(arg);
            Assert(pArrayProjectionInstance != NULL);
            arrayLength = pArrayProjectionInstance->GetLength();
        }

        uint32 readArrayLength = hasLength ? lengthValue : arrayLength;

        // Element type
        RtCONCRETETYPE elementType = ConcreteType::From(type->elementType);
        size_t elementTypeSize = elementType->storageSize;
        size_t totalArraySize = elementTypeSize * arrayLength;

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            if (isArray)
            {
                Output::Print(_u("    Reading data from: Javascript Array\n"));
            }
            else if (isNullArray)
            {
                Output::Print(_u("    Reading data from: %s\n"), typeId == Js::TypeIds_Null ? _u("null") : _u("undefined"));
            }
            else
            {
                if (isArrayProjection)
                {
                    Output::Print(_u("    Reading data from: ArrayProjection %s\n"), StringOfId(ArrayProjection::GetArrayObjectInstance(arg)->GetPropertyId()));
                }
                else
                {
                    Output::Print(_u("    Reading data from: Typed Array %s\n"), ArrayProjection::TypedArrayName(arg));
                }
            }
            Output::Print(_u("    Array Length: %u\n"), arrayLength);
            Output::Print(_u("    Read array Length: %u\n"), readArrayLength);
            Output::Print(_u("    Write Array of type: %s\n"), StringOfId(elementType->fullTypeNameId));
            Output::Flush();
        }
#endif

        if (hasLength && readArrayLength > arrayLength)
        {
            // the lengthIs attribute paramter has value greater than the size of the array
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_IllegalArraySizeAndLength);
        }

        if (totalArraySize < arrayLength || Js::JavascriptConversion::ToUInt32((double)totalArraySize) < arrayLength)
        {
            // This means the array is overshooting the max size it can allocate
            Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, E_OUTOFMEMORY);
        }

        // Allocate or use the existing memory
        *(uint32*)lengthPointer = arrayLength;

        // Copy elements in all cases except when making a call of type PassArray or FillArray that has arrayProjection as in param
        // since we are going to pass the internal buffer itself in that case
        bool copyElements = true; 
        if (resourceCleanup == CalleeTransfersOwnership)
        {
            Assert(!isByRef);

            // REVIEW: use typed array for all array projection
            if (isArrayProjection)
            {
                // Check if we can use existing array block pointer
                ArrayObjectInstance *pArrayProjectionInstance = ArrayProjection::GetArrayObjectInstance(arg);
                Assert(pArrayProjectionInstance != NULL);

                if (pArrayProjectionInstance->GetPropertyId() == elementType->fullTypeNameId)
                {
                    // We dont need to addref on this instance because recycler wont recycle it till the time it is on stack.
                    *(byte **)arrayPointer = pArrayProjectionInstance->GetArrayBlock();
                    copyElements = false;
                }
            }
            else if (isTypedArray)
            {
                if (ArrayProjection::SupportTypedArray(elementType) &&
                    !ArrayProjection::NeedConversion(elementType, arg))
                {
                    Js::TypedArrayBase* typedArrayBase = Js::TypedArrayBase::FromVar(arg);
                    *(byte **)arrayPointer = typedArrayBase->GetByteBuffer();
                    copyElements = false;
                }
            }
            else
            {
                // Array is js array or null/undefined value with Fill/Pass Array pattern
                byte *arraySpace;

                if (!isNullArray && !ConcreteType::IsBlittable(elementType))
                {
#if DBG_DUMP
                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        Output::Print(_u("    Creating Finalizer\n"));
                        Output::Flush();
                    }
#endif

                    Recycler *recycler = scriptContext->GetRecycler();

#if DBG_DUMP
                    auto finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, arrayLength, nullptr, releaseBufferUsingDeleteArray, (ProjectionMemoryInformation*) projectionContext->GetThreadContext()->GetProjectionContextMemoryInformation());
#else
                    auto finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, arrayLength, nullptr, releaseBufferUsingDeleteArray);
#endif

                    // Add to the projectionWriters List of pinned the contents and our list it to unpin the contents when the marshaler is destructed
                    RecordRecyclerVar(finalizableTypedArrayContents, true);

#if DBG_DUMP
                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        Output::Print(_u("    Allocated space using new[]\n"));
                        Output::Flush();
                    }
#endif

                    arraySpace = new byte[totalArraySize];
                    if (arraySpace == nullptr)
                    {
                        // Out of memory
                        Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, E_OUTOFMEMORY);
                    }
                    memset(arraySpace, 0, totalArraySize);
                    finalizableTypedArrayContents->typedArrayBuffer = arraySpace;
                    finalizableTypedArrayContents->Initialize();
                }
                else
                {
#if DBG_DUMP
                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        Output::Print(_u("    Allocated space in arena\n"));
                        Output::Flush();
                    }
#endif
                    arraySpace = AnewArrayZ(alloc, byte, totalArraySize);
                }

                *(byte **)arrayPointer = arraySpace;
                copyElements = !isOut && copyElements;
            }

            if (isArrayProjection || isTypedArray)
            {
                if (copyElements)
                {
                    // The type was mismatch - throw error
                    Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
                }
                else if (isOut)
                {
                    // FillArray pattern - clear out the contents of the array.
#if DBG_DUMP
                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        Output::Print(_u("    Clearing buffer pointed by array\n"));
                        Output::Flush();
                    }
#endif

                    // Release the resources held if any
                    byte * arrayStorage = *(byte **)arrayPointer;
                    for (uint index = 0; index < arrayLength; index++)
                    {
                        RecordTypeToUndo(&ProjectionMarshaler::ClearString, &ProjectionMarshaler::ClearUnknown, elementType, arrayStorage, elementTypeSize);
                        arrayStorage += elementTypeSize;
                    }

                    // Clear out the contents of the buffer
                    memset(*(byte **)arrayPointer, 0, totalArraySize);
                }
                else if (readArrayLength < arrayLength)
                {
                    // This is PassArrayPattern with [in] length parameter and we need to clear out some contents from the existing buffer
#if DBG_DUMP
                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        Output::Print(_u("    Clearing buffer after %u index in the array\n"), readArrayLength);
                        Output::Flush();
                    }
#endif

                    // Release the resources held if any from readLength till arrayLength
                    byte * arrayStorage = (*(byte **)arrayPointer) + (readArrayLength * elementTypeSize);
                    for (uint index = readArrayLength; index < arrayLength; index++)
                    {
                        RecordTypeToUndo(&ProjectionMarshaler::ClearString, &ProjectionMarshaler::ClearUnknown, elementType, arrayStorage, elementTypeSize);
                        arrayStorage += elementTypeSize;
                    }

                    // Clear out the contents of the buffer
                    memset((*(byte **)arrayPointer) + (readArrayLength * elementTypeSize), 0, (arrayLength - readArrayLength) * elementTypeSize);
                }
            }
        }
        // In case of retain ownership (delegates) we already have a buffer in which we need to copy the elements (FillArray Pattern) except for 
        // ReceiveArray (isByRef) scenario where we need to CoTaskMemAlloc, 
        // For PassArray scenario this code path would never hit as it would inonly parameter and hence write to the memory would never be called.
        else if (isByRef)
        {
            Recycler *recycler = scriptContext->GetRecycler();
#if DBG_DUMP
            auto finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, arrayLength, nullptr, releaseBufferUsingCoTaskMemFree, (ProjectionMemoryInformation*) projectionContext->GetThreadContext()->GetProjectionContextMemoryInformation());
#else
            auto finalizableTypedArrayContents = RecyclerNewFinalized(recycler, FinalizableTypedArrayContents, recycler, projectionContext->GetThreadContext()->GetWinRTStringLibrary(), elementType, arrayLength, nullptr, releaseBufferUsingCoTaskMemFree);
#endif

            // Add to the projectionWriters List of pinned the contents and our list it to unpin the contents when the marshaler is destructed
            RecordRecyclerVar(finalizableTypedArrayContents, true);

            LPVOID arraySpace = CoTaskMemAlloc(totalArraySize);
            IfNullMapAndThrowHr(scriptContext, arraySpace, E_OUTOFMEMORY);
#if DBG_DUMP
            if (Js::Configuration::Global.flags.TraceWin8Allocations)
            {
                Output::Print(_u("    Allocated space using CoTaskMemAlloc\n"));
                Output::Flush();
            }
#endif
            finalizableTypedArrayContents->typedArrayBuffer = (byte*)arraySpace;
            finalizableTypedArrayContents->Initialize();
            RecordDelegateReceiveArrayContents(finalizableTypedArrayContents);
            *(byte **)arrayPointer = (byte*)arraySpace;
        }


        if (copyElements)
        {
            // The elements are copied to buffer and buffer would be owner of those contents - that is whenever the buffer is deallocated, 
            // the elements should be released, we shouldnt be holding onto the references. So the Write needs to happen with CalleeRetainsOwnership
            ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false); 

#if DBG_DUMP
            if (Js::Configuration::Global.flags.TraceWin8Allocations)
            {
                Output::Print(_u("    Copying %u Elements\n    Releasing existing values: No\n"), readArrayLength);
                Output::Flush();
            }
#endif

            // Marshal each element into the array
            byte * arrayStorage = *(byte **)arrayPointer;
            for (uint index = 0; index < readArrayLength; index++)
            {
                Var varIndex = Js::JavascriptNumber::ToVar(index, scriptContext);
                Var varElement = Js::JavascriptOperators::OP_GetElementI(arg, varIndex, scriptContext);

                marshal.WriteInType(varElement, elementType, arrayStorage, elementTypeSize, true);
                RecordTypeToUndo(&ProjectionMarshaler::RecordDelegateOutString, &ProjectionMarshaler::RecordDelegateOutUnknown, elementType, arrayStorage, elementTypeSize);
                arrayStorage += elementTypeSize;
            }
        }
    }

    // Info:        Write the given type
    // Parameters:  arg - the input
    //              type - the type to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInType(Var arg, RtTYPE type, __in_bcount(memSize) byte * mem,__in size_t memSize, bool structsByValue)
    {
        RecordRecyclerVar(arg);

        bool wasMissingType = false;
        Assert(CanMarshalType(type, true /* allow missing type */, &wasMissingType));

        if (wasMissingType)
        {
            switch (type->typeCode)
            {
            case tcClassType:
            case tcDelegateType:
            case tcInterfaceType:
                Assert(arg == nullptr);
                return WriteInspectableObject(arg, mem, memSize, structsByValue);
            default:
                Js::Throw::FatalProjectionError();
            }
        }

        switch(type->typeCode)
        {
        case tcBasicType: 
            {
                RtBASICTYPE basicType = BasicType::From(type);
                return WriteInBasicType(arg, basicType, mem, memSize);
            }
        case tcSystemGuidType: 
            return WriteInGuid(arg, structsByValue, mem, memSize);
        case tcWindowsFoundationDateTimeType:
            return WriteInDate(arg, mem, memSize);
        case tcWindowsFoundationTimeSpanType:
            return WriteInTimeSpan(arg, mem, memSize);
        case tcWindowsFoundationEventRegistrationTokenType:
            return WriteInCorElementType(arg, ELEMENT_TYPE_I8, mem, memSize);
        case tcWindowsFoundationHResultType:
            return WriteInCorElementType(arg, ELEMENT_TYPE_I4, mem, memSize);
        case tcInterfaceType:
        case tcDelegateType: 
        case tcEnumType: 
        case tcClassType: 
        case tcStructType: 
            {
                RtTYPEDEFINITIONTYPE typeDefinitionType = TypeDefinitionType::From(type);
                return WriteTypeNameTypeInParameter(arg, typeDefinitionType->typeId, typeDefinitionType->typeDef->id, typeDefinitionType->genericParameters, mem, memSize, structsByValue);
            }
        case tcByRefType:
            {
                RtBYREFTYPE byRefType = ByRefType::From(type);
                RtCONCRETETYPE concrete = ConcreteType::From(byRefType->pointedTo);
                Assert(!ArrayType::Is(concrete));
                if (memSize < sizeof(LPVOID))
                {
                    //Unexpected
                    Js::Throw::FatalProjectionError();
                }
                Assert(*(byte**)mem == nullptr);

                *(byte**)mem = AnewArrayZ(alloc, byte, concrete->storageSize);
                WriteInPointer(*(byte**)mem, mem, memSize);

                WriteInType(arg, concrete, *(byte**)mem, concrete->storageSize, true);
                return mem + sizeof(LPVOID);
                
            }
        case tcArrayType:
            {
                // PassArray Pattern
                RtARRAYTYPE arrayType = ArrayType::From(type);
                WriteInArrayTypeIndividual(arg, arrayType, false, mem, mem+sizeof(LPVOID), false);
                return mem + arrayType->sizeOnStack;
            }
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Write a parameter
    // Parameters:  arg - the input
    //              parameter - the parameter to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInParameter(__in Var arg, __in RtABIPARAMETER parameter, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        return WriteInType(arg, parameter->type, mem, memSize, false);
    }

    // Info:        Write an HFA parameter. This function is only used on ARM64 and plumbs data through to the struct marshalling routine in cases where we know we are
    //              marshaling a struct of exactly 4 or fewer floats or 4 or fewer doubles since extra padding is necessary to make sure they get loaded into the
    //              correct floating point registers.
    // Parameters:  arg - the input
    //              parameter - the parameter to write
    //              mem - pointer to the target memory
    //              memSize - the size of the mem in bytes
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteInHFAParameter(__in Var arg, RtABIPARAMETER parameter, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {

        Assert(parameter->type->typeCode == tcStructType);
        RtTYPEDEFINITIONTYPE typeDefinitionType = TypeDefinitionType::From(parameter->type);
        RtEXPR expr;
        HRESULT hr = projectionContext->GetExpr(typeDefinitionType->typeId, typeDefinitionType->typeDef->id, nullptr, typeDefinitionType->genericParameters, &expr);
        if (FAILED(hr)) {
            Js::VerifyOkCatastrophic(hr);
        }

        Assert(expr->type == exprFunction);

        RtFUNCTION function = Function::From(expr);
        Assert(function->functionType == functionStructConstructor);

        return WriteStructConstructorTypeInParameter(arg, StructConstructor::From(function), false, mem, memSize, true);
    }

    bool ProjectionMarshaler::CanMarshalExpr(RtEXPR expr)
    {
        return projectionContext->GetProjectionBuilder()->CanMarshalExpr(expr);
    }

    bool ProjectionMarshaler::CanMarshalType(RtTYPE type, bool allowMissingTypes, bool *outWasMissingType)
    {
        return type->CanMarshal(projectionContext->GetProjectionBuilder(), allowMissingTypes, outWasMissingType);
    }

    void ProjectionMarshaler::RecordTypeToUndo(
        fnTypeRecordString stringRecorder, 
        fnTypeRecordUnknown unknownsRecorder, 
        RtCONCRETETYPE concrete, 
        __in_bcount(memSize) byte * mem, 
        __in size_t memSize)
    {
        Assert(mem != nullptr);

        if (memSize < concrete->storageSize)
        {
            Js::Throw::FatalProjectionError();
        }

        // Record the HSTRING and unknown pointers to revert after out
        if (!ConcreteType::IsBlittable(concrete))
        {
            switch(concrete->typeCode)
            {
            case tcInterfaceType:
            case tcClassType:
            case tcDelegateType:
                {
                    // IUnknwon * to revert
                    (this->*unknownsRecorder)((IUnknown **)mem);
                }
                break;

            case tcBasicType:
                {
                    RtBASICTYPE basicType = BasicType::From(concrete);
                    if (basicType->typeCor == ELEMENT_TYPE_OBJECT)
                    {
                        (this->*unknownsRecorder)((IUnknown **)mem);
                    }
                    else if (basicType->typeCor == ELEMENT_TYPE_STRING)
                    {
                        (this->*stringRecorder)((HSTRING *)mem);
                    }
                }
                break;

            case tcStructType:
                {
                    RtSTRUCTTYPE structType = StructType::From(concrete);
                    RecordStructTypeToUndo(stringRecorder, unknownsRecorder, structType, mem, memSize);
                }
                break;
            }
        }
    }

    void ProjectionMarshaler::RecordStructTypeToUndo(
        fnTypeRecordString stringRecorder, 
        fnTypeRecordUnknown unknownsRecorder, 
        RtSTRUCTTYPE structType, 
        __in_bcount(memSize) byte * mem, 
        __in size_t memSize)
    {
        ImmutableList<RtABIFIELDPROPERTY> * properties = structType->fields;
        while (properties)
        {
            RtABIFIELDPROPERTY prop = properties->First();
            RecordTypeToUndo(stringRecorder, unknownsRecorder, prop->type, mem + prop->fieldOffset, prop->type->storageSize);
            properties = properties->GetTail();
        }
    }

    // Info:        Write the given type as an out parameter
    // Parameters:  inOutArg - the input
    //              type - the type to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteOutType(__in Var inOutArg, RtTYPE type, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        // If the type should be hidden, return the proper error
        bool isMissingType = true;
        Assert(CanMarshalType(type, true, &isMissingType));

        if (ByRefType::Is(type))
        {
            RtBYREFTYPE byRefType = ByRefType::From(type);
            return WriteOutType(inOutArg, byRefType->pointedTo, mem, memSize);
        }
        if (ArrayType::Is(type))
        {
            if(inOutArg == nullptr)
            {
                // ReceiveArray Pattern
                byte * countStorage = AnewArrayZ(alloc,byte,sizeof(uint));
                byte * arrayStorage = AnewArrayZ(alloc,byte,sizeof(LPVOID));
                byte * next = WriteInPointer(countStorage, mem, memSize);    
                return WriteInPointer(arrayStorage, next, memSize-sizeof(uint));    
            }
            else
            {
                // FillArray Pattern
                RtARRAYTYPE arrayType = ArrayType::From(type);
                WriteInArrayTypeIndividual(inOutArg, arrayType, false, mem, mem+sizeof(LPVOID), true);
                return mem + arrayType->sizeOnStack;
            }
        }

        RtCONCRETETYPE concrete = ConcreteType::From(type);
        size_t size = concrete->storageSize;
        if (size > 0)
        {
            Assert(size != 0xffffffff);
            byte * storage = AnewArrayZ(alloc, byte, size);

            RecordTypeToUndo(&ProjectionMarshaler::RecordOutString, &ProjectionMarshaler::RecordOutUnknown, concrete, storage, size);
            
            return WriteInPointer(storage, mem, memSize);
        }
        Assert(isMissingType);
        //This code path would be exercised only when invoking a delegate with missing parameters, and the flag WinRTDelegateInterfaces turned on
        //The design of this code path hasn't yet been agreed upon and thus is simply following what was written by the PM at the time of this change (pass null for all unresolvable delegate parameters)
        return WriteInPointer(nullptr, mem, memSize);
    }

    // Info:        Initialize the given type as an out parameter
    // Parameters:  type - the type to write
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     false if an out pointer was invalid, true otherwise
    bool ProjectionMarshaler::InitializeDelegateOutType(RtTYPE type, __in_bcount(memSize) byte * mem,__in size_t memSize)
    {
        // If the type should be hidden, return the proper error
        Assert(CanMarshalType(type));

        if (mem == nullptr)
        {
            return false;
        }

        Assert(!ArrayType::Is(type));
        if (ByRefType::Is(type))
        {
            RtBYREFTYPE byRefType = ByRefType::From(type);
            RtCONCRETETYPE concrete = ConcreteType::From(byRefType->pointedTo);
            Assert(!ArrayType::Is(concrete));
            return InitializeDelegateOutType(concrete,*(byte**)mem,concrete->storageSize);
        }

        MemoryCleanupRecord record = {mem, memSize};
        RecordDelegateOutMemory(record);

        return true;
    }

    // Info:        Initialize the given array type as an out parameter
    // Parameters:  arrayType - the type to write
    //              mem1 - pointer to the length memory
    //              memSize1 - the size of mem1 in bytes
    //              mem2 - pointer to the array memory
    //              memSize2 - the size of mem2 in bytes
    //              hasLength - whether the array has an associated length_is
    //              lengthValue - the length_is value
    // Returns:     false if an out pointer was invalid, true otherwise
    bool ProjectionMarshaler::InitializeDelegateOutArrayType(RtARRAYTYPE arrayType, __in_bcount(memSize1) byte * mem1, __in size_t memSize1,  
        __in_bcount(memSize2) byte * mem2, __in size_t memSize2, bool isFillArray, bool hasLength, uint32 lengthValue)
    {
        // If the type should be hidden, return the proper error
        Assert(CanMarshalType(arrayType));

        bool noError = true;
        if ((mem1 == nullptr) || (mem2 == nullptr))
        {
            noError = false;
        }

        if (isFillArray)
        {
            if (!noError)
            {
                return noError;
            }
            uint elementCount = *(uint*)mem1;
            byte * arrayContents = *(byte**)mem2;

            if (arrayContents == nullptr)
            {
                return false;
            }

            uint32 arrayLength = (hasLength && (lengthValue < elementCount)) ? lengthValue : elementCount;
            size_t elementSize = ConcreteType::From(arrayType->elementType)->storageSize;
            MemoryCleanupRecord record = {arrayContents, (arrayLength*elementSize)};
            RecordDelegateOutMemory(record);
            return noError;
        }

        if (mem1)
        {
            MemoryCleanupRecord record = {mem1, memSize1};
            RecordDelegateOutMemory(record);
        }
        if (mem2)
        {
            MemoryCleanupRecord record = {mem2, memSize2};
            RecordDelegateOutMemory(record);
        }

        return noError;
    }

    // Info:        Write an out parameter
    // Parameters:  inOutArg - the input
    //              parameter - the parameter
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     a pointer to the byte right after this write
    byte * ProjectionMarshaler::WriteOutParameter(__in Var inOutArg, __in RtABIPARAMETER parameter, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId)
    {
        return WriteOutType(inOutArg, parameter->type, mem, memSize);
    }

    // Info:        Read as IInspectable. If successful, then marshal as the runtime type.
    // Parameters:  mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadInspectableUnknown(__in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions)
    {
        // From ABIProjectionProvider.cpp(227)-----------------------------------------------------------------------------------------------
        IInspectable *interfacePtr = *(IInspectable**)mem;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (interfacePtr == nullptr)
        {
            return scriptContext->GetLibrary()->GetNull();
        }

        IUnknown *unknown = nullptr;
        HRESULT hr = QueryInterfaceAfterLeaveScript(scriptContext, interfacePtr, IID_IUnknown, (LPVOID *)&unknown);
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
        Assert(unknown != nullptr);
        RecordToUndo(unknown, true);

        Var result = TryReadInspectableUnknown(unknown, interfacePtr, methodNameId, allowIdentity, allowExtensions, true);
        if (Js::JavascriptOperators::GetTypeId(result) == Js::TypeIds_Undefined)
        {
            // GetRuntimeClassName failed, undo interfacePtr
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_ReturnValue_NeedInspectable, (PCWSTR)((methodNameId != MetadataStringIdNil) ? StringOfId(methodNameId) : nullptr));
        }

        return result;
    }

    // Info:        Try to interpret as IInspectable. If successful, then marshal as the runtime type.
    // Parameters:  mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::TryReadInspectableUnknown(__in IUnknown * unknown, __in IInspectable *interfacePtr, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, bool allowEmptyRuntimeClassName, ConstructorArguments* constructorArguments)
    {
        Assert(interfacePtr != NULL);
        Assert(unknown != nullptr);

#if DBG
        IInspectable *inspectable;
        HRESULT hr = S_OK;

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = interfacePtr->QueryInterface(__uuidof(IInspectable), (void**)&inspectable);
            if (SUCCEEDED(hr))
            {
                inspectable->Release();
            }
        }
        END_LEAVE_SCRIPT(scriptContext)

        if (FAILED(hr) || !inspectable)
        {
            Assert(false);
            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);       
        }
#endif
        return TryReadInspectable(unknown, interfacePtr, methodNameId, allowIdentity, allowExtensions, allowEmptyRuntimeClassName, constructorArguments);
    }

    // Info:        Takes owndership of className string and projects the var using className
    // Parameters:  inspectable - pointer to the target IInspectable object
    //              className - GetRuntimeClassName value of the inspectable
    //              methodNameId - PropertyId for the method name
    // Returns:     The resulting Var
    Var ProjectionMarshaler::TransferOwnershipAndReadInterfaceFromClassName(IInspectable *inspectable, HSTRING className, MetadataStringId methodNameId, ConstructorArguments* constructorArguments)
    {

#if DBG
        ProjectionModel::AllowHeavyOperation allow;
#endif

        Assert(inspectable != nullptr);
        Assert(className != nullptr);
        Assert(methodNameId != MetadataStringIdNil);

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        // This is a QI so we do not want to mark the callout for debug-stepping (via MarkerForExternalDebugStep).
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);

        HRESULT hr;
        IUnknown *unknown = nullptr;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = inspectable->QueryInterface(IID_IUnknown, (void **)&unknown);
        }
        END_LEAVE_SCRIPT(scriptContext)

        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
        RecordToUndo(unknown, true);

        // See if we can use the cached one
        Var result = nullptr;
        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
        if (projectionWriter->TryGetTypedInstanceFromCache(unknown, &result, false))
        {
            projectionContext->TypeInstanceCreated(inspectable);
            projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsDeleteString(className);
            return result;
        }

        AutoHSTRING runtimeClassStr(projectionContext->GetThreadContext()->GetWinRTStringLibrary());
        runtimeClassStr.Initialize(className);

        Windows::Foundation::IPropertyValue *propertyValue = nullptr;
        HRESULT hrPV = S_OK;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hrPV = inspectable->QueryInterface(Windows::Foundation::IID_IPropertyValue, (void **)&propertyValue);
        }
        END_LEAVE_SCRIPT(scriptContext)

        if (SUCCEEDED(hrPV))
        {
            RecordToUndo(propertyValue, true);
            return ReadPropertyValueVarFromRuntimeClassName(runtimeClassStr, unknown, propertyValue, true, false, methodNameId);
        }

        return ReadVarFromRuntimeClassName(runtimeClassStr, unknown, inspectable, true, false, false, methodNameId, constructorArguments);
    }

    // Info:        Obtains an IUnknown for the var, representing the interface of the given expression
    // Parameters:  varInput - the var to be marshaled as the interface type
    //              expr - the expression of the interface type
    // Returns:     The resulting IUnknown
    IUnknown * ProjectionMarshaler::GetFastPathInterfaceFromExpr(Var varInput, RtEXPR expr)
    {
        Assert(expr->type == exprFunction);
        RtFUNCTION function = Function::From(expr);
        Assert(function->functionType == functionInterfaceConstructor);

        IUnknown * unknown = nullptr;
        WriteRuntimeInterfaceConstructor(varInput, RuntimeInterfaceConstructor::From(function), (byte *)&unknown, sizeof(unknown));

        return unknown;
    }

    // Info:        Marshal a given IInspectable as the runtime type.
    // Parameters:  inspectable - pointer to the target IInspectable object
    // Returns:     The resulting Var
    Var ProjectionMarshaler::TryReadInspectable(__in IUnknown *unknown, __in IInspectable * inspectable, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, bool allowEmptyRuntimeClassName, ConstructorArguments* constructorArguments)
    {
        Assert(inspectable != NULL);
        Assert(unknown != nullptr);

        // See if we can use the cached one
        Var result = nullptr;
        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
        if (allowIdentity && projectionWriter->TryGetTypedInstanceFromCache(unknown, &result, allowExtensions))
        {
            projectionContext->TypeInstanceCreated(inspectable);
            return result;
        }

        HSTRING runtimeClassName = nullptr;
        HRESULT hr = S_OK;

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = inspectable->GetRuntimeClassName(&runtimeClassName);
        }
        END_LEAVE_SCRIPT(scriptContext)

        if (SUCCEEDED(hr))
        {
            AutoHSTRING runtimeClassStr(projectionContext->GetThreadContext()->GetWinRTStringLibrary());
            runtimeClassStr.Initialize(runtimeClassName);

            // fast path : string compare for basic types
            Windows::Foundation::IPropertyValue *propertyValue = nullptr;
            hr = QueryInterfaceAfterLeaveScript(scriptContext, inspectable, Windows::Foundation::IID_IPropertyValue, (void **)&propertyValue);
            if (SUCCEEDED(hr))
            {
                RecordToUndo(propertyValue, true);
                return ReadPropertyValueVarFromRuntimeClassName(runtimeClassStr, unknown, propertyValue, allowIdentity, allowExtensions, methodNameId);
            }

            return ReadVarFromRuntimeClassName(runtimeClassStr, unknown, inspectable, allowIdentity, allowExtensions, allowEmptyRuntimeClassName, methodNameId, constructorArguments);
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var ProjectionMarshaler::ReadPropertyValueVarFromRuntimeClassName(
        AutoHSTRING &runtimeClassHString, 
        __in IUnknown *unknown,
        __in Windows::Foundation::IPropertyValue *propertyValue, 
        bool allowIdentity,
        bool allowExtensions,
        MetadataStringId methodNameId)
    {
        Assert(propertyValue != nullptr);

        // I believe looking for get_Type and switching on it is more performant compared to 
        // looking for IReference and <T> combination, parsing the T to be type and then calling get_Value
        // But we can time it later

        PCWSTR runtimeClassString = projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsGetStringRawBuffer(runtimeClassHString.Get(), nullptr);
        JS_ETW(EventWriteJSCRIPT_PROJECTION_PROPERTYVALUEVARFROMGRCN_START(runtimeClassString));

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        WCHAR *sIReference = wcsstr(runtimeClassString, _u("Windows.Foundation.IReference`1"));
        // If the string doesnt start with IReference`1 then try and check for IReferenceArray
        if (sIReference == nullptr || sIReference != runtimeClassString)
        {
            WCHAR *sIReferenceArray = wcsstr(runtimeClassString, _u("Windows.Foundation.IReferenceArray`1"));
            // If the string doesnt start with IReferenceArray`1 either then we dont support this type, throw error
            if (sIReferenceArray == nullptr || sIReferenceArray != runtimeClassString)
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_RTCInvalidRTCPropertyValueOut, runtimeClassString);    
            }
        }

        Windows::Foundation::PropertyType propertyType;
        HRESULT hr;
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = propertyValue->get_Type(&propertyType);
        }
        END_LEAVE_SCRIPT(scriptContext)
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        Var result = nullptr;
        switch(propertyType)
        {
        case Windows::Foundation::PropertyType_Boolean:
            {
                boolean booleanFromPropertyValue;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetBoolean(&booleanFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptBoolean::ToVar(booleanFromPropertyValue, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_UInt8:
            {
                byte byteValue;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt8(&byteValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptNumber::ToVar(byteValue, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_Int16:
            {
                INT16 value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetInt16(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptNumber::ToVar(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_UInt16:
            {
                UINT16 value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt16(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptNumber::ToVar(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_Int32:
            {
                INT32 value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetInt32(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptNumber::ToVar(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_UInt32:
            {
                UINT32 value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt32(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptNumber::ToVar(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_Int64:
            {
                INT64 value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetInt64(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptInt64Number::ToVar(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_UInt64:
            {
                UINT64 value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt64(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptUInt64Number::ToVar(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_Single:
            {
                float value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetSingle(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptNumber::ToVarWithCheck(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_Double:
            {
                double value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetDouble(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptNumber::ToVarWithCheck(value, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_Char16:
            {
                char16 value;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetChar16(&value);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = Js::JavascriptString::NewCopyBuffer(&value, 1, scriptContext);
            }
            break;

        case Windows::Foundation::PropertyType_String:
            {
                HSTRING hs;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetString(&hs);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                RecordToUndo(hs, true);
                result = ReadOutString((byte *)&hs, sizeof(HSTRING));
            }
            break;

        case Windows::Foundation::PropertyType_DateTime:
            {
                Windows::Foundation::DateTime dateTime;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetDateTime(&dateTime);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = ReadOutWindowsFoundationDateTimeType((byte *)&dateTime, sizeof(dateTime));                
            }
            break;

        case Windows::Foundation::PropertyType_TimeSpan:
            {
                Windows::Foundation::TimeSpan timeSpan;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetTimeSpan(&timeSpan);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = ReadOutWindowsFoundationTimeSpanType((byte *)&timeSpan, sizeof(timeSpan));                
            }
            break;

        case Windows::Foundation::PropertyType_Guid:
            {
                GUID guidFromPropertyValue;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetGuid(&guidFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                result = ReadOutSystemGuidType(true, (byte *)&guidFromPropertyValue, sizeof(guidFromPropertyValue));                
            }
            break;

        case Windows::Foundation::PropertyType_Inspectable:
            {
                Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_TypeMismatch);
            }
            break;

        case Windows::Foundation::PropertyType_UInt8Array:
            {
                byte *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt8Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetByteBasicType(),
                    projectionContext, 
                    arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_Int16Array:
            {
                INT16 *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetInt16Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetInt16BasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_UInt16Array:
            {
                UINT16 *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt16Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetUint16BasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_Int32Array:
            {
                INT32 *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetInt32Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetInt32BasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_UInt32Array:
            {
                UINT32 *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt32Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetUint32BasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_Int64Array:
            {
                INT64 *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetInt64Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetInt64BasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_UInt64Array:
            {
                UINT64 *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetUInt64Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetUint64BasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_SingleArray:
            {
                float *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetSingleArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetFloatBasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_DoubleArray:
            {
                double *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetDoubleArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetDoubleBasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_Char16Array:
            {
                char16 *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetChar16Array(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetChar16BasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_BooleanArray:
            {
                boolean *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetBooleanArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetBoolBasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_StringArray:
            {
                HSTRING *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetStringArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetStringBasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_DateTimeArray:
            {
                Windows::Foundation::DateTime *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetDateTimeArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    ConcreteType::From(projectionContext->GetProjectionBuilder()->GetWindowsFoundationDateTimeType()), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_TimeSpanArray:
            {
                Windows::Foundation::TimeSpan *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetTimeSpanArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    ConcreteType::From(projectionContext->GetProjectionBuilder()->GetWindowsFoundationTimeSpanType()), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_GuidArray:
            {
                GUID *arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetGuidArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    ConcreteType::From(projectionContext->GetProjectionBuilder()->GetSystemGuidType()), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_InspectableArray:
            {
                IInspectable **arrayFromPropertyValue;
                UINT32 length;
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = propertyValue->GetInspectableArray(&length, &arrayFromPropertyValue);
                }
                END_LEAVE_SCRIPT(scriptContext)
                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

                hr = ArrayProjection::CreateArrayProjectionObject(
                    projectionContext->GetProjectionBuilder()->GetObjectBasicType(), 
                    projectionContext, 
                    (byte *)arrayFromPropertyValue, 
                    length, 
                    true, 
                    &result);

                IfFailedMapAndThrowHr(scriptContext, hr);
            }
            break;

        case Windows::Foundation::PropertyType_Empty:
            {
                result = scriptContext->GetLibrary()->GetNull();
            }
            break;

        default:
            {
                // Since we cant read this var directly and ReadVarFromRuntimeClassName would take care reading the var as well as recording the unknown for undo, 
                // we need to revert our earlier recorded undo action
                Var resultToReturn = ReadVarFromRuntimeClassName(runtimeClassHString, unknown, propertyValue, allowIdentity, allowExtensions, false, methodNameId);
                JS_ETW(EventWriteJSCRIPT_PROJECTION_PROPERTYVALUEVARFROMGRCN_STOP(runtimeClassString));
                return resultToReturn;
            }
        }

        // we have read the type: Release the reference
        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
        Assert(projectionWriter);
        if (allowIdentity)
        {
            projectionContext->TypeInstanceCreated(propertyValue);
        }

        if (!allowExtensions && Js::DynamicObject::Is(result))
        {
            if (!this->projectionContext->AreProjectionPrototypesConfigurable())
            {
                Js::DynamicObject * resultObject = Js::DynamicObject::FromVar(result);
                BOOL succeeded = resultObject->PreventExtensions();
                Js::VerifyCatastrophic(succeeded);
            }
            else
            {
                TRACE_METADATA(_u("ReadPropertyValueVarFromRuntimeClassName(%s) - design mode, extensions permitted\n"), runtimeClassString);
            }
        }

        JS_ETW(EventWriteJSCRIPT_PROJECTION_PROPERTYVALUEVARFROMGRCN_STOP(runtimeClassString));

        return result;
    }

    Var ProjectionMarshaler::ReadVarFromRuntimeClassName(AutoHSTRING &runtimeClassHString, __in IUnknown *unknown, __in IInspectable *inspectable, bool allowIdentity, bool allowExtensions, bool allowEmptyRuntimeClassName, MetadataStringId methodNameId, ConstructorArguments* constructorArguments)
    {
        Assert(unknown != nullptr);
        Assert(inspectable != nullptr);

#ifdef ENABLE_JS_ETW
        PCWSTR runtimeClassString = projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsGetStringRawBuffer(runtimeClassHString.Get(), nullptr);
        EventWriteJSCRIPT_PROJECTION_VARFROMGRCN_START(runtimeClassString);
#endif

        // Emptry GRCN string
        if (runtimeClassHString.Get() == nullptr)
        {
            if (allowEmptyRuntimeClassName)
            {
                Var result = ReadEmptyRuntimeClassNameUnknown(inspectable, allowIdentity, allowExtensions, constructorArguments);
                JS_ETW(EventWriteJSCRIPT_PROJECTION_VARFROMGRCN_STOP(runtimeClassString));

                return result;
            }
            else
            {
                JS_ETW(EventWriteJSCRIPT_PROJECTION_VARFROMGRCN_STOP(runtimeClassString));
                return projectionContext->GetScriptContext()->GetLibrary()->GetUndefined();
            }
        }

        RtEXPR expr = nullptr;
        HRESULT hr = projectionContext->GetExprFromConcreteTypeName(runtimeClassHString.Get(), &expr);
        if (SUCCEEDED(hr))
        {
            if (!CanMarshalExpr(expr))
            {
                Js::Number hostType = projectionContext->GetScriptContext()->GetConfig()->GetHostType();

                if (hostType == Js::HostTypeApplication)
                {
                    Js::VerifyCatastrophic(methodNameId != MetadataStringIdNil);
                    Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), JSERR_FunctionArgument_Invalid, StringOfId(methodNameId));
                }
                else if (hostType == Js::HostTypeWebview)
                {
                    // The above VerifyCatastrophic can terminate the app's process if the AllowForWeb attribute check
                    // is preventing the runtime class from being projected.  We want to be developer friendly if a an
                    // app developer provides the webview with an object without the object - instead of terminating
                    // the process, we should instead log an error (handled at a higher layer, in the webview control)
                    // and just treat the object's value as undefined.
                    JS_ETW(EventWriteJSCRIPT_PROJECTION_VARFROMGRCN_STOP(runtimeClassString));
                    return projectionContext->GetScriptContext()->GetLibrary()->GetUndefined();
                }
                Js::Throw::FatalProjectionError();
            }

            if (expr->type == exprFunction)
            {
                RtFUNCTION function = Function::From(expr);
                switch(function->functionType)
                {
                case functionInterfaceConstructor:
                    {
                        Var result = ReadInterfaceConstructorOutParameterFromUnknown(RuntimeInterfaceConstructor::From(function), unknown, allowIdentity, allowExtensions, constructorArguments);
                        JS_ETW(EventWriteJSCRIPT_PROJECTION_VARFROMGRCN_STOP(runtimeClassString));
                        return result;
                    }

                case functionRuntimeClassConstructor: 
                    {
                        Var result = ReadRuntimeClassConstructorOutParameterFromUnknown(RuntimeClassConstructor::From(function), unknown, allowIdentity, allowExtensions, constructorArguments);
                        JS_ETW(EventWriteJSCRIPT_PROJECTION_VARFROMGRCN_STOP(runtimeClassString));
                        return result;
                    }
                }
            }
            Js::Throw::FatalProjectionError();
        }

        JS_ETW(EventWriteJSCRIPT_PROJECTION_VARFROMGRCN_STOP(runtimeClassString));
        return projectionContext->GetScriptContext()->GetLibrary()->GetUndefined();
    }

    // Info:        Read a delegate
    // Parameters:  constructor - the delegate
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadDelegateConstructorOutParameter(RtDELEGATECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize, bool allowIdentity, bool allowExtensions)
    {
        IUnknown * unknown = *(IUnknown**)mem;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (unknown)
        {
            // Check if the delegate has the method thunks we sent out
            RtRUNTIMEINTERFACECONSTRUCTOR invokeInterface = RuntimeInterfaceConstructor::From(constructor->invokeInterface);
#if DBG
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                IUnknown* temp = nullptr;
                if(SUCCEEDED(unknown->QueryInterface(invokeInterface->iid->instantiated, (void**)&temp)) && temp)
                {
                    temp->Release();
                }
                else
                {
                    // This is a bug, the QI should have been successful.
                    // This assert causes unit tests to fail - opened up bug 251728 to track this issue
                    // Assert(0);
                }
            }
            END_LEAVE_SCRIPT(scriptContext)
#endif

            IUnknown *delegateUnknown;
            HRESULT hr = QueryInterfaceAfterLeaveScript(scriptContext, unknown, invokeInterface->iid->instantiated, (void**)&delegateUnknown);
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
            RecordToUndo(delegateUnknown, true);

            if (*((LPVOID**)delegateUnknown) == (LPVOID *)g_DelegateVtable)
            {
                // At this point if scriptSite is closed then we cant guarantee using the function that was wrapped in here, so we throw E_ACCESSDENIED
                if (projectionContext->GetScriptSite()->IsClosed())
                {
                    Js::JavascriptError::MapAndThrowError(scriptContext, E_ACCESSDENIED);
                }

                // Unwrap the function and return it.
                Delegate * impl = GetClassFromIUnknown(Delegate, delegateUnknown);
                Js::JavascriptFunction *jsFunc = impl->GetCallback();
                if (jsFunc == nullptr)
                {
                    Assert(impl->SupportsWeakDelegate());
                    // The delegate was disconnected from the jsfunction
#ifdef WINRTFINDPREMATURECOLLECTION
                    Assert(false);
#endif
                    Assert(impl->eventInfo != nullptr);
                    if (Js::Configuration::Global.flags.FailFastIfDisconnectedDelegate)
                    {
                        impl->ThrowFatalDisconnectedDelegateError(scriptContext);
                    }
                    else
                    {
                        Js::JavascriptErrorDebug::MapAndThrowErrorWithInfo(scriptContext, RPC_E_DISCONNECTED);
                    }
                }

                if (allowIdentity)
                {
                    return jsFunc;                
                }
                else
                {
                    // Wrap this in another function
                    ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
                    PropertyId nameId = IdOfString(constructor->simpleName);
                    Js::JavascriptFunction *wrappedFunction = projectionWriter->BuildDirectFunction(jsFunc, DelegateForwarderThunk, nameId, false);
                    Var lengthVar = Js::JavascriptOperators::GetProperty(jsFunc, Js::PropertyIds::length, scriptContext, nullptr);
                    projectionWriter->SetProperty(wrappedFunction, projectionWriter->lengthId, lengthVar);
                    return wrappedFunction;
                }
            }

            ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
            RtPROPERTY invokeProperty = invokeInterface->prototype->fields->First();
            RtABIMETHODPROPERTY invokeAbiProperty = AbiMethodProperty::From(invokeProperty);
            RtABIMETHODSIGNATURE methodSignature = invokeAbiProperty->body->signature;
            PropertyId nameId = IdOfString(constructor->simpleName);

            DelegateThis * thisInfo = nullptr;
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                BEGIN_TRANSLATE_OOM_TO_HRESULT
                {
                    thisInfo = RecyclerNewFinalized(scriptContext->GetRecycler(), DelegateThis, constructor, delegateUnknown);
                }
                END_TRANSLATE_OOM_TO_HRESULT(hr)
            }
            END_LEAVE_SCRIPT(scriptContext)
            IfFailedMapAndThrowHr(scriptContext, hr);

            Signature * signature = RecyclerNew(scriptContext->GetRecycler(), Signature, projectionContext, thisInfo, methodSignature);
            Js::JavascriptFunction * function = projectionWriter->BuildDirectFunction(signature, MethodSignatureThunk, nameId, false);
            if (invokeAbiProperty->body->properties)
            {
                projectionWriter->ApplyPropertiesObjectToJsObject(function, invokeAbiProperty->body->properties, thisInfo);
            }

            if (!allowExtensions)
            {
                BOOL succeeded = function->PreventExtensions();
                Js::VerifyCatastrophic(succeeded);
            }

            return function;
        }

        return scriptContext->GetLibrary()->GetNull();        
    }

    // Info:        Read as an interface (or possibly runtime class)
    // Parameters:  definitelyNotRuntimeClass - true if this is definitely not a runtime class
    //              constructor - the runtime interface
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadInterfaceConstructorOutParameter(bool definitelyNotRuntimeClass, RtRUNTIMEINTERFACECONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        IInspectable * inspectable = *(IInspectable**)mem;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (inspectable)
        {
            IUnknown *unknown = nullptr;
            HRESULT hr = S_OK;
            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
#if DBG
                IInspectable* temp = nullptr;
                if(SUCCEEDED(inspectable->QueryInterface(constructor->iid->instantiated, (void**)&temp)) && temp)
                {
                    temp->Release();
                }
                else
                {
                    // This is a bug, the QI should have been successful.
                    // This assert causes unit tests to fail - opened up bug 251728 to track this issue
                    // Assert(0);
                }
#endif

                // Get Unknown and use it.
                hr = inspectable->QueryInterface(&unknown);
            }
            END_LEAVE_SCRIPT(scriptContext)
            IfFailedMapAndThrowHr(scriptContext, hr);
            Assert(unknown != nullptr);
            RecordToUndo(unknown, true);

            if (!definitelyNotRuntimeClass)
            {
                Var result = TryReadInspectableUnknown(unknown, inspectable, methodNameId, allowIdentity, allowExtensions, false, constructorArguments);
                if (Js::JavascriptOperators::GetTypeId(result) != Js::TypeIds_Undefined)
                {
                    return result;
                }
            }

            return ReadInterfaceConstructorOutParameterFromUnknown(constructor, unknown, allowIdentity, allowExtensions, constructorArguments);
        }

        return scriptContext->GetLibrary()->GetNull();        
    }

    // Info:        Read IUnknown as an interface
    // Parameters:  constructor - the runtime interface
    //              unknown - the target object
    // Returns:     The result var
    Var ProjectionMarshaler::ReadInterfaceConstructorOutParameterFromUnknown(RtRUNTIMEINTERFACECONSTRUCTOR constructor, IUnknown * unknown, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        Assert (unknown != nullptr);

#if DBG
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            IUnknown* temp = nullptr;
            if(SUCCEEDED(unknown->QueryInterface(constructor->iid->instantiated, (void**)&temp)) && temp)
            {
                temp->Release();
            }
            else
            {
                // This is a bug, the QI should have been successful.
                // This assert causes unit tests to fail - opened up bug 251728 to track this issue
                // Assert(0);
            }
        }
        END_LEAVE_SCRIPT(scriptContext)
#endif

        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
        Var result = nullptr;
        if (!allowIdentity || !projectionWriter->TryGetTypedInstanceFromCache(unknown, &result, allowExtensions))
        {
            result =  projectionWriter->CreateNewTypeInstance(
                constructor->typeId,
                StringOfId(constructor->typeId), 
                constructor->iid,
                constructor->specialization, 
                constructor->prototype, 
                constructor->properties, 
                constructor->signature, 
                constructor->hasEventHandlers, 
                DefaultGCPressure,
                unknown,
                allowIdentity,
                allowExtensions,
                constructorArguments);
        }

        projectionContext->TypeInstanceCreated(unknown);
        return result;
    }

    // Info:        Read as a runtime class
    // Parameters:  constructor - the constructor of the runtime class
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadRuntimeClassConstructorOutParameter(RtRUNTIMECLASSCONSTRUCTOR constructor, __in_bcount(memSize) byte * mem,__in size_t memSize, bool allowIdentity, bool allowExtensions)
    {
        IInspectable * inspectable = *(IInspectable**)mem;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        if (inspectable)
        {
            IUnknown *unknown = nullptr;
            HRESULT hr = QueryInterfaceAfterLeaveScript(scriptContext, inspectable, IID_IUnknown, (LPVOID *)&unknown);
            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
            Assert(unknown != nullptr);
            RecordToUndo(unknown, true);
            return ReadRuntimeClassConstructorOutParameterFromUnknown(constructor, unknown, allowIdentity, allowExtensions);
        }

        return scriptContext->GetLibrary()->GetNull();      
    }

    Var ProjectionMarshaler::ReadEmptyRuntimeClassNameUnknown(IUnknown *unknown, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        Assert(unknown != nullptr);

        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
        Var result = nullptr;
        if (!allowIdentity || !projectionWriter->TryGetTypedInstanceFromCache(unknown, &result))
        {
            result = projectionWriter->CreateNewTypeInstance(
                projectionContext->IdOfString(_u("")), 
                _u(""),
                nullptr, 
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                false,
                DefaultGCPressure,
                unknown,
                allowIdentity,
                allowExtensions,
                constructorArguments);
        }

        projectionContext->TypeInstanceCreated(unknown);
        return result;
    }

    // Info:        Read IUnknown as a runtime class
    // Parameters:  constructor - the constructor of the runtime class
    //              unknown - the target object
    // Returns:     The result var
    Var ProjectionMarshaler::ReadRuntimeClassConstructorOutParameterFromUnknown(RtRUNTIMECLASSCONSTRUCTOR constructor, IUnknown * unknown, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        Assert (unknown != nullptr);
        
#if DBG
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            IUnknown* temp = nullptr;
            auto defaultInterfaceValue = constructor->defaultInterface.GetValue();
            if(ifRuntimeInterfaceConstructor == defaultInterfaceValue->interfaceType)
            {
                RtRUNTIMEINTERFACECONSTRUCTOR iface = RuntimeInterfaceConstructor::From(defaultInterfaceValue);
                if(SUCCEEDED(unknown->QueryInterface(iface->iid->instantiated, (void**)&temp)) && temp)
                {
                    temp->Release();
                }
                else
                {
                    // This is a bug, the QI should have been successful.
                    // This assert causes unit tests to fail - opened up bug 251728 to track this issue
                    // Assert(0);
                }
            }
        }
        END_LEAVE_SCRIPT(scriptContext)
#endif

        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();
        Var result = nullptr;
        if (!allowIdentity || !projectionWriter->TryGetTypedInstanceFromCache(unknown, &result))
        {
            RtIID defaultInterfaceIID = nullptr;
            if (constructor->defaultInterface.HasValue())
            {
                auto defaultInterface = constructor->defaultInterface.GetValue();
                if (RuntimeInterfaceConstructor::Is(defaultInterface))
                {
                    defaultInterfaceIID = RuntimeInterfaceConstructor::From(defaultInterface)->iid;
                }
            }
            result = projectionWriter->CreateNewTypeInstance(
                constructor->typeId,
                StringOfId(constructor->typeDef->id), 
                defaultInterfaceIID,
                constructor->specialization, 
                constructor->prototype, 
                constructor->properties, 
                constructor->signature, 
                constructor->hasEventHandlers, 
                constructor->gcPressure,
                unknown,
                allowIdentity,
                allowExtensions,
                constructorArguments);
        }

        projectionContext->TypeInstanceCreated(unknown);
        return result;
    }

    // Info:        Read as a struct
    // Parameters:  constructor - the struct
    //              structsByValue - whether struct is passed by value
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadStructConstructorOut(RtSTRUCTCONSTRUCTOR constructor, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowExtensions)
    {
        // From ABIPRojectionProvider.cpp(426) GetStructProjection -------------------------------------------------------------------------------------------------------------
        // BUGBUG (yongqu): create with the prototype of the struct. 
        ProjectionWriter * projectionWriter = projectionContext->GetProjectionWriter();

        HRESULT hr = S_OK;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        HTYPE htype = nullptr;
        hr = projectionWriter->GetStructHType(constructor->structType, &htype);
        IfFailedMapAndThrowHr(scriptContext, hr);

        ScriptEngine *scriptEngine = projectionContext->GetScriptEngine();
        IfNullMapAndThrowHr(scriptContext, scriptEngine, E_ACCESSDENIED);

        Var object = RecyclerNew(scriptContext->GetRecycler(), Js::ExternalObject, (Js::ExternalType *)htype);
        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_WINRT_STRUCT_OBJECT(object, StringOfId(constructor->typeId)));

        byte * storage = mem;
#if _M_X64
        if (!structsByValue && constructor->structType->isPassByReference)
        {
            // Since structByValue be false only for In parameter of the method we can safely assert for CalleeRetainsOwnership
            Assert (resourceCleanup == CalleeRetainsOwnership);
            if (memSize<sizeof(LPVOID))
            {
                Js::Throw::FatalProjectionError();
            }

            // It is by reference so use the indirect pointer instead
            storage = *(byte **)mem;

            // we are reading only LPVOID from the mem so update only that
            Assert(constructor->structType->sizeOnStack == sizeof(LPVOID));
        }
#endif

        Js::DynamicObject * newInstance = Js::DynamicObject::FromVar(object);
        ImmutableList<RtABIFIELDPROPERTY> * properties = constructor->structType->fields;
        size_t remainingStorage = constructor->structType->storageSize;
        while(properties)
        {
            RtABIFIELDPROPERTY prop = properties->First();
            PropertyId id = prop->identifier; 
            byte * fieldMem = storage+prop->fieldOffset;
            Var fieldValue = ReadOutType(nullptr, prop->type, true, fieldMem, remainingStorage, methodNameId);
            remainingStorage -= prop->type->storageSize;
            Js::JavascriptOperators::SetProperty(newInstance, newInstance, id, fieldValue, scriptContext);
            properties = properties->GetTail();
        }

        Assert(remainingStorage<constructor->structType->storageSize); // remainingStorage > 0 when per-struct alignment rounded up  

        if (!allowExtensions && !this->projectionContext->AreProjectionPrototypesConfigurable())
        {
            BOOL succeeded = newInstance->PreventExtensions();
            Js::VerifyCatastrophic(succeeded);
        }

        return newInstance;
    }

    // Info:        Read as a model function
    // Parameters:  definitelyNotRuntimeClass - true if this is definitely not a runtime class
    //              function - the model function
    //              structsByValue - if structs are passed by value
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     The result var
    Var ProjectionMarshaler::ReadTypeFunctionOut(bool definitelyNotRuntimeClass, RtFUNCTION function, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        switch(function->functionType)
        {
        case functionDelegateConstructor:
            return ReadDelegateConstructorOutParameter(DelegateConstructor::From(function), mem, memSize, allowIdentity, allowExtensions);
        case functionInterfaceConstructor:
            return ReadInterfaceConstructorOutParameter(definitelyNotRuntimeClass, RuntimeInterfaceConstructor::From(function), mem, memSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);
        case functionRuntimeClassConstructor:
            return ReadRuntimeClassConstructorOutParameter(RuntimeClassConstructor::From(function), mem, memSize, allowIdentity, allowExtensions);
        case functionStructConstructor:
            // Struct always has its own identity
            return ReadStructConstructorOut(StructConstructor::From(function), structsByValue, mem, memSize, methodNameId, allowExtensions);
        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Read as a model expression
    // Parameters:  definitelyNotRuntimeClass - true if this is definitely not a runtime class
    //              expr - the model expr
    //              structsByValue - struct is passed by value
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     The result var
    Var ProjectionMarshaler::ReadExprTypeOut(bool definitelyNotRuntimeClass, RtEXPR expr, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        switch(expr->type)
        {
        case exprFunction: 
            return ReadTypeFunctionOut(definitelyNotRuntimeClass, Function::From(expr), structsByValue, mem, memSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);
        case exprEnum:
            return ReadOutBasicType(Enum::From(expr)->baseTypeCode, mem, memSize, methodNameId, allowIdentity, allowExtensions);

        }
        Js::Throw::FatalProjectionError();
    }

    // Info:        Read into the named type
    // Parameters:  definitelyNotRuntimeClass - true if this is definitely not a runtime class
    //              typeName - the type name
    //              genericparameters - generic parameters for the type if any
    //              structsByValue - if struct is passed by value
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     nullptr if the read failed
    Var ProjectionMarshaler::TryReadTypeNameOutParameter(bool definitelyNotRuntimeClass, MetadataStringId typeId, MetadataStringId typeNameId, ImmutableList<RtTYPE> * genericParameters, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        RtEXPR expr = nullptr;
        HRESULT hr = projectionContext->GetExpr(typeId, typeNameId, nullptr, genericParameters, &expr);
        if (FAILED(hr))
        {
            return nullptr;
        }
        return ReadExprTypeOut(definitelyNotRuntimeClass, expr, structsByValue, mem, memSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);
    }

    // Info:        Read into the named type
    // Parameters:  definitelyNotRuntimeClass - true if this is definitely not a runtime class
    //              typeName - the type name
    //              genericparameters - generic parameters for the type if any
    //              structsByValue - if struct is passed by value
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     The result var

    Var ProjectionMarshaler::ReadTypeNameOutParameter(bool definitelyNotRuntimeClass, MetadataStringId typeId, MetadataStringId typeNameId, ImmutableList<RtTYPE> * genericParameters, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        Var result = TryReadTypeNameOutParameter(definitelyNotRuntimeClass, typeId, typeNameId, genericParameters, structsByValue, mem, memSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);

        if (result)
        {
            return result;
        }

        // Unknown type
        Assert(typeNameId != MetadataStringIdNil);
        Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), JSERR_UnknownType, StringOfId(typeNameId));
    }

    // Info:        Read as type definition 
    // Parameters:  type - the model type definition
    //              structsByValue - if struct is passed by value
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     The result var
    Var ProjectionMarshaler::ReadTypeDefinitionTypeOutParameter(RtTYPEDEFINITIONTYPE type, bool structsByValue, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        bool definitelyNotRuntimeClass = false;
        definitelyNotRuntimeClass |= type->typeCode==tcStructType;
        definitelyNotRuntimeClass |= type->typeCode==tcEnumType;
        definitelyNotRuntimeClass |= type->typeCode==tcDelegateType;        
        return ReadTypeNameOutParameter(definitelyNotRuntimeClass, type->typeId, type->typeDef->id, type->genericParameters, structsByValue, mem, memSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);
    }

    // Info:        Read a basic type
    // Parameters:  typeCor - the basic type code
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadOutBasicType(CorElementType typeCor, __in_bcount(memSize) byte * mem, __in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions)
    {
        // FROM ABIPRojectionProvider.cpp(160)-------------------------------------------------------------------------------------------------------------
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        switch(typeCor)
        {
        case ELEMENT_TYPE_CHAR: 
            AnalysisAssert(memSize >= sizeof(char16));
            return Js::JavascriptString::NewCopyBuffer((char16*)mem, 1, scriptContext);
        case ELEMENT_TYPE_BOOLEAN:
            AnalysisAssert(memSize >= sizeof(bool));
            return Js::JavascriptBoolean::ToVar(*((bool*)mem), scriptContext);
        case ELEMENT_TYPE_U1:
            AnalysisAssert(memSize >= sizeof(unsigned __int8));
            return Js::JavascriptNumber::ToVar(*(unsigned __int8*)mem, scriptContext);
        case ELEMENT_TYPE_I2:
            AnalysisAssert(memSize >= sizeof(__int16));
            return Js::JavascriptNumber::ToVar(*(__int16*)mem, scriptContext);
        case ELEMENT_TYPE_U2:
            AnalysisAssert(memSize >= sizeof(unsigned __int16));
            return Js::JavascriptNumber::ToVar(*(unsigned __int16*)mem, scriptContext);
        case ELEMENT_TYPE_I4:
            AnalysisAssert(memSize >= sizeof(__int32));
            return Js::JavascriptNumber::ToVar(*(__int32*)mem, scriptContext);
        case ELEMENT_TYPE_U4:
            AnalysisAssert(memSize >= sizeof(unsigned __int32));
            return Js::JavascriptNumber::ToVar(*(unsigned __int32*)mem, scriptContext);
        case ELEMENT_TYPE_I8:
            AnalysisAssert(memSize >= sizeof(__int64));
            return Js::JavascriptInt64Number::ToVar(*(__int64*)mem, scriptContext);
        case ELEMENT_TYPE_U8:
            AnalysisAssert(memSize >= sizeof(unsigned __int64));
            return Js::JavascriptUInt64Number::ToVar(*(unsigned __int64*)mem, scriptContext);
        case ELEMENT_TYPE_R4:
            AnalysisAssert(memSize >= sizeof(float));
            return Js::JavascriptNumber::ToVarWithCheck(*(float*)mem, scriptContext);
        case ELEMENT_TYPE_R8:
            AnalysisAssert(memSize >= sizeof(double));
            return Js::JavascriptNumber::ToVarWithCheck(*(double*)mem, scriptContext);
        case ELEMENT_TYPE_OBJECT:
            {
                return ReadInspectableUnknown(mem, memSize, methodNameId, allowIdentity, allowExtensions);
            }
        case ELEMENT_TYPE_STRING:
            {
                return ReadOutString(mem, memSize);
            }
        }
        Js::Throw::FatalProjectionError();
    }

    Var ProjectionMarshaler::ReadOutString(__in_bcount(memSize) byte *mem, __in size_t memSize)
    {
        UINT32 length = 0;
        HSTRING hs = *(HSTRING*)mem;
        LPCWSTR sz = projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsGetStringRawBuffer(hs, &length); 
        return Js::JavascriptString::NewCopyBuffer(sz, length, projectionContext->GetScriptContext()); // NOTE: We could make a new basic type for HSTRINGs and avoid this copy.
    }

    // Info:        Read as a Windows.Foundation.DateTime instance
    // Parameters:  mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadOutWindowsFoundationDateTimeType(__in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        size_t sizeOfDateTime = sizeof(Windows::Foundation::DateTime); 

        if (memSize < sizeOfDateTime)
        {
            Js::Throw::FatalProjectionError();
        }

        INT64 rtDate = *(INT64*)mem;

        Var marshaledDate = Js::JavascriptWinRTDate::New(rtDate, projectionContext->GetScriptContext());

        return marshaledDate;
    }

    // Info:        Read as a Windows.Foundation.TimeSpan instance
    // Parameters:  mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadOutWindowsFoundationTimeSpanType(__in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        size_t sizeOfTimeSpan = sizeof(Windows::Foundation::TimeSpan); 

        if (memSize < sizeOfTimeSpan)
        {
            Js::Throw::FatalProjectionError();
        }

        INT64 rtSpan = *(INT64*)mem;
        double span;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        IfFailedMapAndThrowHr(scriptContext, WinRTTimeSpanToNumberV6(rtSpan, &span));        

        return Js::JavascriptNumber::ToVarNoCheck(span, scriptContext);
    }

    // Info:        Read as a system guid      
    // Parameters:  structsByValue - if struct is by value
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadOutSystemGuidType(bool structsByValue, __in_bcount(memSize) byte * mem, __in size_t memSize)
    {
        // From ABIPRojectionProvider.cpp(256)--------------------------------------------------------------------------------
        const size_t sizeOfGuid = 37;
        char16 guidString[sizeOfGuid] = _u("");

        byte * storage = mem;
#if _M_X64
        Assert(sizeof(GUID) > sizeof(LPVOID));
        if (!structsByValue)
        {
            // Since structByValue be false only for In parameter of the method we can safely assert for CalleeRetainsOwnership
            Assert (resourceCleanup == CalleeRetainsOwnership);
            if (memSize<sizeof(LPVOID))
            {
                Js::Throw::FatalProjectionError();
            }

            // It is by reference so use the indirect pointer instead
            storage = *(byte **)mem;
        }
#endif

        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        HRESULT hr = GUIDParser::TryGUIDToString((GUID*)storage, guidString, sizeOfGuid);
        IfFailedMapAndThrowHr(scriptContext, hr);
        return Js::JavascriptString::NewCopyBuffer(guidString, sizeOfGuid-1, scriptContext); 
    }

    // Info:        Read as an out array
    // Parameters:  inOutArgument - the in\out arg is present
    //              arrayType - the array type to read
    //              mem1 - pointer to the target memory for length
    //              memSize1 - the size of mem in bytes
    //              mem2 - pointer to the target memory for the array contents
    //              memSize2 - the size of mem in bytes
    // Returns:     The result var
    Var ProjectionMarshaler::ReadOutArrayType(Var inOutArgument, RtARRAYTYPE arrayType, __in_bcount(memSize1) byte * mem1, __in size_t memSize1,  
        __in_bcount(memSize2) byte * mem2, __in size_t memSize2, MetadataStringId methodNameId, bool isByRef, bool isOut, bool allowExtensions,
        bool hasLength, uint32 lengthValue)
    {
        Assert(CanMarshalType(arrayType->elementType));

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            // Identify Pattern and print the info
            Output::Print(_u("MemoryTrace: Reading Array for: %s\n"), (resourceCleanup == CalleeTransfersOwnership) ? _u("Method Call") : _u("Delegate Invoke"));
            Output::Print(_u("    Array Pattern: %s\n"), isByRef ? _u("ReceiveArray") : isOut ? _u("FillArray") : _u("PassArray"));
            Output::Print(_u("    Considering length attribute: %s\n"), hasLength ? _u("true") : _u("false"));
            Output::Flush();
        }
#endif
        TRACE_METADATA(_u("ReadOutArrayType(): Reading Array for: %s,  Array Pattern: %s, Considering length attribute: %s\n"), 
            (resourceCleanup == CalleeTransfersOwnership) ? _u("Method Call") : _u("Delegate Invoke"), isByRef ? _u("ReceiveArray") : isOut ? _u("FillArray") : _u("PassArray"), hasLength ? _u("true") : _u("false"));

        if (memSize1 < sizeof(uint) || memSize2 < sizeof(LPVOID))
        {
            Js::Throw::FatalProjectionError();
        }

        uint elementCount = *(uint*)mem1;
        byte * arrayContents = *(byte**)mem2;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        uint32 readArrayLength = hasLength ? lengthValue : elementCount;

#if DBG_DUMP
        if (Js::Configuration::Global.flags.TraceWin8Allocations)
        {
            Output::Print(_u("    Read Array of type: %s\n"), StringOfId(arrayType->elementType->fullTypeNameId));
            Output::Print(_u("    Array Length: %u\n"), elementCount);
            Output::Print(_u("    Read array Length: %u\n"), readArrayLength);
            Output::Flush();
        }
#endif
        TRACE_METADATA(_u("ReadOutArrayType(): Read Array of type: %s (#%d), Array Length: %u, Read array Length: %u\n"), 
            StringOfId(arrayType->elementType->fullTypeNameId), arrayType->elementType->fullTypeNameId, elementCount, readArrayLength);

        if (hasLength && readArrayLength > elementCount)
        {
            // the lengthIs attribute paramter has value greater than the size of the array
            Js::JavascriptError::ThrowTypeError(scriptContext, JSERR_IllegalArraySizeAndLength);
        }

        // Length is non zero but array pointer is null
        if (elementCount != 0 && arrayContents == nullptr)
        {
            Js::JavascriptError::MapAndThrowError(scriptContext, E_POINTER);
        }

        if (inOutArgument)
        {
            // It is FillArray Scenario where in we need to reflect the updated values into the passed in array/projection

            Assert(resourceCleanup == CalleeTransfersOwnership); // We can update the values only in method call scenario and not in delegate scenario.

            // In case the passed in object was js Array then we need to reflect the values into the array
            // But if the passed in object was null object then we dont need the update. 
            // If the object was arrayprojection then we had already passed in internal buffer so the updates would already be reflecting
            // unless we had to create duplicate buffer because the types of fillarray differed
#if DBG_DUMP
            Js::TypeId typedId = Js::JavascriptOperators::GetTypeId(inOutArgument);
            if (typedId == Js::TypeIds_Null || typedId == Js::TypeIds_Undefined)
            {
                if (Js::Configuration::Global.flags.TraceWin8Allocations)
                {
                    // Identify Pattern and print the info
                    Output::Print(_u("    Reading data into existing: %s\n"), typedId == Js::TypeIds_Null ? _u("null") : _u("undefined"));
                    Output::Flush();
                }
            }
            else 
#endif
                if (ArrayAsCollection::IsArrayInstance(inOutArgument))
                {
#if DBG_DUMP
                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        // Update the existing array
                        Output::Print(_u("    Reading data into existing: Javascript Array\n"));
                        Output::Print(_u("    Copying %u Elements\n"), readArrayLength);
                        Output::Flush();
                    }
#endif

                    uint uElementTypeSize = (uint)ConcreteType::From(arrayType->elementType)->storageSize;
                    Assert(Js::JavascriptConversion::ToUInt32((double)(uElementTypeSize * elementCount)) >= elementCount);
                    ProjectionMarshaler marshal(CalleeRetainsOwnership, projectionContext, false);
                    for (uint iIndex = 0; iIndex < readArrayLength; iIndex++)
                    {
                        Var varElement = marshal.ReadOutType(nullptr, arrayType->elementType, true, arrayContents, uElementTypeSize, methodNameId);

                        // Set Item  at index
                        Var varIndex = Js::JavascriptNumber::ToVar(iIndex, scriptContext);
                        Js::JavascriptOperators::OP_SetElementI(inOutArgument, varIndex, varElement, scriptContext);
                        arrayContents += uElementTypeSize;
                    }

                    if (readArrayLength < elementCount)
                    {
#if DBG_DUMP
                        if (Js::Configuration::Global.flags.TraceWin8Allocations)
                        {
                            Output::Print(_u("    Clearing buffer after %u index in the array\n"), readArrayLength);
                            Output::Flush();
                        }
#endif
                        memset(arrayContents, 0, uElementTypeSize);
                        Var varElement = marshal.ReadOutType(nullptr, arrayType->elementType, true, arrayContents, uElementTypeSize, methodNameId);

                        // Set the readArrayLength to ElementCount elements to var that would get projected because of zeroed elements
                        for (uint iIndex = readArrayLength; iIndex < elementCount; iIndex++)
                        {
                            Var varIndex = Js::JavascriptNumber::ToVar(iIndex, scriptContext);
                            Js::JavascriptOperators::OP_SetElementI(inOutArgument, varIndex, varElement, scriptContext);
                        }
                    }
                }
#if DBG_DUMP
                else if (Js::TypedArrayBase::Is(inOutArgument))
                {
                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        Output::Print(_u("    Reading data into existing: Typed Array %s\n"), ArrayProjection::TypedArrayName(inOutArgument));
                        Output::Flush();
                    }
                }
                else
                {
                    // Check if we had passed in existing buffer or created duplicate
                    ArrayObjectInstance *pArrayProjectionInstance = ArrayProjection::GetArrayObjectInstance(inOutArgument);
                    Assert(pArrayProjectionInstance != NULL);

                    if (Js::Configuration::Global.flags.TraceWin8Allocations)
                    {
                        Output::Print(_u("    Reading data into existing: ArrayProjection %s\n"), StringOfId(pArrayProjectionInstance->GetPropertyId()));
                        Output::Flush();
                    }
                }
#endif
                return inOutArgument;
        }

        // If 0 elements, return null and not array with 0 elements
        if (elementCount == 0)
        {
#if DBG_DUMP
            if (Js::Configuration::Global.flags.TraceWin8Allocations)
            {
                Output::Print(_u("    Projecting: Null object\n"));
                Output::Flush();
            }
#endif

            if (resourceCleanup == CalleeTransfersOwnership && arrayContents != ZERO_LENGTH_ARRAY)
            {
                // ReceiveArray method call with 0 elements
                CoTaskMemFree(arrayContents);
#if DBG_DUMP
                if (Js::Configuration::Global.flags.TraceWin8Allocations)
                {
                    Output::Print(_u("    Deleting the buffer using: CoTaskMemFree\n"));
                    Output::Flush();
                }
#endif
            }

            return scriptContext->GetLibrary()->GetNull();
        }

        // This is either ReceiveArray method call or Pass/FillArray delegate case we need to project the values.
        // In case of ReceiveArray case we dont need to duplicate the array block as it is already under our ownership
        // But in case of delegates we dont own the buffer so we would need to create the duplicate
        // In case of delegate FillArray we shouldnt be copying contents from the original buffer because we are suppose to filling them while going out instead.
        Var result = nullptr;
        HRESULT hr = ArrayProjection::CreateArrayProjectionObject(ConcreteType::From(arrayType->elementType), projectionContext, arrayContents, elementCount, readArrayLength, (resourceCleanup == CalleeTransfersOwnership), &result, (resourceCleanup == CalleeTransfersOwnership) || isByRef || !isOut);
        IfFailedMapAndThrowHr(scriptContext, hr);
        RecordRecyclerVar(result);

        if (!allowExtensions)
        {
            if (!this->projectionContext->AreProjectionPrototypesConfigurable())
            {
                Js::DynamicObject * resultObject = Js::DynamicObject::FromVar(result);
                BOOL succeeded = resultObject->PreventExtensions();
                Js::VerifyCatastrophic(succeeded);
            }
            else
            {
                TRACE_METADATA(_u("ReadOutArrayType() - design mode, extensions permitted\n"));
            }
        }

        return result;
    }

    // Info:        Read as out type 
    // Parameters:  inOutArgument - the in\out arg is present
    //              type - the out type to read
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     The result var
    Var ProjectionMarshaler::ReadOutType(Var inOutArgument, RtTYPE type, bool structsByValue, __in_bcount(memSize) byte * mem, __in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
#if DBG
        {
            ProjectionModel::AllowHeavyOperation allow;
        }
#endif
        bool wasMissingType = false;
        bool canMarshal = CanMarshalType(type, true, &wasMissingType);
        Assert(canMarshal);

        Var result;
        if (wasMissingType)
        {
            result = projectionContext->GetScriptContext()->GetLibrary()->GetNull();
        }
        else if (TypeDefinitionType::Is(type))
        {
            result = ReadTypeDefinitionTypeOutParameter(TypeDefinitionType::From(type), structsByValue, mem, memSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);
        } 
        else if(ByRefType::Is(type))
        {
            RtBYREFTYPE byRefType = ByRefType::From(type);
            RtCONCRETETYPE concrete = ConcreteType::From(byRefType->pointedTo);
            if (ArrayType::Is(concrete))
            {
                // ReceiveArray Pattern = cannot be inout
                Assert(inOutArgument == nullptr);
                result = ReadOutArrayType(nullptr, ArrayType::From(concrete), *(byte**)mem, sizeof(uint), *(byte**)(mem + sizeof(LPVOID)), sizeof(LPVOID), methodNameId, true, true, allowExtensions);
            }
            else
            {
                result = ReadOutType(inOutArgument, concrete, true, *(byte**)mem,concrete->storageSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);
            }
        } 
        else if (BasicType::Is(type))
        {
            RtBASICTYPE basicType = BasicType::From(type);
            result = ReadOutBasicType(basicType->typeCor, mem, memSize, methodNameId, allowIdentity, allowExtensions);
        } 
        else if (SystemGuidType::Is(type))
        {
            result = ReadOutSystemGuidType(structsByValue, mem, memSize);
        } 
        else if (WindowsFoundationDateTimeType::Is(type))
        {
            result = ReadOutWindowsFoundationDateTimeType(mem, memSize);
        } 
        else if (WindowsFoundationTimeSpanType::Is(type))
        {
            return ReadOutWindowsFoundationTimeSpanType(mem, memSize);
        }
        else if (WindowsFoundationEventRegistrationTokenType::Is(type))
        {
            result = ReadOutBasicType(ELEMENT_TYPE_I8, mem, memSize, methodNameId, allowIdentity);
        }
        else if (WindowsFoundationHResultType::Is(type))
        {
            result = ReadOutBasicType(ELEMENT_TYPE_I4, mem, memSize, methodNameId, allowIdentity);
        }
        else if (ArrayType::Is(type))
        {
            Assert(inOutArgument != nullptr);
            RtARRAYTYPE arrayType = ArrayType::From(type);
            result = ReadOutArrayType(inOutArgument, arrayType, mem, sizeof(uint), mem + sizeof(LPVOID), sizeof(LPVOID), methodNameId, false, true, allowExtensions);
        }
        else
        {
            Js::Throw::FatalProjectionError();
        }

        RecordRecyclerVar(result);
        return result;
    }

    // Info:        Read an out parameter
    // Parameters:  inOutArgument - the in\out arg is present
    //              parameter - the model parameter
    //              mem - pointer to the target memory
    //              memSize - the size of mem in bytes
    //              methodName - the name of the calling method
    // Returns:     The result var
    Var ProjectionMarshaler::ReadOutParameter(Var inOutArgument, RtABIPARAMETER parameter, __in_bcount(memSize) byte * mem,__in size_t memSize, MetadataStringId methodNameId, bool allowIdentity, bool allowExtensions, ConstructorArguments* constructorArguments)
    {
        return ReadOutType(inOutArgument, parameter->type, false, mem, memSize, methodNameId, allowIdentity, allowExtensions, constructorArguments);
    }
}
