// Copyright (C) Microsoft. All rights reserved. 

#pragma once

namespace Js
{
    struct JitTimeConstructorCache 
    {
        // TODO (FixedNewObj): Consider making these private and provide getters to ensure the values are not changed.
        const JavascriptFunction* constructor;
        ConstructorCache* runtimeCache;

        const ScriptContext* scriptContext;
        const DynamicType* type;
        BVSparse<JitArenaAllocator>* guardedPropOps;
        int slotCount;
        int16 inlineSlotCount;

        bool skipNewScObject;
        bool ctorHasNoExplicitReturnValue;
        bool typeIsFinal;
        bool isUsed;

    public:
        JitTimeConstructorCache(const JavascriptFunction* constructor, ConstructorCache* runtimeCache)
        {
            Assert(constructor != null);
            Assert(runtimeCache != null);
            this->constructor = constructor;
            this->runtimeCache = runtimeCache;
            this->type = runtimeCache->content.type;
            this->guardedPropOps = null;
            this->scriptContext = runtimeCache->content.scriptContext;
            this->slotCount = runtimeCache->content.slotCount;
            this->inlineSlotCount = runtimeCache->content.inlineSlotCount;
            this->skipNewScObject = runtimeCache->content.skipDefaultNewObject;
            this->ctorHasNoExplicitReturnValue = runtimeCache->content.ctorHasNoExplicitReturnValue;
            this->typeIsFinal = runtimeCache->content.typeIsFinal;
            this->isUsed = false;
        }

        JitTimeConstructorCache(const JitTimeConstructorCache* other)
        {
            Assert(other != null);
            Assert(other->constructor != null);
            Assert(other->runtimeCache != null);
            this->constructor = other->constructor;
            this->runtimeCache = other->runtimeCache;
            this->type = other->type;
            this->guardedPropOps = null;
            this->scriptContext = other->scriptContext;
            this->slotCount = other->slotCount;
            this->inlineSlotCount = other->inlineSlotCount;
            this->skipNewScObject = other->skipNewScObject;
            this->ctorHasNoExplicitReturnValue = other->ctorHasNoExplicitReturnValue;
            this->typeIsFinal = other->typeIsFinal;
            this->isUsed = false;
        }

        JitTimeConstructorCache* Clone(JitArenaAllocator* allocator)
        {
            JitTimeConstructorCache* clone = Anew(allocator, JitTimeConstructorCache, this);
            return clone;
        }

        const BVSparse<JitArenaAllocator>* GetGuardedPropOps() const
        {
            return this->guardedPropOps;
        }

        void EnsureGuardedPropOps(JitArenaAllocator* allocator)
        {
            if (this->guardedPropOps == null)
            {
                this->guardedPropOps = Anew(allocator, BVSparse<JitArenaAllocator>, allocator);
            }
        }

        void SetGuardedPropOp(uint propOpId)
        {
            Assert(this->guardedPropOps != null);
            this->guardedPropOps->Set(propOpId);
        }

        void AddGuardedPropOps(const BVSparse<JitArenaAllocator>* propOps)
        {
            Assert(this->guardedPropOps != null);
            this->guardedPropOps->Or(propOps);
        }

        bool TryGetObjSizeForAllocFromNativeCode(bool allowAuxSlots, size_t& headerAllocSize, size_t& auxSlotsAllocSize) const
        {
            if (slotCount > inlineSlotCount && !allowAuxSlots)
            {
                return false;
            }

            headerAllocSize = sizeof(Js::DynamicObject) + inlineSlotCount * sizeof(Js::Var);
            if (headerAllocSize > MAXINT32 || !HeapInfo::IsAlignedSize(headerAllocSize) || !HeapInfo::IsSmallObject(headerAllocSize))
            {
                AssertMsg(false, "Object header too large or not recycler heap block aligned.");
                return false;
            }

            auxSlotsAllocSize = slotCount > inlineSlotCount ? (slotCount - inlineSlotCount) * sizeof(Js::Var) : 0;
            if (auxSlotsAllocSize > 0 && (auxSlotsAllocSize > MAXINT32 || !HeapInfo::IsAlignedSize(auxSlotsAllocSize)))
            {
                AssertMsg(false, "Object header too large or not recycler heap block aligned.");
                return false;
            }

            // Don't assert here because we allow objects with enough slots as to exceed the small block allocation size.
            if (!HeapInfo::IsSmallObject(auxSlotsAllocSize))
            {
                return false;
            }
        }
    };


#define InitialObjTypeSpecFldInfoFlagValue 0x01

