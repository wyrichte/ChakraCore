//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "ProjectionPch.h"
#include "IBufferProjection.h"
#include "winrtobjectprobe.h"

namespace Projection
{
    EventHandlingProjectionObjectInstance::EventHandlingProjectionObjectInstance(ProjectionContext *projectionContext, HTYPE htype, IUnknown* nativeABI, IID defaultIID, bool supportsIdentity)
        : ProjectionObjectInstance(projectionContext, htype, nativeABI, defaultIID, supportsIdentity), eventProjectionHandler(nullptr), abiWeakReference(nullptr)
    {
        IWeakReferenceSource *weakReferenceSource = nullptr;
        HRESULT hr = unknown->QueryInterface(&weakReferenceSource);
        if (SUCCEEDED(hr))
        {
            weakReferenceSource->GetWeakReference(&abiWeakReference);
            weakReferenceSource->Release();
        }

        if (!projectionContext->SupportsWeakDelegate())
        {
            supportsRefCountProbe = false;
            isRooted = true;
        }
        else
        {
            hr = CanObjectBeRefCountProbed(GetNativeABI());
            if (SUCCEEDED(hr))
            {
                // Supports ref count probing
                supportsRefCountProbe = true;
                isRooted = false;
            }
            else
            {
                supportsRefCountProbe = false;
                isRooted = true;
            }
        }
    }

    // Disconnect all the event handlers
    void EventHandlingProjectionObjectInstance::DisconnectEventHandlers(bool isFinalize)
    {
        Assert(scriptSite && !scriptSite->IsClosed() && supportsRefCountProbe);
        Assert(projectionContext->SupportsWeakDelegate());
        ProjectionWriter *writer = projectionContext->GetProjectionWriter();
        writer->OnDisposeEventHandlingInstance(this);

        // If we dont have event projection handler that means we arent tracking the events, so dont unregister them
        if (eventProjectionHandler != nullptr)
        {
#ifdef WINRTFINDPREMATURECOLLECTION
            if (!isRooted)
            {
                // Need to make sure we are the only one holding reference
                Assert(SUCCEEDED(DoesRefCountMatch(GetNativeABI(), 2)));
            }
#endif
            eventProjectionHandler->UnRegister(isFinalize ? !isRooted : false);
            eventProjectionHandler = nullptr;
        }
    }

    void EventHandlingProjectionObjectInstance::Finalize(bool isShutdown)
    {
        if (!isShutdown && scriptSite && !scriptSite->IsClosed() && supportsRefCountProbe) 
        {
            DisconnectEventHandlers(true);
        }
    }

    void EventHandlingProjectionObjectInstance::ReleaseNativePointers(bool isShutdown, bool isDispose)
    {
        if (!isShutdown && scriptSite && !scriptSite->IsClosed())
        {
            // If called from msRelease.. method the finalize wasnt called so we need to unregister event handlers
            if (supportsRefCountProbe && !isDispose)
            {
                DisconnectEventHandlers(false);
            }
            ReleasePointer(abiWeakReference);
        }

        ProjectionObjectInstance::ReleaseNativePointers(isShutdown, isDispose);
    }

    void EventHandlingProjectionObjectInstance::Mark(Recycler *recycler)
    {
        if (this->supportsRefCountProbe)
        {
            UpdateRootedState();
        }

        // Since the object is not rooted, then we would need to mark all the event handlers
        // Otherwise the event handlers are already rooted and we dont need to mark them again
        if (!isRooted && scriptSite && !scriptSite->IsClosed())
        {
            Assert(projectionContext->SupportsWeakDelegate());
            Assert(supportsRefCountProbe);

            // Mark all the event handling delegates that this instance has
            if (eventProjectionHandler != nullptr)
            {
                eventProjectionHandler->Mark(recycler);
            }
        }
    }

