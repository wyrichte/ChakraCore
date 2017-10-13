//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Implementation for the ABI events

#pragma once

namespace Projection
{
    class Delegate;

    class NamedEventRegistration : public FinalizableObject
    {
        RtEVENT abiEvent;
        PropertyId nameId;
        CExternalWeakReferenceImpl *m_weakDelegate;
        __int64 m_eventCookie;
    
    public:
        NamedEventRegistration(RtEVENT abiEvent, PropertyId nameId, CExternalWeakReferenceImpl *weakDelegate, __int64 eventCookie)
            : abiEvent(abiEvent), nameId(nameId), m_weakDelegate(weakDelegate)
        {
            Assert(weakDelegate != nullptr);
            m_weakDelegate->AddRef();
            m_eventCookie = eventCookie;
        }

        virtual void Finalize(bool isShutdown) override 
        { 
            if (!isShutdown)
            {
                ResetDelegate();
            }
        }
        
        virtual void Dispose(bool isShutdown) override {}
        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

        Delegate *GetDelegate()
        {
            if (m_weakDelegate)
            {
                auto unknownImpl = m_weakDelegate->ResolveUnknownImpl();
                if (unknownImpl)
                {
                    Assert(unknownImpl->GetWinrtTypeFlags() == PROFILER_HEAP_OBJECT_FLAGS_WINRT_DELEGATE);
                    return (Delegate *)unknownImpl;
                }
            }

            return nullptr;
        }

        void ResetDelegate();

        void SetDelegate(CExternalWeakReferenceImpl *weakDelegate) 
        {
            Assert(m_weakDelegate == nullptr);
            Assert(weakDelegate != nullptr);

            m_weakDelegate = weakDelegate;
            m_weakDelegate->AddRef();
        }

        // The returned IUnknown is not addref
        IUnknown *GetUnknown()
        {
            if (m_weakDelegate)
            {
                return m_weakDelegate->ResolveUnknown();
            }

            return nullptr;
        }

        __int64 GetEventCookie() { return m_eventCookie; }
        void SetEventCookie(__int64 eventCookie) { m_eventCookie = eventCookie; }

        PropertyId GetNameId() { return nameId; }
        RtEVENT GetEvent() { return abiEvent; }
    }; 

    typedef SList<NamedEventRegistration*, Recycler, RealCount> NamedEventRegistrationList;

    class EventProjectionHandler
    {
    private:
        NamedEventRegistrationList * events;
        NamedEventRegistrationList * eventHandlers;
        Js::PropertyId typeId;
        ULONG gcTrackedRefCount;

    public:
        EventProjectionHandler(Js::PropertyId typeId = Js::PropertyIds::_none) : typeId(typeId), events(nullptr), eventHandlers(nullptr), gcTrackedRefCount(0) { }
        ~EventProjectionHandler() { }

        Js::PropertyId GetTypeId() { return typeId; }

        NamedEventRegistrationList * GetEvents(Recycler * recycler);
        NamedEventRegistrationList * GetEventHandlers(Recycler *recycler);

        NamedEventRegistrationList * GetExistingEvents() { return events; }
        NamedEventRegistrationList * GetExistingEventHandlers() { return eventHandlers; }

        void RemoveAllEventsAndEventHandlers(IUnknown *unknown, Js::ScriptContext *scriptContext);
        void Mark(Recycler *recycler);
        void UnRegister(bool fRemoveStrongRef);

        void ReleaseGCTrackedRef() { gcTrackedRefCount--; }
        void AddGCTrackedRef() { gcTrackedRefCount++; }
        ULONG GetGCTrackedRefCount() { return gcTrackedRefCount; }
    };

    Var AddEventListenerThunk(Var method, Js::CallInfo callInfo, ...);
    Var RemoveEventListenerThunk(Var method, Js::CallInfo callInfo, ...);

    Var SetEventHandlerThunk(Var method, Js::CallInfo callInfo, ...);
    Var GetEventHandlerThunk(Var method, Js::CallInfo callInfo, ...);
}