    struct FixedFieldInfo
    {
        Var fieldValue;
        Type* type;
        bool nextHasSameFixedField; // set to true if the next entry in the FixedFieldInfo array on ObjTypeSpecFldInfo has the same type
    };

    class ObjTypeSpecFldInfo
    {
    private:
        DynamicObject* protoObject;
        PropertyGuard* propertyGuard;
        EquivalentTypeSet* typeSet;
        Type* initialType;
        JitTimeConstructorCache* ctorCache;
        FixedFieldInfo* fixedFieldInfoArray;

        PropertyId propertyId;
        Js::TypeId typeId;
        uint id;

        // Union with uint16 flags for fast default initialization
        union {
            struct {
                bool falseReferencePreventionBit : 1;
                bool isPolymorphic: 1;
                bool isRootObjectNonConfigurableField: 1;
                bool isRootObjectNonConfigurableFieldLoad: 1;
                bool usesAuxSlot: 1;
                bool isLocal: 1;
                bool isLoadedFromProto: 1;
                bool usesAccessor: 1;
                bool hasFixedValue: 1;
                bool keepFieldValue: 1;
                bool isBeingStored: 1;
                bool isBeingAdded: 1;
                bool doesntHaveEquivalence: 1;
            };
            struct {
                uint16 flags;
            };
        };

        uint16 slotIndex;
       
        uint16 fixedFieldCount; // currently used only for fields that are functions

    public:
        ObjTypeSpecFldInfo():
            id(0), typeId(TypeIds_Limit), typeSet(null), initialType(null), flags(InitialObjTypeSpecFldInfoFlagValue), 
            slotIndex(Constants::NoSlot), propertyId(Constants::NoProperty), protoObject(null), propertyGuard(null), 
            ctorCache(null), fixedFieldInfoArray(null) {}

        ObjTypeSpecFldInfo(uint id, TypeId typeId, Type* initialType,
            bool usesAuxSlot, bool isLoadedFromProto, bool usesAccessor, bool isFieldValueFixed, bool keepFieldValue,
            uint16 slotIndex, PropertyId propertyId, DynamicObject* protoObject, PropertyGuard* propertyGuard,
            JitTimeConstructorCache* ctorCache, FixedFieldInfo* fixedFieldInfoArray) :
            id(id), typeId(typeId), typeSet(null), initialType(initialType), flags(InitialObjTypeSpecFldInfoFlagValue),
            slotIndex(slotIndex), propertyId(propertyId), protoObject(protoObject), propertyGuard(propertyGuard),
            ctorCache(ctorCache), fixedFieldInfoArray(fixedFieldInfoArray)
        {
            this->isPolymorphic = false;
            this->usesAuxSlot = usesAuxSlot;
            this->isLocal = !isLoadedFromProto && !usesAccessor;
            this->isLoadedFromProto = isLoadedFromProto;
            this->usesAccessor = usesAccessor;
            this->hasFixedValue = isFieldValueFixed;
            this->keepFieldValue = keepFieldValue;
            this->isBeingAdded = initialType != null;
            this->doesntHaveEquivalence = true; //doesn't mean anything for data from a monomorphic cache
            this->fixedFieldCount = 1;
        }

