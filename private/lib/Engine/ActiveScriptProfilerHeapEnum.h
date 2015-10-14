//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

namespace Projection
{
    class CUnknownImpl;
};

class ActiveScriptProfilerHeapEnum sealed : public IActiveScriptProfilerHeapEnum
{
public:
    typedef HRESULT (__stdcall *GetHeapObjectInfoPtr)(Var instance, HostProfilerHeapObject** heapObjOut, HeapObjectInfoReturnResult& returnResult);

    enum ProfilerHeapObjectType
    {
        HeapObjectType_Undefined                   =0,
        HeapObjectType_Null                        =0x1,
        HeapObjectType_Boolean                     =0x2,
        HeapObjectType_Number                      =0x3,
        HeapObjectType_String                      =0x4,
        HeapObjectType_ArgumentObject              =0x5,
        HeapObjectType_ArrayObject                 =0x6,
        HeapObjectType_ArrayBuffer                 =0x7,
        HeapObjectType_BooleanObject               =0x8,
        HeapObjectType_CanvasPixelArray            =0x9,
        HeapObjectType_DataView                    =0xa,
        HeapObjectType_DateObject                  =0xb,
        HeapObjectType_ExtensionEnumeratorObject   =0xc,
        HeapObjectType_ErrorObject                 =0xd,
        HeapObjectType_GetVarDateFunctionObject    =0xe,
        HeapObjectType_FunctionObject              =0xf,
        HeapObjectType_NumberObject                =0x10,
        HeapObjectType_ObjectObject                =0x11,
        HeapObjectType_RegexObject                 =0x12,
        HeapObjectType_EnumeratorIterator          =0x13,
        HeapObjectType_StringObject                =0x14,
        HeapObjectType_TypedArrayObject            =0x15,
        HeapObjectType_GlobalObject                =0x16,
        HeapObjectType_FormObject                  =0x17,
        HeapObjectType_Scope                       =0x18,
        HeapObjectType_HostObject                  =0x19,
        HeapObjectType_DOM                         =0x1a,
        HeapObjectType_WinRT                       =0x1b,
        HeapObjectType_MapObject                   =0x1c,
        HeapObjectType_SetObject                   =0x1d,
        HeapObjectType_WeakMapObject               =0x1e,
        HeapObjectType_WeakSetObject               =0x1f,
        HeapObjectType_Symbol                      =0x20,
        HeapObjectType_SymbolObject                =0x21,
        HeapObjectType_ProxyObject                 =0x22,
        HeapObjectType_ArrayIterator               =0x23,
        HeapObjectType_MapIterator                 =0x24,
        HeapObjectType_SetIterator                 =0x25,
        HeapObjectType_StringIterator              =0x26,
        HeapObjectType_Generator                   =0x27,
        HeapObjectType_Promise                     =0x28,
        HeapObjectType_SIMD                        =0x29,
        HeapObjectType_JsrtExternalObject          =0x2a,

        HeapObjectType_Last                        =HeapObjectType_JsrtExternalObject,
        HeapObjectType_Invalid                     =0xff
    };

    static const ULONG PROFILER_RELATIONSHIP_INFO_MASK;
    static const ULONG PROFILER_RELATIONSHIP_FLAGS_MASK;

    static UINT GetHeapObjectInternalPropertyInfoSize();
    void FillHeapObjectInternalUnnamedJSVarProperty(ProfilerHeapObjectOptionalInfo *optionalInfo, Var jsVar);
    void FillHeapObjectInternalUnnamedExternalProperty(ProfilerHeapObjectOptionalInfo *optionalInfo, PROFILER_EXTERNAL_OBJECT_ADDRESS externalObjectAddress);
    static UINT GetHeapObjectIndexPropertiesInfoSize(int propertyCount);
    void VisitRoot(Projection::CUnknownImpl *unknownImpl);
    void CloseHeapEnum();

    static PROFILER_RELATIONSHIP_INFO GetRelationshipInfo(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship);
    static bool IsRelationshipInfoType(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_RELATIONSHIP_INFO value);
    static bool IsRelationshipFlagSet(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS flag);
    static bool AreAnyRelationshipFlagsSet(const PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship);
    void SetRelationshipInfo(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_RELATIONSHIP_INFO value);

private:
    struct InternalTypeIdMap {
        Js::TypeId typeId;
        ProfilerHeapObjectType profilerType;
    };
    static const InternalTypeIdMap internalTypeIdMap[];

