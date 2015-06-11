/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
interface ITypeOperations;
class JavascriptDispatch;
// forward declcaration from edgescriptdirect.h as we don't want to include it in library.

namespace Js
{
    class ExternalType : public DynamicType
    {
        friend class ExternalObject;
    protected:
        PropertyId nameId;
        ITypeOperations * operations;
        JavascriptMethod nativeMethod;

        ExternalType(ExternalType * type) : DynamicType(type),  nameId(type->nameId), operations(type->operations), nativeMethod(type->nativeMethod) {}
        ExternalType(ScriptContext* scriptContext, TypeId typeId, RecyclableObject* prototype, JavascriptMethod entryPoint,
            DynamicTypeHandler * typeHandler, bool isLocked, bool isShared, ITypeOperations * operations, PropertyId nameId);
    public:
        ExternalType(ScriptContext* scriptContext, TypeId typeId, RecyclableObject* prototype, JavascriptMethod entryPoint,
            DynamicTypeHandler * typeHandler, bool isLocked, bool isShared, PropertyId nameId);
        PropertyId GetNameId() const { return nameId; }
        ITypeOperations * GetTypeOperations() { return operations; }

        static size_t GetOffsetOfOperations() { return offsetof(ExternalType, operations); }

        static Var __cdecl ExternalEntryThunk(RecyclableObject*, CallInfo, ...);
        static Var __cdecl CrossSiteExternalEntryThunk(RecyclableObject*, CallInfo, ...);
    private:
        void Initialize(JavascriptMethod entryPoint);
    };
    AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(ExternalType, &RecyclableObject::DumpObjectFunction);

    class ExternalObject : public DynamicObject
    {
        friend class ScriptSite; // just for CONTAINING_RECORD
#if DBG_EXTRAFIELD
        friend class DOMFastPathInfo; // additionalByteCount for chk build only
#endif
    protected:
        DEFINE_VTABLE_CTOR(ExternalObject, DynamicObject);

        void* finalizer;
        // This is used in CustomExternalObject only. If we have JavascriptDispatch hosting CustomExternalObject, the
        // life time of JavascriptDispatch should be the same as the hosted CustomExternalObject. This way we can
        // keep the propertyids cached with the JavascriptDispatch to have the same lifetime of the CEO.
        JavascriptDispatch* cachedJavascriptDispatch;
#if DBG_EXTRAFIELD
        UINT additionalByteCount;
#endif

        PropertyId GetNameId() const;
        ExternalType * GetExternalType() const { return (ExternalType *)this->GetType(); }
    public:
        ExternalObject(ExternalType * type
#if DBG
                    , UINT byteCount = 0
#endif
            );
        BOOL IsObjectAlive();
        BOOL VerifyObjectAlive();

        bool IsCustomExternalObject() const { return this->GetTypeOperations() != nullptr ;}
        ITypeOperations * GetTypeOperations() const { return this->GetExternalType()->GetTypeOperations(); }

        static bool Is(Var instance)
        {
            if (!Js::RecyclableObject::Is(instance))
            {
                return false;
            }
            if (!Js::RecyclableObject::FromVar(instance)->IsExternal())
            {
                return false;
            }
            return true;
        }

        static ExternalObject* FromVar(Var instance)  {
            Assert(Is(instance));
            ExternalObject* obj = static_cast<ExternalObject*>(instance);
            return obj;
        }

        virtual bool HasReadOnlyPropertiesInvisibleToTypeHandler() override { return true; }

        virtual BOOL HasProperty(PropertyId propertyId) override;
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetInternalProperty(PropertyId internalPropertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL InitProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags = PropertyOperation_None, PropertyValueInfo* info = nullptr) override;
        virtual BOOL SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override;
        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual BOOL HasItem(uint32 index) override;
        virtual BOOL GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override;
        virtual BOOL GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override;
        virtual DescriptorFlags GetItemSetter(uint32 index, Var* setterValue, ScriptContext* requestContext) override;
        virtual BOOL SetItem(uint32 index, Var value, PropertyOperationFlags flags) override;
        virtual BOOL DeleteItem(uint32 index, PropertyOperationFlags flags) override;

        virtual BOOL GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext* scriptContxt, bool preferSnapshotSemantics = true, bool enumSymbols = false) override;

        virtual BOOL IsWritable(PropertyId propertyId) override;
        virtual BOOL IsConfigurable(PropertyId propertyId) override;
        virtual BOOL IsEnumerable(PropertyId propertyId) override;

        virtual BOOL SetEnumerable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetWritable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetConfigurable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetAttributes(PropertyId propertyId, PropertyAttributes attributes) override;

        virtual BOOL GetAccessors(PropertyId propertyId, Var *getter, Var *setter, ScriptContext * requestContext) override;
        virtual BOOL SetAccessors(PropertyId propertyId, Var getter, Var setter, Js::PropertyOperationFlags flags = Js::PropertyOperation_None) override;
        virtual DescriptorFlags GetSetter(PropertyId propertyId, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual DescriptorFlags GetSetter(JavascriptString* propertyNameString, Var *setterValue, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual HRESULT QueryObjectInterface(REFIID riid, void **ppvObj) override;

        virtual BOOL Equals(Var other, BOOL* value, ScriptContext * requestContext) override;
        virtual BOOL StrictEquals(Var other, BOOL* value, ScriptContext * requestContext) override;
        
        // Used only in JsVarToExtension where it may be during dispose and the type is not availible
        virtual BOOL IsExternalVirtual() const override { return TRUE; }
        virtual JavascriptString* GetClassName(ScriptContext * requestContext) override sealed;

        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual void RemoveFromPrototype(ScriptContext * requestContext) override;
        virtual void AddToPrototype(ScriptContext * requestContext) override;
        virtual void SetPrototype(RecyclableObject* newPrototype) override;

        virtual DynamicType* DuplicateType() override;
        virtual void MarshalToScriptContext(ScriptContext * scriptContext) override;

        virtual Var InvokePut(Js::Arguments args) override;

        HRESULT Reinitialize(ExternalType* type, BOOL keepProperties);

#if DBG
    private:        
        virtual BOOL DbgCanHaveInterceptors() const override { return TRUE; }
#endif
    };
    AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(ExternalObject, &RecyclableObject::DumpObjectFunction);
}
