//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class CustomExternalType : public ExternalType
    {
    public:
        CustomExternalType(CustomExternalType * type) : ExternalType(type), finalizer(type->finalizer), usage(type->usage), isSimpleWrapper(type->isSimpleWrapper) {}
        CustomExternalType(ScriptContext* scriptContext, TypeId typeId, RecyclableObject* prototype, JavascriptMethod entryPoint, DynamicTypeHandler * typeHandler, bool isLocked, bool isShared, ITypeOperations * operations, PropertyId nameId) :
            ExternalType(scriptContext, typeId, prototype, entryPoint, typeHandler, isLocked, isShared, operations, nameId), isSimpleWrapper(false) {}
        HRESULT Initialize();

        FinalizeMethod GetFinalizer() const { return finalizer; }
        OperationUsage GetOperationUsage() const { return usage; }
        BOOL IsSimpleWrapper() const { return isSimpleWrapper; }

        static void __cdecl DeferredInitializer(DynamicObject * instance, DeferredTypeHandlerBase * typeHandler, DeferredInitializeMode mode);

        static size_t GetOffsetOfUsage() { return offsetof(CustomExternalType, usage); }

    private:
        FinalizeMethod finalizer;
        OperationUsage usage;
        BOOL           isSimpleWrapper;
    };
    AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(CustomExternalType, &Type::DumpObjectFunction);

// log an etw event before calling out to ITypeOperations
#define BEGIN_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, thisTypeId, propertyId, Operation) \
    if (IS_JS_ETW(EventEnabledJSCRIPT_HOSTING_CEO_START()))    \
    { \
      JS_ETW(EventWriteJSCRIPT_HOSTING_CEO_START(scriptContext, thisTypeId, (void*)propertyId, Operation)); \
    } \
   BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext) \

