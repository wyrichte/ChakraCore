//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "ProjectionPch.h"

// Description: Implementation for the ABI events

namespace Projection
{
    void NamedEventRegistration::ResetDelegate()
    {
        if (m_weakDelegate)
        {
            auto unknownImpl = m_weakDelegate->ResolveUnknownImpl();
            if (unknownImpl)
            {
                Assert(unknownImpl->GetWinrtTypeFlags() == PROFILER_HEAP_OBJECT_FLAGS_WINRT_DELEGATE);
                ((Delegate *)unknownImpl)->SetEventProjectionHandler(nullptr);
            }

            m_weakDelegate->Release();
            m_weakDelegate = nullptr;
        }
    }

    EventProjectionHandler * ProjectionObjectInstance::GetEventProjectionHandler()
    {
        Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), VBSERR_TypeMismatch);
    }

    EventProjectionHandler * EventHandlingProjectionObjectInstance::GetEventProjectionHandler()
    {
        if (unknown == nullptr)
        {
            return nullptr;
        }

        if (eventProjectionHandler == nullptr)
        {
            Assert(unknown != nullptr);
            if (abiWeakReference != nullptr)
            {
                eventProjectionHandler = projectionContext->GetProjectionWriter()->GetEventHandlerFromWeakReference(this->GetNameId(), abiWeakReference);
            }
            else
            {
                eventProjectionHandler = RecyclerNew(projectionContext->GetScriptContext()->GetRecycler(), EventProjectionHandler, this->GetNameId());
            }

            if (supportsRefCountProbe)
            {
                eventProjectionHandler->AddGCTrackedRef();
            }
        }

        return eventProjectionHandler;
    }

    uint GetEventAndEventHandlerCount(EventProjectionHandler *eventProjectionHandler)
    {
        NamedEventRegistrationList *events = (eventProjectionHandler != nullptr) ? eventProjectionHandler->GetExistingEvents() : nullptr;
        NamedEventRegistrationList *eventHandlers = (eventProjectionHandler != nullptr) ? eventProjectionHandler->GetExistingEventHandlers() : nullptr;
        uint eventCount = ((events != nullptr) ? events->Count() : 0) + ((eventHandlers != nullptr) ? eventHandlers->Count() : 0);
        return eventCount;
    }

    uint EventHandlingProjectionObjectInstance::GetEventAndEventHandlerCount() 
    {
        return Projection::GetEventAndEventHandlerCount(eventProjectionHandler);
    }

    uint RuntimeClassThis::GetEventAndEventHandlerCount() 
    {
        if (!CanHoldEventCookies())
        {
            return 0;
        }
        return Projection::GetEventAndEventHandlerCount(&eventProjectionHandler);
    }

    void PopulateProfilerEventInfo(
        ActiveScriptProfilerHeapEnum* heapEnum,
        EventProjectionHandler *eventProjectionHandler,
        PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST *eventList)
    {
        NamedEventRegistrationList *events = (eventProjectionHandler != nullptr) ? eventProjectionHandler->GetExistingEvents() : nullptr;
        NamedEventRegistrationList *eventHandlers = (eventProjectionHandler != nullptr) ? eventProjectionHandler->GetExistingEventHandlers() : nullptr;
#if DBG
        uint eventCount = ((events != nullptr) ? events->Count() : 0) + ((eventHandlers != nullptr) ? eventHandlers->Count() : 0);
#endif
        uint eventIndex = 0;

        auto populateFromEventRegistrationList = [&] (NamedEventRegistrationList *eventRegistrationList) {
            if (eventRegistrationList != nullptr)
            {
                NamedEventRegistrationList::Iterator iterEvents(eventRegistrationList);
                while(iterEvents.Next())
                {
                    Assert(eventIndex < eventCount);
                    NamedEventRegistration * current = iterEvents.Data();
                    IUnknown *eventUnknown = current->GetUnknown();
                    if (eventUnknown != nullptr)
                    {
                        heapEnum->SetRelationshipInfo(eventList->elements[eventIndex], PROFILER_PROPERTY_TYPE_EXTERNAL_OBJECT);
                        eventList->elements[eventIndex].externalObjectAddress = (PROFILER_EXTERNAL_OBJECT_ADDRESS)eventUnknown;
                        eventList->elements[eventIndex].relationshipId = current->GetNameId();
                        eventIndex++;
                    }
                }
            }
        };

        // Populate events added using addEventListener
        populateFromEventRegistrationList(events);

        // Populate events added using oneventname
        populateFromEventRegistrationList(eventHandlers);

        Assert(eventIndex <= eventCount);
        eventList->count = eventIndex;
    }

    void EventHandlingProjectionObjectInstance::PopulateProfilerEventInfo(ActiveScriptProfilerHeapEnum* heapEnum, PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST *eventList)
    {
        return Projection::PopulateProfilerEventInfo(heapEnum, eventProjectionHandler, eventList);
    }

    void RuntimeClassThis::PopulateProfilerEventInfo(ActiveScriptProfilerHeapEnum* heapEnum, PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST *eventList) 
    { 
        if (!CanHoldEventCookies())
        {
            eventList->count = 0; 
            return;
        }
        return Projection::PopulateProfilerEventInfo(heapEnum, &eventProjectionHandler, eventList);
    }

    NamedEventRegistrationList * EventProjectionHandler::GetEvents(Recycler * recycler)
    {
        if (events == nullptr)
        {
            events = RecyclerNew(recycler, NamedEventRegistrationList, recycler);
        }

        return events;
    }

    NamedEventRegistrationList * EventProjectionHandler::GetEventHandlers(Recycler * recycler)
    {
        if (eventHandlers == nullptr)
        {
            eventHandlers = RecyclerNew(recycler, NamedEventRegistrationList, recycler);
        }

        return eventHandlers;
    }

    // Info:        Call the underlying add_eventName method.
    // Parameters:  addMethod - add_eventname methods information from winmd
    //              inspectable - inspectable ptr on which add_eventName method to be called
    //              eventHandler - the delegate pointer for the eventhandler which is a wrapper around js function that needs to be set as event handler
    //              scriptContext - script context
    //              isDefaultInterface - inspectable does not need to be released if it is the default interface pointer
    // Returns:     on success event cookie corresponding to the event handler that is added
    __int64 AddEventHandlerCore(RtABIMETHODSIGNATURE addMethod, IInspectable *inspectable, IUnknown *eventHandler, Js::ScriptContext *scriptContext, bool isDefaultInterface)
    {
        Assert(inspectable != nullptr);
        Assert(eventHandler != nullptr);

        Assert(scriptContext->GetThreadContext()->IsScriptActive());

        typedef HRESULT (STDMETHODCALLTYPE *AddEventType)(IInspectable* thisPtr, IUnknown *eventPtr, __int64 *eventCookie);
        AddEventType pfnAddEventListener = (AddEventType)(*((LPVOID**)inspectable))[addMethod->vtableIndex+6];
        VerifyDeprecatedAttributeOnce(addMethod, scriptContext, DeprecatedInvocation_Event);
        
        HRESULT hr = S_OK;
        __int64 eventCookie;
        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            MarkerForExternalDebugStep();
            hr = pfnAddEventListener(inspectable, eventHandler, &eventCookie);

            if (!isDefaultInterface)
            {
                inspectable->Release();
            }
        }
        END_LEAVE_SCRIPT(scriptContext)

        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);

        return eventCookie;
    }

    // Info:        Marshal jsfunction into a delegate that can be invoked as event handler and call the underlying add_eventName method.
    // Parameters:  eventInfo - eventInfo from winmd
    //              inspectable - inspectable ptr on which add_eventName method to be called
    //              jsEventFunction - js function that needs to be set as event handler
    //              methodName - incase of any errors methodname to use for error reporting
    //              projectionContext - projection context
    //              isDefaultInterface - inspectable does not need to be released if it is the default interface pointer
    //              weakDelegate - ptr to the weakReference of the delegate
    //              pDelegate - ptr to the strong reference of the delegate
    // Returns:     on success event cookie corresponding to the event handler that is added
    __int64 AddEventHandler(RtEVENT eventInfo, IInspectable *inspectable, Js::JavascriptFunction *jsEventFunction, 
        LPCWSTR methodName, ProjectionContext *projectionContext, bool isDefaultInterface, 
        CComPtr<CExternalWeakReferenceImpl> &weakDelegate, CComPtr<IUnknown> &pDelegate)
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        RtABIMETHODSIGNATURE addMethod = AbiMethodSignature::From(eventInfo->addOn);
#if DBG
        ProjectionModel::AllowHeavyOperation allow;
