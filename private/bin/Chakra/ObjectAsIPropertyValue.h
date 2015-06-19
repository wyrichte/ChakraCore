//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Represents a marshaled Javascript array as a IVectorView in JavaScript

namespace Projection
{
    class ProjectionContext;
    class ObjectAsIReference;

    Declare_InspectableImpl_Extern_VTable(g_IPropertyValueVtable);

    // *******************************************************
    // Represents a projection of an Array as IIterable Methods
    // *******************************************************
    class ObjectAsIPropertyValue sealed : public CUnknownImpl
    {
        friend class ObjectAsIReference;

    private:
        ObjectAsIReference *objectIReference;

    private:
        ObjectAsIPropertyValue(ProjectionContext *projectionContext);
        virtual ~ObjectAsIPropertyValue() { }

        HRESULT Initialize(
            __in ObjectAsIReference *objectReference);


        static HRESULT Create(
            __in ProjectionContext *projectionContext, 
            __in ObjectAsIReference *objectReference, 
            __out ObjectAsIPropertyValue **newObjectAsIPropertyValue);

    public:

        //
        // IUnknown members
        //
        CUnknownMethodNoError_Def(QueryInterface, REFIID riid, void **ppv);
        CUnknownMethod_ULONGReturn_Def(AddRef);
        CUnknownMethod_ULONGReturn_Def(Release);

        //
        // IInspectable Methods
        //
        CUnknownMethodNoError_Def(GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids);

        //
        // IPropertyValue members
        //
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, get_Type, __RPC__out enum Windows::Foundation::PropertyType *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, get_IsNumericScalar, __RPC__out boolean *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt8, __RPC__out BYTE *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetInt16, __RPC__out INT16 *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt16, __RPC__out UINT16 *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetInt32, __RPC__out INT32 *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt32, __RPC__out UINT32 *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetInt64, __RPC__out INT64 *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt64, __RPC__out UINT64 *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetSingle, __RPC__out float *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetDouble, __RPC__out double *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetChar16, __RPC__out WCHAR *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetBoolean, __RPC__out boolean *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetString, __RPC__out HSTRING *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetGuid, __RPC__out GUID *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetDateTime, __RPC__out Windows::Foundation::DateTime *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetTimeSpan, __RPC__out Windows::Foundation::TimeSpan *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetPoint, __RPC__out Windows::Foundation::Point *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetSize, __RPC__out Windows::Foundation::Size *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetRect, __RPC__out Windows::Foundation::Rect *value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt8Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetInt16Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt16Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetInt32Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt32Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetInt64Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetUInt64Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetSingleArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetDoubleArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetChar16Array, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetBooleanArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetStringArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetInspectableArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetGuidArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetDateTimeArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetTimeSpanArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetPointArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetSizeArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value);
        CUnknownMethodImpl_Def(ObjectAsIPropertyValue, GetRectArray, __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value);
    };
};