    void EventHandlingProjectionObjectInstance::TrackRefCount(Recycler *recycler) 
    {
        Assert(projectionContext->SupportsWeakDelegate());
        Assert(supportsRefCountProbe);

        // Check if this needs to be rooted
        Assert(unknown != nullptr);

        // If we dont have eventProjectionHandler see if we have cached one somewhere
        if (eventProjectionHandler == nullptr && abiWeakReference != nullptr)
        {
            eventProjectionHandler = projectionContext->GetProjectionWriter()->GetExistingEventHandlerFromWeakReference(abiWeakReference);
            if (eventProjectionHandler != nullptr)
            {
                eventProjectionHandler->AddGCTrackedRef();
            }
        }

        RecyclerHeapObjectInfo heapObject;
        recycler->FindHeapObjectWithClearedAllocators(this, heapObject);
        if (!heapObject.IsObjectMarked())
        {
            UpdateRootedState();
        }

        if (isRooted)
        {
            // If the object is externally referenced and we dont own all the references to it, then we need to mark the delegates explicitly
            if (eventProjectionHandler && eventProjectionHandler->GetGCTrackedRefCount() == 1)
            {
                eventProjectionHandler->Mark(recycler);
            }
        }
    }

    //We check the rooted state in mark (mark child when it's not externally rooted) and in 
    //ResolveExternalReference (mark child when it's externally rooted). Our state changed 
    //between mark & resolveexternalreference, leaving the eventhandlers not marked in both
    //code path, thus prematurally disconnect the delegates.
    //The change is to actually check if the projection object is rooted in both mark & externalrefcount
    //check time. We won't check in later code path if it's already marked to avoid calling
    //external code unnecessarily.
    void EventHandlingProjectionObjectInstance::UpdateRootedState() 
    {
        Assert(supportsRefCountProbe);

        IUnknown* unknown = GetNativeABI();

        // Need to make sure the unknown is still around before trying to probe it for RefCount.
        // We might have already released the unknown via msReleaseWinRTObject.
        if (!unknown || SUCCEEDED(DoesRefCountMatch(unknown, 2)))
        {
            isRooted = false;
        }
        else
        {
            isRooted = true;
        }
    }

    WEAKPROPERTYBAG *EventHandlingProjectionObjectInstance::GetWeakPropertyBag(bool fCreate)
    {
        Assert(unknown != nullptr);

        if (weakPropertyBag == nullptr)
        {
            if (abiWeakReference != nullptr)
            {
                projectionContext->GetProjectionWriter()->GetWeakPropertyBagFromWeakRef(fCreate, abiWeakReference, &weakPropertyBag);
            }
            else
            {
                // Throw error
                Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), VBSERR_ActionNotSupported);
            }
        }

        return weakPropertyBag;
    }

    ProjectionObjectInstance::ProjectionObjectInstance(ProjectionContext *projectionContext, HTYPE htype, IUnknown* nativeABI, IID defaultIID, bool supportsIdentity) 
        : Js::CustomExternalObject((Js::CustomExternalType *)htype), 
        supportsIdentity(supportsIdentity),
        projectionContext(projectionContext), 
        scriptSite(projectionContext->GetScriptSite()), 
        weakReference(nullptr), 
        unknown(nativeABI),
        defaultIID(defaultIID),
        weakPropertyBag(nullptr)
#ifndef DISABLE_ISTRINGABLE_QI
        ,queriedStringable(false),
        stringable(nullptr)
#endif // !defined(DISABLE_ISTRINGABLE_QI)
    {
        scriptSite->AddRef();
        taggedUnknown = UnknownToTagged(nativeABI);
        Assert(nativeABI != nullptr);
        nativeABI->AddRef();

#if DBG_DUMP
        ULONG ulUnknownRefCount = unknown->AddRef();

        this->projectionMemoryInformation = (ProjectionMemoryInformation*) projectionContext->GetThreadContext()->GetProjectionContextMemoryInformation();
        this->projectionMemoryInformation->AddProjectionObject(this, ulUnknownRefCount);
#else
        unknown->AddRef();
#endif
    }

    void ProjectionObjectInstance::Dispose(bool isShutdown)
    {
        ReleaseNativePointers(isShutdown, true);
    }