#endif
        auto eventHandlerParam  = addMethod->GetParameters()->allParameters->First()->type;
        if (!eventHandlerParam->CanMarshal(projectionContext->GetProjectionBuilder()))
        {
            if(!isDefaultInterface)
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    inspectable->Release();
                }
                END_LEAVE_SCRIPT(scriptContext)
            }
            Js::JavascriptError::ThrowTypeError(projectionContext->GetScriptContext(), JSERR_InvalidFunctionSignature, methodName);
        }

        auto typeDefinitionType = TypeDefinitionType::From(eventHandlerParam);
        RtEXPR expr = nullptr;
        HRESULT hr = projectionContext->GetExpr(typeDefinitionType->typeId, typeDefinitionType->typeDef->id, nullptr, typeDefinitionType->genericParameters, &expr);
        if (FAILED(hr))
        {
            if(!isDefaultInterface)
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    inspectable->Release();
                }
                END_LEAVE_SCRIPT(scriptContext)
            }
            Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, hr);
        }

        RtDELEGATECONSTRUCTOR delegateConstructor = DelegateConstructor::From(expr);
        RtRUNTIMEINTERFACECONSTRUCTOR invokeInterface = RuntimeInterfaceConstructor::From(delegateConstructor->invokeInterface);
        RtPROPERTY invokeProperty = invokeInterface->prototype->fields->First();
        RtABIMETHODPROPERTY invokeAbiProperty = AbiMethodProperty::From(invokeProperty);

        Delegate* delegateObject = NULL;
        hr = Delegate::Create(projectionContext, StringOfId(scriptContext, typeDefinitionType->typeId), invokeAbiProperty->body->signature, jsEventFunction, eventInfo, false, &delegateObject);
        if (FAILED(hr))
        {
            if(!isDefaultInterface)
            {
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    inspectable->Release();
                }
                END_LEAVE_SCRIPT(scriptContext)
            }
            Js::JavascriptErrorDebug::MapAndThrowError(scriptContext, hr);
        }

        pDelegate.p = delegateObject->GetUnknown();
        weakDelegate.p = delegateObject->GetPrivateWeakReference();

        return AddEventHandlerCore(addMethod, inspectable, pDelegate, scriptContext, isDefaultInterface);
    }

    // Info:        Call the underlying remove_eventName method when script is not active.
    //              If failed, release the inspectable pointer.
    //              If succeeded, inspectable is released only if specified. (This helps in using same inspectable to set new event handler
    // Parameters:  removeMethod - remove_eventname method's information from winmd
    //              inspectable - inspectable ptr on which remove_eventName method to be called
    //              registration - registration to remove
    //              fReleaseInspectableOnSuccess - whether to release the inspectable on success
    //              isDefaultInterface - inspectable does not need to be released if it is the default interface pointer
    HRESULT RemoveEventHandlerCore(RtABIMETHODSIGNATURE removeOn, IInspectable *inspectable, NamedEventRegistration * registration, bool fReleaseInspectableOnSuccess, bool isDefaultInterface, Js::ScriptContext *scriptContext)
    {
        typedef HRESULT (STDMETHODCALLTYPE *RemoveEventType)(IInspectable* thisPtr, __int64 eventCookie);
        RemoveEventType pfnRemoveListener =  (RemoveEventType)(*((LPVOID**)inspectable))[removeOn->vtableIndex+6];;
        auto eventCookie = registration->GetEventCookie();
        HRESULT hr;

        Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);

        MarkerForExternalDebugStep();
        hr = pfnRemoveListener(inspectable, eventCookie);
        if (!isDefaultInterface && (fReleaseInspectableOnSuccess || FAILED(hr)))
        {
            inspectable->Release();
        }
        if (SUCCEEDED(hr))
        {
            registration->ResetDelegate();
        }
        return hr;
    }

    // Info:        Call the underlying remove_eventName method when script is active.
    //              If failed, release the inspectable pointer.
    //              If succeeded, inspectable is released only if specified. (This helps in using same inspectable to set new event handler
    // Parameters:  removeMethod - remove_eventname method's information from winmd
    //              inspectable - inspectable ptr on which remove_eventName method to be called
    //              registration - registration to remove
    //              scriptContext - script context
    //              fReleaseInspectableOnSuccess - whether to release the inspectable on success
    //              isDefaultInterface - inspectable does not need to be released if it is the default interface pointer
    void RemoveEventHandlerFromScript(RtABIMETHODSIGNATURE removeMethod, IInspectable *inspectable, NamedEventRegistration * registration, Js::ScriptContext *scriptContext, bool fReleaseInspectableOnSuccess, bool isDefaultInterface)
    {
        Assert(inspectable != nullptr);
        Assert(scriptContext->GetThreadContext()->IsScriptActive());

        HRESULT hr = S_OK;
        VerifyDeprecatedAttributeOnce(removeMethod, scriptContext, DeprecatedInvocation_Event);
        
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            hr = RemoveEventHandlerCore(removeMethod, inspectable, registration, fReleaseInspectableOnSuccess, isDefaultInterface, scriptContext);
        }
        END_LEAVE_SCRIPT(scriptContext)
        IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
    }

    // Info:        Query for the inspectable and call the underlying remove_eventName method when script not active
    // Parameters:  unknown - IUnknown ptr on which remove_eventName method to be called
    //              removeOn - remove_eventname method's information from winmd
    //              registration - registration to remove
    //              scriptContext - script context
    HRESULT RemoveEventHandler(IUnknown *unknown, RtABIMETHODSIGNATURE removeOn, NamedEventRegistration * registration, Js::ScriptContext *scriptContext)
    {
        Assert(unknown != NULL);
        Assert(!scriptContext->GetThreadContext()->IsScriptActive());

        IInspectable *inspectable;
        HRESULT hr = unknown->QueryInterface(removeOn->iid->instantiated, (LPVOID *)&inspectable);
        Assert(SUCCEEDED(hr));
        IfFailedReturn(hr);
        
        VerifyDeprecatedAttributeOnce(removeOn, scriptContext, DeprecatedInvocation_Event);
        return RemoveEventHandlerCore(removeOn, inspectable, registration, true, false, scriptContext);
    }

    // Remove all events and event handlers from the Runtimeclass instance or projection object instance
    void EventProjectionHandler::RemoveAllEventsAndEventHandlers(IUnknown *unknown, Js::ScriptContext *scriptContext)
    {
        auto removeAllFromEventRegistrationList = [&] (NamedEventRegistrationList *&eventList) {
            if (eventList != nullptr)
            {
                NamedEventRegistrationList::EditingIterator iterEvents(eventList);
                while(iterEvents.Next())
                {
                    NamedEventRegistration * current = iterEvents.Data();
                    if (current->GetDelegate() != nullptr)
                    {
                        RemoveEventHandler(unknown, AbiMethodSignature::From(current->GetEvent()->removeOn), current, scriptContext);
                    }
                    iterEvents.RemoveCurrent();
                }
    
                eventList = nullptr;
            }
        };

        // For each events
        removeAllFromEventRegistrationList(events);

        // For each event handler
        removeAllFromEventRegistrationList(eventHandlers);
    }

    // Info:        Get the registration token for given eventName and function pair
    // Parameters:  events - event list to loop over
    //              eventNameId - event name
    //              jsFunc - javascript function that is event handler
    //              fRemoveExistingListener - if true remove the event listener from the events list if found
    NamedEventRegistration * GetExistingNamedEventRegistration(NamedEventRegistrationList * events, PropertyId eventNameId, Var jsFunc, bool fRemoveExistingListener)
    {
        // Loop over recorded events
        NamedEventRegistrationList::EditingIterator iter(events);
        while(iter.Next())
        {
            NamedEventRegistration * current = iter.Data();
            if(current->GetNameId() == eventNameId)
            {
                Delegate *eventHandler = current->GetDelegate();
                if (eventHandler && eventHandler->GetCallback() == jsFunc)
                {
                    // Remove the event registration from the list
                    if (fRemoveExistingListener)
                    {
                        iter.RemoveCurrent();
                    }

                    return current;
                }
            }
        }

        return nullptr;
    }

    // Info:        Get the registration token for given eventName eventhandler
    // Parameters:  eventHandlers - event handlers list to loop over
    //              eventNameId - event name
    NamedEventRegistration * GetExistingNamedEventRegistration(NamedEventRegistrationList * eventHandlers, PropertyId eventNameId)
    {
        // Loop over recorded events
        NamedEventRegistration * registration = nullptr;
        NamedEventRegistrationList::EditingIterator iter(eventHandlers);
        while(iter.Next())
        {
            NamedEventRegistration * current = iter.Data();
            if(current->GetNameId() == eventNameId)
            {
                return current;
            }
        }

        return registration;
    }

    // Get the eventProjectionHandler from the interface or runtimeclass
    EventProjectionHandler *GetEventProjectionHandlerFromThisInfo(__in Js::ScriptContext *scriptContext, __in ThisInfo * thisInfo, __in Js::Var instance)
    {
        Assert(thisInfo->CanHoldEventCookies());

        switch(thisInfo->thisType)
        {
        case thisUnknownEventHandling:
            {
                ProjectionObjectInstance * projectionObjectInstance = GetProjectionObjectInstanceFromVar(scriptContext, instance);
                return projectionObjectInstance->GetEventProjectionHandler();
            }
        case thisRuntimeClass:
            {
                RuntimeClassThis * runtimeClassThis = reinterpret_cast<RuntimeClassThis*>(thisInfo);
                return runtimeClassThis->GetEventProjectionHandler();
            }
        }
        Js::Throw::FatalProjectionError();
    }
    
    // Info:        Adds the eventProjectionHandler * to the delegate so we could track these delegates connection through JSProxy for the winrt object
    // Parameters:  thisInfo - this Info
    //              thisInstance - instance on which the event Handlers are present
    //              weakDelegate - weak Reference to delegate
    //              eventProjectionHandler - event projection handler that this event handling delegate is added to
    void AddEventProjectionHandler(ThisInfo * thisInfo, Var thisInstance, CExternalWeakReferenceImpl *weakDelegate, EventProjectionHandler *eventProjectionHandler)
    {
        Assert(thisInfo->CanHoldEventCookies());
        if (thisInfo->thisType == thisUnknownEventHandling)
        {
            Assert(ProjectionObjectInstance::Is(thisInstance));
            if (((EventHandlingProjectionObjectInstance *)thisInstance)->SupportsRefCountProbe())
            {
                Assert(((EventHandlingProjectionObjectInstance *)thisInstance)->GetEventProjectionHandler() == eventProjectionHandler);
                Assert(weakDelegate->GetUnknownImpl()->GetWinrtTypeFlags() == PROFILER_HEAP_OBJECT_FLAGS_WINRT_DELEGATE);

                ((Delegate *)(weakDelegate->GetUnknownImpl()))->SetEventProjectionHandler(eventProjectionHandler);
            }
        }
    }
    
    // Info:        Get the inspectable pointer for calling add or remove method for events
    // Parameters:  thisInfo - this info
    //              rtEventMethod - event method inforamtion from winmd
    //              _this - this pointer passed to the method
    //              projectionContext - projection context
    IInspectable * GetInspectableForAddOrRemoveEvent(ThisInfo * thisInfo, RtABIMETHODSIGNATURE rtEventMethod, Var _this, ProjectionContext *projectionContext, bool * isDefaultInterface)
    {
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();

        switch(thisInfo->thisType)
        {
        case thisRuntimeClass:
            {
                RuntimeClassThis * runtimeClassThis = reinterpret_cast<RuntimeClassThis*>(thisInfo);
                IInspectable * factory;
                HRESULT hr = S_OK;

                if (runtimeClassThis->factory == nullptr)
                {
                    FastPathPopulateRuntimeClassThis(runtimeClassThis, projectionContext);
                }
                
                // This is a QI so we do not want to mark the callout for debug-stepping (via MarkerForExternalDebugStep).
                Js::JavascriptErrorDebug::ClearErrorInfo(scriptContext);
                BEGIN_LEAVE_SCRIPT(scriptContext)
                {
                    // InvokeUnknown takes ownership of this unknown and would release it.
                    hr = runtimeClassThis->factory->QueryInterface(rtEventMethod->iid->instantiated, (void **)&factory);
                }
                END_LEAVE_SCRIPT(scriptContext)

                IfFailedMapAndThrowHrWithInfo(scriptContext, hr);
                *isDefaultInterface = false;

                return factory;
            }
        case thisUnknownEventHandling:
            {
                // At this point we have done all checks on this object that is we know that it is ProjectionObjectInstance as well as GetNamtiveABI() != nullptr
                ProjectionObjectInstance * thisObject = (ProjectionObjectInstance *)_this;
                return (IInspectable *)(thisObject->GetInterfaceOfNativeABI(rtEventMethod->iid->instantiated, projectionContext->GetScriptContext(), isDefaultInterface));
            }
        }

        Js::Throw::FatalProjectionError();
    }

    // Info:        Called to add an event listenter
    // Parameters:  standard thunk parameters
    Var AddEventListenerThunk(Var method, Js::CallInfo callInfo, ...)
    {

#if DBG
        Js::JavascriptFunction* func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);

        EventsSignature * signature = reinterpret_cast<EventsSignature*>(function->GetSignature());
        ProjectionContext *projectionContext = signature->projectionContext;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        ThisInfo * thisInfo = signature->thisInfo;
        Assert(thisInfo->CanHoldEventCookies());

        // in + this
        if (size_t(args.Info.Count) < 3)
        {
            Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, _u("addEventListener"));
        }

        if (Js::RecyclableObject::Is(args[2]) && ((Js::RecyclableObject *)args[2])->GetTypeId() == Js::TypeIds_Function)
        {
            // Get the matching event info
            LPCWSTR eventName = Js::JavascriptConversion::ToString(args[1], scriptContext)->GetSz();
            MetadataStringId eventNameId = IdOfString(scriptContext, eventName);
            Option<RtEVENT> eventInfoOpt = signature->events->WhereFirst([&](RtEVENT evnt) {
                return (evnt->nameId == eventNameId);
            });


            // We dont throw errors on addEventListener
            if (eventInfoOpt.HasValue())
            {
                RtEVENT eventInfo = (*eventInfoOpt.GetValue());
                RtMETHODSIGNATURE methodSignature = eventInfo->addOn;
                RtABIMETHODSIGNATURE rtmethod = AbiMethodSignature::From(methodSignature);

                if (projectionContext->GetProjectionWriter()->CanResolveParamaters(methodSignature, false))
                {
#ifdef ENABLE_JS_ETW
                    if (EventEnabledJSCRIPT_PROJECTION_ADDEVENTLISTENER_START())
                    {
                        LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                        LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                        EventWriteJSCRIPT_PROJECTION_ADDEVENTLISTENER_START(runtimeClassName, methodName);
                    }
#endif
                    Recycler *recycler = scriptContext->GetRecycler();
                    EventProjectionHandler *eventProjectionHandler = GetEventProjectionHandlerFromThisInfo(scriptContext, thisInfo, args[0]);
                    if (eventProjectionHandler == nullptr)
                    {
                        Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject, _u("addEventListener"));
                    }

                    NamedEventRegistrationList * events = eventProjectionHandler->GetEvents(recycler);

                    // Loop over recorded events
                    NamedEventRegistration * registration = GetExistingNamedEventRegistration(events, eventNameId, args[2], false);

                    // Register event only if not already registered
                    if (registration == nullptr)
                    {
                        bool isDefaultInterface = false;
                        IInspectable *inspectable = GetInspectableForAddOrRemoveEvent(thisInfo, rtmethod, args[0], projectionContext, &isDefaultInterface);
                        CComPtr<CExternalWeakReferenceImpl> weakDelegate;
                        CComPtr<IUnknown> pDelegate;
                        __int64 eventCookie = AddEventHandler(eventInfo, inspectable, Js::JavascriptFunction::FromVar(args[2]), 
                            _u("addEventListener"), projectionContext, isDefaultInterface, weakDelegate, pDelegate);

                        // Now, add the cookie to the list
                        NamedEventRegistration * namedEvent = RecyclerNewFinalized(recycler, NamedEventRegistration, eventInfo, eventNameId, weakDelegate, eventCookie);
                        events->Prepend(namedEvent);
                        AddEventProjectionHandler(thisInfo, args[0], weakDelegate, eventProjectionHandler);
                    }

#ifdef ENABLE_JS_ETW
                    if (EventEnabledJSCRIPT_PROJECTION_ADDEVENTLISTENER_STOP())
                    {
                        LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                        LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                        EventWriteJSCRIPT_PROJECTION_ADDEVENTLISTENER_STOP(runtimeClassName, methodName);
                    }
#endif
                }
            }
        }
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // Info:        Called to remove an event listenter
    // Parameters:  standard thunk parameters
    Var RemoveEventListenerThunk(Var method, Js::CallInfo callInfo, ...)
    {
#if DBG
        Js::JavascriptFunction* func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);

        EventsSignature * signature = reinterpret_cast<EventsSignature*>(function->GetSignature());
        ProjectionContext *projectionContext = signature->projectionContext;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        ThisInfo * thisInfo = signature->thisInfo;
        Assert(thisInfo->CanHoldEventCookies());

        // in + this
        if (size_t(args.Info.Count) < 3)
        {
            Js::JavascriptError::ThrowError(scriptContext, JSERR_WinRTFunction_TooFewArguments, _u("removeEventListener"));
        }

        if (Js::RecyclableObject::Is(args[2]) && ((Js::RecyclableObject *)args[2])->GetTypeId() == Js::TypeIds_Function)
        {
            // Get the matching event info
            LPCWSTR eventName = Js::JavascriptConversion::ToString(args[1], scriptContext)->GetSz();
            MetadataStringId eventNameId = IdOfString(scriptContext, eventName);
            Option<RtEVENT> eventInfoOpt = signature->events->WhereFirst([&](RtEVENT evnt) {
                return (evnt->nameId == eventNameId);
            });

            if (eventInfoOpt.HasValue())
            {
                RtEVENT eventInfo = (*eventInfoOpt.GetValue());
                RtMETHODSIGNATURE methodSignature = eventInfo->removeOn;
                RtABIMETHODSIGNATURE rtmethod = AbiMethodSignature::From(methodSignature);
                if (projectionContext->GetProjectionWriter()->CanResolveParamaters(methodSignature, false))
                {
#ifdef ENABLE_JS_ETW
                    if (EventEnabledJSCRIPT_PROJECTION_REMOVEEVENTLISTENER_START())
                    {
                        LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                        LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                        EventWriteJSCRIPT_PROJECTION_REMOVEEVENTLISTENER_START(runtimeClassName, methodName);
                    }
#endif

                    EventProjectionHandler *eventProjectionHandler = GetEventProjectionHandlerFromThisInfo(scriptContext, thisInfo, args[0]);
                    if (eventProjectionHandler == nullptr)
                    {
                        Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject, _u("removeEventListener"));
                    }

                    NamedEventRegistrationList * events = eventProjectionHandler->GetEvents(scriptContext->GetRecycler());

                    // Loop over recorded events
                    NamedEventRegistration * registration = GetExistingNamedEventRegistration(events, eventNameId, args[2], true);

                    // If the event was registered remove it
                    if (registration != nullptr)
                    {
                        bool isDefaultInterface = false;
                        IInspectable *inspectable = GetInspectableForAddOrRemoveEvent(thisInfo, rtmethod, args[0], projectionContext, &isDefaultInterface);
                        RemoveEventHandlerFromScript(rtmethod, inspectable, registration, scriptContext, true, isDefaultInterface);
                    }

#ifdef ENABLE_JS_ETW
                    if (EventEnabledJSCRIPT_PROJECTION_REMOVEEVENTLISTENER_STOP())
                    {
                        LPCWSTR runtimeClassName = StringOfId(scriptContext, rtmethod->runtimeClassNameId);
                        LPCWSTR methodName = StringOfId(scriptContext, rtmethod->nameId);
                        EventWriteJSCRIPT_PROJECTION_REMOVEEVENTLISTENER_STOP(runtimeClassName, methodName);
                    }
#endif
                }
            }
        }
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // Info:        Called to set the event Handler
    // Parameters:  standard thunk parameters
    Var SetEventHandlerThunk(Var method, Js::CallInfo callInfo, ...)
    {

#if DBG
        Js::JavascriptFunction* func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);

        EventHandlerSignature * signature = reinterpret_cast<EventHandlerSignature*>(function->GetSignature());
        ProjectionContext *projectionContext = signature->projectionContext;
        Js::ScriptContext *scriptContext = projectionContext->GetScriptContext();
        ThisInfo * thisInfo = signature->thisInfo;
        Assert(thisInfo->CanHoldEventCookies());

        // in + this
        if (size_t(args.Info.Count) < 2)
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);
        }

        RtMETHODSIGNATURE methodSignature = signature->abiEvent->addOn;
        RtABIMETHODSIGNATURE addMethod = AbiMethodSignature::From(methodSignature);

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_SETEVENTHANDLER_START())
        {
            LPCWSTR runtimeClassName = StringOfId(scriptContext, addMethod->runtimeClassNameId);
            LPCWSTR eventName = StringOfId(scriptContext, signature->abiEvent->nameId);
            EventWriteJSCRIPT_PROJECTION_SETEVENTHANDLER_START(runtimeClassName, eventName);
        }
