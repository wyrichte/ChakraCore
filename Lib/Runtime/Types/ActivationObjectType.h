//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class ActivationObject : public DynamicObject
    {
    protected:
        DEFINE_VTABLE_CTOR(ActivationObject, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ActivationObject);
    public:
        ActivationObject(DynamicType * type) : DynamicObject(type)
        {}

        virtual BOOL HasOwnPropertyCheckNoRedecl(PropertyId propertyId) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetInternalProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL EnsureProperty(PropertyId propertyId) override;
        virtual BOOL EnsureNoRedeclProperty(PropertyId propertyId) override;
        virtual BOOL InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags = PropertyOperation_None, PropertyValueInfo* info = NULL) override;
        virtual BOOL InitPropertyScoped(PropertyId propertyId, Var value) override;
        virtual BOOL InitFuncScoped(PropertyId propertyId, Var value) override;
        virtual BOOL DeleteItem(uint32 index, PropertyOperationFlags flags) override;
        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;       
        virtual DynamicObject* MakeCopyOnWriteObject(ScriptContext* scriptContext) override;        
        static bool Is(void* instance);
    };

    // A block-ActivationObject is a scope for an ES6 block that should only receive block-scoped inits,
    // including function, let, and const.
    class BlockActivationObject : public ActivationObject
    {
    private:
        DEFINE_VTABLE_CTOR(BlockActivationObject, ActivationObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(BlockActivationObject);
    public:
        BlockActivationObject(DynamicType * type) : ActivationObject(type) {}

        virtual BOOL EnsureProperty(PropertyId propertyId) override;
        virtual BOOL EnsureNoRedeclProperty(PropertyId propertyId) override;
        virtual BOOL InitPropertyScoped(PropertyId propertyId, Var value) override;
        virtual BOOL InitFuncScoped(PropertyId propertyId, Var value) override;
        static bool Is(void* instance)
        {
            return VirtualTableInfo<Js::BlockActivationObject>::HasVirtualTable(instance);
        }
    };

    // A pseudo-ActivationObject is a scope like a "catch" scope that shouldn't receive var inits.
    class PseudoActivationObject : public ActivationObject
    {
    private:
        DEFINE_VTABLE_CTOR(PseudoActivationObject, ActivationObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(PseudoActivationObject);
    public:
        PseudoActivationObject(DynamicType * type) : ActivationObject(type) {}

        virtual BOOL EnsureProperty(PropertyId propertyId) override;
        virtual BOOL EnsureNoRedeclProperty(PropertyId propertyId) override;
        virtual BOOL InitFuncScoped(PropertyId propertyId, Var value) override;
        virtual BOOL InitPropertyScoped(PropertyId propertyId, Var value) override;
        static bool Is(void* instance)
        {
            return VirtualTableInfo<Js::PseudoActivationObject>::HasVirtualTable(instance);
        }
    };

    class ActivationObjectEx : public ActivationObject
    {
    private:
        DEFINE_VTABLE_CTOR(ActivationObjectEx, ActivationObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ActivationObjectEx);

        void GetPropertyCore(PropertyValueInfo *info, ScriptContext *requestContext);
    public:
        ActivationObjectEx(
            DynamicType * type, ScriptFunction *func, uint cachedFuncCount, uint firstFuncSlot, uint lastFuncSlot)
            : ActivationObject(type),
              parentFunc(func),
              cachedFuncCount(cachedFuncCount),
              firstFuncSlot(firstFuncSlot),
              lastFuncSlot(lastFuncSlot),
              committed(false)
        {
            if (cachedFuncCount != 0)
            {
                cache[0].func = null;
            }
        }

        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var *value, PropertyValueInfo *info, ScriptContext *requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var *value, PropertyValueInfo *info, ScriptContext *requestContext) override;
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var *value, PropertyValueInfo *info, ScriptContext *requestContext) override;
        virtual void InvalidateCachedScope() override sealed;

        bool IsCommitted() const { return committed; }
        void SetCommit(bool set) { committed = set; }
        ScriptFunction *GetParentFunc() const { return parentFunc; }
        uint GetFirstFuncSlot() const { return firstFuncSlot; }
        uint GetLastFuncSlot() const { return lastFuncSlot; }
        bool HasCachedFuncs() const { return cachedFuncCount != 0 && cache[0].func != null; }

        void SetCachedFunc(uint i, ScriptFunction *func);

        FuncCacheEntry *GetFuncCacheEntry(uint i)
        {
            Assert(i < cachedFuncCount);
            return &cache[i];
        }

        static uint32 GetOffsetOfCache() { return offsetof(ActivationObjectEx, cache); }
        static uint32 GetOffsetOfCommitFlag() { return offsetof(ActivationObjectEx, committed); }
        static uint32 GetOffsetOfParentFunc() { return offsetof(ActivationObjectEx, parentFunc); }

        static const PropertyId *GetCachedScopeInfo(const PropertyIdArray *propIds)
        {
            // Cached scope info is appended to the "normal" prop ID array elements.
            return &propIds->elements[propIds->count];
        }

        // Cached scope info:
        // [0] - cached func count
        // [1] - first func slot
        // [2] - first var slot

        static PropertyId GetCachedFuncCount(const PropertyIdArray *propIds)
        {
            return ActivationObjectEx::GetCachedScopeInfo(propIds)[0];
        }

        static PropertyId GetFirstFuncSlot(const PropertyIdArray *propIds)
        {
            return ActivationObjectEx::GetCachedScopeInfo(propIds)[1];
        }

        static PropertyId GetFirstVarSlot(const PropertyIdArray *propIds)
        {
            return ActivationObjectEx::GetCachedScopeInfo(propIds)[2];
        }

        static bool Is(void* instance)
        {
            return VirtualTableInfo<Js::ActivationObjectEx>::HasVirtualTable(instance);
        }

    private:
        ScriptFunction *parentFunc;
        uint cachedFuncCount;
        uint firstFuncSlot;
        uint lastFuncSlot;
        bool committed;
        FuncCacheEntry cache[1];
    };
};