    struct TypeNameIdMap {
        ProfilerHeapObjectType typeId;
        PROFILER_HEAP_OBJECT_NAME_ID typeNameId;
    };
    TypeNameIdMap typeNameIdMap[HeapObjectType_Last + 1];

    static const ULONG PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_SCOPE_SLOT_ARRAY;
    static const ULONG PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_BOUND_FUNCTION_ARGUMENT_LIST;
    static const ULONG INTERNAL_PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_MASK;

    long m_cref;
    ScriptEngine& m_scriptEngine;
    Recycler& m_recycler;
    Recycler::AutoSetupRecyclerForNonCollectingMark* m_autoSetupRecyclerForNonCollectingMark;
    BOOL isClosed;
    BOOL isInitialized;
    bool m_preEnumHeap2Behavior;
    PROFILER_HEAP_ENUM_FLAGS m_enumFlags;
#ifdef HEAP_ENUMERATION_VALIDATION
    static int libObjectCount;
    static int userObjectCount;
    static ActiveScriptProfilerHeapEnum* currentEnumerator;
    static const ULONG PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_LIBRARY_OBJECT;
    static const ULONG PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_UNREPORTED_USER_OBJECT;
#endif

    CComPtr<IHeapEnumHost> m_heapEnumHost;

    struct ProfilerHeapObjectJS
    {
        UINT size;
        union
        {
            PROFILER_HEAP_OBJECT_ID objectId;
            PROFILER_EXTERNAL_OBJECT_ADDRESS externalAddress;
        };
        PROFILER_HEAP_OBJECT_NAME_ID typeNameId;
        ULONG flags;
        USHORT unused;
        USHORT optionalInfoCount;
        ProfilerHeapObjectOptionalInfo *optionalInfo;
        ProfilerHeapObjectJS& operator=(const HostProfilerHeapObject& rhs)
        {
            this->size = rhs.size; this->objectId = rhs.objectId; this->typeNameId = rhs.typeNameId; this->flags = rhs.flags; this->optionalInfoCount = rhs.optionalInfoCount; return *this;
        }
    };

    // ProfilerHeapObject is our internal representation of the data that will be passed out as a PROFILER_HEAP_OBJECT*
    // via IActiveScriptProfilerHeapEnum::Next. It consists of three parts:
    //
    // 1. Header information in jsInfo member.
    //    --This should be indentical to the PROFILER_HEAP_OBJECT format. We pass out a pointer to the jsInfo member as
    //      a handle to the PROFILER_HEAP_OBJECT*, and need to convert it to a ProfilerHeapObject when we are
    //      called with one eg in GetOptionalInfo or FreeObjectAndOptionalInfo
    //
    // 2. Optional info in the jsInfo member.
    //    -- This is memory embedded in the allocation of jsInfo. We pass out pointers to within that structure
    //       via IActiveScriptProfilerHeapEnum::GetOptionalInfo
    //
    // 3. Optional info in the hostInfo member
    //    -- A pointer to a memory block that, like jsInfo member, has memory embedded within for the host optional info.
    //       The total optional info count in the jsInfo structure includes the count in this member.
    //
    // These data structrues achieve a number of design goals:
    // -- Only two memory allocations need to done for each PROFILER_HEAP_OBJECT:
    //    1. jsInfo
    //    2. optionally host object info if present.
    //
    // -- Limit number of function calls needed to gather complete object information to two:
    //    1. To allocate the object (although this can be batched as can retrieve several at once)
    //    2. To retrieve the optional information
    //

