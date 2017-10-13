//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

namespace Projection 
{
    typedef JsUtil::BaseDictionary<const Js::PropertyRecord *, RecyclerWeakReference<Js::RecyclableObject> *, Recycler, PowerOf2SizePolicy> WEAKPROPERTYBAG;

    class ProjectionObjectInstance : public Js::CustomExternalObject
    {
        friend class ProjectionWriter;

    public:
        static HRESULT Create(
            __in HTYPE htype,
            __in bool hasEventHandlers,
            __in IUnknown* nativeABI,
            __in IID defaultIID,
            __in ProjectionContext *projectionContext,
            __out ProjectionObjectInstance** newInstance,
            __in bool supportsIdentity,
            __in INT32 gcPressure);

        IUnknown * GetNativeABI() 
        {
            return TaggedToUnknown(taggedUnknown);
        }

        IUnknown * GetUnknown()
        {
            return unknown;
        }

        ProjectionContext* GetProjectionContext() const
        {
            return this->projectionContext;
        }

        // Info:        Get the interface from the native pointer. 
        // Parameters:  iidOfInterface - iid of the interface requested
        //              scriptContext - the script context
        //              isDefaultInterface - out parameter indicating whether the requested interface was the default interface
        //              addRefDefault - boolean indicating whether the default interface should be addRef'd before returning
        // Returns:     The IUnknown of the interface and a boolean indicating whether that interface was the default interface.
        //              If the interface was the default it will be returned with no additional ref-count, unless addRefDefault is true.
        IUnknown * GetInterfaceOfNativeABI(const IID & iidOfInterface, Js::ScriptContext * scriptContext, bool * isDefaultInterface, bool addRefDefault = false);

        virtual EventProjectionHandler * GetEventProjectionHandler();
        virtual uint GetEventAndEventHandlerCount() { return 0; }
        virtual void PopulateProfilerEventInfo(ActiveScriptProfilerHeapEnum* heapEnum, PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST *eventList) { eventList->count = 0; }

        static BOOL Is(Var instance);
        BOOL IsEqual(__in ProjectionObjectInstance *other);

        virtual void Finalize(bool isShutdown) { }
        // Call after sweeping is done.  
        // Can call other script or cause another collection.
        virtual void Dispose(bool isShutdown) override;

        virtual void ReleaseNativePointers(bool isShutdown, bool isDispose);

        void SetWeakReference(RecyclerWeakReference<Js::DynamicObject> *weakRef)
        {
            Assert(weakReference == nullptr);
            weakReference = weakRef;
        }

        HRESULT GetFullHeapObjectInfo(HostProfilerHeapObject** result, HeapObjectInfoReturnResult* returnResult);

        virtual WEAKPROPERTYBAG *GetWeakPropertyBag(bool fCreate);

        static HRESULT ArrayBufferFromExternalObjectDispatch(__in RecyclableObject *pObj, __out Js::ArrayBuffer **ppArrayBuffer);

#ifndef DISABLE_ISTRINGABLE_QI
        static Var ToStringThunk(Var method, Js::CallInfo callInfo, ...);
#endif // !defined(DISABLE_ISTRINGABLE_QI)

    protected:
        DEFINE_VTABLE_CTOR(ProjectionObjectInstance, Js::CustomExternalObject);
        ProjectionObjectInstance(ProjectionContext *projectionContext, HTYPE htype, IUnknown* nativeABI, IID defaultIID, bool supportsIdentity);

        // This is either the IInspectable of the ABI - for strong typed instance or it is IWeakReference for weakly refereced typed instance
        // For the IInspectable case, this will be the default interface for the runtime class, or the most-derived interface for an interface.
        // This default interface invariant is ensured by first tagging the low bit of the unknown with '1'. This '1' is removed the first time 
        // that GetDefaultInterfaceOfNativeABI is called.
        size_t taggedUnknown; 
        IUnknown *unknown;

        IID defaultIID;

        // This is used as a key and value in the object map cache. 
        // When we add the RecyclerWeakRef on the object in the dictionary, it can happen that the object is marked for disposing but not yet disposed.
        // In the mean time you get the same instance back and trying to project it out, since the object is not usable any more we would create the new object
        // and add it to the dictionary. As part of dispose of the first object we dont want to remove the newly updated entry and hence we maitain the keyValue pair
        // that can be used during remove.
        RecyclerWeakReference<Js::DynamicObject> *weakReference;

