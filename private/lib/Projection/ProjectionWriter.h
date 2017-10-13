//---------------------------------------------------------------------------
// Copyright (C) 1995 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------
#pragma once

#define WinRTObjectType _u("WinRTObject")

namespace Projection
{
    class ProjectionWriter;

    // Holds cached information about runtime classes.
    class RuntimeClassTypeInformation
    {
        Var prototypeVar;
        SpecialProjection * specialProjection;
        ThisInfo * thisInfo;
        RuntimeClassThis * runtimeClassThisInfo;
        Js::JavascriptWinRTFunction * constructorFunction; 
        bool hasEventHandlers;
        INT32 gcPressure;
        HTYPE htype;
    public:

        RuntimeClassTypeInformation()
            : prototypeVar(nullptr), specialProjection(nullptr), thisInfo(nullptr), constructorFunction(nullptr), hasEventHandlers(false), htype(nullptr)
        { }
        Var GetPrototypeVar() { return prototypeVar; }
        void SetPrototypeVar(Var value) { prototypeVar=value; }
        SpecialProjection * GetSpecialProjection() { return specialProjection; }
        void SetSpecialProjection(SpecialProjection * value) { specialProjection=value; }
        ThisInfo * GetThisInfo() { return thisInfo; }
        void SetRuntimeClassThisInfo(RuntimeClassThis * value) { runtimeClassThisInfo=value; }
        RuntimeClassThis * GetRuntimeClassThisInfo() { return runtimeClassThisInfo; }
        void SetThisInfo(ThisInfo * value) { thisInfo=value; }
        Js::JavascriptWinRTFunction * GetConstructorFunction() { return constructorFunction; }
        void SetConstructorFunction(Js::JavascriptWinRTFunction * value) { constructorFunction=value; }
        void SetHasEventHandlers(bool value) { hasEventHandlers=value; }
        bool HasEventHandlers() { return hasEventHandlers; }
        void SetGCPressure(INT32 value) { gcPressure = value; }
        INT32 GCPressure() { return gcPressure; }
        void SetHTYPE(HTYPE newHTYPE) { htype = newHTYPE; }
        HTYPE GetHTYPE() const { return htype; }
    };

    HostProfilerHeapObject* CreateWinrtConstructorObjectElement(ActiveScriptProfilerHeapEnum* heapEnum, Js::RecyclableObject* obj);

    class UnknownsReleaser : public FinalizableObject
    {
    private:
        ImmutableList<IUnknown *> *unknowns; 
        ImmutableList<IUnknown **> *unknownRefs; 
        ArenaAllocator allocator;

    public:
        //UnknownsReleaser(ImmutableList<IUnknown *> *unknowns);
        UnknownsReleaser(PageAllocator *pageAllocator) 
            : unknowns(nullptr), unknownRefs(nullptr), allocator(_u("ProjectionUnknownReleaser"), pageAllocator, nullptr)
        {
        }

        void SetUnknownsList(ImmutableList<IUnknown *> *unknowns, ImmutableList<IUnknown **> *unknownRefs, ArenaAllocator *allocatorOfUnknown);

