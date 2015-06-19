//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once
namespace Projection
{
    class InspectableObjectTypeOperations sealed : public ProjectionTypeOperations
    {
    private:
        SpecialProjection * specialization;
#if DBG
        Js::ScriptContext * scriptContext;
#endif

        // TODO: defer init.
        InspectableObjectTypeOperations(ProjectionContext *projectionContext, SpecialProjection * specialization);
        ~InspectableObjectTypeOperations() { }

        bool HasVectorOrVectorViewArrayLikeProjection() 
        {
            return (specialization != nullptr && VectorArray::IsVectorOrVectorView(specialization)); 
        }

        bool HasMapOrMapViewSpecialization()
        {
            return MapWithStringKey::IsMapOrMapViewWithStringKey(specialization);
        }

    public:
        static HRESULT Create(
            __in ProjectionContext *projectionContext, 
            __in SpecialProjection * specialization, 
            __out InspectableObjectTypeOperations** newInstance);

        virtual ProjectionType GetProjectionType() override { return InspectableProjectionType; }

        virtual HRESULT STDMETHODCALLTYPE HasOwnProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE GetOwnProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var *value,
            /* [out] */ BOOL *propertyPresent) override;

        virtual HRESULT STDMETHODCALLTYPE GetPropertyReference(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var *value,
            /* [out] */ BOOL *propertyPresent) override;

        virtual HRESULT STDMETHODCALLTYPE SetProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE DeleteProperty(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE GetOwnItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ Var *value,
            /* [out] */ BOOL *itemPresent) override;

        virtual HRESULT STDMETHODCALLTYPE SetItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [in] */ Var value,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE DeleteItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE GetEnumerator(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ BOOL enumNonEnumerable,
            /* [in] */ BOOL enumSymbols,
            /* [out] */ IVarEnumerator **enumerator) override;

        virtual HRESULT STDMETHODCALLTYPE IsEnumerable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE IsWritable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE IsConfigurable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE SetEnumerable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value) override;

        virtual HRESULT STDMETHODCALLTYPE SetWritable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value) override;

        virtual HRESULT STDMETHODCALLTYPE SetConfigurable(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ BOOL value) override;

        virtual HRESULT STDMETHODCALLTYPE GetAccessors(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var* getter,
            /* [out] */ Var* setter,
            /* [out] */ BOOL* result) override;

        virtual HRESULT STDMETHODCALLTYPE SetAccessors(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var getter,
            /* [in] */ Var setter) override;

        virtual HRESULT STDMETHODCALLTYPE GetSetter( 
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags) override;

        virtual HRESULT STDMETHODCALLTYPE Equals( 
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var other,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE StrictEquals( 
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var other,
            /* [out] */ BOOL *result) override;

        virtual HRESULT STDMETHODCALLTYPE QueryObjectInterface(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ REFIID riid,
            /* [out] */ void **ppvObj) override;

        virtual HRESULT STDMETHODCALLTYPE GetHeapObjectInfo(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance, 
            /* [in] */ ProfilerHeapObjectInfoFlags flags,
            /* [out] */ HostProfilerHeapObject** result,
            /* [out] */ HeapObjectInfoReturnResult* returnResult) override;

        virtual HRESULT STDMETHODCALLTYPE HasOwnItem(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance, 
            /* [in] */ Var index,
            /* [out] */ BOOL* result) override;

        virtual HRESULT STDMETHODCALLTYPE GetItemSetter(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ Var index, 
            /* [out] */ Var* setter,
            /* [out] */ ::DescriptorFlags* flags) override;

        virtual HRESULT STDMETHODCALLTYPE SetPropertyWithAttributes(
            /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
            /* [in] */ Var instance,
            /* [in] */ PropertyId propertyId,
            /* [in] */ Var value,
            /* [in] */ PropertyAttributes attributes,
            /* [in] */ SideEffects effects, 
            /* [out] */ BOOL* result) override;

    };
}