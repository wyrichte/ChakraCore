//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Implementation for IVector and IVectorView projected into array
 
namespace Projection
{
    // *******************************************************
    // Represents a projection of an VectorAsArray Methods
    // *******************************************************
    class VectorArray
    {
    public:
        static HRESULT CreateProtypeObject(__in bool fSpecialisedProjection, __in ProjectionContext *projectionContext, __out Var *pPrototypeVar);

        static Var SetLengthOfSpecialProjection(__in SpecialProjection * specialization, __in Js::Arguments* arguments);

        static HRESULT HasItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *itemPresent, bool fIndexIsUInt32 = false, uint32 uint32Index = 0);
        static HRESULT GetItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out Var *value, __out BOOL *itemPresent, bool fIndexIsUInt32 = false, uint32 uint32Index = 0);
        static HRESULT SetItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __in Var value, __out BOOL *result, bool fIndexIsUInt32 = false, uint32 uint32Index = 0);
        static HRESULT DeleteItem(__in SpecialProjection * specialization, __in Var instance, __in Var index, __out BOOL *result, bool fIndexIsUInt32 = false, uint32 uint32Index = 0);

        static HRESULT GetEnumerator(__in SpecialProjection * specialization, __in Var instance, __in IVarEnumerator *pPropertyEnumerator, __out IVarEnumerator **enumerator);

        static HRESULT HasOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result);
        static HRESULT GetOwnProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out Var *value, __out BOOL *propertyPresent);
        static HRESULT SetProperty(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __in Var value, __out BOOL *result);
        static HRESULT IsWritable(__in SpecialProjection * specialization, __in Var instance, __in Js::PropertyId propertyId, __out BOOL *result);

        static HRESULT GetAccessors(__in SpecialProjection * specialization, __in Js::PropertyId propertyId, __out BOOL *result);
        static HRESULT SetAccessors(__in SpecialProjection * specialization, __in Js::PropertyId propertyId);

        static uint32 GetLength(__in SpecialProjection * specialization, Var instance, Js::ScriptContext * scriptContext);

        static Var GetAt(__in SpecialProjection * specialization, Var instance, Var index);
        static void SetAt(__in SpecialProjection * specialization, Var instance, Var index, Var value);
        static void Append(__in SpecialProjection * specialization, Var instance, Var value);
        static void RemoveAtEnd(__in SpecialProjection * specialization, Var instance);

        static bool IsVector(__in SpecialProjection * specialization);
        static bool IsVectorView(__in SpecialProjection * specialization);
        static bool IsVectorOrVectorView(__in SpecialProjection * specialization);
        // TODO: shkamat : Do we implement some of the array prototype methods so that the performance is better? 
        // eg shift unshift
    };
};