    private:
        virtual void Finalize(bool isShutdown) { }
        virtual void Dispose(bool isShutdown) override;
        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }
    };

    typedef JsUtil::BaseDictionary<IWeakReference *, EventProjectionHandler *, Recycler, PowerOf2SizePolicy> EVENTHANDLERMAP;
    typedef JsUtil::BaseDictionary<IUnknown *, RecyclerWeakReference<Js::DynamicObject> *, Recycler, PowerOf2SizePolicy> PROJECTIONINSTANCEMAP;
    typedef JsUtil::KeyValuePair<IUnknown *, RecyclerWeakReference<Js::DynamicObject> *> PROJECTIONINSTANCEKEYVALUEPAIR;
    typedef JsUtil::BaseDictionary<PropertyId, RuntimeClassTypeInformation*, Recycler, PowerOf2SizePolicy> TYPEINFORMATIONMAP;
    typedef JsUtil::BaseDictionary<IWeakReference *, WEAKPROPERTYBAG *, Recycler, PowerOf2SizePolicy> WEAKREFERENCETOWEAKPROPERTYBAGMAP;

    typedef SList<EventHandlingProjectionObjectInstance *, ArenaAllocator> EVENTHANDLINGINSTANCELIST;

    class ProjectionWriter : public ExternalWeakReferenceCache
    {
        friend class Delegate;
        friend class ProjectionMarshaler;
        friend class VectorArray;

        // Since type is for full script context lifetime the dictionary need not be in recycler
        typedef JsUtil::BaseDictionary<PropertyId, HTYPE, ArenaAllocator, PowerOf2SizePolicy> HTYPEMAP;

        typedef JsUtil::BaseDictionary<PropertyId, ArrayProjection*, Recycler, PowerOf2SizePolicy> ArrayTypeMap;

        typedef JsUtil::BaseHashSet<CUnknownImpl *, ArenaAllocator, PowerOf2SizePolicy> CUNKNOWNIMPLHASHSET;
        typedef JsUtil::List<RuntimeClassThis *, ArenaAllocator> RUNTIMECLASSTHISLIST;

        typedef JsUtil::List<UnknownsReleaser *> UNKNOWNSRELEASERLIST;

        ProjectionContext * projectionContext;
        PropertyId lengthId;
        Recycler * recycler;

        Var two;
        Var one;

        bool fResolvingNow;

#if DBG
        bool supportsWeakDelegate;
        bool isMarkedForClose;
#endif

        // PropertyIds for the event members:
        PropertyId targetId;
        PropertyId detailId;
        PropertyId typeId;

        HTYPEMAP * htypesEnum; // Htypes of enums
        HTYPEMAP * htypesStruct; // Htypes of struct

        // This list would be accessed across different threads because one can release a reference to this on different thread.
        // We need to synchronise access to this list
        CUNKNOWNIMPLHASHSET *unknownsToMark;
        CriticalSectionForUnknownImpl *criticalSectionForUnknownsToMark;

        RUNTIMECLASSTHISLIST *runtimeClassThisToCleanupOnClose;

        EVENTHANDLINGINSTANCELIST eventHandlingInstanceList;
        EVENTHANDLINGINSTANCELIST rootInPartialGCInstanceList;

        // This is the recycler data that needs to stay for ScriptContext's lifetime
        struct RecyclerData 
        {
            ThisInfo * unknownThis;
            ThisInfo * unknownEventHandlingThis;
            ThisInfo * namespaceThis;

            TYPEINFORMATIONMAP * runtimeClassTypeInformation; // Runtime class type information
            ArrayTypeMap *m_mapArrayProjection;

            PROJECTIONINSTANCEMAP *m_InspectablesCache;
            EVENTHANDLERMAP *m_eventHandlerCache;

            // This list would be the list of unknown releasers that projection marshaler instances are using
            // We need to keep them alive so that in case of exceptions, projectionmarshaler could put its delayed finalizer stuff in these.
            UNKNOWNSRELEASERLIST *inUseUnknownReleasers;

            // This list would be the list of unknownReleasers that projectionMarshaler took but since there was no exception during destructor,
            // didnt use it and hence can be reused for future use.
            // Note that we need to keep these alive and the only difference is we could reuse these already created instances later when new projectionmarshaler is created.
            UNKNOWNSRELEASERLIST *unUsedUnknownReleasers;

            // This is the map that will contain the IWeakReference to WeakPropertyBag mapping
            WEAKREFERENCETOWEAKPROPERTYBAGMAP *weakReferenceToWeakPropertyBagMap;

            RecyclerData() : 
                unknownThis(nullptr),
                unknownEventHandlingThis(nullptr),
                namespaceThis(nullptr),
                runtimeClassTypeInformation(nullptr),
                m_mapArrayProjection(nullptr),
                m_InspectablesCache(nullptr), 
                m_eventHandlerCache(nullptr), 
                inUseUnknownReleasers(nullptr),
                unUsedUnknownReleasers(nullptr),
                weakReferenceToWeakPropertyBagMap(nullptr)
            {
            }
        };
        RecyclerRootPtr<RecyclerData> recyclerData;

        Js::JavascriptFunction * GetPromiseMaker();

        PropertyId IdOfString(LPCWSTR propertyName);
        LPCWSTR StringOfId(PropertyId id);
        void SetProperty(Js::DynamicObject * object, PropertyId id, Var function);
        Var PropertiesObjectToJsVar(__in RtPROPERTIESOBJECT propertiesObject, __in ThisInfo * thisInfo);
        SpecialProjection * SpecializationToSpecialProjection(ThisInfo * thisInfo, Js::DynamicObject *prototypeObject);
        void GetOrCreateRuntimeClassTypeInformation(MetadataStringId typeNameId, LPCWSTR typeName, RtIID defaultInterface, RtSPECIALIZATION specialization, RtEXPR prototype, RtPROPERTIESOBJECT properties, regex::Option<ProjectionModel::MethodSignature> signature, bool hasEventHandlers, INT32 gcPressure, 
            HTYPE * htype, RuntimeClassTypeInformation ** typeInformation);
        Js::JavascriptWinRTFunction * UnconstructableClassThunkOfTypename(LPCWSTR typeName, bool fConstructor);
        Js::JavascriptWinRTFunction * RuntimeClassInfoToFunction(MetadataStringId typeNameId, LPCWSTR typeName, RtIID defaultInterface, RtSPECIALIZATION specialization, RtEXPR prototype, RtPROPERTIESOBJECT properties, regex::Option<ProjectionModel::MethodSignature> signature, bool hasEventHandlers, INT32 gcPressure);
        Js::JavascriptWinRTFunction * FunctionOfSignature(RtMETHODSIGNATURE signature, RtPROPERTIESOBJECT properties, ThisInfo * thisInfo, bool fConstructor, bool boundsToUndefined = false);
        Js::JavascriptWinRTFunction * ContinueFunctionOfSignature(RtMETHODSIGNATURE signature, ThisInfo * thisInfo, bool fConstructor, bool boundsToUndefined = false);
        Var EnumToObject(RtENUM _enum, ThisInfo * thisInfo);
        Js::JavascriptWinRTFunction * FunctionToJsFunction(RtFUNCTION function, ThisInfo * thisInfo);
        Var ExprToJsVar(__in RtEXPR expr, __in ThisInfo * thisInfo);

        Var GetAsyncOperationSource();

        Windows::Foundation::IPropertyValueStatics *propertyValueFactory;

        static Var DelayedFunctionOfSignatureThunk(Var method, Js::CallInfo callInfo, ...);

        void AddKnownPrototypeMethods(__in ProjectionContext *projectionContext, __in Var prototype, __in RuntimeClassTypeInformation *pTypeInformation);

#ifndef DISABLE_ISTRINGABLE_QI
        void AddToString(__in ProjectionContext *projectionContext, __in Var prototype);
#endif // !defined(DISABLE_ISTRINGABLE_QI)

    public:
        bool CanResolveParamaters(RtMETHODSIGNATURE signature, bool isDelegate);
        ProjectionWriter(ProjectionContext * projectionContext);
        bool GetRuntimeClassTypeInformation(PropertyId typeNameId, HTYPE * htype, RuntimeClassTypeInformation ** typeInformation);
        Var CreateProjectionObjectTypedInstance(
            __in PropertyId typeNameId, 
            __in HTYPE htype,
            __in RuntimeClassTypeInformation * typeInformation,
            __in IUnknown* unknown, 
            __in bool allowIdentity,
            __in bool allowExtensions,
            __in_opt ConstructorArguments* constructorArguments = nullptr);

        bool TryEnsureRuntimeClassTypeExists(RtTYPEDEFINITIONTYPE type, HTYPE * htype, RuntimeClassTypeInformation ** typeInformation);

        void MarkForClose(bool disableUnregister = false);
        HRESULT Close();
        Var WriteExpr(__in RtEXPR expr);
        bool TryGetTypedInstanceFromCache(IUnknown *unknown, Var *result, bool allowExtensions = false);
        Var CreateNewTypeInstance(
            __in MetadataStringId typeNameId, 
            __in_z LPCWSTR typeName, 
            __in RtIID defaultInterface, 
            __in RtSPECIALIZATION specialization, 
            __in RtEXPR prototype, 
            __in RtPROPERTIESOBJECT properties, 
            __in regex::Option<ProjectionModel::MethodSignature> signature, 
            __in bool hasEventHandlers, 
            __in INT32 gcPressure, 
            __in IUnknown *unknown, 
            __in bool allowIdentity, 
            __in bool allowExtensions,
            __in_opt ConstructorArguments* constructorArguments = nullptr);

        Js::JavascriptWinRTFunction * BuildDirectFunction(Var signature, void * entryPoint, PropertyId nameId, bool fConstructor);
        void ApplyPropertiesObjectToJsObject(Var object, RtPROPERTIESOBJECT propertiesObject, ThisInfo * thisInfo);
        HRESULT GetStructHType(RtSTRUCTTYPE structType, HTYPE *htype);
        HRESULT InspectableObjectCacheCollect();
        void RemoveInspectableFromCache(PROJECTIONINSTANCEKEYVALUEPAIR &keyValuePair);
        EventProjectionHandler *GetEventHandlerFromWeakReference(Js::PropertyId typeId, IWeakReference *weakReference);
        EventProjectionHandler *GetExistingEventHandlerFromWeakReference(IWeakReference *weakReference);
        void RemoveEventHandlerFromWeakReference(IWeakReference *weakReference);

        void OnNewEventHandlingInstance(EventHandlingProjectionObjectInstance *instance);
        void OnDisposeEventHandlingInstance(EventHandlingProjectionObjectInstance *instance);
        void MarkNow(Recycler *recycler, bool inPartialCollect) override;
        void ResolveNow(Recycler *recycler) override;
        void RegisterUnknownToCleanOnClose(CUnknownImpl *unknownImpl);
        void UnRegisterUnknownToCleanOnClose(CUnknownImpl *unknownImpl);
        void AddRuntimeClassThisToCleanupOnClose(RuntimeClassThis *runtimeClassThis);
        void ApplyPropertyToJsObject(RtPROPERTY prop, Js::DynamicObject * dynamicObject, ThisInfo * thisInfo);

        void ReportUnknownImpls(ActiveScriptProfilerHeapEnum *profilerEnum);

        void AddArrayProjection(PropertyId propertyId, ArrayProjection *arrayProjection);
        ArrayProjection *GetArrayProjection(PropertyId propertyId);

        Windows::Foundation::IPropertyValueStatics *GetPropertyValueFactory();

        UnknownsReleaser * GetUnknownsReleaser();

        void SetUnknownsReleaserAsUnused(
#if DBG
            UnknownsReleaser *unknownsReleaser
#endif
            );

        void SetUnknownsListForDisposingLater(
#if DBG
            UnknownsReleaser *unknownsReleaser,
#endif
            ImmutableList<IUnknown *> *unknowns, 
            ImmutableList<IUnknown **> *unknownRefs, 
            ArenaAllocator *alloc);

        CriticalSectionForUnknownImpl *GetCriticalSectionForUnknownsToMark()
        {
            return criticalSectionForUnknownsToMark;
        }

#ifdef PROJECTION_METADATA_TRACE
        static void Trace(const char16 *form, ...) // const
        {
            if (Js::Configuration::Global.flags.Trace.IsEnabled(Js::ProjectionMetadataPhase))
            {
                va_list argptr;
                va_start(argptr, form);
                Output::Print(_u("ProjectionModel: "));
                Output::VPrint(form, argptr);
                Output::Flush();
            }
        }
#endif

        // Returns whether the IUnknown supports IWeakReference
        bool GetWeakPropertyBagFromUnknown(bool fCreate, IUnknown *unknown, WEAKPROPERTYBAG **weakPropertyBag);
        void GetWeakPropertyBagFromWeakRef(bool fCreate, IWeakReference *weakReference, WEAKPROPERTYBAG **weakPropertyBag);

        Var GetWeakWinRTProperty(ProjectionObjectInstance *instance, const Js::PropertyRecord *propertyRecord);
        void SetWeakWinRTProperty(ProjectionObjectInstance *instance, const Js::PropertyRecord *propertyRecord, Var propertyValue);
    };
}