#endif

        // Get the existing event
        Recycler *recycler = scriptContext->GetRecycler();

        EventProjectionHandler *eventProjectionHandler = GetEventProjectionHandlerFromThisInfo(scriptContext, thisInfo, args[0]);
        if (eventProjectionHandler == nullptr)
        {
            Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject, StringOfId(scriptContext, signature->eventPropertyNameId));
        }

        NamedEventRegistrationList * eventHandlers = eventProjectionHandler->GetEventHandlers(recycler);
        PropertyId nameId = signature->abiEvent->nameId;
        NamedEventRegistration * registration = GetExistingNamedEventRegistration(eventHandlers, nameId);

        bool fSettingHandler = ((Js::RecyclableObject::Is(args[1])) && (((Js::RecyclableObject *)args[1])->GetTypeId() == Js::TypeIds_Function));
        bool fRemoveExistingHandler = ((registration != nullptr) && (registration->GetDelegate() != nullptr));
        bool isDefaultInterface = false;
        IInspectable *inspectable = (fRemoveExistingHandler || fSettingHandler) ? GetInspectableForAddOrRemoveEvent(thisInfo, addMethod, args[0], projectionContext, &isDefaultInterface) : nullptr;

        // Remove the existing event handler if any
        if (fRemoveExistingHandler)
        {
            Assert(inspectable != nullptr);
            RemoveEventHandlerFromScript(AbiMethodSignature::From(signature->abiEvent->removeOn), inspectable, registration, scriptContext, !fSettingHandler, isDefaultInterface);
        }

        // If we are adding new event handler add it
        if (fSettingHandler)
        {
            Assert(inspectable != nullptr);
            CComPtr<CExternalWeakReferenceImpl> weakDelegate;
            CComPtr<IUnknown> pDelegate;
            __int64 eventCookie = AddEventHandler(signature->abiEvent, inspectable, Js::JavascriptFunction::FromVar(args[1]),
                StringOfId(scriptContext, signature->eventPropertyNameId), projectionContext, isDefaultInterface, weakDelegate, pDelegate);

            if (registration != nullptr)
            {
                // Set the new event info in the existing registration
                registration->SetDelegate(weakDelegate);
                registration->SetEventCookie(eventCookie);
            }
            else
            {
                // Create new one and add it to the list
                NamedEventRegistration * namedEvent = RecyclerNewFinalized(recycler, NamedEventRegistration, signature->abiEvent, nameId, weakDelegate, eventCookie);
                eventHandlers->Prepend(namedEvent);
            }
            AddEventProjectionHandler(thisInfo, args[0], weakDelegate, eventProjectionHandler);
        }