        ObjTypeSpecFldInfo(uint id, TypeId typeId, Type* initialType, EquivalentTypeSet* typeSet,
            bool usesAuxSlot, bool isLoadedFromProto, bool usesAccessor, bool isFieldValueFixed, bool keepFieldValue, bool doesntHaveEquivalence, bool isPolymorphic,
            uint16 slotIndex, PropertyId propertyId, DynamicObject* protoObject, PropertyGuard* propertyGuard,
            JitTimeConstructorCache* ctorCache, FixedFieldInfo* fixedFieldInfoArray, uint16 fixedFieldCount) :
            id(id), typeId(typeId), typeSet(typeSet), initialType(initialType), flags(InitialObjTypeSpecFldInfoFlagValue),
            slotIndex(slotIndex), propertyId(propertyId), protoObject(protoObject), propertyGuard(propertyGuard),
            ctorCache(ctorCache), fixedFieldInfoArray(fixedFieldInfoArray)
        {
            this->isPolymorphic = isPolymorphic;
            this->usesAuxSlot = usesAuxSlot;
            this->isLocal = !isLoadedFromProto && !usesAccessor;
            this->isLoadedFromProto = isLoadedFromProto;
            this->usesAccessor = usesAccessor;
            this->hasFixedValue = isFieldValueFixed;
            this->keepFieldValue = keepFieldValue;
            this->isBeingAdded = initialType != null;
            this->doesntHaveEquivalence = doesntHaveEquivalence;
            this->fixedFieldCount = fixedFieldCount;
        }

        static ObjTypeSpecFldInfo* CreateFrom(uint id, InlineCache* cache, uint cacheId, 
            EntryPointInfo *entryPoint, FunctionBody* const topFunctionBody, FunctionBody *const functionBody, FieldAccessStatsPtr inlineCacheStats);

        static ObjTypeSpecFldInfo* CreateFrom(uint id, PolymorphicInlineCache* cache, uint cacheId, 
            EntryPointInfo *entryPoint, FunctionBody* const topFunctionBody, FunctionBody *const functionBody, FieldAccessStatsPtr inlineCacheStats);

        uint GetObjTypeSpecFldId() const
        {
            return this->id;
        }

        bool IsMono() const 
        {
            return !this->isPolymorphic;
        }

        bool IsPoly() const 
        {
            return this->isPolymorphic;
        }

        bool UsesAuxSlot() const 
        {
            return this->usesAuxSlot;
        }

        void SetUsesAuxSlot(bool value)
        {
            this->usesAuxSlot = value;
        }

        bool IsLoadedFromProto() const
        {
            return this->isLoadedFromProto;
        }

        bool IsLocal() const
        {
            return this->isLocal;
        }

        bool UsesAccessor() const 
        {
            return this->usesAccessor;
        }

        bool HasFixedValue() const
        {
            return this->hasFixedValue;
        }

        void SetHasFixedValue(bool value)
        {
            this->hasFixedValue = value;
        }

        bool IsBeingStored() const
        {
            return this->isBeingStored;
        }

        void SetIsBeingStored(bool value)
        {
            this->isBeingStored = value;
        }

        bool IsBeingAdded() const
        {
            return this->isBeingAdded;
        }

        void SetIsBeingAdded(bool value)
        {
            this->isBeingAdded = value;
        }

        bool IsRootObjectNonConfigurableField() const
        {
            return this->isRootObjectNonConfigurableField;
        }

        bool IsRootObjectNonConfigurableFieldLoad() const
        {
            return this->isRootObjectNonConfigurableField && this->isRootObjectNonConfigurableFieldLoad;
        }

        void SetRootObjectNonConfigurableField(bool isLoad)
        {
            this->isRootObjectNonConfigurableField = true;
            this->isRootObjectNonConfigurableFieldLoad = isLoad;
        }

        bool DoesntHaveEquivalence() const
        {
            return this->doesntHaveEquivalence;
        }

