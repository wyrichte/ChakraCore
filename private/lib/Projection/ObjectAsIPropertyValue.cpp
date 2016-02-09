//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// *******************************************************
// Represents a marshaled Javascript array as a IIterable in JavaScript
// *******************************************************


#include "ProjectionPch.h"

#define GetNumericTypeFromObjectReference(typeName)                                                                                                             \
        if (value == nullptr)                                                                                                                                   \
        {                                                                                                                                                       \
            return E_POINTER;                                                                                                                                   \
        }                                                                                                                                                       \
        ZeroMemory(value, sizeof(typeName));                                                                                                                    \
                                                                                                                                                                \
        /* enums are scalar and support all numeric conversion */                                                                                               \
        if (!this->objectIReference->IsArray() && this->objectIReference->IsElementTypeEnumType())                                                              \
        {                                                                                                                                                       \
            __int32 baseValue;                                                                                                                                  \
            hr = this->objectIReference->get_Value(sizeof(__int32), (byte *)&baseValue);                                                                        \
            if (SUCCEEDED(hr))                                                                                                                                  \
            {                                                                                                                                                   \
                *value = (typeName)baseValue;                                                                                                                   \
            }                                                                                                                                                   \
        }                                                                                                                                                       \
        else                                                                                                                                                    \
        {                                                                                                                                                       \
            hr = TYPE_E_TYPEMISMATCH;                                                                                                                           \
        }

#define GetUnsupportedTypeFromObjectReference()                                                                                                                     \
        if (value != nullptr)                                                                                                                                       \
        {                                                                                                                                                           \
            hr = TYPE_E_TYPEMISMATCH;                                                                                                                               \
        }                                                                                                                                                           \
        else                                                                                                                                                        \
        {                                                                                                                                                           \
            hr = E_POINTER;                                                                                                                                         \
        }