        // We shouldnt be accessing scriptSite from finalizer as it might be called when scriptSite is closed and callrootlevel = 0 
        // (that javascript function doesnt exist on the current stack) and hence the projectionContext might have been destroyed
        // Hence the IsClosed check in the finalizer should use the directly stored scriptsite.
        // 
        // But to unregister the object from cache we should always use the projectionContext directly stored so that incase there was recycler gc invoke we are not using already 
        // destroyed scriptSite and the GetScriptEngine or GetScriptSiteContext on m_pScriptSite doesnt return null. 
        // ProjectionContext would be able to provide us scriptContext irrespective of scriptSite close and we shouldnt be using scriptEngine if scriptSite is closed.
        ScriptSite *scriptSite;
        ProjectionContext *projectionContext;

#ifndef DISABLE_ISTRINGABLE_QI
        // The cached IStringable interface through which toString calls are routed
        Windows::Foundation::IStringable *stringable;

        // Indications as to whether we have cached the IStringable nature of this object.
        bool queriedStringable;
#endif // !defined(DISABLE_ISTRINGABLE_QI)

        // The object wont support identity in case it corresponds to the ev object from the event handler.
        bool supportsIdentity;

        // Weak property bag
        WEAKPROPERTYBAG *weakPropertyBag;

#if DBG_DUMP
        // Store the ProjectionMemoryInformation when this object is created - when it is disposed the ProjectionContext may be garbage so we won't be able to get it.
        ProjectionMemoryInformation* projectionMemoryInformation;
#endif

        // Info:        Remove any low-bit tag and convert to IUnknown *
        // Parameters:  taggedUnknown - the tagged int
        // Returns:     The pointer
        static IUnknown * TaggedToUnknown(size_t taggedUnknown)
        {
            const size_t lowBitZero = (size_t) -2;
            auto pointer = reinterpret_cast<IUnknown*>(taggedUnknown & lowBitZero); 
            return pointer;
        }

        // Info:        Convert an unknown to a tagged int with the low bit set to '1'. 
        //              This means the object is not yet QI-ed for default interface yet.
        // Parameters:  unknown - The unknown to convert
        // Returns:     The tagged int
        static size_t UnknownToTagged(IUnknown * unknown)
        {
            size_t result = (size_t)unknown;
            ++result; // Set the low bit
            return result;
        }

        // Info:        Return true if this tagged int hasn't yet been QI-ed for default interface
        // Parameters:  taggedUnknown - The tagged int representing the interface
        static bool IsNotYetTaggedAsDefaultInterface(size_t taggedUnknown)
        {
            return taggedUnknown%2==1;
        }

        // Info:        Convert a tagged into to be marked as the default interface.
        // Parameters:  taggedUnknown - The tagged int representing the interface
        // Returns:     The tagged int with low-bit set to 0
        static size_t TagAsDefaultInterface(size_t taggedUnknown)
        {
            Assert(IsNotYetTaggedAsDefaultInterface(taggedUnknown));
            return taggedUnknown-1;
        }
    };

    
    class EventHandlingProjectionObjectInstance : public ProjectionObjectInstance
    {
        friend class ProjectionObjectInstance;
        friend class ProjectionWriter;

    public:
        virtual EventProjectionHandler * GetEventProjectionHandler();
        virtual uint GetEventAndEventHandlerCount();
        virtual void PopulateProfilerEventInfo(ActiveScriptProfilerHeapEnum* heapEnum, PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST *eventList);

        bool SupportsRefCountProbe() { return supportsRefCountProbe; }

        virtual void Finalize(bool isShutdown) override;
        virtual void ReleaseNativePointers(bool isShutdown, bool isDispose) override;
        virtual void Mark(Recycler *recycler) override;
        virtual WEAKPROPERTYBAG *GetWeakPropertyBag(bool fCreate) override;

    protected:
        DEFINE_VTABLE_CTOR(EventHandlingProjectionObjectInstance, ProjectionObjectInstance);

    private:
        EventHandlingProjectionObjectInstance(ProjectionContext *projectionContext, HTYPE htype, IUnknown* nativeABI, IID defaultIID, bool supportsIdentity);

        void DisconnectEventHandlers(bool isFinalize);

        IWeakReference *abiWeakReference;
        EventProjectionHandler *eventProjectionHandler; // Event information for this instance

        // See if we need to be rooted
        // Also mark the event handlers if we are rooted
        void TrackRefCount(Recycler *recycler);
        void UpdateRootedState();

        void MarkAsRoot() 
        {
            Assert(supportsRefCountProbe);
            isRooted = true;
        }

        void MarkAsNotRoot() 
        {
            Assert(supportsRefCountProbe);
            isRooted = false;
        }

        bool supportsRefCountProbe;
        bool isRooted;
    };
}