    struct ProfilerHeapObject
    {
        HostProfilerHeapObject* hostInfo;
        ProfilerHeapObjectJS jsInfo;
        ProfilerHeapObject() : hostInfo(NULL) {};
        USHORT OptionalInfoCountJsAndHost() { return jsInfo.optionalInfoCount; }
        USHORT OptionalInfoCountJSOnly() { return jsInfo.optionalInfoCount - (hostInfo ? hostInfo->optionalInfoCount : 0); }
        static size_t AllocHeaderSize() { return offsetof(ProfilerHeapObject, jsInfo) + offsetof(ProfilerHeapObjectJS, optionalInfo); }
        PROFILER_HEAP_OBJECT* AsPublicFacing() { return (PROFILER_HEAP_OBJECT*)&jsInfo; }
        static ProfilerHeapObject* AsInternal(PROFILER_HEAP_OBJECT* obj)
        { return (ProfilerHeapObject*)((char*)obj - (offsetof(ProfilerHeapObject, jsInfo))); }
    };
    static_assert(offsetof(ProfilerHeapObjectJS, size) == offsetof(PROFILER_HEAP_OBJECT, size), "ProfilerHeapObjectJS vs PROFILERHEAPOBJECT mismatch");
    static_assert(offsetof(ProfilerHeapObjectJS, objectId) == offsetof(PROFILER_HEAP_OBJECT, objectId), "ProfilerHeapObjectJS vs PROFILERHEAPOBJECT mismatch");
    static_assert(offsetof(ProfilerHeapObjectJS, typeNameId) == offsetof(PROFILER_HEAP_OBJECT, typeNameId), "ProfilerHeapObjectJS vs PROFILERHEAPOBJECT mismatch");
    static_assert(offsetof(ProfilerHeapObjectJS, flags) == offsetof(PROFILER_HEAP_OBJECT, flags), "ProfilerHeapObjectJS vs PROFILERHEAPOBJECT mismatch");
    static_assert(offsetof(ProfilerHeapObjectJS, optionalInfoCount) == offsetof(PROFILER_HEAP_OBJECT, optionalInfoCount), "ProfilerHeapObjectJS vs PROFILERHEAPOBJECT mismatch");
    static_assert(offsetof(ProfilerHeapObjectJS, optionalInfo) == sizeof(PROFILER_HEAP_OBJECT), "ProfilerHeapObjectJS vs PROFILERHEAPOBJECT mismatch");


    struct HeapScanQueueElement
    {
        RecyclerHeapObjectInfo m_heapObject;
        ProfilerHeapObject* m_profHeapObject;
        HeapScanQueueElement() {}
        HeapScanQueueElement(ProfilerHeapObject* profHeapObject) :
        m_profHeapObject(profHeapObject) { }
        HeapScanQueueElement(RecyclerHeapObjectInfo& heapObject, ProfilerHeapObject* profHeapObject) :
        m_heapObject(heapObject), m_profHeapObject(profHeapObject) { }
    };
    typedef DList<HeapScanQueueElement, ArenaAllocator> HeapScanQueue;
    HeapScanQueue m_scanQueue;

    void SetRelationshipFlags(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_RELATIONSHIP_FLAGS flag);

    bool LegacyBehavior() { return m_preEnumHeap2Behavior; }
    bool ShouldStoreRelationshipFlags() const { return m_enumFlags & PROFILER_HEAP_ENUM_FLAGS_STORE_RELATIONSHIP_FLAGS; }

#ifdef ENABLE_TEST_HOOKS
    static GetHeapObjectInfoPtr pfGetHeapObjectInfo;
#endif

    PROFILER_HEAP_OBJECT_NAME_ID GetTypeNameId(Js::RecyclableObject* instance);
    PROFILER_HEAP_OBJECT_NAME_ID GetTypeNameId(ProfilerHeapObjectType objectType);
    PROFILER_HEAP_OBJECT_NAME_ID GetPropertyId(LPCWSTR propertyName);
    void CreateTypeNameIds();
    HRESULT VerifyOnEntry();

#if DBG
    static void ValidateTypeMap();
#endif

    template <typename Fn>
    static void IterateArrayHelper(Var var, uint32 index, Fn* callback)
    {
        (*callback)(var, index);
    }