        void ClearFlags()
        {
            this->flags = 0;
        }

        void SetFlags(uint16 flags)
        {
            this->flags = flags | 0x01;
        }

        uint16 GetSlotIndex() const
        {
            return this->slotIndex;
        }

        void SetSlotIndex(uint16 index)
        {
            this->slotIndex = index;
        }

        PropertyId GetPropertyId() const
        {
            return this->propertyId;
        }

        Js::DynamicObject* GetProtoObject() const 
        {
            Assert(IsLoadedFromProto());
            return this->protoObject;
        }

        Var GetFieldValue() const
        {
            Assert (IsMono() || (IsPoly() && !DoesntHaveEquivalence()));
            return this->fixedFieldInfoArray[0].fieldValue;
        }

        Var GetFieldValue(uint i) const
        {
            Assert(IsPoly());
            return this->fixedFieldInfoArray[i].fieldValue;
        }

        void SetFieldValue(Var value)
        {
            Assert (IsMono() || (IsPoly() && !DoesntHaveEquivalence()));
            this->fixedFieldInfoArray[0].fieldValue = value;
        }

        Var GetFieldValueAsFixedDataIfAvailable() const;

        Js::JavascriptFunction* GetFieldValueAsFixedFunction() const;
        Js::JavascriptFunction* GetFieldValueAsFixedFunction(uint i) const;

        Js::JavascriptFunction* GetFieldValueAsFunction() const;

        Js::JavascriptFunction* GetFieldValueAsFunctionIfAvailable() const;
        
        Js::JavascriptFunction* GetFieldValueAsFixedFunctionIfAvailable() const;
        Js::JavascriptFunction* GetFieldValueAsFixedFunctionIfAvailable(uint i) const;

        bool GetKeepFieldValue() const
        {
            return this->keepFieldValue;
        }

        Js::JitTimeConstructorCache* GetCtorCache() const
        {
            return this->ctorCache;
        }

        Js::PropertyGuard* GetPropertyGuard() const 
        {
            return this->propertyGuard;
        }

        bool IsObjTypeSpecCandidate() const
        {
            return true;
        }

        bool IsMonoObjTypeSpecCandidate() const
        {
            return IsObjTypeSpecCandidate() && IsMono();
        }

        bool IsPolyObjTypeSpecCandidate() const
        {
            return IsObjTypeSpecCandidate() && IsPoly();
        }

        Js::TypeId GetTypeId() const
        {
            Assert(typeId != TypeIds_Limit);
            return this->typeId;
        }

        Js::TypeId GetTypeId(uint i) const
        {
            Assert(IsPoly());
            return this->fixedFieldInfoArray[i].type->GetTypeId();
        }

        Js::Type * GetType() const
        {
            Assert(IsObjTypeSpecCandidate() && IsMono());
            return this->fixedFieldInfoArray[0].type;
        }

        Js::Type * GetType(uint i) const
        {
            Assert(IsPoly());
            return this->fixedFieldInfoArray[i].type;
        }

        bool HasInitialType() const
        {
            return IsObjTypeSpecCandidate() && IsMono() && !IsLoadedFromProto() && this->initialType != nullptr;
        }

        Js::Type * GetInitialType() const
        {
            Assert(IsObjTypeSpecCandidate() && IsMono() && !IsLoadedFromProto());
            return this->initialType;
        }

        bool HasEquivalentTypeSet() const
        {
            Assert(IsObjTypeSpecCandidate());
            return this->typeSet != nullptr;
        }

        Js::EquivalentTypeSet * GetEquivalentTypeSet() const
        {
            Assert(IsObjTypeSpecCandidate());
            return this->typeSet;
        }

        Js::Type * GetFirstEquivalentType() const
        {
            Assert(IsObjTypeSpecCandidate() && this->typeSet);
            return this->typeSet->GetFirstType();
        }

