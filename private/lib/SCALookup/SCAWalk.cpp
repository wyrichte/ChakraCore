//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

//
// Skips a primitive value layout.
//
HRESULT SkipWalker::WalkPrimitive(_In_ const StreamReader& reader, _In_ SCATypeId typeId)
{
    HRESULT hr = S_OK;

    switch (typeId)
    {
    case SCA_None:
    case SCA_NullValue:
    case SCA_UndefinedValue:
    case SCA_TrueValue:
    case SCA_FalseValue:
        break;

    case SCA_Reference:
    case SCA_Int32Value:
        hr = reader.Skip(sizeof(DWORD));
        break;

    case SCA_DoubleValue:
        hr = reader.Skip(sizeof(double));
        break;

    case SCA_StringValue:
        hr = SkipStringWalker::Walk(reader);
        break;

    case SCA_Int64Value:
    case SCA_Uint64Value:
        hr = reader.Skip(sizeof(__int64));
        break;

    default:
        ATLASSERT(FALSE); // Should never happen
        hr = E_UNEXPECTED;
    }

    return hr;
}

//
// Skips a host object value layout.
//
HRESULT SkipWalker::WalkHostObject(_In_ const StreamReader& reader, _In_ SCATypeId /* typeId */)
{
    // Just walk object properties
    return WalkObjectProperties(reader);
}

//
// Skips an object properties layout.
//
HRESULT SkipWalker::WalkObjectProperties(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    for (;;)
    {
        // Skip the property name
        IfFailGo(SkipStringWalker::Walk(reader, true));

        if (hr == S_FALSE)
        {
            // seen SCA_PROPERTY_TERMINATOR
            break;
        }

        // Skip the property value
        IfFailGo(SkipWalker::Walk(reader));
    }

Error:
    return hr;
}

//
// Skips and walks a dense array index properties layout: [length] ...values...
//
HRESULT SkipWalker::WalkDenseArrayIndexProperties(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    UINT32 length;
    IfFailGo(reader.Read(&length));

    for (UINT32 i = 0; i < length; i++)
    {
        SkipWalker::Walk(reader);
    }

Error:
    return hr;
}

//
// Skips and walks a sparse array index properties layout: [length] {[index] [value]} -1
//
HRESULT SkipWalker::WalkSparseArrayIndexProperties(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    IfFailGo(reader.Skip(sizeof(UINT32))); // Skip length

    UINT32 index;
    while (true)
    {
        IfFailGo(reader.Read(&index));
        if (index == SCA_PROPERTY_TERMINATOR)
        {
            break;
        }

        SkipWalker::Walk(reader); // Skip value
    }

Error:
    return hr;
}

//
// Skips an object value layout. This includes anything other than primitives and host objects.
//
HRESULT SkipWalker::WalkObject(_In_ const StreamReader& reader, _In_ SCATypeId typeId)
{
    HRESULT hr = S_OK;

    switch (typeId)
    {
    case SCA_BooleanTrueObject:
    case SCA_BooleanFalseObject:
        break;

    case SCA_DateObject:
    case SCA_NumberObject:
        hr = reader.Skip(sizeof(double));
        break;

    case SCA_StringObject:
    case SCA_RegExpObject:
        {
            hr = SkipStringWalker::Walk(reader);
            if (SUCCEEDED(hr) && typeId == SCA_RegExpObject)
            {
                hr = reader.Skip(sizeof(DWORD)); // Regex flags
            }
        }
        break;

    case SCA_Object:
        hr = WalkObjectProperties(reader);
        break;

    case SCA_Map:
        hr = WalkMap(reader);
        break;

    case SCA_Set:
        hr = WalkSet(reader);
        break;

    case SCA_DenseArray:
        {
            IfFailGo(WalkDenseArrayIndexProperties(reader));
            hr = WalkObjectProperties(reader);
        }
        break;

    case SCA_SparseArray:
        {
            IfFailGo(WalkSparseArrayIndexProperties(reader));
            hr = WalkObjectProperties(reader);
        }
        break;

    case SCA_ArrayBuffer:
        hr = WalkByteArrayData(reader);
        break;

    default:
        {
            if (IsSCATypedArray(typeId) || typeId == SCA_DataView)
            {
                IfFailGo(SkipWalker::Walk(reader)); // ArrayBuffer
                IfFailGo(reader.Skip(sizeof(DWORD) * 2)); // byteOffset + length
                break;
            }

            // Unknown SCATypeID indicates data corruption
            hr = E_SCA_DATACORRUPT;
        }
        break;
    }

Error:
    return hr;
}

//
// Walks and skips: [byteLen] [byte data] [padding]
//
HRESULT SkipWalker::WalkByteArrayData(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    DWORD rawLen;
    IfFailGo(reader.Read(&rawLen));

    UINT byteLen;
    IfFailGo(DWordToUInt(rawLen, &byteLen));

    UINT unalignedByteLen = byteLen % sizeof(DWORD);
    UINT paddingByteLen = unalignedByteLen ? sizeof(DWORD) - unalignedByteLen : 0;

    IfFailGo(reader.Skip(byteLen + paddingByteLen));
Error:
    return hr;
}

//
// Walks and skips the current Map value in the stream.
//
HRESULT SkipWalker::WalkMap(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    IfFailGo(WalkMapData(reader));
    IfFailGo(WalkObjectProperties(reader));

Error:
    return hr;
}

//
// Walks and skips the current Set value in the stream.
//
HRESULT SkipWalker::WalkSet(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    IfFailGo(WalkSetData(reader));
    IfFailGo(WalkObjectProperties(reader));

Error:
    return hr;
}

//
// Skips a Map object's internal data layout.
//
HRESULT SkipWalker::WalkMapData(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    INT32 size;
    IfFailGo(reader.Read(&size));

    for (int i = 0; i < size; i += 1)
    {
        IfFailGo(Walk(reader)); // Walk the key
        IfFailGo(Walk(reader)); // Walk the value
    }

Error:
    return hr;
}

//
// Skips a Set object's internal data layout.
//
HRESULT SkipWalker::WalkSetData(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    INT32 size;
    IfFailGo(reader.Read(&size));

    for (int i = 0; i < size; i += 1)
    {
        IfFailGo(Walk(reader)); // Walk the value
    }

Error:
    return hr;
}

//
// Walks and skips the current SCA value in the stream.
//
HRESULT SkipWalker::Walk(_In_ const StreamReader& reader)
{
    return SCAWalker<SkipWalker>().Walk(reader);
}