// log an etw event before returning back from an ITypeOperation call. 
#define END_CUSTOM_EXTERNAL_OBJECT_CALL(scriptContext, thisTypeId, propertyId, Operation) \
    END_LEAVE_SCRIPT_WITH_EXCEPTION(scriptContext); \
    if (IS_JS_ETW(EventEnabledJSCRIPT_HOSTING_CEO_STOP()))    \
    { \
      JS_ETW(EventWriteJSCRIPT_HOSTING_CEO_STOP(scriptContext, thisTypeId, (void*)propertyId, Operation)); \
    } \

    typedef enum CustomExternalOperation {
        CustomExternalObject_HasOwnProperty =0,
        CustomExternalObject_GetOwnProperty =1,
        CustomExternalObject_GetPropertyReference =2,
        CustomExternalObject_SetProperty =3,
        CustomExternalObject_SetPropertyWithAttributes =4,
        CustomExternalObject_DeleteProperty =5,
        CustomExternalObject_HasOwnItem =6,
        CustomExternalObject_GetOwnItem =7,
        CustomExternalObject_GetItemReference =8,
        CustomExternalObject_SetItem =9,
        CustomExternalObject_DeleteItem =10,    //10
        CustomExternalObject_GetEnumerator =11, 
        CustomExternalObject_IsEnumerable =12,
        CustomExternalObject_IsWritable =13,
        CustomExternalObject_IsConfigurable =14,
        CustomExternalObject_SetEnumerable =15,
        CustomExternalObject_SetWritable =16,
        CustomExternalObject_SetConfigurable =17,
        CustomExternalObject_SetAccessors =18,
        CustomExternalObject_GetAccessors =19,
        CustomExternalObject_GetSetter =20,    // 20
        CustomExternalObject_GetItemSetter =21, 
        CustomExternalObject_Equals =22,
        CustomExternalObject_StrictEquals =23,
        CustomExternalObject_HasInstance =24,
        CustomExternalObject_GetNamespaceParent =25,
        CustomExternalObject_CrossDomainCheck =26,  
        CustomExternalObject_GetHeapObjectInfo =27,
        CustomExternalObject_QueryObjectInterface =28,
    };

    class CustomExternalObject : public ExternalObject
    {
        friend class DefaultScriptOperations;
        friend class CustomExternalType;
    private:

        OperationUsage GetOperationUsage() const { return this->GetCustomExternalType()->GetOperationUsage(); }

        HRESULT QueryObjectInterfaceInternal(REFIID riid, void **ppvObj);
        CustomEnumerator* CreateEnumerator(ScriptContext* scriptContext, IVarEnumerator* varEnumerator);

        CustomExternalType * GetCustomExternalType() const { return (CustomExternalType *)this->GetExternalType(); }
    protected:
        DEFINE_VTABLE_CTOR(CustomExternalObject, ExternalObject);

    public:
        CustomExternalObject(CustomExternalType * type
#if DBG
                    , UINT byteCount = 0
#endif
            );

        static bool Is(Var instance);
        static CustomExternalObject* FromVar(Var instance);

        virtual void Finalize(bool isShutdown) override;
        virtual void Dispose(bool isShutdown) override;

#if DBG
        virtual BOOL DbgSkipsPrototype() const override { return (GetOperationUsage().useWhenPropertyNotPresentInPrototypeChain & OperationFlagsForNamespaceOrdering_allGetPropertyOperations) ? true : false; }
#endif
        virtual BOOL HasProperty(PropertyId propertyId) override;
        virtual BOOL UseDynamicObjectForNoHostObjectAccess() override { return TRUE; }
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override sealed;
        virtual BOOL InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags = PropertyOperation_None, PropertyValueInfo* info = NULL) override sealed;
        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual BOOL HasItem(uint32 index) override;
        virtual BOOL GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override;
        virtual BOOL GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override sealed;
        virtual DescriptorFlags GetItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext) override;
        virtual BOOL SetItem(uint32 index, Var value, PropertyOperationFlags flags) override;
        virtual BOOL DeleteItem(uint32 index, PropertyOperationFlags flags) override;

        virtual BOOL GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics = true, bool enumSymbols = false) override;

        virtual BOOL IsWritable(PropertyId propertyId) override;
        virtual BOOL IsConfigurable(PropertyId propertyId) override;
        virtual BOOL IsEnumerable(PropertyId propertyId) override;

        // TODO: WARNING: slow perf as it calls GetConfigurable/Enumerable/Writable individually.
        virtual BOOL SetAttributes(PropertyId propertyId, PropertyAttributes attributes) override;

        virtual BOOL SetEnumerable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetWritable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetConfigurable(PropertyId propertyId, BOOL value) override sealed;

        virtual BOOL GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext) override;
        virtual BOOL SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags = PropertyOperation_None) override sealed;
        virtual DescriptorFlags GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual DescriptorFlags GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;

        virtual HRESULT QueryObjectInterface(REFIID riid, void **ppvObj) override sealed;

        virtual BOOL Equals(Var other, BOOL* value, ScriptContext * requestContext) override;
        virtual BOOL StrictEquals(Var other, BOOL* value, ScriptContext * requestContext) override;

        virtual DynamicType* DuplicateType() override;
        virtual BOOL HasInstance(Var instance, ScriptContext* scriptContext, IsInstInlineCache* inlineCache = NULL) override;
        virtual Var GetNamespaceParent(Var instance) override;
        virtual RecyclableObject* GetConfigurablePrototype(ScriptContext * requestContext) override;
        virtual void SetPrototype(RecyclableObject* newPrototype) override;

        template <bool checkLocal>
        BOOL GetPropertyImpl(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext);
        template <bool checkLocal>
        BOOL GetPropertyReferenceImpl(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext);
        template <bool checkLocal>
        BOOL SetPropertyImpl(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info);
        template <bool checkLocal>
        BOOL DeletePropertyImpl(PropertyId propertyId, PropertyOperationFlags flags);

        PropertyId GetTypeNameId() const
        {
            return this->GetNameId();
        }

        void CacheJavascriptDispatch(JavascriptDispatch* javascriptDispatch);
        JavascriptDispatch* GetCachedJavascriptDispatch() const { return cachedJavascriptDispatch; }
        FinalizeMethod GetFinalizer() const { return (FinalizeMethod)finalizer; }

    };
    // We will provide one offset for IASD to add extra data at IASD:CreateTypedObject
    C_ASSERT(sizeof(CustomExternalObject) == sizeof(ExternalObject));
    // MockExternalObject is used in static lib to calculate the object size. If CustomExternalObject size is changed
    // we need to change lib\static\base\MockExernalObject.h accordingly
    C_ASSERT(sizeof(CustomExternalObject) == sizeof(MockExternalObject));
    AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(CustomExternalObject, &RecyclableObject::DumpObjectFunction);
}