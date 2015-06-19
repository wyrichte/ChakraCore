//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C"{
#endif

// Read an indexable property of a keyPath from SCA serialized data.
//  pStream:                A stream containing SCA serialized data. Supports random access.
//  count:                  Count of property name parts in the keyPath.
//  parts:                  Property name parts split from the keyPath.
//  pValue:                 Will contain the property value on successful read.
//  pfCanPropertyBeAdded    Will indicate whether or not a property will be able to be added to the existing keyPath.
//                              For all success codes (including S_FALSE) this should be valid.
//                              If the property was found, this will always be true.
//
// SCA error results
//  E_SCA_NEWVERSION    : The SCA layout format is of newer version.
//  E_SCA_DATACORRUPT   : The SCA layout format contains unexpected data, indicating data corruption.
HRESULT STDMETHODCALLTYPE ReadIndexableProperty(
    _In_ IStream* pStream,
    _In_ UINT count,
    _In_reads_(count) BSTR* parts,
    _Out_ VARIANT* pValue,
    _Out_opt_ bool* pfCanPropertyBeAdded);

// Similar to ReadIndexableProperty, ReadIndexablePropertyEx reads an indexable property of a keyPath from SCA serialized data.
// However, DenseArray's are allowed as Keys when using this method to lookup.
//
//  pStream:                A stream containing SCA serialized data. Supports random access.
//  count:                  Count of property name parts in the keyPath.
//  parts:                  Property name parts split from the keyPath.
//  pValue:                 Will contain the property value on successful read.
//  pfCanPropertyBeAdded    Will indicate whether or not a property will be able to be added to the existing keyPath.
//                              For all success codes (including S_FALSE) this should be valid.
//                              If the property was found, this will always be true.
//
// SCA error results
//  E_SCA_NEWVERSION    : The SCA layout format is of newer version.
//  E_SCA_DATACORRUPT   : The SCA layout format contains unexpected data, indicating data corruption.
HRESULT STDMETHODCALLTYPE ReadIndexablePropertyEx(
    _In_ IStream* pStream,
    _In_ UINT count,
    _In_reads_(count) BSTR* parts,
    _Out_ VARIANT* pValue,
    _Out_opt_ bool* pfCanPropertyBeAdded);

// Attempts to parse the given stream as a valid key, and return the result in a VARIANT. This will do the same walk as VerifyStreamIsValidKey but will allocate VARIANTS.
//
// pStream              :   A stream containing SCA serialized data; The stream must supports random access. The serialized data in the stream must start with SCA version header.
// pValue               :   Contains a VARIANT representing the key in pStream. If the key is not valid, it will be VT_NULL|VT_EMPTY
//
// SCA error results
//  E_SCA_DATACORRUPT   :   The SCA layout format contains unexpected data, indicating data corruption.
HRESULT STDMETHODCALLTYPE KeyFromStreamToVariant(
    _In_ IStream* pStream,
    _Out_ VARIANT* pValue);


#ifdef __cplusplus
}
#endif