#ifndef DISABLE_ISTRINGABLE_QI
    Var ProjectionObjectInstance::ToStringThunk(Var method, Js::CallInfo callInfo, ...)
    {
#if DBG
        auto func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif

        ARGUMENTS(args, callInfo);
        auto function = Js::JavascriptFunction::FromVar(method);
        auto scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(scriptContext->GetThreadContext()->IsScriptActive());   

        ProjectionObjectInstance *instance = GetProjectionObjectInstanceFromVarNoThrow(args[0]);

        if (instance == nullptr)
        {
            return Js::JavascriptObject::ToStringInternal(args[0], scriptContext);
        }

        ProjectionContext *projectionContext = instance->GetProjectionContext();

        IUnknown *unknown = instance->GetUnknown();
        if (unknown == nullptr)
        {
            // Already released
            Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_AlreadyReleasedInspectableObject);
        }

        if (!instance->queriedStringable)
        {
            if (FAILED(unknown->QueryInterface(__uuidof(Windows::Foundation::IStringable), (void **)&instance->stringable)))
            {
                instance->stringable = nullptr;
            }
            instance->queriedStringable = true;
        }

        if (instance->stringable == nullptr)
        {
            return Js::JavascriptObject::ToStringInternal(instance, scriptContext);
        }
        else
        {
            HRESULT hr = S_OK;
            AutoHSTRING objectString(projectionContext->GetThreadContext()->GetWinRTStringLibrary());
            PCWSTR rawString = nullptr;
            UINT32 length = 0;
            HSTRING hstring;

            Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);

            BEGIN_LEAVE_SCRIPT(scriptContext)
            {
                hr = instance->stringable->ToString(&hstring);
            }
            END_LEAVE_SCRIPT(scriptContext)

            if (SUCCEEDED(hr))
            {
                objectString.Initialize(hstring);
                rawString = projectionContext->GetThreadContext()->GetWinRTStringLibrary()->WindowsGetStringRawBuffer(hstring, &length);
            }

            IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

            return Js::JavascriptString::NewCopyBuffer(rawString, length, scriptContext); 
        }
    }
