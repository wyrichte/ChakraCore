//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Implementation for IVector and IVectorView projected into array

namespace Projection
{
    // *******************************************************
    // Represents a projection of an VectorAsArray Methods
    // *******************************************************
    class MapWithStringKey
    {
    private:
        static BOOL HasKey(__in SpecialProjection *specialization, Var instance, Var key);
        static void Insert(__in SpecialProjection *specialization, Var instance, Var key, Var value);
        static Var Lookup(__in SpecialProjection *specialization, Var instance, Var key);
        static void Remove(__in SpecialProjection *specialization, Var instance, Var key);
        static Var First(__in SpecialProjection *specialization, Var instance);

        static Var GetPropertyIdVar(PropertyId propertyId, Js::ScriptContext *scriptContext);

        static BOOL HasPrototypeItem(__in SpecialProjection * specialization, uint32 index);

        static BOOL HasOwnPropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar);
        static BOOL GetOwnPropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar, __out Var *value);
        static void SetPropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar, __in Var value);
        static BOOL DeletePropertyCore(__in SpecialProjection * specialization, __in Var instance, __in Var propertyIdVar);

    public:
        static bool IsMapWithStringKey(__in SpecialProjection * specialization);
        static bool IsMapViewWithStringKey(__in SpecialProjection * specialization);
        static bool IsMapOrMapViewWithStringKey(__in SpecialProjection * specialization);
        static BOOL HasPrototypeProperty(__in SpecialProjection * specialization, Js::PropertyId propertyId);
        
        static Var GetVarFromHSTRING(HSTRING hstring, __in SpecialProjection * specialization);
        static HRESULT HasOwnKey(__in SpecialProjection * specialization, __in Var instance, __in HSTRING key, __out BOOL *result);
        static HRESULT GetOwnValue(__in SpecialProjection * specialization, __in Var instance, __in HSTRING key, __out Var *value, __out BOOL *propertyPresent);

        static HRESULT HasOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result);
        static HRESULT GetOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out Var *value, __out BOOL *propertyPresent);
        static HRESULT SetProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __in Var value, __out BOOL *result);
        static HRESULT DeleteProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result);

        static HRESULT HasOwnItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *result);
        static HRESULT GetOwnItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out Var *value, __out BOOL *itemPresent);
        static HRESULT SetItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __in Var value, __out BOOL *result);
        static HRESULT DeleteItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *result);

        static HRESULT GetEnumerator(__in SpecialProjection * specialization, __in Var instance, __out IVarEnumerator **enumerator);

        static HRESULT GetAccessors(__in SpecialProjection * specialization, __in Js::PropertyId propertyId, __out BOOL *result);
        static HRESULT SetAccessors(__in SpecialProjection * specialization, __in Js::PropertyId propertyId);
    };
};