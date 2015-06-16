//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#include "stdafx.h"

//
// Find a property name in the current object properties layout: {name:value} terminator
//
HRESULT SCAPropertyReader::FindObjectPropertyName(_In_ const BSTR name)
{
    HRESULT hr = S_OK;

    UINT len = ::SysStringLen(name);
    SCAStringWalker<FindStringWalker> findStringWalker;
    findStringWalker.SetTarget(name, len);

    for (;;)
    {
        // Walk and check property name
        IfFailGo(findStringWalker.Walk(m_reader, true));

        if (hr == S_FALSE)
        {
            // seen SCA_PROPERTY_TERMINATOR
            m_found = false;
            break;
        }

        if (findStringWalker.Found())
        {
            m_found = true;
            break;
        }

        // Not the property we are looking for, skip its value
        IfFailGo(SkipWalker::Walk(m_reader));
    }

Error:
    return hr;
}

//
// Try to lookup a property name in known internal property names using the given InternalPropertyReader.
// If found, set the given InternalPropertyReader as current InternalPropertyReader.
//
bool SCAPropertyReader::TryFindInternalProperty(_In_ InternalPropertyReader* pInternalPropertyReader, _In_ const BSTR name)
{
    ASSERT(!this->Found());
    ASSERT(m_pInternalPropertyReader == nullptr);

    if (pInternalPropertyReader->FindProperty(name))
    {
        m_pInternalPropertyReader = pInternalPropertyReader;
        m_found = true;
    }

    return m_found;
}

//
// Find a property name in the current value layout. Reader must be at the beginning position of
// current SCAValue, except when we were reading an internal property.
//
HRESULT SCAPropertyReader::FindPropertyByName(_In_ const BSTR name)
{
    IfNullReturnError(name, E_INVALIDARG);

    m_found = false;
    m_canPropertyBeAdded = false;

    HRESULT hr = S_OK;

    // If we were reading an internal property, continue lookup in internal property
    if (m_pInternalPropertyReader != nullptr)
    {
        m_found = m_pInternalPropertyReader->FindProperty(name);
        return hr; // Done
    }

    SCATypeId typeId;
    IfFailGo(m_reader.Read(&typeId));

    switch (typeId)
    {
    case SCA_Reference:
        {
            scaposition_t pos;
            IfFailGo(m_reader.Read(&pos));
            IfFailGo(m_reader.Seek(pos));
            IfFailGo(FindPropertyByName(name));
        }
        break;

    case SCA_StringValue:
        TryFindInternalProperty(&m_stringInternalPropertyReader, name);
        m_canPropertyBeAdded = false; // Can't add property to StringValue
        break;

    case SCA_StringObject:
        TryFindInternalProperty(&m_stringInternalPropertyReader, name);
        m_canPropertyBeAdded = true; // Can add other property to StringObject
        break;

    case SCA_RegExpObject:
        TryFindInternalProperty(&m_regExpInternalPropertyReader, name);
        m_canPropertyBeAdded = true; // Can add other property to RegExpObject
        break;

    case SCA_Object:
        IfFailGo(FindObjectPropertyName(name));
        m_canPropertyBeAdded = true;
        break;

    case SCA_Map:
        IfFailGo(SkipWalker::WalkMapData(m_reader));
        IfFailGo(FindObjectPropertyName(name));
        m_canPropertyBeAdded = true;
        break;

    case SCA_Set:
        IfFailGo(SkipWalker::WalkSetData(m_reader));
        IfFailGo(FindObjectPropertyName(name));
        m_canPropertyBeAdded = true;
        break;

    case SCA_DenseArray:
        if (!TryFindInternalProperty(&m_arrayInternalPropertyReader, name))
        {
            IfFailGo(SkipWalker::WalkDenseArrayIndexProperties(m_reader));
            IfFailGo(FindObjectPropertyName(name));
        }
        m_canPropertyBeAdded = true;
        break;

    case SCA_SparseArray:
        if (!TryFindInternalProperty(&m_arrayInternalPropertyReader, name))
        {
            IfFailGo(SkipWalker::WalkSparseArrayIndexProperties(m_reader));
            IfFailGo(FindObjectPropertyName(name));
        }
        m_canPropertyBeAdded = true;
        break;

    default:
        if (IsSCAHostObject(typeId))
        {
            // Host object uses the same object properties layout
            IfFailGo(FindObjectPropertyName(name));
        }
        m_canPropertyBeAdded = !IsSCAPrimitive(typeId);
        break;
    }

Error:
    return hr;
}