#ifdef ENABLE_JS_ETW
        if (EventEnabledJSCRIPT_PROJECTION_SETEVENTHANDLER_STOP())
        {
            LPCWSTR runtimeClassName = StringOfId(scriptContext, addMethod->runtimeClassNameId);
            LPCWSTR eventName = StringOfId(scriptContext, signature->abiEvent->nameId);
            EventWriteJSCRIPT_PROJECTION_SETEVENTHANDLER_STOP(runtimeClassName, eventName);
        }
#endif
        return scriptContext->GetLibrary()->GetUndefined();
    }

    // Info:        Called to get the event handler
    // Parameters:  standard thunk parameters
    Var GetEventHandlerThunk(Var method, Js::CallInfo callInfo, ...)
    {
#if DBG
        Js::JavascriptFunction* func = Js::JavascriptFunction::FromVar(method);
        Assert(Js::JavascriptOperators::GetTypeId(func) == Js::TypeIds_Function);
        Assert(func->IsWinRTFunction());
#endif
        ARGUMENTS(args, callInfo);
        Js::JavascriptWinRTFunction * function = Js::JavascriptWinRTFunction::FromVar(method);
        EventHandlerSignature * signature = reinterpret_cast<EventHandlerSignature*>(function->GetSignature());
        Js::ScriptContext *scriptContext = signature->projectionContext->GetScriptContext();
        Var result = scriptContext->GetLibrary()->GetNull();
        ThisInfo * thisInfo = signature->thisInfo;
        Assert(thisInfo->CanHoldEventCookies());

        // in + this
        if (size_t(args.Info.Count) < 1)
        {
            Js::JavascriptError::ThrowTypeError(scriptContext, VBSERR_ActionNotSupported);
        }

        // Get the existing event
        EventProjectionHandler *eventProjectionHandler = GetEventProjectionHandlerFromThisInfo(scriptContext, thisInfo, args[0]);
        if (eventProjectionHandler == nullptr)
        {
            Js::JavascriptError::ThrowReferenceError(scriptContext, JSERR_This_ReleasedInspectableObject, StringOfId(scriptContext, signature->eventPropertyNameId));
        }

        NamedEventRegistrationList * eventHandlers = eventProjectionHandler->GetEventHandlers(scriptContext->GetRecycler());
        PropertyId nameId = signature->abiEvent->nameId;
        NamedEventRegistration * registration = GetExistingNamedEventRegistration(eventHandlers, nameId);

        // Register event only if not already registered
        if (registration != nullptr)
        {
            Delegate *eventHandler = registration->GetDelegate();
            if (eventHandler)
            {
                auto jsFunc = eventHandler->GetCallback();
                if (jsFunc)
                {
                    result = jsFunc;
                }
            }
        }
        return result;
    }

    // Mark the delegate functions held
    void EventProjectionHandler::Mark(Recycler *recycler)
    {
        auto markAllEventRegistrations = [&](NamedEventRegistrationList *eventList) {
            if (eventList != nullptr)
            {
                NamedEventRegistrationList::Iterator iterEvents(eventList);
                while(iterEvents.Next())
                {
                    NamedEventRegistration * eventRegistration = iterEvents.Data();
                    auto delegatePtr = eventRegistration->GetDelegate();
                    if (delegatePtr != nullptr)
                    {
                        delegatePtr->StrongMark(recycler);
                    }
                }
            }
        };

        // For each events
        markAllEventRegistrations(events);

        // For each event handler
        markAllEventRegistrations(eventHandlers);
    }

    void EventProjectionHandler::UnRegister(bool fRemoveStrongRef)
    {
        auto unregisterAllRefCount = [&](NamedEventRegistrationList *eventList) {
            if (eventList != nullptr)
            {
                NamedEventRegistrationList::Iterator iterEvents(eventList);
                while(iterEvents.Next())
                {
                    NamedEventRegistration * eventRegistration = iterEvents.Data();
                    auto delegatePtr = eventRegistration->GetDelegate();
                    if (delegatePtr != nullptr)
                    {
                        if (fRemoveStrongRef && !delegatePtr->IsRooted())
                        {
                            delegatePtr->RemoveStrongRef();
                        }
                        delegatePtr->SetEventProjectionHandler(nullptr);
                    }
                }
            }
        };

        // For each events
        unregisterAllRefCount(events);

        // For each event handler
        unregisterAllRefCount(eventHandlers);

        // Release the gc tracked reference on these registrations
        Assert((LONG)gcTrackedRefCount > 0);
        ReleaseGCTrackedRef();
    }
}