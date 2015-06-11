//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once
namespace Js
{

    class SpreadArgument : public DynamicObject
    {
    private:
        Var iterable;
        RecyclableObject* iterator;
        typedef JsUtil::List<Var, Recycler> VarList;
        VarList* iteratorIndices;
        //need to return something even though we are asserting
        int Cauterizing() { AssertMsg(false, "This function should not be exposed"); return 0; }
    protected:
        DEFINE_VTABLE_CTOR(SpreadArgument, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(SpreadArgument);

    public:
        static bool Is(Var aValue);
        static SpreadArgument* FromVar(Var value);
        SpreadArgument(Var iterable, RecyclableObject* iterator, DynamicType * type);
        Var GetArgument() const { return iterable; }
        const Var* GetArgumentSpread() const { return iteratorIndices ? iteratorIndices->GetBuffer() : nullptr; }
        uint GetArgumentSpreadCount()  const { return iteratorIndices ? iteratorIndices->Count() : 0; }

#if DBG
        virtual BOOL HasProperty(PropertyId propertyId) override { return Cauterizing(); };
        virtual BOOL HasOwnProperty(PropertyId propertyId) override { return Cauterizing(); };
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override { return Cauterizing(); };
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override { return Cauterizing(); };
        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override{ return Cauterizing(); };
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override { Cauterizing(); return None; };
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override { return Cauterizing(); };
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override { return Cauterizing(); };
        virtual DescriptorFlags GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override { Cauterizing(); return None; };
        virtual DescriptorFlags GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override { Cauterizing(); return None; };
        virtual int GetPropertyCount() override { return Cauterizing(); };
        virtual PropertyId GetPropertyId(PropertyIndex index) override { return Cauterizing(); };
        virtual PropertyId GetPropertyId(BigPropertyIndex index) override { return Cauterizing(); };
        virtual BOOL SetInternalProperty(PropertyId internalPropertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override { return Cauterizing(); };
        virtual BOOL InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags = PropertyOperation_None, PropertyValueInfo* info = NULL) override { return Cauterizing(); };
        virtual BOOL SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override { return Cauterizing(); };
        virtual BOOL IsFixedProperty(PropertyId propertyId) override { return Cauterizing(); };
        virtual BOOL HasItem(uint32 index) override { return Cauterizing(); };
        virtual BOOL HasOwnItem(uint32 index) override { return Cauterizing(); };
        virtual BOOL GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override { return Cauterizing(); };
        virtual BOOL GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override { return Cauterizing(); };
        virtual DescriptorFlags GetItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext) override { Cauterizing(); return None; };
        virtual BOOL SetItem(uint32 index, Var value, PropertyOperationFlags flags) override { return Cauterizing(); };
        virtual BOOL DeleteItem(uint32 index, PropertyOperationFlags flags) override { return Cauterizing(); };
        virtual BOOL ToPrimitive(JavascriptHint hint, Var* result, ScriptContext * requestContext) override { return Cauterizing(); };
        virtual BOOL GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext* scriptContext, bool preferSnapshotSemantics = true, bool enumSymbols = false) override { return Cauterizing(); };
        virtual BOOL SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags = PropertyOperation_None) override { return Cauterizing(); };
        virtual BOOL GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext) override { return Cauterizing(); };
        virtual BOOL IsWritable(PropertyId propertyId) override { return Cauterizing(); };
        virtual BOOL IsConfigurable(PropertyId propertyId) override { return Cauterizing(); };
        virtual BOOL IsEnumerable(PropertyId propertyId) override { return Cauterizing(); };
        virtual BOOL SetEnumerable(PropertyId propertyId, BOOL value) override { return Cauterizing(); };
        virtual BOOL SetWritable(PropertyId propertyId, BOOL value) override { return Cauterizing(); };
        virtual BOOL SetConfigurable(PropertyId propertyId, BOOL value) override { return Cauterizing(); };
        virtual BOOL SetAttributes(PropertyId propertyId, PropertyAttributes attributes) override { return Cauterizing(); };
        virtual BOOL IsExtensible() override { return false; };
        virtual BOOL PreventExtensions() override { return Cauterizing(); };
        virtual BOOL Seal() override { return Cauterizing(); };
        virtual BOOL Freeze() override { return Cauterizing(); };
        virtual BOOL IsSealed() override { return Cauterizing(); };
        virtual BOOL IsFrozen() override { return Cauterizing(); };
        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override { return Cauterizing(); };
        virtual Var GetTypeOfString(ScriptContext * requestContext) override { Cauterizing(); return RecyclableObject::GetTypeOfString(requestContext); };
        virtual bool DbgIsDynamicObject() const override { return true; }
#endif

    };
}