#endif // !defined(DISABLE_ISTRINGABLE_QI)

    void ProjectionObjectInstance::ReleaseNativePointers(bool isShutdown, bool isDispose)
    {
        if (!isShutdown && supportsIdentity && scriptSite && !scriptSite->IsClosed())
        {
            Assert(unknown != nullptr);
            PROJECTIONINSTANCEKEYVALUEPAIR keyValuePair(GetUnknown(), weakReference);
            projectionContext->GetProjectionWriter()->RemoveInspectableFromCache(keyValuePair);
        }

        ReleasePointer(scriptSite);
            
        IUnknown *nativeABI = GetNativeABI();

#ifndef DISABLE_ISTRINGABLE_QI
        if ((stringable != nullptr) && !isShutdown)
        {
            stringable->Release();
            stringable = nullptr;
        }
#endif // !defined(DISABLE_ISTRINGABLE_QI)

#if DBG_DUMP
        if ((nativeABI != nullptr) && !isShutdown)
        {
            nativeABI->Release();
            taggedUnknown = 0;

            Assert(unknown != nullptr);
            ULONG ulUnknownRefCount = unknown->Release();
            unknown = nullptr;

            this->projectionMemoryInformation->DisposeProjectionObject(this, isDispose, ulUnknownRefCount);
        }
        else
        {
            this->projectionMemoryInformation->DisposeProjectionObject(this, isDispose);
        }
#else
        if (nativeABI != nullptr && !isShutdown)
        {
            nativeABI->Release();
            taggedUnknown = 0;

            Assert(unknown != nullptr);
            unknown->Release();
            unknown = nullptr;
        }
#endif
    }

    HRESULT ProjectionObjectInstance::Create(
        __in HTYPE htype,
        __in bool hasEventHandlers,
        __in IUnknown* nativeABI,
        __in IID defaultIID, 
        __in ProjectionContext *projectionContext,
        __out ProjectionObjectInstance** newInstance,
        __in bool supportsIdentity,
        __in INT32 gcPressure)
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        Recycler* recycler = scriptContext->GetRecycler();

        HRESULT hr = S_OK;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT
            {
                if (hasEventHandlers)
                {
                    EventHandlingProjectionObjectInstance* instance = RecyclerNewTracked(recycler, EventHandlingProjectionObjectInstance, projectionContext, htype, nativeABI, defaultIID, supportsIdentity);
                    projectionContext->GetProjectionWriter()->OnNewEventHandlingInstance(instance);
                    *newInstance = instance;
                }
                else
                {
                    ProjectionObjectInstance* instance = RecyclerNewFinalized(recycler, ProjectionObjectInstance, projectionContext, htype, nativeABI, defaultIID, supportsIdentity);
                    *newInstance = instance;
                }
                if(gcPressure >= 0)
                {
                    recycler->AddExternalMemoryUsage(Projection::GetApproximateSizeForGCPressure(gcPressure));
                }
            }
            END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
        }
        END_LEAVE_SCRIPT(scriptContext)

        return hr;
    }

    BOOL ProjectionObjectInstance::Is(Var instance)
    {
        Js::CustomExternalObject* externalObject = Js::JavascriptOperators::TryFromVar<Js::CustomExternalObject>(instance);
        if (!externalObject)
        {
            return FALSE;
        }
        
        Js::ExternalType * externalType = (Js::ExternalType *)externalObject->GetType();
        ProjectionTypeOperations* projectionTypeOperations = NULL;
        if (SUCCEEDED(externalType->GetTypeOperations()->QueryInterface(IID_IProjectionTypeOperations, (void**)&projectionTypeOperations)))
        {
            ProjectionType projectionType = projectionTypeOperations->GetProjectionType();
            projectionTypeOperations->Release();
            return projectionType == InspectableProjectionType;
        }

        return FALSE;
    }

    BOOL ProjectionObjectInstance::IsEqual(
        __in ProjectionObjectInstance *other)
    {
        if (this == other)
        {
            return TRUE;
        }

        if (this->supportsIdentity && other->supportsIdentity && (this->scriptSite == other->scriptSite))
        {
            IUnknown *unknown = this->GetUnknown();
            IUnknown *otherUnknown = other->GetUnknown();

            if (unknown == nullptr || otherUnknown == nullptr)
            {
                // We cannot identify disposed objects
                return FALSE;
            }

            return (unknown == otherUnknown);
        }

        return FALSE;
    }

    // Info:        Get the interface from the native pointer. 
    // Parameters:  iidOfInterface - iid of the interface requested
    //              scriptContext - the script context
    //              isDefaultInterface - out parameter indicating whether the requested interface was the default interface
    //              addRefDefault - boolean indicating whether the default interface should be addRef'd before returning
    // Returns:     The IUnknown of the interface and a boolean indicating whether that interface was the default interface.
    //              If the interface was the default it will be returned with no additional ref-count, unless addRefDefault is true.
    IUnknown * ProjectionObjectInstance::GetInterfaceOfNativeABI(const IID & iidOfInterface, Js::ScriptContext * scriptContext, bool * isDefaultInterface, bool addRefDefault) 
    {
        bool isDefault = (defaultIID != GUID_NULL) && (iidOfInterface == defaultIID);

        if (isDefaultInterface)
        {
            *isDefaultInterface = isDefault;
        }

        HRESULT hr = S_OK;
        IUnknown * pInterface = nullptr;

        if (isDefault)
        {
            // If the requested interface was the default interface of this type,
            // get the default interface pointer and return.
            if (IsNotYetTaggedAsDefaultInterface(taggedUnknown))
            {
                // Hasn't yet been QI-ed for default interface
                auto unknown = GetNativeABI();
                Assert(unknown != nullptr);
                Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    hr = unknown->QueryInterface(iidOfInterface, (void**)&pInterface);
                    if (!addRefDefault && SUCCEEDED(hr))
                    {
                        unknown->Release();
                    }
                }
                END_LEAVE_SCRIPT(scriptContext)

                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                taggedUnknown = (size_t)pInterface;
                Assert(!IsNotYetTaggedAsDefaultInterface(taggedUnknown));
                return pInterface;
            }
            pInterface = TaggedToUnknown(taggedUnknown);
            if (addRefDefault)
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    pInterface->AddRef();
                }
                END_LEAVE_SCRIPT(scriptContext)
            }
            return pInterface;
        }

        // Otherwise, QI the unknown pointer for the requested interface
        // This is a QI so we do not want to mark the callout for debug-stepping (via MarkerForExternalDebugStep).
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = unknown->QueryInterface(iidOfInterface, (LPVOID *)&pInterface);
        }
        END_LEAVE_SCRIPT(scriptContext);

        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
        return pInterface;
    }
    
    HRESULT ProjectionObjectInstance::GetFullHeapObjectInfo(HostProfilerHeapObject** result, HeapObjectInfoReturnResult* returnResult)
    {
        AssertMsg(returnResult != nullptr, "The return result must be supplied.");

        HRESULT hr = S_OK;

        HostProfilerHeapObject **externalObjects = nullptr;
        USHORT externalObjectCount = 1; // we want to allocate 1 external element info
        HostProfilerHeapObject *mainHeapInfo = nullptr;

        // Main HeapObjectInfo
        UINT allocSize = offsetof(HostProfilerHeapObject, optionalInfo) // size till optionalInfo
            + ActiveScriptProfilerHeapEnum::GetHeapObjectInternalPropertyInfoSize(); // InternalPropertySize
        mainHeapInfo = (HostProfilerHeapObject *)CoTaskMemAlloc(allocSize);
        IFNULLMEMGO(mainHeapInfo);
        memset(mainHeapInfo, 0, allocSize);

        allocSize = sizeof(HostProfilerHeapObject *) * externalObjectCount;
        externalObjects = (HostProfilerHeapObject **)CoTaskMemAlloc(allocSize);
        IFNULLMEMGO(externalObjects);
        memset(externalObjects, 0, allocSize);

        // Get number of events
        uint eventCount = this->GetEventAndEventHandlerCount();

        // External Object information
        allocSize = offsetof(HostProfilerHeapObject, optionalInfo) // size till optionalInfo
            + offsetof(ProfilerHeapObjectOptionalInfo, eventList.elements) // size of eventCount
            + (sizeof(PROFILER_HEAP_OBJECT_RELATIONSHIP) * eventCount); // size to hold events
        externalObjects[0] = (HostProfilerHeapObject *)CoTaskMemAlloc(allocSize);
        IFNULLMEMGO(externalObjects[0]);
        memset(externalObjects[0], 0, allocSize);

        // Main HeapObject information
        mainHeapInfo->flags = PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE;
        mainHeapInfo->optionalInfoCount = 1;
        ProfilerHeapObjectOptionalInfo *optionalInfo = (ProfilerHeapObjectOptionalInfo *)((byte *)(mainHeapInfo) + offsetof(HostProfilerHeapObject, optionalInfo));
        ActiveScriptProfilerHeapEnum* heapEnum = reinterpret_cast<ActiveScriptProfilerHeapEnum*>(this->GetScriptContext()->GetHeapEnum());
        heapEnum->FillHeapObjectInternalUnnamedExternalProperty(optionalInfo, (PROFILER_EXTERNAL_OBJECT_ADDRESS)this->GetUnknown());
        mainHeapInfo->externalObjectCount = externalObjectCount;
        mainHeapInfo->externalObjects = externalObjects;

        // External object information for native pointer
        externalObjects[0]->externalAddress = this->GetUnknown();
        externalObjects[0]->typeNameId = this->GetNameId();
        externalObjects[0]->flags = PROFILER_HEAP_OBJECT_FLAGS_NEW_STATE_UNAVAILABLE | PROFILER_HEAP_OBJECT_FLAGS_EXTERNAL_UNKNOWN | PROFILER_HEAP_OBJECT_FLAGS_WINRT_INSTANCE;
        externalObjects[0]->optionalInfoCount = 1;

        HTYPE hType;
        RuntimeClassTypeInformation *typeInformation = NULL;
#if DBG
        bool foundTypeInfo = 
#endif
        this->projectionContext->GetProjectionWriter()->GetRuntimeClassTypeInformation(externalObjects[0]->typeNameId, &hType, &typeInformation); 
        Assert(foundTypeInfo);
        Assert(((Js::CustomExternalType *)hType)->GetNameId() == this->GetTypeNameId());
        externalObjects[0]->size = (UINT)Projection::GetApproximateSizeForGCPressure(typeInformation->GCPressure());
        if (externalObjects[0]->size == 0)
        {
            externalObjects[0]->flags = externalObjects[0]->flags | PROFILER_HEAP_OBJECT_FLAGS_SIZE_UNAVAILABLE;
        }
        else
        {
            externalObjects[0]->flags = externalObjects[0]->flags | PROFILER_HEAP_OBJECT_FLAGS_SIZE_APPROXIMATE;
        }

        // Event information optionalInfo
        optionalInfo = (ProfilerHeapObjectOptionalInfo *)((byte *)(externalObjects[0]) + offsetof(HostProfilerHeapObject, optionalInfo));
        optionalInfo->infoType = PROFILER_HEAP_OBJECT_OPTIONAL_INFO_WINRTEVENTS;

        this->PopulateProfilerEventInfo(heapEnum, &(optionalInfo->eventList));

        *result = mainHeapInfo;
        *returnResult = HeapObjectInfoReturnResult_Success;
        return S_OK;

LReturn:
        if (externalObjects)
        {
            for (UINT i=0; i < externalObjectCount; i++)
            {
                if (externalObjects[i]) 
                {
                    CoTaskMemFree(externalObjects[i]);
                }
            }
            CoTaskMemFree(externalObjects);
        }
        if (mainHeapInfo != nullptr)
        {
            CoTaskMemFree(mainHeapInfo);
        }

        *returnResult = FAILED(hr) ? HeapObjectInfoReturnResult_NoResult : HeapObjectInfoReturnResult_Success;
        return hr;
    }

    WEAKPROPERTYBAG *ProjectionObjectInstance::GetWeakPropertyBag(bool fCreate)
    {
        Assert(unknown != nullptr);

        if (weakPropertyBag == nullptr)
        {
            bool fWeakReferencedObject = projectionContext->GetProjectionWriter()->GetWeakPropertyBagFromUnknown(fCreate, GetUnknown(), &weakPropertyBag);
            if (!fWeakReferencedObject)
            {
                // Throw Error
                Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), VBSERR_ActionNotSupported);
            }
        }

        return weakPropertyBag;
    }

    // Inside Lib we can only tell if an object is a equal or lower to ExternalObject on the class heirarchy.
    HRESULT ProjectionObjectInstance::ArrayBufferFromExternalObjectDispatch(__in RecyclableObject *obj,
                                                                            __out Js::ArrayBuffer **ppArrayBuffer)
    {
        Assert(obj != nullptr && obj->IsExternal() && ppArrayBuffer != nullptr);
        HRESULT hr = S_OK;

        IfFailGo(Is(obj) ? S_OK : E_FAIL);

        Projection::ProjectionObjectInstance *prObj = static_cast<Projection::ProjectionObjectInstance *>(obj);
        OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("Projection Instance found; attempting to source ArrayBuffer from IBuffer\n"));
        hr = AttemptCreateArrayBufferFromIBuffer(prObj, ppArrayBuffer);
        IfFailGo(hr);

        OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("ArrayBuffer from IBuffer succeeded with HR=0x%08X\n"), hr);
        goto Return;

    Error:
        OUTPUT_TRACE(Js::ProjectionMetadataPhase, _u("ArrayBuffer from IBuffer failed with HR=0x%08X\n"), hr);
        *ppArrayBuffer = nullptr;

    Return:
         return hr;
    }
}