    // IterateArray knows how to iterate over plain Js::JavascriptArray as well as Js::ES5Array.
    // If arr is a simple Js::JavascriptArray, we will enumerate the data properties of the array.
    // If arr is a Js::ES5Array, we will enumerate both data properties and indexed properties with getter/setters.
    template <typename Fn>
    static void IterateArray(Js::JavascriptArray* arr, Fn callback)
    {
        Assert(arr);
        Assert(!Js::JavascriptNativeIntArray::Is(arr) && !Js::JavascriptNativeFloatArray::Is(arr));

        uint32 index = Js::JavascriptArray::InvalidIndex;
        uint32 dataIndex = Js::JavascriptArray::InvalidIndex;
        uint32 descriptorIndex = Js::JavascriptArray::InvalidIndex;
        Js::ES5Array* es5Arr = NULL;

        if (Js::ES5Array::Is(arr))
        {
            es5Arr = Js::ES5Array::FromVar(arr);
        }

        while (true)
        {
            if (index == dataIndex)
            {
                dataIndex = arr->GetNextIndex(dataIndex);
            }
            if (NULL != es5Arr && index == descriptorIndex)
            {
                // If we haven't begun searching through the es5 array yet, start looking at index 0.
                // Otherwise, begin search at the next index to avoid returning the same descriptor again.
                uint32 i = descriptorIndex == Js::JavascriptArray::InvalidIndex ? 0 : descriptorIndex + 1;

                // We cannot use something simple like GetNextDescriptor because it allocates memory using the Recycler.
                // Instead, walk across the array searching for items.
                for (i; i < es5Arr->GetLength(); i++)
                {
                    if (es5Arr->HasItem(i))
                    {
                        descriptorIndex = i;
                        break;
                    }
                }

                // If we went through all the items of es5Arr, we are done.
                if (es5Arr->GetLength() <= i)
                {
                    descriptorIndex = Js::JavascriptArray::InvalidIndex;
                }
            }

            // Try to report properties in key order without creating an index.
            index = min(dataIndex, descriptorIndex);
            if (index == Js::JavascriptArray::InvalidIndex) // End of array
            {
                break;
            }

            // Use to store data item or getter+setter.
            Js::Var var1 = NULL;
            Js::Var var2 = NULL;

            // DirectGetItemAt will return us a data item.
            if (! arr->DirectGetVarItemAt(index, &var1, arr->GetScriptContext()) && NULL != es5Arr)
            {
                // If DirectGetItemAt fails, the indexed property has getter/setter.
                Js::IndexPropertyDescriptor *descriptor = NULL;

                // Fetch the descriptor, and get the getter/setter out of it.
                if (es5Arr->GetDescriptor(index, &descriptor) && NULL != descriptor && ! (descriptor->Attributes & PropertyDeleted))
                {
                    var1 = descriptor->Getter;
                    var2 = descriptor->Setter;
                }
            }

            // According to spec, we skip undefined elements in arrays for purpose
            // of sparse arrays. Note, we don't skip undefined properties.
            if (NULL != var1 && ! Js::JavascriptOperators::IsUndefinedObject(var1))
            {
                IterateArrayHelper(var1, index, &callback);
            }
            if (NULL != var2 && ! Js::JavascriptOperators::IsUndefinedObject(var2))
            {
                IterateArrayHelper(var2, index, &callback);
            }
        }
    }

    // IterateArray knows how to iterate over a native array.
    // We will enumerate the data properties of the array.
    template <typename TNativeArray, typename Fn>
    static void IterateNativeArray(TNativeArray* arr, Fn VisitElement)
    {
        TemplateParameter::SameOrDerivedFrom<TNativeArray, Js::JavascriptNativeArray>();
        Assert(arr);

        uint32 index = TNativeArray::InvalidIndex;
        while (true)
        {
            index = arr->GetNextIndex(index);

            // Try to report properties in key order without creating an index.
            if (index == Js::JavascriptArray::InvalidIndex) // End of array
            {
                break;
            }

            typename TNativeArray::TElement elementValue;
            if (arr->DirectGetItemAt(index, &elementValue))
            {
                VisitElement(elementValue, index);
            }
        }
    }

    bool IsScopeSlotArray(ULONG flags) const { return (flags & PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_SCOPE_SLOT_ARRAY) != 0; }
    bool IsRecyclableObject(ULONG flags) const { return ! IsScopeSlotArray(flags) && !IsBoundFunctionArgs(flags); }
    bool IsSiteClosed(ULONG flags) const { return (flags & PROFILER_HEAP_OBJECT_FLAGS_SITE_CLOSED) != 0; }
    bool IsJavascriptString(Js::RecyclableObject* obj);
    bool IsJavascriptString(Js::Var obj) { Assert(Js::RecyclableObject::Is(obj)); return IsJavascriptString(Js::RecyclableObject::FromVar(obj)); }
    bool IsBoundFunction(Js::Var obj) { return Js::RecyclableObject::Is(obj) && Js::JavascriptFunction::Is(obj) && (Js::JavascriptFunction::FromVar(obj))->IsBoundFunction(); }
    bool IsBoundFunctionArgs(ULONG flags) const { return (flags & PROFILER_HEAP_OBJECT_INTERNAL_FLAGS_BOUND_FUNCTION_ARGUMENT_LIST) != 0; }

