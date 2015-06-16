//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------
#pragma once

//
// SCAWalker walks SCA values in a SCA layout stream.
//  T: The base class that implements how to walk primitive/object/hostobject.
//
template <class T>
class SCAWalker: public T
{
public:
    HRESULT Walk(_In_ const StreamReader& reader);
};

//
// Walk the current SCA value in a SCA stream. This method simply reads the current value's
// SCATypeId and dispatches to walking primitive/object/hostobject.
//
template <class T>
HRESULT SCAWalker<T>::Walk(_In_ const StreamReader& reader)
{
    HRESULT hr = S_OK;

    SCATypeId typeId;
    IfFailGo(reader.Read(&typeId));

    if (IsSCAPrimitive(typeId))
    {
        hr = WalkPrimitive(reader, typeId);
    }
    else if (IsSCAHostObject(typeId))
    {
        hr = WalkHostObject(reader, typeId);
    }
    else
    {
        hr = WalkObject(reader, typeId);
    }

Error:
    return hr;
}

//
// SkipWalker simply skips everything.
//
class SkipWalker
{
private:
    HRESULT WalkByteArrayData(_In_ const StreamReader& reader);
    static HRESULT WalkMap(_In_ const StreamReader& reader);
    static HRESULT WalkSet(_In_ const StreamReader& reader);

protected:
    HRESULT WalkPrimitive(_In_ const StreamReader& reader, _In_ SCATypeId typeId);
    HRESULT WalkHostObject(_In_ const StreamReader& reader, _In_ SCATypeId typeId);
    HRESULT WalkObject(_In_ const StreamReader& reader, _In_ SCATypeId typeId);

public:
    static HRESULT WalkObjectProperties(_In_ const StreamReader& reader);
    static HRESULT Walk(_In_ const StreamReader& reader);
    static HRESULT WalkDenseArrayIndexProperties(_In_ const StreamReader& reader);
    static HRESULT WalkSparseArrayIndexProperties(_In_ const StreamReader& reader);
    static HRESULT WalkMapData(_In_ const StreamReader& reader);
    static HRESULT WalkSetData(_In_ const StreamReader& reader);
};