HRESULT SCAPropertyReader::ReadValueOfType(_In_ bool allowDenseArrayForKeys, _In_ SCATypeId typeId, _Out_ VARIANT* pValue) const
{
    HRESULT hr = S_OK;

    switch (typeId)
    {
    case SCA_Reference:
        {
            scaposition_t pos;
            IfFailGo(m_reader.Read(&pos));
            IfFailGo(m_reader.Seek(pos));
            hr = ReadIndexablePropertyValue(allowDenseArrayForKeys, pValue);
        }
        break;

    case SCA_NullValue:
        V_VT(pValue) = VT_NULL;
        break;

    case SCA_Int32Value:
        {
            INT32 n;
            IfFailGo(m_reader.Read(&n));
            V_VT(pValue) = VT_I4;
            V_I4(pValue) = n;
        }
        break;

    case SCA_DoubleValue:
    case SCA_DateObject:
    case SCA_NumberObject:
        {
            double dbl;
            IfFailGo(m_reader.Read(&dbl));
            if (typeId == SCA_DateObject)
            {
                V_VT(pValue) = VT_DATE;
                V_DATE(pValue) = dbl;
            }
            else
            {
                V_VT(pValue) = VT_R8;
                V_R8(pValue) = dbl;
            }
        }
        break;

    case SCA_Int64Value:
        {
            INT64 n;
            IfFailGo(m_reader.Read(&n));
            V_VT(pValue) = VT_I8;
            V_I8(pValue) = n;
        }
        break;

    case SCA_Uint64Value:
        {
            UINT64 n;
            IfFailGo(m_reader.Read(&n));
            V_VT(pValue) = VT_UI8;
            V_UI8(pValue) = n;
        }
        break;

    case SCA_StringValue:
    case SCA_StringObject:
        IfFailGo(InternalPropertyReader::ReadString(m_reader, pValue));
        break;

    case SCA_DenseArray:
        if (allowDenseArrayForKeys)
        {
            IfFailGo(InternalPropertyReader::ReadDenseArray(this, m_reader, pValue));
            break;
        }
        //else fall-through

    default:
        // Other value types are considered not indexable. Returns with S_OK/VT_EMPTY.
        ASSERT(pValue->vt == VT_EMPTY);
        break;
    }

Error:
    return hr;
}

//
// Read the value if current value layout is an indexable value, otherwise return S_OK/VT_EMPTY.
// Reader must be at the beginning position of current SCAValue, except when we were reading an
// internal property.
//
HRESULT SCAPropertyReader::ReadIndexablePropertyValue(_In_ bool allowDenseArrayForKeys, _Out_ VARIANT* pValue) const
{
    ASSERT(Found()); // We must have found the property

    // If we were reading an internal property, read the value from internal property
    if (m_pInternalPropertyReader != nullptr)
    {
        return m_pInternalPropertyReader->ReadIndexableProperty(m_reader, pValue);
    }

    HRESULT hr = S_OK;

    SCATypeId typeId;
    IfFailGo(m_reader.Read(&typeId));

    IfFailGo(ReadValueOfType(allowDenseArrayForKeys, typeId, pValue));

Error:
    return hr;
}

HRESULT STDMETHODCALLTYPE InternalReadIndexableProperty(
    _In_ bool allowDenseArrayForKeys,
    _In_ IStream* pStream,
    _In_ UINT count,
    _In_reads_(count) BSTR* parts,
    _Out_ VARIANT* pValue,
    _Out_opt_ bool* pfCanPropertyBeAdded)
{
    IfNullReturnError(pStream, E_INVALIDARG);
    IfFalseReturnError(count > 0, E_INVALIDARG);
    IfNullReturnError(parts, E_INVALIDARG);
    IfNullReturnError(pValue, E_POINTER);

    VariantInit(pValue);
    if (pfCanPropertyBeAdded != nullptr)
    {
        *pfCanPropertyBeAdded = false;
    }

    HRESULT hr = S_OK;
    StreamReader reader(pStream);

    DWORD header;
    IfFailGo(reader.Read(&header));
    if (GetSCAMajor(header) > SCA_FORMAT_MAJOR)
    {
        // Can't read higher version
        IfFailGo(E_SCA_NEWVERSION);
    }

    // Walk the SCA layout looking for each property name part
    {
        SCAPropertyReader propertyReader(reader);
        while (count > 0)
        {
            IfFailGo(propertyReader.FindPropertyByName(*parts));

            if (!propertyReader.Found())
            {
                break;
            }

            count--;
            parts++;
        }

        IfFailGo(propertyReader.Found() ? propertyReader.ReadIndexablePropertyValue(allowDenseArrayForKeys, pValue) : S_FALSE);
        if (pfCanPropertyBeAdded != nullptr)
        {
            *pfCanPropertyBeAdded = propertyReader.CanPropertyBeAdded();
        }
    }

Error:
    return hr;
}

HRESULT STDMETHODCALLTYPE ReadIndexablePropertyEx(
    _In_ IStream* pStream,
    _In_ UINT count,
    _In_reads_(count) BSTR* parts,
    _Out_ VARIANT* pValue,
    _Out_opt_ bool* pfCanPropertyBeAdded)
{
    return InternalReadIndexableProperty(true /* allowDenseArrayForKeys */, pStream, count, parts, pValue, pfCanPropertyBeAdded);
}

HRESULT STDMETHODCALLTYPE ReadIndexableProperty(
    _In_ IStream* pStream,
    _In_ UINT count,
    _In_reads_(count) BSTR* parts,
    _Out_ VARIANT* pValue,
    _Out_opt_ bool* pfCanPropertyBeAdded)
{
    return InternalReadIndexableProperty(false /* allowDenseArrayForKeys */, pStream, count, parts, pValue, pfCanPropertyBeAdded);
}

HRESULT STDMETHODCALLTYPE KeyFromStreamToVariant(
    _In_ IStream* pStream,
    _Out_ VARIANT* pValue)
{
    HRESULT hr = S_OK;
    StreamReader reader(pStream);
    SCAPropertyReader propertyReader(reader);

    DWORD dwHeader;
    IfFailGo(reader.Read(&dwHeader));
    if (GetSCAMajor(dwHeader) > SCA_FORMAT_MAJOR)
    {
        // Can't read higher version
        IfFailGo(E_SCA_NEWVERSION);
    }

    SCATypeId typeId;
    IfFailGo(reader.Read(&typeId));

    IfFailGo(propertyReader.ReadValueOfType(true, typeId, pValue));

Error:
    return hr;
}