    ProfilerHeapObject* CreateElement(void* obj, size_t size, ULONG flags, UINT numberOfElements = 0);
    ProfilerHeapObject* CreateJavascriptElement(Js::RecyclableObject *obj, PROFILER_HEAP_OBJECT_NAME_ID typeNameId, HostProfilerHeapObject* ext);
    ProfilerHeapObject* CreateObjectElement(Js::RecyclableObject* obj, size_t size);
    ProfilerHeapObject* CreateBoundFunctionArgsElement(Var* obj, UINT allocatedSizeInGC, UINT numberOfElements);
    ProfilerHeapObject* CreateScopeSlotArrayElement(void** scopeSlotArray, UINT allocatedSize);
    HostProfilerHeapObject* CreateExternalObjectElement(PROFILER_HEAP_OBJECT_NAME_ID typeNameId, Js::RecyclableObject* obj);
    ProfilerHeapObject* AllocateElement(UINT allocSize, PROFILER_HEAP_OBJECT_NAME_ID typeNameId);
    HRESULT GetHeapObjectInfo(Var instance, HostProfilerHeapObject** heapObjOut, HeapObjectInfoReturnResult& returnResult);
    ProfilerHeapObjectOptionalInfo* ActiveScriptProfilerHeapEnum::GetNextOptionalInfo(ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FreeObjectAndOptionalInfo(ProfilerHeapObject* obj);
    void FreeHostObjectExternalObjectList(HostProfilerHeapObject& hostInfo);

    UINT GetObjectSize(void* obj, size_t size);
    LPCWSTR GetFunctionName(Js::RecyclableObject* obj);
    UINT GetNamePropertyCount(Js::RecyclableObject* obj);
    Js::ArrayObject* GetIndexPropertyArray(Js::RecyclableObject* obj);
    UINT GetIndexPropertyCount(Js::RecyclableObject* obj);
    uint16 GetScopeCount(Js::RecyclableObject* obj);
    UINT GetNamePropertySlotCount(Js::RecyclableObject* obj, Js::PropertyId relationshipId);
    void FillScopes(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo);
    Js::RecyclableObject* GetPrototype(Js::RecyclableObject* obj);
    USHORT GetInternalPropertyCount(Js::RecyclableObject* obj);
    UINT GetMapCollectionCount(Js::JavascriptMap* map);
    UINT GetSetCollectionCount(Js::JavascriptSet* set);
    UINT GetWeakMapCollectionCount(Js::JavascriptWeakMap* weakMap);
    UINT GetWeakSetCollectionCount(Js::JavascriptWeakSet* weakSet);

    // Can't report the string value if it isn't available, eg. unfinalized concat string
    bool IsReportableJavascriptString(Js::RecyclableObject *obj) { return IsJavascriptString(obj) && Js::JavascriptString::FromVar(obj)->UnsafeGetBuffer() != NULL; }
    void FillInternalProperty(Js::RecyclableObject* property, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FillNameProperties(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FillIndexProperties(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FillMapCollectionList(Js::JavascriptMap* map, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FillSetCollectionList(Js::JavascriptSet* set, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FillWeakMapCollectionList(Js::JavascriptWeakMap* weakMap, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FillWeakSetCollectionList(Js::JavascriptWeakSet* weakSet, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FillRelationships(Js::RecyclableObject* obj, ProfilerHeapObjectOptionalInfo* optionalInfo);
    void FreeBSTR(HostProfilerHeapObject *externalObject);
    void FreeSubString(ProfilerHeapObject* obj);
    void FillProperty(Js::ScriptContext* scriptContext, Js::Var property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element);
    void FillNumberProperty(int32 property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element);
    void FillNumberProperty(double property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element);
    void FillStringProperty(LPCWSTR property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element);
    void FillObjectProperty(Js::RecyclableObject* property, PROFILER_HEAP_OBJECT_RELATIONSHIP& element);
    void FillNumberRelationship(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_NAME_ID relationshipId, int32 value);
    void FillStringRelationship(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_NAME_ID relationshipId, LPCWSTR value);
    void FillObjectRelationship(PROFILER_HEAP_OBJECT_RELATIONSHIP& relationship, PROFILER_HEAP_OBJECT_NAME_ID relationshipId, Js::RecyclableObject* value);
    static bool GetModuleBounds(INT_PTR& dllLoadAddress, INT_PTR& dllHighAddress);
    void AddObjectToSummary(ProfilerHeapObject* obj, PROFILER_HEAP_SUMMARY* pHeapSummary);
    template <typename Fn>
    bool EnumerateHeapHelper(ProfilerHeapObject* obj, Fn* callback);
    template <typename Fn>
    void EnumerateHeap(Fn callback, PROFILER_HEAP_SUMMARY* pHeapSummary=NULL);
    void EnqueueExternalObjects(HostProfilerHeapObject* hostInfo);
    void Visit(void* obj, ULONG flags=0, UINT numberOfElements = 0);
    void VisitRoot(JavascriptDispatch* javascriptDispatch);
    void VisitRelationshipList(PROFILER_HEAP_OBJECT_RELATIONSHIP_LIST& list);
    template <typename T>
    void VisitDependencies(T* obj, USHORT optionalInfoCount, ULONG flags);
    ULONG ActiveScriptProfilerHeapEnum::GetInternalPropertyFlags(PROFILER_HEAP_OBJECT_RELATIONSHIP *internalProp);
    void VisitAllDependencies(ProfilerHeapObject* obj);
    template <typename T>
    void GetOptionalInfo(__in T* obj,
        __in ULONG celt,
        __out_ecount(celt) PROFILER_HEAP_OBJECT_OPTIONAL_INFO optionalInfo[]);

    Js::PropertyId GetPropertyIdByIndex(Js::RecyclableObject * obj, Js::PropertyIndex i);
    uint GetEnginePropertyCount();

public:
    DECLARE_IUNKNOWN();
    ActiveScriptProfilerHeapEnum(Recycler& recycler, ScriptEngine& scriptEngine, ArenaAllocator* allocator, bool preEnumHeap2Behavior, PROFILER_HEAP_ENUM_FLAGS enumFlags);
    ~ActiveScriptProfilerHeapEnum();

    STDMETHOD(Summarize)(PROFILER_HEAP_SUMMARY* pHeapSummary);

    STDMETHOD(Init)();

    STDMETHOD(Next)(__in ULONG celt,
        __out_ecount_part(celt, *pceltFetched) PROFILER_HEAP_OBJECT* elements[],
        __out ULONG *pceltFetched);

    STDMETHOD(GetOptionalInfo)(__in PROFILER_HEAP_OBJECT* object,
        __in ULONG celt,
        __out_ecount(celt) PROFILER_HEAP_OBJECT_OPTIONAL_INFO optionalInfo[]);

    STDMETHOD(FreeObjectAndOptionalInfo)(__in ULONG celt,
        __in_ecount(celt) PROFILER_HEAP_OBJECT* optionalInfo[]);

    STDMETHOD(GetNameIdMap)(
        __out_ecount(*pcelt) LPCWSTR* pPropertyIdMap[],
        __out UINT *pcelt);

#ifdef ENABLE_TEST_HOOKS
    static void SetGetHeapObjectInfoPtr(GetHeapObjectInfoPtr callback)
    {
        pfGetHeapObjectInfo = callback;
    }
#endif

#ifdef HEAP_ENUMERATION_VALIDATION
    static void EnsureRecyclableObjectsAreVisitedCallback(const RecyclerHeapObjectInfo& heapObject, void *data);
    class VtableMap
    {
        VtableHashMap *m_vtableMapHash;
        PageAllocator pageAllocator;
        ArenaAllocator arenaAllocator;
        public:
        VtableMap();
        ~VtableMap();
        void Initialize();
        bool TryGetValue(INT_PTR key, LPCSTR* value) { return m_vtableMapHash->TryGetValue(key, value); }
    };
    VtableMap m_vtableMap;
    UINT enumerationCount;
#endif
};