        Js::FixedFieldInfo* GetFixedFieldInfoArray()
        {
            return this->fixedFieldInfoArray;
        }

        uint16 GetFixedFieldCount()
        {
            return this->fixedFieldCount;
        }
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        const wchar_t *GetCacheLayoutString() const;
#endif

    };


    class ObjTypeSpecFldInfoArray
    {
    private:
        ObjTypeSpecFldInfo** infoArray;
#if DBG
        uint infoCount;
#endif
    public:
        ObjTypeSpecFldInfoArray();

    private:
        void EnsureArray(Recycler *const recycler, FunctionBody *const functionBody);

    public:
        ObjTypeSpecFldInfo* GetInfo(FunctionBody *const functionBody, const uint index) const;

        void SetInfo(Recycler *const recycler, FunctionBody *const functionBody,
            const uint index, ObjTypeSpecFldInfo* info);
        
        void Reset();

        template <class Fn>
        void Map(Fn fn, uint count) const
        {
            if (this->infoArray != null)
            {
                for (uint i = 0; i < count; i++)
                {
                    ObjTypeSpecFldInfo* info = this->infoArray[i];

                    if (info != null)
                    {
                        fn(info);
                    }
                }
            }
        };

        PREVENT_COPY(ObjTypeSpecFldInfoArray)
    };



    // - Data generated for jitting purposes
    // - Recycler-allocated, lifetime is from when a code gen work item is added to the jit queue, to when jitting is complete
    //     - Also keeps the function body and inlinee function bodies alive while jitting.

    class FunctionCodeGenJitTimeData
    {
    private:
        FunctionInfo *const functionInfo;

        // Point's to an entry point if the work item needs the entry point alive- null for cases where the entry point isn't used
        EntryPointInfo *const entryPointInfo; 

        // These cloned inline caches are guaranteed to have stable data while jitting, but will be collectible after jitting
        ObjTypeSpecFldInfoArray objTypeSpecFldInfoArray;

        // Globally ordered list of all object type specialized property access information (monomorphic and polymorphic caches combined).
        uint globalObjTypeSpecFldInfoCount;
        ObjTypeSpecFldInfo** globalObjTypeSpecFldInfoArray;

        // There will be a non-null entry for each profiled call site where a function is to be inlined
        FunctionCodeGenJitTimeData **inlinees;
        FunctionCodeGenJitTimeData **ldFldInlinees;
        RecyclerWeakReference<FunctionBody> *weakFuncRef;

        // Number of functions that are to be inlined (this is not the length of the 'inlinees' array above, inludes getter setter inlinee count)
        uint inlineeCount;
        // Number of counts of getter setter to be inlined. This is not an exact count as inline caches are shared and we have no way of knowing 
        // accurate count.
        uint ldFldInlineeCount;

        //For polymorphic call site we will have linked list of FunctionCodeGenJitTimeData
        //Each is differeniated by id starting from 0, 1
        FunctionCodeGenJitTimeData *next;
        bool isInlined;

        //This indicates the function is aggresively Inlined(see NativeCodeGenerator::TryAggressiveInlining) .
        bool isAggressiveInliningEnabled;

        // The profiled iterations need to be determined at the time of gathering code gen data on the main thread
        const uint16 profiledIterations;

#ifdef FIELD_ACCESS_STATS
    public:
        FieldAccessStatsPtr inlineCacheStats;

        void EnsureInlineCacheStats(Recycler* recycler);
        void AddInlineeInlineCacheStats(FunctionCodeGenJitTimeData* inlineeJitTimeData);
#endif

    public:
        FunctionCodeGenJitTimeData(FunctionInfo *const functionInfo, EntryPointInfo *const entryPoint, bool isInlined = true);

    public:
        BVFixed *inlineesBv;

        FunctionInfo *GetFunctionInfo() const;
        FunctionBody *GetFunctionBody() const;
        FunctionCodeGenJitTimeData *GetNext() const { return next;};

