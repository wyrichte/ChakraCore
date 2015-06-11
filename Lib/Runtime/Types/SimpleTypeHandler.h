//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    // TODO (jedmiad): Consider adding full support for fixed fields on SimpleTypeHandler.
    template<size_t size>
    class SimpleTypeHandler sealed: public DynamicTypeHandler
    {
        friend class NullTypeHandlerBase;
    private:
        int propertyCount;
        SimplePropertyDescriptor descriptors[size];

    public:
        DEFINE_GETCPPNAME();

    private:
        SimpleTypeHandler(Recycler*);        // only used by NullTypeHandler
        SimpleTypeHandler(SimpleTypeHandler<size> * typeHandler);
        DEFINE_VTABLE_CTOR_NO_REGISTER(SimpleTypeHandler, DynamicTypeHandler);

    public:
        SimpleTypeHandler(const PropertyRecord* id, PropertyAttributes attributes = PropertyNone, PropertyTypes propertyTypes = PropertyTypesNone, uint16 inlineSlotCapacity = 0, uint16 offsetOfInlineSlots = 0);
        // Constructor of a shared typed handler

        SimpleTypeHandler(SimplePropertyDescriptor (&SharedFunctionPropertyDescriptors)[size], PropertyTypes propertyTypes = PropertyTypesNone, uint16 inlineSlotCapacity = 0, uint16 offsetOfInlineSlots = 0);

        virtual BOOL IsLockable() const override { return true; }
        virtual BOOL IsSharable() const override { return true; }
        virtual int GetPropertyCount() override;
        virtual PropertyId GetPropertyId(ScriptContext* scriptContext, PropertyIndex index) override;
        virtual PropertyId GetPropertyId(ScriptContext* scriptContext, BigPropertyIndex index) override;
        virtual BOOL FindNextProperty(ScriptContext* scriptContext, PropertyIndex& index, __out JavascriptString** propertyString,
            __out PropertyId* propertyId, __out PropertyAttributes* attributes, Type* type, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols = false) override;
        virtual PropertyIndex GetPropertyIndex(PropertyRecord const* propertyRecord) override;
        virtual bool GetPropertyEquivalenceInfo(PropertyRecord const* propertyRecord, PropertyEquivalenceInfo& info) override;
        virtual bool IsObjTypeSpecEquivalent(const Type* type, const TypeEquivalenceRecord& record, uint& failedPropertyIndex) override;
        virtual bool IsObjTypeSpecEquivalent(const Type* type, const EquivalentPropertyEntry* entry) override;
        virtual BOOL HasProperty(DynamicObject* instance, PropertyId propertyId, __out_opt bool *noRedecl = nullptr) override;
        virtual BOOL HasProperty(DynamicObject* instance, JavascriptString* propertyNameString) override;
        virtual BOOL GetProperty(DynamicObject* instance, Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(DynamicObject* instance, Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(DynamicObject* instance, PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(DynamicObject* instance, JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual DescriptorFlags GetSetter(DynamicObject* instance, PropertyId propertyId, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual DescriptorFlags GetSetter(DynamicObject* instance, JavascriptString* propertyNameString, Var* setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL DeleteProperty(DynamicObject* instance, PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual BOOL IsEnumerable(DynamicObject* instance, PropertyId propertyId) override;
        virtual BOOL IsWritable(DynamicObject* instance, PropertyId propertyId) override;
        virtual BOOL IsConfigurable(DynamicObject* instance, PropertyId propertyId) override;
        virtual BOOL SetEnumerable(DynamicObject* instance, PropertyId propertyId, BOOL value) override;
        virtual BOOL SetWritable(DynamicObject* instance, PropertyId propertyId, BOOL value) override;
        virtual BOOL SetConfigurable(DynamicObject* instance, PropertyId propertyId, BOOL value) override;
        virtual BOOL SetAccessors(DynamicObject* instance, PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags = PropertyOperation_None) override;
        virtual BOOL PreventExtensions(DynamicObject *instance) override;
        virtual BOOL Seal(DynamicObject* instance) override;
        virtual BOOL SetPropertyWithAttributes(DynamicObject* instance, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override;
        virtual BOOL SetAttributes(DynamicObject* instance, PropertyId propertyId, PropertyAttributes attributes) override;
        virtual BOOL GetAttributesWithPropertyIndex(DynamicObject * instance, PropertyId propertyId, BigPropertyIndex index, PropertyAttributes * attributes) override;

        virtual void SetAllPropertiesToUndefined(DynamicObject* instance, bool invalidateFixedFields) override;
        virtual void MarshalAllPropertiesToScriptContext(DynamicObject* instance, ScriptContext* targetScriptContext, bool invalidateFixedFields) override;
        virtual DynamicTypeHandler* ConvertToTypeWithItemAttributes(DynamicObject* instance) override;

        virtual void SetIsPrototype(DynamicObject* instance) override;

#if DBG
        virtual bool SupportsPrototypeInstances() const { return !ChangeTypeOnProto() && !(GetIsOrMayBecomeShared() && IsolatePrototypes()); }
        virtual bool CanStorePropertyValueDirectly(const DynamicObject* instance, PropertyId propertyId, bool allowLetConst) override;
#endif

#if DBG
        bool HasSingletonInstanceOnlyIfNeeded() const
        {
            // Naturally, if we add support for fixed fields to this type handler we will have to update this implementation.
            return true;
        }
#endif

    private:
        template <typename T>
        T* ConvertToTypeHandler(DynamicObject* instance);

        DictionaryTypeHandler* ConvertToDictionaryType(DynamicObject* instance);
        SimpleDictionaryTypeHandler* ConvertToSimpleDictionaryType(DynamicObject* instance);
        ES5ArrayTypeHandler* ConvertToES5ArrayType(DynamicObject* instance);
        SimpleTypeHandler<size>* ConvertToNonSharedSimpleType(DynamicObject * instance);

        BOOL GetDescriptor(PropertyId propertyId, int * index);
        BOOL SetAttribute(DynamicObject* instance, int index, PropertyAttributes attribute);
        BOOL ClearAttribute(DynamicObject* instance, int index, PropertyAttributes attribute);
        BOOL AddProperty(DynamicObject* instance, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects);
        virtual BOOL FreezeImpl(DynamicObject* instance, bool isConvertedType) override;
    };

    typedef SimpleTypeHandler<1> SimpleTypeHandlerSize1;
    typedef SimpleTypeHandler<2> SimpleTypeHandlerSize2;
    
}