#define GetWindowsFoundationTypeFromObjectReference(typeName)                                                                                                   \
        if (value == nullptr)                                                                                                                                   \
        {                                                                                                                                                       \
            return E_POINTER;                                                                                                                                   \
        }                                                                                                                                                       \
        ZeroMemory(value, sizeof(Windows::Foundation::typeName));                                                                                              \
                                                                                                                                                                \
        if (!this->objectIReference->IsArray() && wcscmp(this->objectIReference->GetFullElementTypeName(), L"Windows.Foundation." L#typeName) == 0)              \
        {                                                                                                                                                       \
            hr = this->objectIReference->get_Value(sizeof(Windows::Foundation::typeName), (byte *)value);                                                     \
        }                                                                                                                                                       \
        else                                                                                                                                                    \
        {                                                                                                                                                       \
            hr = TYPE_E_TYPEMISMATCH;                                                                                                                           \
        }

#define GetNumericTypeArrayFromObjectReference(typeName)                                                                                                        \
        if (length == nullptr)                                                                                                                                  \
        {                                                                                                                                                       \
            return E_POINTER;                                                                                                                                   \
        }                                                                                                                                                       \
        *length = 0;                                                                                                                                            \
        if (value != nullptr)                                                                                                                                   \
        {                                                                                                                                                       \
            *value = nullptr;                                                                                                                                   \
        }                                                                                                                                                       \
                                                                                                                                                                \
        /* enums are scalar and support all numeric conversion  */                                                                                              \
        if (this->objectIReference->IsArray() && this->objectIReference->IsElementTypeEnumType())                                                               \
        {                                                                                                                                                       \
            uint32 numberOfElements = this->objectIReference->GetNumberOfElements();                                                                            \
            __int32 *baseValue = new __int32[numberOfElements];                                                                                                 \
            if (baseValue != nullptr)                                                                                                                           \
            {                                                                                                                                                   \
                hr = this->objectIReference->get_Value(sizeof(__int32) * numberOfElements, (byte *)baseValue);                                                  \
                if (SUCCEEDED(hr))                                                                                                                              \
                {                                                                                                                                               \
                    *value = (typeName *)CoTaskMemAlloc(sizeof(typeName) * numberOfElements);                                                                   \
                    if (*value)                                                                                                                                 \
                    {                                                                                                                                           \
                        for (UINT32 index = 0; index < numberOfElements; index++)                                                                               \
                        {                                                                                                                                       \
                            (*value)[index] = (typeName)baseValue[index];                                                                                       \
                        }                                                                                                                                       \
                        *length = numberOfElements;                                                                                                             \
                    }                                                                                                                                           \
                    else                                                                                                                                        \
                    {                                                                                                                                           \
                        hr = E_OUTOFMEMORY;                                                                                                                     \
                    }                                                                                                                                           \
                }                                                                                                                                               \
                                                                                                                                                                \
                delete [] baseValue;                                                                                                                            \
            }                                                                                                                                                   \
            else                                                                                                                                                \
            {                                                                                                                                                   \
                hr = E_OUTOFMEMORY;                                                                                                                             \
            }                                                                                                                                                   \
        }                                                                                                                                                       \
        else                                                                                                                                                    \
        {                                                                                                                                                       \
            hr = TYPE_E_TYPEMISMATCH;                                                                                                                           \
        }

#define GetUnsupportedTypeArrayFromObjectReference()                                                                                                            \
        if (length == nullptr)                                                                                                                                  \
        {                                                                                                                                                       \
            return E_POINTER;                                                                                                                                   \
        }                                                                                                                                                       \
        *length = 0;                                                                                                                                            \
        if (value != nullptr)                                                                                                                                   \
        {                                                                                                                                                       \
            *value = nullptr;                                                                                                                                   \
        }                                                                                                                                                       \
                                                                                                                                                                \
        hr = TYPE_E_TYPEMISMATCH;


#define GetWindowsFoundationTypeArrayFromObjectReference(typeName)                                                                                              \
        if (length == nullptr)                                                                                                                                  \
        {                                                                                                                                                       \
            return E_POINTER;                                                                                                                                   \
        }                                                                                                                                                       \
        *length = 0;                                                                                                                                            \
        if (value != nullptr)                                                                                                                                   \
        {                                                                                                                                                       \
            *value = nullptr;                                                                                                                                   \
        }                                                                                                                                                       \
                                                                                                                                                                \
        if (this->objectIReference->IsArray() && wcscmp(this->objectIReference->GetFullElementTypeName(), L"Windows.Foundation." L#typeName) == 0)               \
        {                                                                                                                                                       \
            uint32 numberOfElements = this->objectIReference->GetNumberOfElements();                                                                            \
            *value = (Windows::Foundation::##typeName *)CoTaskMemAlloc(sizeof(Windows::Foundation::##typeName) * numberOfElements);                             \
            if (*value != nullptr)                                                                                                                              \
            {                                                                                                                                                   \
                hr = this->objectIReference->get_Value(sizeof(Windows::Foundation::##typeName) * numberOfElements, (byte *)*value);                             \
                if (SUCCEEDED(hr))                                                                                                                              \
                {                                                                                                                                               \
                    *length = numberOfElements;                                                                                                                 \
                }                                                                                                                                               \
                else                                                                                                                                            \
                {                                                                                                                                               \
                    CoTaskMemFree(*value);                                                                                                                      \
                }                                                                                                                                               \
            }                                                                                                                                                   \
            else                                                                                                                                                \
            {                                                                                                                                                   \
                hr = E_OUTOFMEMORY;                                                                                                                             \
            }                                                                                                                                                   \
        }                                                                                                                                                       \
        else                                                                                                                                                    \
        {                                                                                                                                                       \
            hr = TYPE_E_TYPEMISMATCH;                                                                                                                           \
        }


namespace Projection
{
    Define_InspectableImpl_VTable(g_IPropertyValueVtable,
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, get_Type), 
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, get_IsNumericScalar),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt8),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetInt16),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt16),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetInt32),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt32),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetInt64),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt64),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetSingle),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetDouble),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetChar16),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetBoolean),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetString),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetGuid),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetDateTime),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetTimeSpan),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetPoint),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetSize),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetRect),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt8Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetInt16Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt16Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetInt32Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt32Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetInt64Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetUInt64Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetSingleArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetDoubleArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetChar16Array),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetBooleanArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetStringArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetInspectableArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetGuidArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetDateTimeArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetTimeSpanArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetPointArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetSizeArray),
        CUnknownImpl_VTableEntry(ObjectAsIPropertyValue, GetRectArray));

    ObjectAsIPropertyValue::ObjectAsIPropertyValue(ProjectionContext *projectionContext)
        : CUnknownImpl(projectionContext, g_IPropertyValueVtable
#if DBG_DUMP
            , propertyValueWrapper
#endif
        ), 
        objectIReference(nullptr)
    {
    }

    HRESULT ObjectAsIPropertyValue::Initialize(
            __in ObjectAsIReference *objectReference)
    {
        HRESULT hr = __super::Initialize(Windows::Foundation::IID_IPropertyValue, objectReference->GetFullTypeName(), false);
        IfFailedReturn(hr);

        this->objectIReference = objectReference;

        AddRef();
        return hr;
    }

    HRESULT ObjectAsIPropertyValue::Create(
            __in ProjectionContext *projectionContext, 
            __in ObjectAsIReference *objectReference, 
            __out ObjectAsIPropertyValue **newObjectAsIPropertyValue)
    {
        Assert(objectReference != nullptr);
        Assert(projectionContext != nullptr);
        IfNullReturnError(newObjectAsIPropertyValue, E_POINTER);
        *newObjectAsIPropertyValue = nullptr;

        ObjectAsIPropertyValue *pIPropertyValue = new ObjectAsIPropertyValue(projectionContext);
        IfNullReturnError(pIPropertyValue, E_OUTOFMEMORY);

        HRESULT hr = pIPropertyValue->Initialize(objectReference);
        if (FAILED(hr))
        {
            delete pIPropertyValue;
            return hr;
        }

        *newObjectAsIPropertyValue = pIPropertyValue;
        return hr;
    }

    CUnknownMethodNoError_Prolog(ObjectAsIPropertyValue, QueryInterface, REFIID riid, void **ppv)
    {
        HRESULT hr = OwnQueryInterface(riid, ppv);

        if (hr == E_NOINTERFACE)
        {
            hr = objectIReference->QueryInterface(riid, ppv);
        }

        return hr;
    }
    CUnknownMethodNoError_Epilog()

    CUnknownMethod_ULONGReturn_Prolog(ObjectAsIPropertyValue, AddRef)
    { 
        uRetVal = objectIReference->AddRef(); 
    }
    CUnknownMethod_ULONGReturn_Epilog()

    CUnknownMethod_ULONGReturn_Prolog(ObjectAsIPropertyValue, Release)
    {
        uRetVal = objectIReference->Release();
    }
    CUnknownMethod_ULONGReturn_Epilog()
        
    CUnknownMethodNoError_Prolog(ObjectAsIPropertyValue, GetIids, __RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        return GetTwoIids(this->objectIReference->GetOwnIID(), iidCount, iids);
    }
    CUnknownMethodNoError_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, get_Type, (value), __out __RPC__out enum Windows::Foundation::PropertyType *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = Windows::Foundation::PropertyType_Empty;

        if (wcscmp(this->objectIReference->GetFullElementTypeName(), L"Windows.Foundation.DateTime") == 0)
        {
            *value = (this->objectIReference->IsArray() ? Windows::Foundation::PropertyType_DateTimeArray : Windows::Foundation::PropertyType_DateTime);
        }
        else if (wcscmp(this->objectIReference->GetFullElementTypeName(), L"Windows.Foundation.TimeSpan") == 0)
        {
            *value = (this->objectIReference->IsArray() ? Windows::Foundation::PropertyType_TimeSpanArray : Windows::Foundation::PropertyType_TimeSpan);
        }
        else if (wcscmp(this->objectIReference->GetFullElementTypeName(), L"Windows.Foundation.Point") == 0)
        {
            *value = (this->objectIReference->IsArray() ? Windows::Foundation::PropertyType_PointArray : Windows::Foundation::PropertyType_Point);
        }
        else if (wcscmp(this->objectIReference->GetFullElementTypeName(), L"Windows.Foundation.Size") == 0)
        {
            *value = (this->objectIReference->IsArray() ? Windows::Foundation::PropertyType_SizeArray : Windows::Foundation::PropertyType_Size);
        }
        else if (wcscmp(this->objectIReference->GetFullElementTypeName(), L"Windows.Foundation.Rect") == 0)
        {
            *value = (this->objectIReference->IsArray() ? Windows::Foundation::PropertyType_RectArray : Windows::Foundation::PropertyType_Rect);
        }
        else
        {
            *value = this->objectIReference->IsArray() ? Windows::Foundation::PropertyType_OtherTypeArray : Windows::Foundation::PropertyType_OtherType;
        }
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, get_IsNumericScalar, (value), __RPC__out boolean *value)
    {
        IfNullReturnError(value, E_POINTER);

        // enums are scalar
        // rest all - that is delegates, structs are non scalar
        *value = this->objectIReference->IsElementTypeEnumType();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt8, (value), __out __RPC__out BYTE *value)
    {
        GetNumericTypeFromObjectReference(BYTE);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetInt16, (value), __out __RPC__out INT16 *value)
    {
        GetNumericTypeFromObjectReference(INT16);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt16, (value), __out __RPC__out UINT16 *value)
    {
        GetNumericTypeFromObjectReference(UINT16);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetInt32, (value), __out __RPC__out INT32 *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = 0;

        // Since enums are int32 we can directly get it
        // enums are scalar and support all numeric conversion
        if (!this->objectIReference->IsArray() && this->objectIReference->IsElementTypeEnumType())
        {
            hr = this->objectIReference->get_Value(sizeof(INT32), (byte *)value);
        }
        else
        {
            hr = TYPE_E_TYPEMISMATCH;
        }
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt32, (value), __out __RPC__out UINT32 *value)
    {
        GetNumericTypeFromObjectReference(UINT32);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetInt64, (value), __out __RPC__out INT64 *value)
    {
        GetNumericTypeFromObjectReference(INT64);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt64, (value), __out __RPC__out UINT64 *value)
    {
        GetNumericTypeFromObjectReference(UINT64);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetSingle, (value), __out __RPC__out float *value)
    {
        GetNumericTypeFromObjectReference(float);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetDouble, (value), __out __RPC__out double *value)
    {
        GetNumericTypeFromObjectReference(double);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetChar16, (value), __out __RPC__out WCHAR *value)
    {
        GetUnsupportedTypeFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetBoolean, (value), __out __RPC__out boolean *value)
    {
        GetUnsupportedTypeFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetString, (value), __out __RPC__out HSTRING *value)
    {
        GetUnsupportedTypeFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetGuid, (value), __out __RPC__out GUID *value)
    {
        GetUnsupportedTypeFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetDateTime, (value), __out __RPC__out Windows::Foundation::DateTime *value)
    {
        GetUnsupportedTypeFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetTimeSpan, (value), __out __RPC__out Windows::Foundation::TimeSpan *value)
    {
        GetWindowsFoundationTypeFromObjectReference(TimeSpan);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetPoint, (value), __out __RPC__out Windows::Foundation::Point *value)
    {
        GetWindowsFoundationTypeFromObjectReference(Point);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetSize, (value), __out __RPC__out Windows::Foundation::Size *value)
    {
        GetWindowsFoundationTypeFromObjectReference(Size);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetRect, (value), __out __RPC__out Windows::Foundation::Rect *value)
    {
        GetWindowsFoundationTypeFromObjectReference(Rect);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt8Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value)
    {
        GetNumericTypeArrayFromObjectReference(BYTE);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetInt16Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value)
    {
        GetNumericTypeArrayFromObjectReference(INT16);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt16Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value)
    {
        GetNumericTypeArrayFromObjectReference(UINT16);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetInt32Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value)
    {
        IfNullReturnError(length, E_POINTER);
        *length = 0;
        if (value != nullptr)
        {
            *value = nullptr;
        }

        // enums are scalar and support all numeric conversion
        if (this->objectIReference->IsArray() && this->objectIReference->IsElementTypeEnumType())
        {
            uint32 numberOfElements = this->objectIReference->GetNumberOfElements();
            *value = (INT32 *)CoTaskMemAlloc(sizeof(INT32) * numberOfElements);
            if (*value != nullptr)
            {
                hr = this->objectIReference->get_Value(sizeof(INT32) * numberOfElements, (byte *)*value);
                if (SUCCEEDED(hr))
                {
                    *length = numberOfElements;
                }
                else
                {
                    CoTaskMemFree(*value);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = TYPE_E_TYPEMISMATCH;
        }
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt32Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value)
    {
        GetNumericTypeArrayFromObjectReference(UINT32);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetInt64Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value)
    {
        GetNumericTypeArrayFromObjectReference(INT64);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetUInt64Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value)
    {
        GetNumericTypeArrayFromObjectReference(UINT64);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetSingleArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **value)
    {
        GetNumericTypeArrayFromObjectReference(float);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetDoubleArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **value)
    {
        GetNumericTypeArrayFromObjectReference(double);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetChar16Array, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value)
    {
        GetUnsupportedTypeArrayFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetBooleanArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value)
    {
        GetUnsupportedTypeArrayFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetStringArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
    {
        GetUnsupportedTypeArrayFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetInspectableArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value)
    {
        GetUnsupportedTypeArrayFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetGuidArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value)
    {
        GetUnsupportedTypeArrayFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetDateTimeArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value)
    {
        GetUnsupportedTypeArrayFromObjectReference();
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetTimeSpanArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value)
    {
        GetWindowsFoundationTypeArrayFromObjectReference(TimeSpan);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetPointArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value)
    {
        GetWindowsFoundationTypeArrayFromObjectReference(Point);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetSizeArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value)
    {
        GetWindowsFoundationTypeArrayFromObjectReference(Size);
    }
    CUnknownMethodImpl_Epilog()

    CUnknownMethodImpl_Prolog(ObjectAsIPropertyValue, GetRectArray, (length, value), __RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value)
    {
        GetWindowsFoundationTypeArrayFromObjectReference(Rect);
    }
    CUnknownMethodImpl_Epilog()
}