        const ObjTypeSpecFldInfoArray* GetObjTypeSpecFldInfoArray() const { return &this->objTypeSpecFldInfoArray; }
        ObjTypeSpecFldInfoArray* GetObjTypeSpecFldInfoArray() { return &this->objTypeSpecFldInfoArray; }
        EntryPointInfo* GetEntryPointInfo() const { return this->entryPointInfo; }

    public:
        const FunctionCodeGenJitTimeData *GetInlinee(const ProfileId profiledCallSiteId) const;
        const FunctionCodeGenJitTimeData *GetLdFldInlinee(const InlineCacheIndex inlineCacheIndex) const;
        FunctionCodeGenJitTimeData *AddInlinee(
            Recycler *const recycler,
            const ProfileId profiledCallSiteId,
            FunctionInfo *const inlinee,
            bool isInlined = true);
        uint InlineeCount() const;
        bool isLdFldInlineePresent() const { return ldFldInlineeCount != 0;}

        RecyclerWeakReference<FunctionBody> *GetWeakFuncRef() const { return this->weakFuncRef; }
        void SetWeakFuncRef(RecyclerWeakReference<FunctionBody> *weakFuncRef)
        {
            Assert(this->weakFuncRef == null || weakFuncRef == null || this->weakFuncRef == weakFuncRef);
            this->weakFuncRef = weakFuncRef;
        }

        FunctionCodeGenJitTimeData *AddLdFldInlinee(
            Recycler *const recycler,
            const InlineCacheIndex inlineCacheIndex,
            FunctionInfo *const inlinee);

       bool IsPolymorphicCallSite(const ProfileId profiledCallSiteId) const;
       //This function walks all the chained jittimedata and returns the one which match the functionInfo.
       //This can return null, if the functionInfo doesn't match.
       const FunctionCodeGenJitTimeData *GetJitTimeDataFromFunctionInfo(FunctionInfo *polyFunctionInfo) const;

       ObjTypeSpecFldInfo* GetGlobalObjTypeSpecFldInfo(uint propertyInfoId) const
       { 
           Assert(this->globalObjTypeSpecFldInfoArray != null && propertyInfoId < this->globalObjTypeSpecFldInfoCount); 
           return this->globalObjTypeSpecFldInfoArray[propertyInfoId]; 
       }

       void SetGlobalObjTypeSpecFldInfo(uint propertyInfoId, ObjTypeSpecFldInfo* info) const
       { 
           Assert(this->globalObjTypeSpecFldInfoArray != null && propertyInfoId < this->globalObjTypeSpecFldInfoCount); 
           this->globalObjTypeSpecFldInfoArray[propertyInfoId] = info; 
       }

       void SetGlobalObjTypeSpecFldInfoArray(ObjTypeSpecFldInfo** array, uint count)
       {
           Assert(array != null);
           this->globalObjTypeSpecFldInfoArray = array;
           this->globalObjTypeSpecFldInfoCount = count;
       }

       bool GetIsInlined() const
       {
           return isInlined;
       }
       bool GetIsAggressiveInliningEnabled() const
       {
           return isAggressiveInliningEnabled;
       }
       void SetIsAggressiveInliningEnabled()
       {
           isAggressiveInliningEnabled = true;
       }

       void SetupRecursiveInlineeChain(
           Recycler *const recycler,
           const ProfileId profiledCallSiteId)
       {

           if (!inlinees)
           {
               inlinees = RecyclerNewArrayZ(recycler, FunctionCodeGenJitTimeData *, GetFunctionBody()->GetProfiledCallSiteCount());
           }
           inlinees[profiledCallSiteId] = this;
           inlineeCount++;
           this->isInlined = isInlined;
       }


       uint16 GetProfiledIterations() const;

       PREVENT_COPY(FunctionCodeGenJitTimeData)
    };

}

