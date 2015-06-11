// Copyright (C) Microsoft. All rights reserved.

#pragma once

namespace Js
{
    class JavascriptStringObject : public DynamicObject
    {
    private:
        static PropertyId specialPropertyIds[];

    protected:
        JavascriptString* value;

        DEFINE_VTABLE_CTOR(JavascriptStringObject, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptStringObject);
        JavascriptString* InternalUnwrap();

    private:
        bool GetPropertyBuiltIns(PropertyId propertyId, Var* value, ScriptContext* requestContext, BOOL* result);
        bool SetPropertyBuiltIns(PropertyId propertyId, PropertyOperationFlags flags, bool* result);
        bool GetSetterBuiltIns(PropertyId propertyId, PropertyValueInfo* info, DescriptorFlags* descriptorFlags);

    public:
        JavascriptStringObject(DynamicType * type);
        JavascriptStringObject(JavascriptString* value, DynamicType * type);
        static bool Is(Var aValue);
        static JavascriptStringObject* FromVar(Var aValue);

        void Initialize(JavascriptString* value);
        JavascriptString* Unwrap() { return InternalUnwrap(); }

        virtual bool HasReadOnlyPropertiesInvisibleToTypeHandler() override { return true; }

        virtual BOOL HasProperty(PropertyId propertyId) override;
        virtual BOOL IsConfigurable(PropertyId propertyId) override;
        virtual BOOL IsEnumerable(PropertyId propertyId) override;
        virtual BOOL IsWritable(PropertyId propertyId) override;

        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual DescriptorFlags GetSetter(PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual DescriptorFlags GetSetter(JavascriptString* propertyNameString, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL HasItem(uint32 index) override;
        virtual BOOL GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override;
        virtual BOOL GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override;
        virtual BOOL SetItem(uint32 index, Var value, PropertyOperationFlags flags) override;
        virtual BOOL GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics = true, bool enumSymbols = false) override;
        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext) override;
        virtual uint GetSpecialPropertyCount() const override;
        virtual PropertyId* GetSpecialPropertyIds() const override;

        virtual JavascriptStringObject* MakeCopyOnWriteObject(ScriptContext* scriptContext) override;
    };
}
